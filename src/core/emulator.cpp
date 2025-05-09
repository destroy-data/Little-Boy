#include "core/emulator.hpp"
#include "core/memory.hpp"

template<typename Tcartridge, typename Tmemory, typename Tcpu, typename Tppu>
void Emulator<Tcartridge, Tmemory, Tcpu, Tppu>::tick() {
    const bool cpuDoubleSpeed = memory.read( addr::key1 ) & ( 1 << 7 );
    cpu.tick();
    ppu.tick();
    if( cpuDoubleSpeed )
        cpu.tick();
    //apu.tick();
}
