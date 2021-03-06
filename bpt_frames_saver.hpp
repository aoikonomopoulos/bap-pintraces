#ifndef BAP_PIN_FRAMES_SAVER_HPP
#define BAP_PIN_FRAMES_SAVER_HPP
#include <memory>
#include <boost/scoped_ptr.hpp>
#include <libtrace/trace.container.hpp>
#include "bpt_saver.hpp"

namespace bpt {

namespace trc = SerializedTrace;

template <typename addr_type, typename thread>
struct frames_saver : saver<addr_type, thread> {
    static const frame_architecture arch = frame_arch_i386;
#if defined(TARGET_IA32)
    static const uint64_t machine = frame_mach_i386_i386
#elif defined(TARGET_IA32E)
    static const uint64_t machine = frame_mach_x86_64;
#else
#error "Usupported machine"
#endif
    enum usage {R, W};
    explicit frames_saver(const std::string& path)
        : out(path, arch, machine) {}

    void modload(const std::string& name, addr_type low, addr_type high) {
        frame f;
        f.mutable_modload_frame()->set_module_name(name);
        f.mutable_modload_frame()->set_low_address(low);
        f.mutable_modload_frame()->set_high_address(high);
        out.add(f);
    }

    void code_exec(const std::string& dis,
                   addr_type addr,
                   const bytes_type& bytes,
                   thread tid) {
        current.reset(new holder(*this, addr, bytes, tid));
    }

    void memory_load(addr_type addr, const bytes_type& data) {
        current->mem_operand(R, addr, data);
    }

    void memory_store(addr_type addr, const bytes_type& data) {
        current->mem_operand(W, addr, data);
    }

    void register_read(const std::string& name,
                       const bytes_type& data,
                       int bitsize) {
        current->reg_operand(R, name, data, bitsize);
    }

    void register_write(const std::string& name,
                        const bytes_type& data,
                        int bitsize) {
        current->reg_operand(W, name, data, bitsize);
    }

    ~frames_saver() {
        current.reset();
        out.finish();
    }
private:
    struct holder {
        holder(frames_saver& p,
               addr_type addr, const bytes_type& bytes, thread tid)
            : parent(p) {
            ::std_frame* sf = frm.mutable_std_frame();
            sf->set_address(addr);
            sf->set_thread_id(tid);
            sf->set_rawbytes(std::string(bytes.begin(), bytes.end()));
            sf->mutable_operand_pre_list();
            sf->mutable_operand_post_list();
        }

        void mem_operand(usage u, addr_type addr,
                         const bytes_type& data) {
            ::operand_info_specific* ois = add_operand(u, data);
            ::mem_operand* mo = ois->mutable_mem_operand();
            mo->set_address(addr);
        }

        void reg_operand(usage u, const std::string& name,
                         const bytes_type& data, int bitsize) {
            ::operand_info_specific* ois = add_operand(u, data, bitsize);
            ::reg_operand* ro = ois->mutable_reg_operand();
            ro->set_name(name);
        }

        ~holder() {
            parent.out.add(frm);
        }
    private:
        ::operand_info_specific* add_operand(usage u,
                                             const bytes_type& data,
                                             int bitsize = 0) {
            ::std_frame* sf = frm.mutable_std_frame();
            ::operand_value_list* ovl = 0;
            switch(u) {
            case R: ovl = sf->mutable_operand_pre_list(); break;
            case W: ovl = sf->mutable_operand_post_list(); break;
            }
            ::operand_info* oi = ovl->add_elem();
            oi->set_bit_length(bitsize == 0 ? 8*data.size() : bitsize);
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
        frames_saver& parent;
        ::frame frm;
    };

private:
    trc::TraceContainerWriter out;
    boost::scoped_ptr<holder> current;
};

} //namespace bpt

#endif //BAP_PIN_FRAMES_SAVER_HPP
