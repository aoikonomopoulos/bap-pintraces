#ifndef BPT_EVENTS_HPP
#define BTP_EVENTS_HPP

#include <vector>
#include <string>

#include <pin.H>

#include "bpt_fwd.hpp"

namespace bpt {

struct event {
    void accept(visitor&);
    virtual ~event();
    virtual std::ostream& operator<<(std::ostream&) const = 0;
private:
    virtual void do_accept(visitor&) const = 0;
};

std::ostream& operator<<(std::ostream&, const event&);

struct operation_event : event {
    operation_event(const char*, OPCODE, ADDRINT, UINT32, THREADID);
    OPCODE opcode() const;
    ADDRINT addr() const;
    THREADID tid() const;
    std::string name() const;
    const bytes_type& bytes() const;
    const char* disasm() const;
    virtual std::ostream& operator<<(std::ostream&) const;
private:
    virtual void do_accept(visitor&) const;
    struct impl;
    boost::shared_ptr<impl> pimpl;
};

struct register_event : event {
    register_event(OPCODE, REG, const CONTEXT*);
    const bytes_type& bytes() const;
    std::size_t width() const;
    std::string name() const;
    OPCODE opcode() const;
private:
    struct impl;
    boost::shared_ptr<impl> pimpl;
};

struct memory_event : event {
    memory_event(ADDRINT addr, UINT32 size);
    const bytes_type& bytes() const;
    ADDRINT addr() const;
private:
    struct impl;
    boost::shared_ptr<impl> pimpl;
};

struct read_event : register_event {
    read_event(OPCODE, REG, const CONTEXT*);
    virtual std::ostream& operator<<(std::ostream&) const;
private:
    virtual void do_accept(visitor&) const;
};

struct write_event : register_event {
    write_event(OPCODE, REG, const CONTEXT*);
    virtual std::ostream& operator<<(std::ostream&) const;
private:
    virtual void do_accept(visitor&) const;
};

const int CLR = 1; //The flag is always cleared to 0.
const int SET = 2; //The flag is always set to 1.
const int AH = 4; //The flag is loaded with value from AH register
const int MOD = 8; //The flag is modified, depending on the results of the instruction.
const int POP = 16; //The flag is loaded with value popped off of the stack.
const int TST = 32; //The flag is tested.
const int UND = 64; //The effect on the flag is undefined.

struct flag {
    flag(const std::string&, std::size_t, std::size_t width = 1);
    const std::string& name() const;
    int effect(OPCODE) const;
    std::size_t width() const;
    bytes_type::value_type value(const bytes_type&) const;
private:
    struct impl;
    boost::shared_ptr<impl> pimpl;
};

typedef std::vector<flag> flags_type;
struct read_flags_event : read_event {
    read_flags_event(OPCODE, REG, const CONTEXT*);
    const flags_type& flags() const;
};

struct write_flags_event : write_event {
    write_flags_event(OPCODE, REG, const CONTEXT*);
    const flags_type& flags() const;
};

struct load_event : memory_event {
    load_event(ADDRINT addr, UINT32 size);
    virtual std::ostream& operator<<(std::ostream&) const;
private:
    virtual void do_accept(visitor&) const;
};

struct store_event : memory_event {
    store_event(ADDRINT addr, UINT32 size);
    virtual std::ostream& operator<<(std::ostream&) const;
private:
    virtual void do_accept(visitor&) const;
};

} //namespace bpt

#endif //BPT_EVENTS_HPP
