#ifndef INTERNETRADIOSIK_AUDIOPACKAGE_H
#define INTERNETRADIOSIK_AUDIOPACKAGE_H

#include <string>
#include <cstdint>

// Class containing single audio package, exactly from task description pdf.
class AudioPackage {
    uint64_t session_id;
    uint64_t first_byte_num;
    std::string audio_data; // Raw data stored as characters.
};

#endif //INTERNETRADIOSIK_AUDIOPACKAGE_H
