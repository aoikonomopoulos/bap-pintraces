#ifndef BPT_FULL_FLAGS_SPLITTER_HPP
#define BPT_FULL_FLAGS_SPLITTER_HPP

#include <algorithm>
#include <iterator>
#include <boost/range.hpp>
#include <boost/bind.hpp>
#include "bpt_flags_splitter.hpp"
#include "bpt_insn_flags.hpp"
namespace bpt {

struct full_flags_splitter : flags_splitter {
    explicit full_flags_splitter(bool rflags) : rflags_(rflags) {}
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
        if (rflags_) {
            flag f = {name, bytes_type(data), data.size()*8};
            flags.push_back(f);
        }
        std::transform(boost::begin(descr),
                       boost::end(descr),
                       std::back_inserter(flags),
                       boost::bind(&get_flag, _1, data));
        return flags;
    }
private:
    bool rflags_;
};

} //namespace bpt

#endif //BPT_FULL_FLAGS_SPLITTER_HPP
