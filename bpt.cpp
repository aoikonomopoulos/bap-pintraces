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
#include "bpt_iarg_list.hpp"

namespace bpt {


typedef  std::vector<events::event_ptr> buffer;
static void process_block(buffer&, BBL);
static void process_instruction(buffer&, INS);
static void process_branching(buffer&, INS);
static void process_regs(buffer&, INS);
static void process_mem(buffer&, INS);
static VOID handle_operation(buffer*, const char*,
                             OPCODE, ADDRINT, UINT32, THREADID);
static ADDRINT handle_reads(BOOL, buffer*,
                            OPCODE, const CONTEXT*, UINT32, ...);
static VOID handle_writes(buffer*,
                          OPCODE, const CONTEXT*, UINT32, ...);
static VOID handle_loads(buffer*, UINT32, ...);
static VOID handle_stores(buffer*, UINT32, ...);

struct tracer {
    static void trace(TRACE trace, saver* out) {
        flush(out);
        for(BBL b = TRACE_BblHead(trace);
            BBL_Valid(b); b = BBL_Next(b)) {
            process_block(buff, b);
        }
    }

    static void fini(INT32, saver* out) {
        flush(out);
    }
private:
    static buffer buff;
    static void flush(saver* out) {
        using namespace events;
        std::for_each(boost::begin(buff),
                      boost::end(buff),
                      boost::bind(&event::accept, _1, out));
        buff.clear();
    }
};

buffer tracer::buff;

VOID trace(TRACE trace, saver* out) { tracer::trace(trace, out); }
VOID fini(INT32 code, saver* out) { tracer::fini(code, out); }

void process_block(buffer& buff, BBL b) {
    INS ins = BBL_InsHead(b);
    //counting all instruction except last one, which is single exit
    for(UINT32 i = 1, I = BBL_NumIns(b);
        i < I; ++i, ins = INS_Next(ins)) {
        process_instruction(buff, ins);
    }

    INS tail = BBL_InsTail(b);
    if (INS_HasRealRep(tail)) {
        process_instruction(buff, tail);
    } else {
        process_branching(buff, tail);
    }
}

void process_instruction(buffer& buff, INS ins) {
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
    process_regs(buff, ins);
    process_mem(buff, ins);
}

void process_regs(buffer& buff, INS ins) {
    iarg_list reads;
    iarg_list writes;
    for (UINT32 i = 0, I = INS_OperandCount(ins); i < I; ++i) {
        if (INS_OperandIsReg(ins, i)) {
            REG reg = INS_OperandReg(ins, i);
            if (REG_valid(reg)) {
                if (INS_OperandRead(ins, i)) reads(IARG_UINT32, reg);
                if (INS_OperandWritten(ins, i)) {
                    writes(IARG_UINT32, reg);

                    /* this is special case, when writes occure
                       to the part of GPR */
                    if (REG_is_gr16(reg) || REG_is_gr8(reg)) {
                        reads(IARG_UINT32, reg);
                    }
                }
            }

        }

        if (INS_OperandIsMemory(ins, i) ||
            INS_OperandIsAddressGenerator(ins, i)) {

            REG seg = INS_OperandMemorySegmentReg(ins, i);
            switch (seg) {
            case REG_SEG_FS:
                reads(IARG_UINT32, REG_SEG_FS_BASE);
                break;
            case REG_SEG_GS:
                reads(IARG_UINT32, REG_SEG_GS_BASE);
                break;
            case REG_INVALID_: break;
            default:
                std::cerr << "warning: untraceable segment "
                          << REG_StringShort(seg)
                          << "in instruction "
                          << INS_Disassemble(ins)
                          << std::endl;
            }

            REG base = INS_OperandMemoryBaseReg(ins, i);
            if (REG_valid(base) && base != REG_INST_PTR) {
                reads(IARG_UINT32, base);
            }
            REG index = INS_OperandMemoryIndexReg(ins, i);
            if (REG_valid(index)) reads(IARG_UINT32, index);
        }
    }

    iarg_list common;
    common(IARG_PTR, &buff)
        (IARG_UINT32, INS_Opcode(ins))
        (IARG_CONST_CONTEXT);

    if (INS_IsPredicated(ins)) {
        iarg_list preds;
        if (INS_HasRealRep(ins)) {
            preds(IARG_UINT32, INS_RepCountRegister(ins));
        }

        if (INS_RegRContain(ins, REG_AppFlags())) {
            preds(IARG_UINT32, REG_AppFlags());
        }
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
        INS_InsertCall(ins, IPOINT_BEFORE,
                       (AFUNPTR)(handle_reads),
                       IARG_BOOL, true,
                       IARG_IARGLIST, common.value(),
                       IARG_UINT32, reads.size(),
                       IARG_IARGLIST, reads.value(),
                       IARG_END);

    }
    
    INS_InsertPredicatedCall(ins, IPOINT_AFTER,
                             (AFUNPTR)(&handle_writes),
                             IARG_IARGLIST, common.value(),
                             IARG_UINT32, writes.size(),
                             IARG_IARGLIST, writes.value(),
                             IARG_END);
}

void process_mem(buffer& buff, INS ins) {
    iarg_list loads;
    iarg_list stores;
    for (UINT32 i = 0, I = INS_MemoryOperandCount(ins); i < I; ++i) {
        if (INS_MemoryOperandIsRead(ins, i)) {
            loads(IARG_MEMORYOP_EA, i,
                  IARG_UINT32, INS_MemoryReadSize(ins));
        }

        if (INS_MemoryOperandIsWritten(ins, i)) {
            stores(IARG_MEMORYOP_EA, i,
                   IARG_UINT32, INS_MemoryReadSize(ins));
        }
    }

    INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
                             (AFUNPTR)(handle_loads),
                             IARG_PTR, &buff,
                             IARG_UINT32, loads.size(),
                             IARG_IARGLIST, loads.value(),
                             IARG_END);
    INS_InsertPredicatedCall(ins, IPOINT_AFTER,
                             (AFUNPTR)(handle_stores),
                             IARG_PTR, &buff,
                             IARG_UINT32, stores.size(),
                             IARG_IARGLIST, stores.value(),
                             IARG_END);
}


VOID handle_operation(buffer* buff, const char* disasm, OPCODE opcode,
                      ADDRINT addr, UINT32 size, THREADID tid) {
    events::event_ptr e(new events::operation(disasm, opcode, addr,
                                              size, tid));
    buff->push_back(e);

}

static ADDRINT handle_reads(BOOL exec, buffer* buff, OPCODE opcode,
                            const CONTEXT* ctx, UINT32 args_count, ...) {
    if (exec) {
        va_list args;
        va_start(args, args_count);
        for (UINT32 i=0; i < args_count; ++i) {
            REG reg = static_cast<REG>(va_arg(args, UINT32));
            events::event_ptr e(new events::read(opcode, reg, ctx));
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
        events::event_ptr e(new events::write(opcode, reg, ctx));
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
        events::event_ptr e(new events::load(addr, size));
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
        events::event_ptr e(new events::store(addr, size));
        buff->push_back(e);
    }
    va_end(args);
}

void process_branching(buffer& buff, INS ins) {
    /*FIXME: unimplemented*/
}

} //namespace bpt
