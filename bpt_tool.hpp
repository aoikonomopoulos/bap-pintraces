#ifndef BPT_TOOL_HPP
#define BPT_TOOL_HPP
#include <set>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include "pin.H"
#include "bpt_tracer.hpp"
#include "bpt_tool_reg.hpp"
#include "bpt_tool_mem.hpp"


typedef bpt::tracer<ADDRINT, THREADID> tracer_type;
namespace bpt { namespace tool {

const char* disassemble(::INS insn) {
    static std::set<std::string> insns;
    std::string dis = INS_Disassemble(insn);
    boost::algorithm::to_upper(dis);
    std::pair<std::set<std::string>::iterator, bool> ret =
        insns.insert(dis);
    return ret.first->c_str();
}

void code_exec(tracer_type* tracer, BOOL cond, const CONTEXT* ctx,
               const char* dis, ADDRINT addr, UINT32 size,
               THREADID tid) {
    reg::update_context(tracer, ctx);
    mem::update_context(tracer, ctx);
    if (cond) {
        bpt::bytes_type bytes(size);
        PIN_SafeCopy(&bytes[0], (const void*) addr, size);
        tracer->save().code_exec(dis, addr, bytes, tid);
    }
}

VOID instruction(::INS ins, VOID* ptr) {
    const char* dis = disassemble(ins);
    if (INS_HasRealRep(ins)) {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)(code_exec),
                       IARG_PTR, ptr,
                       IARG_FIRST_REP_ITERATION,
                       IARG_CONTEXT,
                       IARG_PTR, dis,
                       IARG_INST_PTR,
                       IARG_UINT32, INS_Size(ins),
                       IARG_THREAD_ID,
                       IARG_END);
    } else {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)(code_exec),
                       IARG_PTR, ptr,
                       IARG_BOOL, true,
                       IARG_CONTEXT,
                       IARG_PTR, dis,
                       IARG_INST_PTR,
                       IARG_UINT32, INS_Size(ins),
                       IARG_THREAD_ID,
                       IARG_END);
    }
    reg::instruction(dis, ins, ptr);
    mem::instruction(dis, ins, ptr);
}

VOID fini(INT32 code, VOID* ptr) {
    tracer_type *tracer = static_cast<tracer_type*>(ptr);
    delete tracer;
}

}} //namespace btp::tool
#endif //BPT_TOOL_HPP
