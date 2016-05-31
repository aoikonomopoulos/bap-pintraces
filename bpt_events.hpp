#ifndef BPT_EVENT_HPP
#define BTP_EVENT_HPP
#include "boost/shared_ptr.hpp"

namespace bpt {
struct saver;
namespace events {
struct event {
    virtual void accept(saver*) = 0;
    virtual std::ostream& operator<<(std::ostream&) = 0;
};

typedef boost::shared_ptr<event> event_ptr;

struct operation : event {
    operation(OPCODE, ADDRINT, UINT32 size, THREADID, const CONTEXT*);
    void accept(saver*);
    std::ostream& operator<<(std::ostream&);
private:
    struct impl;
    boost::shared_ptr<impl> pimpl;
};

// struct memory : event {
//     memory(ADDRINT addr, UINT32 size);
// private:
//     struct impl;
//     boost::shared_ptr<impl> pimpl;
// };

// struct load : memory {
//     memory(ADDRINT addr, UINT32 size);
//     void accept(saver&);
//     std::ostream& operator<<(std::ostream&);
// }

// struct store : memory {
//     memory(ADDRINT addr, UINT32 size);
//     void accept(saver&);
//     std::ostream& operator<<(std::ostream&);
// };

// struct 

}} //namespace bpt::events

#endif //BPT_EVENT_HPP
