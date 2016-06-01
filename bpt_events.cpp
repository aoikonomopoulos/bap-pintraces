#include <iomanip>
#include <iostream>
#include <numeric>
#include <algorithm>
#include <functional>

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

namespace io {
struct ios_flag_saver {
    explicit ios_flag_saver(std::ostream& os)
        : os_(os), f_(os.flags()) {}
    ~ios_flag_saver() { os_.flags(f_); }
private:
    std::ostream& os_;
    std::ios::fmtflags f_;
};

struct bytes_printer : std::binary_function<const std::string&,
                                            char,
                                           std::string> {
    bytes_printer(std::ostream& o, const std::string& s)
        : out(o), sep(s) {}
    std::string operator()(const std::string& init, char b) {
        ios_flag_saver s(out);
        out << init << std::hex << std::uppercase << std::setfill('0') << std::setw(2)
            << static_cast<int>(static_cast<unsigned char>(b));
        return sep;
    }
private:
    std::ostream& out;
    const std::string& sep;
};

namespace code {
std::ostream& operator<<(std::ostream& out,
                         const bytes_type& bytes) {
    std::accumulate(bytes.begin(), bytes.end(),
                    std::string(""), bytes_printer(out, " "));
    return out;
}
} //namespace code

namespace data {
std::ostream& operator<<(std::ostream& out,
                         const bytes_type& data) {
    bytes_type::const_reverse_iterator it =
        std::find_if(data.rbegin(), data.rend(),
                     std::bind1st(std::not_equal_to<char>(), 0));
    if (it != data.rend()) {
        out << "0x";
        std::accumulate(it, data.rend(),
                        std::string(""), bytes_printer(out, ""));
    } else {
        out << "0x0";
    }
    return out;
}
} //namespace data
} //namespace io

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

OPCODE operation::opcode() const {
    return pimpl->opcode;
}

ADDRINT operation::addr() const {
    return pimpl->addr;
}

THREADID operation::tid() const {
    return pimpl->tid;
}

const bytes_type& operation::bytes() const {
    return pimpl->bytes;
}

std::string operation::name() const {
    return OPCODE_StringShort(pimpl->opcode);
}

const char* operation::disasm() const {
    return pimpl->disasm;
}

std::ostream& operation::operator<<(std::ostream& out) const {
    using namespace io::code;
    return out << this->name()
               << '('
               << this->tid()
               << ','
               << "0x"
               << std::hex
               << this->addr()
               << std::dec
               << ':'
               << sizeof(ADDRINT)*8
               << ')'
               << '['
               << this->bytes()
               << "] "
               << this->disasm();
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
    using namespace io::data;
    out << this->name() << " => "
        << this->bytes() << ':' << this->width();
    return out;
}

write::write(OPCODE opcode, REG reg, const CONTEXT* ctx)
    : register_io(opcode, reg, ctx) {}

std::ostream& write::operator<<(std::ostream& out) const {
    using namespace io::data;
    out << this->name() << " <= "
        << this->bytes() << ':' << this->width();
    return out;
}
// void write::accept(saver* s) { s->visit(*this); }

}} //namespace bpt::events
