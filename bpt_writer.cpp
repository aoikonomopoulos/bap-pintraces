#include "bpt_writer.hpp"
#include "bpt_events.hpp"

namespace bpt {

template<typename T>
inline void visit_event(writer& w, const T& e) {
    w.visit(static_cast<const event&>(e));
}

void writer::visit(const operation_event& e) { visit_event(*this, e); }
void writer::visit(const read_event& e) { visit_event(*this, e); }
void writer::visit(const read_flags_event& e) { visit_event(*this, e); }
void writer::visit(const write_event& e) { visit_event(*this, e); }
void writer::visit(const write_flags_event& e) { visit_event(*this, e); }
void writer::visit(const load_event& e) { visit_event(*this, e); }
void writer::visit(const store_event& e) { visit_event(*this, e); }
writer::~writer() {}

}
