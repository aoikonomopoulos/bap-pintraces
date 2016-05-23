#ifndef BAP_PIN_TRACER_FLAGS_SPLITTER_HPP
#define BAP_PIN_TRACER_FLAGS_SPLITTER_HPP

#include <string>
#include <vector>

#include "tracer.hpp"
namespace bap {

struct flag {
    const std::string name;
    const bytes_type data;
    const int bitsize;
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

} //namespace bap



#endif //BAP_PIN_TRACER_FLAGS_SPLITTER_HPP
