#ifndef BAP_PIN_TRACER_NONE_FLAGS_SPLITTER_HPP
#define BAP_PIN_TRACER_NONE_FLAGS_SPLITTER_HPP

#include "flags_splitter.hpp"

namespace bap {

struct none_flags_splitter : flags_splitter {
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
        flag f = {name, bytes_type(data), data.size()*8};
        return flags_type(1, f);
    }
};

} //namespace bap

#endif //BAP_PIN_TRACER_NONE_FLAGS_SPLITTER_HPP
