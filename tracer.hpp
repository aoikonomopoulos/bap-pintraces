#ifndef BAP_PIN_TRACER_HPP
#define BAP_PIN_TRACER_HPP

#include <stdexcept>
#include <boost/scoped_ptr.hpp>
#include "text_saver.hpp"
#include "frames_saver.hpp"
#include "none_flags_splitter.hpp"
#include "arch_size_flags_splitter.hpp"
#include "full_flags_splitter.hpp"

namespace bap {

template <typename addr_type, typename thread>
struct tracer {
    typedef saver<addr_type, thread> saver_type;
    typedef flags_splitter splitter_type;
    saver_type& save() { return *saver_; }
    splitter_type& split() { return *splitter_; }
    tracer(const std::string& fmt,
           const std::string& path,
           const std::string& split)
        : saver_(create_saver(fmt, path))
        , splitter_(create_splitter(split)) {}

private:
    static saver_type* create_saver(const std::string& fmt,
                                    const std::string path) {
        if ("text" == fmt)
            return new text_saver<addr_type, thread>(path);
        else if ("frames" == fmt)
            return new frames_saver<addr_type, thread>(path);
        else
            throw std::invalid_argument("unknown trace format " + fmt);
    }

    static splitter_type* create_splitter(const std::string& split) {
        if ("none" == split)
            return new none_flags_splitter();
        else if ("arch" == split)
            return new arch_size_flags_splitter<addr_type>();
        else if ("full" == split)
            return new full_flags_splitter();
        else
            throw std::invalid_argument("unknown split format " + split);
    }
private:
    boost::scoped_ptr<saver_type> saver_;
    boost::scoped_ptr<splitter_type> splitter_;
};


} //namespace bap
#endif //BAP_PIN_TRACER_HPP
