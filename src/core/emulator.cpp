#include "core/emulator.hpp"
#include "core/memory.hpp"

template<typename Tcartridge, typename Tmemory, typename Tcpu, typename Tppu>
void Emulator<Tcartridge, Tmemory, Tcpu, Tppu>::tick() {
    const auto cpuTicks = cpu.tick();
    for( auto i = 1u; i < cpuTicks; i++ ) {
        ppu.tick();
    }

    const bool cpuDoubleSpeed = memory.read( addr::key1 ) & ( 1 << 7 );
    if( cpuDoubleSpeed )
        cpu.tick();

    //apu.tick();
}
