#include <boost/algorithm/string.hpp>
#include "bpt_events.hpp"

namespace bpt { namespace events {

std::ostream& operator<<(std::ostream& out, const event& e) {
    return e.operator<<(out);
}

template <typename T1, typename T2>
T1 pointer_cast(T2* ptr) {
    return static_cast<T1>(static_cast<void*>(ptr));
}

//operation event
struct operation::impl {
    impl(const char* d, OPCODE o, ADDRINT a, UINT32 size, THREADID t)
#ifdef BPT_DEBUG
        : disasm(d)
#else
        : disasm("")
#endif
        , opcode(o)
        , addr(a)
        , tid(t)
        , bytes(size) {
        PIN_SafeCopy(&bytes[0], (const void*) addr, size);
    }
    const char* disasm;
    OPCODE opcode;
    ADDRINT addr;
    THREADID tid;
    bytes_type bytes;
};

operation::operation(const char* disasm, OPCODE opcode, ADDRINT addr, UINT32 size,
                     THREADID tid)
    : pimpl(new impl(disasm, opcode, addr, size, tid)) {}

std::ostream& operation::operator<<(std::ostream& out) const {
    return out << OPCODE_StringShort(pimpl->opcode) << ": " << pimpl->disasm;
}

//register reads/writes events
struct register_io::impl {
    impl(OPCODE op, REG r, const CONTEXT* ctx)
        : opcode(op)
        , reg(REG_FullRegName(r))
        , bytes(REG_Size(reg)) {
        PIN_GetContextRegval(ctx, reg, pointer_cast<UINT8*>(&bytes[0]));
    }

    
    OPCODE opcode;
    REG reg;
    bytes_type bytes;
};

register_io::register_io(OPCODE opcode, REG reg, const CONTEXT* ctx)
    : pimpl(new impl(opcode, reg, ctx)) {}

const bytes_type& register_io::bytes() const {
    return pimpl->bytes;
}

std::size_t register_io::width() const {
    switch (REG_Width(pimpl->reg)) {
    case REGWIDTH_8: return 8;
    case REGWIDTH_16: return 16;
    case REGWIDTH_32: return 32;
    case REGWIDTH_64: return 64;
    case REGWIDTH_80: return 80;
    case REGWIDTH_128: return 128;
    case REGWIDTH_256: return 256;
    case REGWIDTH_512: return 512;
    case REGWIDTH_FPSTATE: return 16; /*FIXME: this is FSW register?*/
    case REGWIDTH_INVALID:
        std::cerr << "warning: invalid register width "
                  << name()
                  << std::endl;
        break;
    }
    return bytes().size() * 8;
}

std::string register_io::name() const {
    switch(pimpl->reg) {
    case REG_SEG_FS_BASE: return "FS_BASE";
    case REG_SEG_GS_BASE: return "GS_BASE";
    default:
        std::string name(REG_StringShort(pimpl->reg));
        boost::algorithm::to_upper(name);
        return name;
    }
}

OPCODE register_io::opcode() const {
    return pimpl->opcode;
}

read::read(OPCODE opcode, REG reg, const CONTEXT* ctx)
    : register_io(opcode, reg, ctx) {}

// void read::accept(saver* out) { s->visit(*this);}

std::ostream& read::operator<<(std::ostream& out) const {
    return out << this->name() << " => ";
}

write::write(OPCODE opcode, REG reg, const CONTEXT* ctx)
    : register_io(opcode, reg, ctx) {}

std::ostream& write::operator<<(std::ostream& out) const {
    return out << this->name() << " <= ";
}
// void write::accept(saver* s) { s->visit(*this); }

}} //namespace bpt::events
