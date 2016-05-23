#ifndef BAP_PIN_TRACER_FULL_FLAGS_SPLITTER_HPP
#define BAP_PIN_TRACER_FULL_FLAGS_SPLITTER_HPP

#include <algorithm>
#include "flags_splitter.hpp"

namespace bap {

namespace {

inline bool get_bit(const bytes_type& bytes, int pos) {
    int byte = pos/8;
    int bit = pos%8;
    return (bytes[byte] >> bit) & 0x1;
}

struct flags_descr {
    const char* name;
    int pos;
};

inline flag get_flag(const flags_descr& descr,
                     const bytes_type& data) {
    flag f = {descr.name, bytes_type(1, get_bit(data, descr.pos)), 1};
    return f;
}

} //namespace

struct full_flags_splitter : flags_splitter {
    flags_type read(const std::string&,
                    const std::string& name,
                    const bytes_type& data) {
        return make(name, data);
    }

    flags_type write(const std::string&,
                     const std::string& name,
                     const bytes_type& data) {
        return make(name, data);
    }

private:
    flags_type make(const std::string& name,
                    const bytes_type& data) {
        static flags_descr descr [] = {
            {"CF", 0},
            {"PF", 2},
            {"AF", 4},
            {"ZF", 6},
            {"SF", 7},
            {"DF", 10},
            {"OF", 11}
        };
        int size = sizeof(descr)/sizeof(flags_descr);
        flags_type flags;
        flags.reserve(size);
        for (flags_descr *first = descr,
                 *last = descr + size;
             first != last; ++first) {
            flags.push_back(get_flag(*first, data));
        }
        return flags;
    }
};

} //namespace bap

#endif //BAP_PIN_TRACER_FULL_FLAGS_SPLITTER_HPP
