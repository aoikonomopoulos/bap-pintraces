#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <set>

#include <boost/range.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

#include "bpt.hpp"
#include "bpt_events.hpp"
#include "bpt_args_list.hpp"
#include "bpt_inspection.hpp"
#include "bpt_visitor.hpp"
namespace bpt {

typedef  std::vector<event_ptr> buffer;
static void process_block(buffer&, BBL);
static void process_instruction(buffer&, INS);
static void process_branching(buffer&, INS);
static void process_inst(buffer&, INS);
static void process_regs(buffer&, INS);
static void process_reads(buffer&, INS);
static void process_writes(buffer&, INS, IPOINT);
static void process_mem(buffer&, INS);
static void process_loads(buffer&, INS);
static void process_stores(buffer&, INS, IPOINT);
static VOID handle_operation(buffer*, const char*,
                             OPCODE, ADDRINT, UINT32, THREADID);
static ADDRINT handle_reads(BOOL, buffer*,
                            OPCODE, const CONTEXT*, UINT32, ...);
static VOID handle_writes(buffer*,
                          OPCODE, const CONTEXT*, UINT32, ...);
static VOID handle_loads(buffer*, UINT32, ...);
static VOID handle_stores(buffer*, UINT32, ...);

struct tracer {
    static void trace(TRACE trace, visitor& out) {
        flush(out);
        for(BBL b = TRACE_BblHead(trace);
            BBL_Valid(b); b = BBL_Next(b)) {
            process_block(buff, b);
        }
    }

    static void fini(INT32, visitor& out) {
        flush(out);
    }
private:
    static buffer buff;
    static void flush(visitor& out) {
        std::for_each(boost::begin(buff),
                      boost::end(buff),
                      boost::bind(&event::accept, _1, boost::ref(out)));
        buff.clear();
    }
};

buffer tracer::buff;

VOID trace(TRACE trace, visitor* out) { tracer::trace(trace, *out); }
VOID fini(INT32 code, visitor* out) { tracer::fini(code, *out); }

static void process_block(buffer& buff, BBL b) {
    INS ins = BBL_InsHead(b);
    //counting all instruction except last one, which is single exit
    for(UINT32 i = 1, I = BBL_NumIns(b);
        i < I; ++i, ins = INS_Next(ins)) {
        process_instruction(buff, ins);
    }

    INS tail = BBL_InsTail(b);
    if (INS_HasFallThrough(tail)) {
        process_instruction(buff, tail);
    } else {
        process_branching(buff, ins);
    }
}

static void process_instruction(buffer& buff, INS ins) {
    process_inst(buff, ins);
    process_regs(buff, ins);
    process_mem(buff, ins);
}

static void process_inst(buffer& buff, INS ins) {
#ifdef BPT_DEBUG
    static std::set<std::string> disasms;
    std::string d = INS_Disassemble(ins);
    boost::algorithm::to_upper(d);
    const char* disasm = disasms.insert(d).first->c_str();
#else
    const char* disasm = "N/A";
#endif
    INS_InsertCall(ins,
                   IPOINT_BEFORE, (AFUNPTR)(handle_operation),
                   IARG_PTR, &buff,
                   IARG_PTR, disasm,
                   IARG_UINT32, INS_Opcode(ins),
                   IARG_INST_PTR,
                   IARG_UINT32, INS_Size(ins),
                   IARG_THREAD_ID,
                   IARG_END);
}

static void process_regs(buffer& buff, INS ins) {
    process_reads(buff, ins);
    process_writes(buff, ins, IPOINT_AFTER);
}

static void process_reads(buffer& buff, INS ins) {
    args_list common;
    common(IARG_PTR, &buff);
    common(IARG_UINT32, INS_Opcode(ins));
    common(IARG_CONST_CONTEXT);

    args_list reads;
    inspect_inst_reads(ins, reads);

    if (INS_IsPredicated(ins)) {
        args_list preds;
        if (INS_HasRealRep(ins))
            preds(IARG_UINT32, INS_RepCountRegister(ins));

        if (INS_RegRContain(ins, REG_AppFlags()))
            preds(IARG_UINT32, REG_AppFlags());

        INS_InsertIfCall(ins, IPOINT_BEFORE,
                         (AFUNPTR)(handle_reads),
                         IARG_EXECUTING,
                         IARG_IARGLIST, common.value(),
                         IARG_UINT32, reads.size(),
                         IARG_IARGLIST, reads.value(),
                         IARG_END);

        INS_InsertThenCall(ins, IPOINT_BEFORE,
                           (AFUNPTR)(handle_reads),
                           IARG_BOOL, true,
                           IARG_IARGLIST, common.value(),
                           IARG_UINT32, preds.size(),
                           IARG_IARGLIST, preds.value(),
                           IARG_END);
    } else {
        if (reads.size() != 0) {
            INS_InsertCall(ins, IPOINT_BEFORE,
                           (AFUNPTR)(handle_reads),
                           IARG_BOOL, true,
                           IARG_IARGLIST, common.value(),
                           IARG_UINT32, reads.size(),
                           IARG_IARGLIST, reads.value(),
                           IARG_END);
        }
    }
}

static void process_writes(buffer& buff, INS ins, IPOINT point) {
    args_list common;
    common(IARG_PTR, &buff);
    common(IARG_UINT32, INS_Opcode(ins));
    common(IARG_CONST_CONTEXT);

    args_list writes;
    inspect_inst_writes(ins, writes);
    if (writes.size() != 0) {
        INS_InsertPredicatedCall(ins, point,
                                 (AFUNPTR)(&handle_writes),
                                 IARG_IARGLIST, common.value(),
                                 IARG_UINT32, writes.size(),
                                 IARG_IARGLIST, writes.value(),
                                 IARG_END);
    }
}

static void process_mem(buffer& buff, INS ins) {
    process_loads(buff, ins);
    process_stores(buff, ins, IPOINT_AFTER);
}

static void process_loads(buffer& buff, INS ins) {
    args_list loads;
    inspect_inst_loads(ins, loads);
    if (loads.size() != 0) {
        INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
                                 (AFUNPTR)(handle_loads),
                                 IARG_PTR, &buff,
                                 IARG_UINT32, loads.size(),
                                 IARG_IARGLIST, loads.value(),
                                 IARG_END);
    }
}

static void process_stores(buffer& buff, INS ins, IPOINT point) {
    args_list stores;
    inspect_inst_stores(ins, stores);
    if (stores.size() != 0) {
        INS_InsertPredicatedCall(ins, point,
                                 (AFUNPTR)(handle_stores),
                                 IARG_PTR, &buff,
                                 IARG_UINT32, stores.size(),
                                 IARG_IARGLIST, stores.value(),
                                 IARG_END);
    }
}


static VOID handle_operation(buffer* buff, const char* disasm,
                             OPCODE opcode, ADDRINT addr, UINT32 size,
                             THREADID tid) {
    event_ptr e(new operation_event(disasm, opcode, addr, size, tid));
    buff->push_back(e);

}

static ADDRINT handle_reads(BOOL exec, buffer* buff, OPCODE opcode,
                            const CONTEXT* ctx, UINT32 args_count, ...) {
    if (exec) {
        va_list args;
        va_start(args, args_count);
        for (UINT32 i=0; i < args_count; ++i) {
            REG reg = static_cast<REG>(va_arg(args, UINT32));
            event_ptr e(REG_is_flags(reg) ?
                        new read_flags_event(opcode, reg, ctx) :
                        new read_event(opcode, reg, ctx));
            buff->push_back(e);
        }
        va_end(args);
    }
    return exec ? 0 : 1;
}

static VOID handle_writes(buffer* buff, OPCODE opcode,
                          const CONTEXT* ctx, UINT32 args_count, ...) {
    va_list args;
    va_start(args, args_count);
    for (UINT32 i=0; i < args_count; ++i) {
        REG reg = static_cast<REG>(va_arg(args, UINT32));
        event_ptr e(REG_is_flags(reg) ?
                    new write_flags_event(opcode, reg, ctx) :
                    new write_event(opcode, reg, ctx));
        buff->push_back(e);
    }
    va_end(args);
}

static VOID handle_loads(buffer* buff, UINT32 args_count, ...) {
    va_list args;
    va_start(args, args_count);
    for (UINT32 i=0; i < args_count; i+=2) {
        ADDRINT addr = va_arg(args, ADDRINT);
        UINT32 size = va_arg(args, UINT32);
        event_ptr e(new load_event(addr, size));
        buff->push_back(e);
    }
    va_end(args);
}

static VOID handle_stores(buffer* buff, UINT32 args_count, ...) {
    va_list args;
    va_start(args, args_count);
    for (UINT32 i=0; i < args_count; i+=2) {
        ADDRINT addr = va_arg(args, ADDRINT);
        UINT32 size = va_arg(args, UINT32);
        event_ptr e(new store_event(addr, size));
        buff->push_back(e);
    }
    va_end(args);
}

static void process_branching(buffer& buff, INS ins) {
    process_inst(buff, ins);
    process_reads(buff, ins);
    process_loads(buff, ins);
    if (INS_IsCall(ins)) {
        process_writes(buff, ins, IPOINT_TAKEN_BRANCH);
        process_stores(buff, ins, IPOINT_TAKEN_BRANCH);
    } else if (INS_IsSyscall(ins)) {
        std::cerr << "syscall arguments not stored" << std::endl;
    } else {
        args_list writes, stores;
        inspect_inst_writes(ins, writes);
        inspect_inst_stores(ins, stores);
        if (writes.size() != 0 || stores.size() != 0) {
            std::cerr << INS_Disassemble(ins)
                      << " writes : "
                      << writes.size()
                      << " stores : "
                      << stores.size()
                      << std::endl;
        }
    }
}

} //namespace bpt
