#include "bpt_writer_text.hpp"
#include "bpt_events.hpp"

namespace bpt {
writer_text::writer_text(const char* file)
    : out(file, std::ofstream::out) {}

void writer_text::visit(const event& e) {
    out << e << std::endl;
}
} //namespace bpt
