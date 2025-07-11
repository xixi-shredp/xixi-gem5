#ifndef __CPU_O3_SPMM_BUFFER_HH__
#define __CPU_O3_SPMM_BUFFER_HH__

#include <cstdint>
#include <string>
#include <vector>

namespace gem5
{

namespace o3
{

class SpMMBuffer
{
  public:
    /** Creates SpMM Buffer with given size and word bytes. */
    SpMMBuffer(int _size, int _word_byte);

    /** Default destructor. */
    ~SpMMBuffer();

    /** read the buffer into data. */
    void read(int index, uint8_t* data);

    /** write the data into buffer. */
    void write(int index, uint8_t* data);

  private:
    /** size for the whole buffer, in word */
    int size;

    /** size for one word, in byte */
    int word_byte;

    unsigned int index_mask;

    /** actual buffer storage. */
    std::vector<uint8_t*> storage;

  private:
    std::string get_string(uint8_t* data);
};

} // namespace o3
} // namespace gem5

#endif // __CPU_O3_SPMM_BUFFER_HH__
