#ifndef BAP_PIN_TRACER_FLAGS_SPLITTER_HPP
#define BAP_PIN_TRACER_FLAGS_SPLITTER_HPP

#include <string>
#include <vector>

#include "saver.hpp"
#include "insn_flags.hpp"
namespace bap {

struct flag {
    std::string name;
    bytes_type data;
    int bitsize;
};

typedef std::vector<flag> flags_type;

struct flags_splitter {
    virtual flags_type read(const std::string& dis,
                            const std::string& name,
                            const bytes_type&) = 0;

    virtual flags_type write(const std::string& dis,
                             const std::string& name,
                             const bytes_type&) = 0;
    virtual ~flags_splitter() {}
};


inline bool get_value(const bytes_type& bytes, int pos, int mask) {
    int byte = pos/8;
    int bit = pos%8;
    return (bytes[byte] >> bit) & mask;
}

inline flag get_flag(const flags_descr& descr,
                     const bytes_type& data) {
    flag f = {descr.name, bytes_type(1, get_value(data,
                                                  descr.pos,
                                                  descr.mask)),
              descr.size};
    return f;
}

inline flag get_flag_2(const flags& i,
                       const bytes_type& data) {
    flag f = {descr[i].name, bytes_type(1, get_value(data,
                                                  descr[i].pos,
                                                  descr[i].mask)),
              descr[i].size};
    return f;

}

} //namespace bap



#endif //BAP_PIN_TRACER_FLAGS_SPLITTER_HPP
