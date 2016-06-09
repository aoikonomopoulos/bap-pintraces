#include "bpt_writer_text.hpp"
#include "bpt_events.hpp"

namespace bpt {
writer_text::writer_text(const std::string& file)
    : out(file.c_str(), std::ofstream::out) {}

void writer_text::visit(const event& e) {
    out << e << std::endl;
}
} //namespace bpt
