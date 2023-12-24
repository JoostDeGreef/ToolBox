#pragma once

#include "FIFO_NoLock.h"

template<typename T,size_t BLOCK_SIZE = 256>
using FIFO = FIFO_NoLock<T,BLOCK_SIZE>;

