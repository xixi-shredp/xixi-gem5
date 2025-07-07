#include "cpu/o3/spmm_buffer.hh"

#include <cstdint>
#include <cstring>

#include "base/logging.hh"

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
}

SpMMBuffer::~SpMMBuffer(){
    for (auto p: storage){
        delete [] p;
    }
}

void
SpMMBuffer::read(int index, uint8_t* data){
    auto i = index & index_mask;
    auto p = storage[i];
    std::memcpy(data, p, word_byte);
}

void
SpMMBuffer::write(int index, uint8_t* data){
    auto i = index & index_mask;
    auto p = storage[i];
    std::memcpy(p, data, word_byte);
}

} // namespace o3
} // namespace gem5
