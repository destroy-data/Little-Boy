#include "core/emulator.hpp"
#include "core/logging.hpp"
#include "dummy_types.hpp"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

using json                     = nlohmann::json;
using ramAddressValueMapping_t = std::unordered_map<uint16_t, uint8_t>;

class Flat64KMemory {
    uint8_t memory[64*1024];
    public:
    uint8_t read( const uint16_t index ) const {
        return memory[index];
    }
    void write( const uint16_t index, uint8_t value ) {
        memory[index] = value;
    }
    Flat64KMemory( [[maybe_unused]] CoreCartridge* cartridge_ ){}
};

struct CpuState {
    uint8_t a, b, c, d, e, f, h, l;
    uint16_t pc, sp;
    ramAddressValueMapping_t ram;
};

CpuState parseState( const json& state_json ) {
    CpuState state;
    state.a  = state_json["a"];
    state.b  = state_json["b"];
    state.c  = state_json["c"];
    state.d  = state_json["d"];
    state.e  = state_json["e"];
    state.f  = state_json["f"];
    state.h  = state_json["h"];
    state.l  = state_json["l"];
    state.pc = state_json["pc"];
    state.sp = state_json["sp"];

    for( const auto& ram_entry: state_json["ram"] ) {
        uint16_t address   = ram_entry[0];
        uint8_t value      = ram_entry[1];
        state.ram[address] = value;
    }
    return state;
}

class TestCpu final : public Cpu {
public:
    void setCpuState( const CpuState& state) {
        writeR8( Operand_t::a, state.a );
        writeR8( Operand_t::b, state.b );
        writeR8( Operand_t::c, state.c );
        writeR8( Operand_t::d, state.d );
        writeR8( Operand_t::e, state.e );
        writeR8( Operand_t::f, state.f );
        writeR8( Operand_t::h, state.h );
        writeR8( Operand_t::l, state.l );
        PC = state.pc;
        SP = state.sp;

        for( const auto& [address, value]: state.ram ) {
            bus.write( address, value );
        }
    }

    CpuState getCpuState( const ramAddressValueMapping_t& expectedRamState ) {
        CpuState state;
        state.a  = readR8( Operand_t::a );
        state.b  = readR8( Operand_t::b );
        state.c  = readR8( Operand_t::c );
        state.d  = readR8( Operand_t::d );
        state.e  = readR8( Operand_t::e );
        state.f  = readR8( Operand_t::f );
        state.h  = readR8( Operand_t::h );
        state.l  = readR8( Operand_t::l );
        state.pc = PC;
        state.sp = SP;

        for( const auto& [address, _]: expectedRamState ) {
            state.ram[address] = bus.read( address );
        }

        return state;
    }

    void clearMopQueue() {
        mopQueue = { MicroOperationType_t::END };
        atMicroOperationNr = 0;
    }

    TestCpu( IBus& bus_ ) : Cpu( bus_ ) {
        mopQueue = { MicroOperationType_t::END };
    }
};

uint16_t getOpcodeFromFileName( const std::string& stem ) {
    if( stem.starts_with( "cb" ) ) {
        return static_cast<uint16_t>( ( 0xCB << 8 ) | std::stoi( stem.substr( 2 ), nullptr, 16 ) );
    }
    return static_cast<uint16_t>( std::stoi( stem, nullptr, 16 ) );
}

unsigned getCycles( uint16_t opcode, bool branchTaken ) {
    if( opcode <= 0xFF ) {
        return branchTaken ? cycles::opcodeCyclesBranched[opcode] : cycles::opcodeCycles[opcode];
    }
    return cycles::opcodeCyclesCb[opcode & 0xFF];
}

void printCpuState( const CpuState& state, const std::string& label ) {
    UNSCOPED_INFO( label << ":" );
    UNSCOPED_INFO( "  Registers: A=" << toHex( state.a ) << ", B=" << toHex( state.b )
                                     << ", C=" << toHex( state.c ) << ", D=" << toHex( state.d ) );
    UNSCOPED_INFO( "             E=" << toHex( state.e ) << ", F=" << toHex( state.f )
                                     << ", H=" << toHex( state.h ) << ", L=" << toHex( state.l ) );
    UNSCOPED_INFO( "  PC=" << toHex( state.pc ) << ", SP=" << toHex( state.sp ) );

    if( ! state.ram.empty() ) {
        std::string ramStr = "  RAM: ";
        bool first         = true;
        for( const auto& [address, value]: state.ram ) {
            if( ! first )
                ramStr += ", ";
            ramStr += std::format( "[{}]={}", toHex( address ), toHex( value ) );
            first = false;
        }
        UNSCOPED_INFO( ramStr );
    }
}

bool checkCpuState( const CpuState& expected, const CpuState& actual ) {
    bool passed = true;

    if( expected.a != actual.a )
        passed = false;
    if( expected.b != actual.b )
        passed = false;
    if( expected.c != actual.c )
        passed = false;
    if( expected.d != actual.d )
        passed = false;
    if( expected.e != actual.e )
        passed = false;
    if( expected.f != actual.f )
        passed = false;
    if( expected.h != actual.h )
        passed = false;
    if( expected.l != actual.l )
        passed = false;
    if( expected.pc != actual.pc )
        passed = false;
    if( expected.sp != actual.sp )
        passed = false;

    for( const auto& [address, expected_value]: expected.ram ) {
        if( expected_value != actual.ram.at( address ) ) {
            passed = false;
            break;
        }
    }

    if( ! passed ) {
        printCpuState( expected, "Expected" );
        printCpuState( actual, "Actual" );
        FAIL( "CPU state mismatch" );
    }

    return passed;
}

TEST_CASE( "CPU opcodes", "[cpu][opcodes]" ) {
    Emulator<DummyPpu, TestCpu, Flat64KMemory> emu( std::make_unique<DummyCartridge>(), dummyJoypadHandler );

    for( const auto& entry: std::filesystem::directory_iterator( OPCODE_TESTS_PATH ) ) {
        std::ifstream file( entry.path() );
        REQUIRE( file.is_open() );

        const std::string filename = entry.path().filename().string();
        const uint16_t opcode      = getOpcodeFromFileName( entry.path().stem() );
        json opcodeTests           = json::parse( file );

        int testIndex        = 0;
        bool fileTestsPassed = true;

        for( const auto& test: opcodeTests ) {
            if( ! fileTestsPassed )
                break; // Skip remaining tests in this file

            std::string testName = test.contains( "name" ) ? test["name"].get<std::string>() : "unnamed";
            std::string context =
                    "File: " + filename + ", Test #" + std::to_string( testIndex ) + " (" + testName + ")";

            INFO( context );
            emu.cpu.setCpuState( parseState( test["initial"] ) );
            emu.cpu.clearMopQueue();
            for( unsigned i = 0; i < getCycles( opcode, true ) / 4; i++ ) {
                emu.tick();
            }

            const CpuState expectedState = parseState( test["final"] );
            const CpuState actualState   = emu.cpu.getCpuState( expectedState.ram );

            try {
                fileTestsPassed = checkCpuState( expectedState, actualState );
            } catch( ... ) {
                fileTestsPassed = false;
            }

            testIndex++;
        }
    }
}
