#include "cpu/o3/spmm_buffer.hh"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

#include "base/logging.hh"
#include "base/trace.hh"
#include "debug/SpMMBuffer.hh"
#include "sim/bufval.hh"
#include "sim/byteswap.hh"

namespace gem5
{

namespace o3
{

SpMMBuffer::SpMMBuffer(int _size, int _word_byte)
    :size(_size), word_byte(_word_byte), index_mask(_size-1)
{
    if (size <= 0){
        panic("SpMMBuffer: size should bigger than 0.");
    }
    if (word_byte <= 0){
        panic("SpMMBuffer: word_byte should bigger than 0.");
    }
    uint8_t* p = nullptr;
    for (int i = 0; i < size; i++){
        p = new uint8_t(word_byte);
        storage.push_back(p);
    }
    DPRINTF(SpMMBuffer, "SpMMBuffer Initialized.\n"
            "\tsize: %#x\n"
            "\tword_byte: %#x\n"
            "\tindex_mask: %#x\n",
            size, word_byte, index_mask);
}

SpMMBuffer::~SpMMBuffer(){
    for (auto p: storage){
        delete [] p;
    }
}

std::string
SpMMBuffer::get_string(uint8_t* data)
{
    auto [reg_val_str, reg_val_success] =
        printUintX(data, word_byte, HostByteOrder);
    if (reg_val_success)
        return reg_val_str;

    return printByteBuf(data, word_byte, ByteOrder::little);
}

void
SpMMBuffer::read(int index, uint8_t* data)
{
    auto i = index & index_mask;
    auto p = storage[i];
    std::memcpy(data, p, word_byte);
    DPRINTF(SpMMBuffer, "read SpMM Buffer[%d] : %s.\n", i, get_string(data));
}

void
SpMMBuffer::write(int index, uint8_t* data)
{
    auto i = index & index_mask;
    auto p = storage[i];
    std::memcpy(p, data, word_byte);
    DPRINTF(SpMMBuffer, "write SpMM Buffer[%d] : %s.\n", i, get_string(data));
}

} // namespace o3
} // namespace gem5
