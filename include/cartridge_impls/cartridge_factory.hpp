#pragma once
#include "core/cartridge.hpp"
#include <memory>
#include <vector>

namespace CartridgeFactory {
std::unique_ptr<CoreCartridge> create( std::vector<uint8_t>&& rom );
}
