#ifndef BAP_PIN_TRACER_FULL_FLAGS_SPLITTER_HPP
#define BAP_PIN_TRACER_FULL_FLAGS_SPLITTER_HPP

#include <algorithm>
#include <iterator>
#include <boost/range.hpp>
#include <boost/bind.hpp>
#include "flags_splitter.hpp"
#include "insn_flags.hpp"
namespace bap {

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
