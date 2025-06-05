#include "core/emulator.hpp"
#include "core/memory.hpp"

template<typename Tcpu, typename Tppu>
void Emulator<Tcpu, Tppu>::tick() {
    const bool cpuDoubleSpeed = memory.read( addr::key1 ) & ( 1 << 7 );
    const auto cpuTicks       = cpu.tick();
    if( cpuDoubleSpeed )
        cpuTicks / 2;
    for( auto i = 1u; i < cpuTicks; i++ ) {
        ppu.tick();
    }

    //apu.tick();
}
