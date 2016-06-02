#ifndef BAP_WRITER_TEXT_HPP
#define BAP_WRITER_TEXT_HPP

#include <fstream>
#include "bpt_writer.hpp"

namespace bpt {

struct writer_text : writer {
    explicit writer_text(const char* file);
    virtual void visit(const event&);
private:
    std::ofstream out;
};

} //namespace bpt

#endif //BAP_WRITER_TEXT_HPP
