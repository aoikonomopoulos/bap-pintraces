#include "bpt_events.hpp"

namespace bpt {

read_flags_event::read_flags_event(OPCODE opcode, REG reg, const CONTEXT* ctx)
    : read_event(opcode, reg, ctx) {}

write_flags_event::write_flags_event(OPCODE opcode, REG reg, const CONTEXT* ctx)
    : write_event(opcode, reg, ctx) {}

} //namespace bpt
