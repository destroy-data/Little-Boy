#include "core/core_constants.hpp"
#include "core/cpu.hpp"
#include "dummy_types.hpp"
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <utility>

TEST_CASE( "isConditionMet", "[cpu][helper function]" ) {
    DummyBus bus;
    DummyCpu cpu( bus );

    //--------------------------------------------------
    cpu.registers[std::to_underlying( CoreCpu::Operand_t::f )] =
            bitMask::zeroFlag; // all registers except Z to false
    REQUIRE_FALSE( cpu.isConditionMet( CoreCpu::Operand_t::condNZ ) );
    REQUIRE( cpu.isConditionMet( CoreCpu::Operand_t::condZ ) );

    REQUIRE( cpu.isConditionMet( CoreCpu::Operand_t::condNC ) );
    REQUIRE_FALSE( cpu.isConditionMet( CoreCpu::Operand_t::condC ) );

    //--------------------------------------------------
    cpu.registers[std::to_underlying( CoreCpu::Operand_t::f )] =
            bitMask::carryFlag; // all registers except C to false
    REQUIRE( cpu.isConditionMet( CoreCpu::Operand_t::condNZ ) );
    REQUIRE_FALSE( cpu.isConditionMet( CoreCpu::Operand_t::condZ ) );

    REQUIRE_FALSE( cpu.isConditionMet( CoreCpu::Operand_t::condNC ) );
    REQUIRE( cpu.isConditionMet( CoreCpu::Operand_t::condC ) );
}

TEST_CASE( "Cpu branch handling", "[cpu][branch]" ) {
    DummyBus bus;
    using MopType = CoreCpu::MicroOperationType_t;
    std::unique_ptr<DummyCpu> cpu;

    cpu                                                         = std::make_unique<DummyCpu>( bus );
    cpu->registers[std::to_underlying( CoreCpu::Operand_t::f )] = 0; // all registers set to false
    cpu->PC                                                     = 0x100;
    cpu->mopQueue                                               = {
            { { MopType::CHECK_COND, CoreCpu::Operand_t::condZ }, MopType::INVALID, MopType::INVALID } };
    cpu->tick();
    REQUIRE( cpu->mopQueue[1].type == MopType::NOP );
    REQUIRE( cpu->mopQueue[2].type == MopType::END );
    cpu->tick();
    cpu->tick();
    REQUIRE( cpu->PC == 0x101 );


    cpu                                                         = std::make_unique<DummyCpu>( bus );
    cpu->registers[std::to_underlying( CoreCpu::Operand_t::f )] = 0xFF; // all registers set to true
    cpu->mopQueue                                               = {
            { { MopType::CHECK_COND, CoreCpu::Operand_t::condZ }, MopType::INVALID, MopType::INVALID } };
    cpu->tick();
    REQUIRE( cpu->mopQueue[1].type == MopType::INVALID );
    REQUIRE( cpu->mopQueue[2].type == MopType::INVALID );

    //--------------------------------------------------
    cpu                                                         = std::make_unique<DummyCpu>( bus );
    cpu->registers[std::to_underlying( CoreCpu::Operand_t::f )] = 0; // all registers set to false
    cpu->PC                                                     = 0x100;
    cpu->mopQueue = { { { MopType::COND_CHECK__LD_IMM_TO_Z, CoreCpu::Operand_t::condZ },
                        MopType::INVALID,
                        MopType::INVALID } };
    cpu->tick();
    REQUIRE( cpu->mopQueue[1].type == MopType::NOP );
    REQUIRE( cpu->mopQueue[2].type == MopType::END );
    cpu->tick();
    cpu->tick();
    REQUIRE( cpu->PC == 0x102 );
}
