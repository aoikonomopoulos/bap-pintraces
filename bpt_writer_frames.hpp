#ifndef BPT_WRITER_FRAMES_HPP
#define BPT_WRITER_FRAMES_HPP

#include <boost/shared_ptr.hpp>
#include "bpt_writer.hpp"

namespace bpt {

struct writer_frames : writer {
    explicit writer_text(const char* file);
    virtual void visit(const event&);
    virtual void visit(const operation_event&);
    virtual void visit(const read_event&);
    virtual void visit(const read_flags_event&);
    virtual void visit(const write_event&);
    virtual void visit(const write_flags_event&);
    virtual void visit(const load_event&);
    virtual void visit(const store_event&);
private:
    struct impl;
    boost::shared_ptr<impl> pimpl;
};

} //namespace bpt

#endif //BPT_WRITER_FRAMES_HPP
