#ifndef BAP_WRITER_HPP
#define BAP_WRITER_HPP

#include <boost/noncopyable.hpp>
#include "bpt_fwd.hpp"

namespace bpt {

struct writer : boost::noncopyable {
    virtual void visit(const event&) = 0;
    virtual void visit(const operation_event&);
    virtual void visit(const read_event&);
    virtual void visit(const read_flags_event&);
    virtual void visit(const write_event&);
    virtual void visit(const write_flags_event&);
    virtual void visit(const load_event&);
    virtual void visit(const store_event&);
    virtual ~writer();
};

} //namespace bpt

#endif //BAP_WRITER_HPP
