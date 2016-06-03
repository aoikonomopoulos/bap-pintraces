#include <string>

#include <boost/range.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/foreach.hpp>

#include <libtrace/trace.container.hpp>

#include "bpt_writer_frames.hpp"
#include "bpt_events.hpp"

namespace bpt {

namespace proto = SerializedTrace;
typedef proto::TraceContainerWriter container_type;

inline std::string string_of_bytes(const bytes_type& bytes) {
    return std::string(boost::begin(bytes), boost::end(bytes));
}

struct std_frame_element : boost::noncopyable {
    enum usage {R, W};
    std_frame_element(container_type& c,
                      const operation_event& op)
        : cont(c) {
        ::std_frame* sf = frm.mutable_std_frame();
        sf->set_address(op.addr());
        sf->set_thread_id(op.tid());
        sf->set_rawbytes(string_of_bytes(op.bytes()));
        sf->mutable_operand_pre_list();
        sf->mutable_operand_post_list();
    }

    void add(const read_event& e) { add(R, e); }
    void add(const write_event& e) { add(W, e); }
    void add(const load_event& e) { add(R, e); }
    void add(const store_event& e) { add(W, e); }
    void add(const read_flags_event& e) {
        BOOST_FOREACH(const flag& f, e.flags()) {
            if (f.effect(e.opcode()) & TST) {
                bytes_type::value_type b(f.value(e.bytes()));
                add(R, f.name(), bytes_type(1, b), f.width());
            }
        }
    }
    void add(const write_flags_event& e) {
        BOOST_FOREACH(const flag& f, e.flags()) {
            if (f.effect(e.opcode()) & (CLR | SET | AH | MOD | POP | UND)) {
                bytes_type::value_type b(f.value(e.bytes()));
                add(R, f.name(), bytes_type(1, b), f.width());
            }
        }
    }

    ~std_frame_element() {
        cont.add(frm);
    }

private:
    void add(usage u, const register_event& e) {
        add(u, e.name(), e.bytes(), e.width()); 
    }

    void add(usage u, const memory_event& e) {
        add(u, e.addr(), e.bytes());
    }

    void add(usage u, ADDRINT addr, const bytes_type& data) {
        ::operand_info_specific* ois = create_operand(u, data);
        ::mem_operand* mo = ois->mutable_mem_operand();
        mo->set_address(addr);
    }

    void add(usage u, const std::string& name,
                     const bytes_type& data, int width) {
        ::operand_info_specific* ois = create_operand(u, data, width);
        ::reg_operand* ro = ois->mutable_reg_operand();
        ro->set_name(name);
    }


    ::operand_info_specific* create_operand(usage u,
                                            const bytes_type& data,
                                            int width = 0) {
        ::std_frame* sf = frm.mutable_std_frame();
        ::operand_value_list* ovl = 0;
        switch(u) {
        case R: ovl = sf->mutable_operand_pre_list(); break;
        case W: ovl = sf->mutable_operand_post_list(); break;
        }
        ::operand_info* oi = ovl->add_elem();
        oi->set_bit_length(width == 0 ? 8*data.size() : width);
        ::operand_usage* ou = oi->mutable_operand_usage();
        ou->set_read(u == R);
        ou->set_written(u == W);
        ou->set_index(false);
        ou->set_base(false);
        oi->set_value(std::string(data.begin(), data.end()));
        oi->mutable_taint_info()->set_no_taint(true);
        return oi->mutable_operand_info_specific();
    }

private:
    container_type& cont;
    ::frame frm;
};

static const frame_architecture arch = frame_arch_i386;
#if defined(TARGET_IA32)
    static const uint64_t machine = frame_mach_i386_i386;
#elif defined(TARGET_IA32E)
    static const uint64_t machine = frame_mach_x86_64;
#else
#error "Usupported machine"
#endif

struct writer_frames::impl {
    explicit impl(const char* file) : cont (file, arch, machine) {}
    container_type cont;
    boost::scoped_ptr<std_frame_element> elem;
    ~impl() {
        elem.reset();
        cont.finish();
    }
};

writer_frames::writer_frames(const char* file) : pimpl(new impl(file)) {}

void writer_frames::visit(const event& e) {
    std::cerr << "warning: skipped event "
              << e
              << " in frames protocol"
              << std::endl;
}

void writer_frames::visit(const operation_event& e) {
    pimpl->elem.reset(new std_frame_element(pimpl->cont, e));
}

void writer_frames::visit(const read_event& e) { pimpl->elem->add(e); }
void writer_frames::visit(const read_flags_event& e) { pimpl->elem->add(e); }
void writer_frames::visit(const write_event& e) { pimpl->elem->add(e); }
void writer_frames::visit(const write_flags_event& e) { pimpl->elem->add(e); }
void writer_frames::visit(const load_event& e) { pimpl->elem->add(e); }
void writer_frames::visit(const store_event& e) { pimpl->elem->add(e); }

} //namespace bpt
