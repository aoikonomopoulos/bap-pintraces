#ifndef BAP_PIN_TRACER_FULL_FLAGS_SPLITTER_HPP
#define BAP_PIN_TRACER_FULL_FLAGS_SPLITTER_HPP

#include <algorithm>
#include <iterator>
#include <boost/range.hpp>
#include <boost/bind.hpp>
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
        flags_type flags;
        flags.reserve(std::distance(boost::begin(descr),
                                    boost::end(descr)) + 1);
        flag f = {name, bytes_type(data), data.size()*8};
        flags.push_back(f);
        std::transform(boost::begin(descr),
                       boost::end(descr),
                       std::back_inserter(flags),
                       boost::bind(&get_flag, _1, data));
        return flags;
    }
};

} //namespace bap

#endif //BAP_PIN_TRACER_FULL_FLAGS_SPLITTER_HPP
