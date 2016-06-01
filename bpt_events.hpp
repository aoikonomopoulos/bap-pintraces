#ifndef BPT_EVENTS_HPP
#define BTP_EVENTS_HPP

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

#include <pin.H>

#include "bpt_saver.hpp"

namespace bpt {

namespace events {

struct event;

std::ostream& operator<<(std::ostream& out, const event& e);

struct event {
    // virtual void accept(saver*) = 0;
    virtual std::ostream& operator<<(std::ostream&) const = 0;
    virtual void accept(saver* out) { *out << *this << std::endl; }
    virtual ~event() {};

};

typedef boost::shared_ptr<event> event_ptr;
typedef std::vector<char> bytes_type;

struct operation : event {
    operation(const char*, OPCODE, ADDRINT, UINT32, THREADID);
//    virtual void accept(saver*);
    virtual std::ostream& operator<<(std::ostream&) const;
    OPCODE opcode() const;
    ADDRINT addr() const;
    THREADID tid() const;
    std::string name() const;
    const bytes_type& bytes() const;
    const char* disasm() const;
private:
    struct impl;
    boost::shared_ptr<impl> pimpl;
};

struct register_io : event {
    register_io(OPCODE, REG, const CONTEXT*);
    const bytes_type& bytes() const;
    std::size_t width() const;
    std::string name() const;
    OPCODE opcode() const;
private:
    struct impl;
    boost::shared_ptr<impl> pimpl;
};

struct read : register_io {
    read(OPCODE, REG, const CONTEXT*);
    // virtual void accept(saver*);
    virtual std::ostream& operator<<(std::ostream&) const;
};

struct write : register_io {
    write(OPCODE, REG, const CONTEXT*);
    // virtual void accept(saver*);
    virtual std::ostream& operator<<(std::ostream&) const;
};


struct memory_io : event {
    memory_io(ADDRINT addr, UINT32 size);
    const bytes_type& bytes() const;
    ADDRINT addr() const;
private:
    struct impl;
    boost::shared_ptr<impl> pimpl;
};

struct load : memory_io {
    load(ADDRINT addr, UINT32 size);
    //virtual void accept(saver*);
    virtual std::ostream& operator<<(std::ostream&) const;
};

struct store : memory_io {
    store(ADDRINT addr, UINT32 size);
    //virtual void accept(saver*);
    virtual std::ostream& operator<<(std::ostream&) const;
};

}} //namespace bpt::events

#endif //BPT_EVENTS_HPP
