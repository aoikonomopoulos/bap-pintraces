#ifndef BPT_NONE_FLAGS_SPLITTER_HPP
#define BPT_NONE_FLAGS_SPLITTER_HPP

#include "bpt_flags_splitter.hpp"

namespace bpt {

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

} //namespace bpt

#endif //BPT_NONE_FLAGS_SPLITTER_HPP
