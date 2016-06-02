
#include <libtrace/trace.container.hpp>
#include "bpt_writer_frames.hpp"

namespace bpt {

struct std_frame_writer {
    std_frame_writer(const operation_event& e) {
        ::std_frame* sf = frm.mutable_std_frame();
        sf->set_address(e.addr());
        sf->set_thread_id(e.tid());
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





} //namespace bpt
