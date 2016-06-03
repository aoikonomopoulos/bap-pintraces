#include "boost/range.hpp"
#include "bpt_events.hpp"

namespace bpt {

inline std::size_t make_mask(std::size_t width) {
    assert(width == 1 || width == 2);
    return width == 1 ? 0x1 : 0x3;
}

struct flag::impl {
    impl(const std::string& n,
         std::size_t p,
         std::size_t w)
         : name(n), pos(p), width(w), mask(make_mask(w)) {}
    std::string name;
    std::size_t pos;
    std::size_t width;
    std::size_t mask;
};

flag::flag(const std::string& name, std::size_t pos, std::size_t width)
    : pimpl(new impl(name, pos, width)) {}

const std::string& flag::name() const {
    return pimpl->name;
}

int flag::effect(OPCODE opcode) const {
    return CLR | SET | AH | MOD | POP | TST | UND;
}

std::size_t flag::width() const {
    return pimpl->width;
}

bytes_type::value_type flag::value(const bytes_type& bytes) const {
    std::size_t byte = pimpl->pos/8;
    std::size_t bit = pimpl->pos%8;
    std::size_t mask = pimpl->mask;
    return (bytes[byte] >> bit) & mask;
}

read_flags_event::read_flags_event(OPCODE opcode, REG reg, const CONTEXT* ctx)
    : read_event(opcode, reg, ctx) {}


static const flag all[] = { flag("ID", 21),
                            flag("VIP", 20),
                            flag("VIF", 19),
                            flag("AC", 18),
                            flag("VM", 17),
                            flag("RF", 16),
                            flag("NT", 14),
                            flag("IOPL", 12, 2),
                            flag("OF", 11),
                            flag("DF", 10),
                            flag("IF", 9),
                            flag("TF", 8),
                            flag("SF", 7),
                            flag("ZF", 6),
                            flag("AF", 4),
                            flag("PF", 2),
                            flag("CF", 0) };

const flags_type& read_flags_event::flags() const {
    static flags_type f(boost::begin(all), boost::end(all));
    return f;
}

write_flags_event::write_flags_event(OPCODE opcode, REG reg, const CONTEXT* ctx)
    : write_event(opcode, reg, ctx) {}

const flags_type& write_flags_event::flags() const {
    static flags_type f(boost::begin(all), boost::end(all));
    return f;
}

} //namespace bpt

