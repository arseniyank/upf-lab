#pragma once

#include <cstddef>  // size_t
#include <cstdint>  // uint8_t

#include "ue_storage.h"  // UeStorage

void handleArp(int rawFd, const uint8_t* frame, std::size_t len, const UeStorage& ues);
