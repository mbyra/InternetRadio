// Buffer class used by receiver (so far) for input-output operations.
// Data from transmitter are saved in buffer. Buffer implements function used
// to

#ifndef INTERNETRADIOSIK_BUFFER_H
#define INTERNETRADIOSIK_BUFFER_H


#include <cstdint>
#include <vector>
#include "parameters.h"

class Buffer {
    // All types must be implementation independent
    std::vector<uint32_t> buffer;

public:
    Buffer() {
        buffer.resize(BSIZE); // buffer will have constant size
    }

};


#endif //INTERNETRADIOSIK_BUFFER_H
