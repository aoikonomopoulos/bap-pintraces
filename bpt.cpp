#include <iostream>
#include <fstream>
#include <algorithm>

#include <boost/range.hpp>
#include <boost/bind.hpp>
#include "bpt.hpp"
#include "bpt_events.hpp"
#include "bpt_iarg_list.hpp"

namespace bpt {

typedef std::vector<events::event_ptr> buffer;

void process_block(buffer&, BBL);
void process_instruction(buffer&, INS);
void process_branching(buffer&, INS);
void process_regs(buffer&, INS);
void process_mem(buffer&, INS);
VOID handle_instruction(buffer*, OPCODE, ADDRINT, UINT32, THREADID);
VOID handle_reads(buffer*, OPCODE, const CONTEXT*, UINT32, ...);
VOID handle_writes(buffer*, OPCODE, const CONTEXT*, UINT32, ...);
VOID handle_loads(buffer*, UINT32, ...);
VOID handle_stores(buffer*, UINT32, ...);

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
    INS_InsertCall(ins,
                   IPOINT_BEFORE, (AFUNPTR)(handle_instruction),
                   IARG_PTR, &buff,
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
            default: break;
            }

            REG base = INS_OperandMemoryBaseReg(ins, i);
            if (REG_valid(base)) reads(IARG_UINT32, base);
        }
    }
    INS_InsertCall(ins, IPOINT_BEFORE,
                   (AFUNPTR)(handle_reads),
                   IARG_PTR, &buff,
                   IARG_UINT32, INS_Opcode(ins),
                   IARG_CONST_CONTEXT,
                   IARG_UINT32, reads.size(),
                   IARG_IARGLIST, reads.value(),
                   IARG_END);
    INS_InsertPredicatedCall(ins, IPOINT_AFTER,
                             (AFUNPTR)(handle_writes),
                             IARG_PTR, &buff,
                             IARG_UINT32, INS_Opcode(ins),
                             IARG_CONST_CONTEXT,
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
    INS_InsertCall(ins, IPOINT_BEFORE,
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




void process_branching(buffer& buff, saver* out, INS ins) {
    // INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)(handle_branch),
    //                IARG_PTR, out,
    //                IARG_UINT32, buff.tc,
    //                IARG_UINT32, buff.bc,
    //                IARG_UINT32, buff.ic,
    //                IARG_UINT32, INS_Opcode(ins),
    //                IARG_END);
}

// VOID handle_operation(saver* out, UINT32 tc, UINT32 bc, UINT32 ic, UINT32 opcode) {
//     *out << tc << "." << bc << "." << ic << ": " << OPCODE_StringShort(opcode) << std::endl;
// }

// VOID handle_tail(saver* out, UINT32 tc, UINT32 bc, UINT32 ic, UINT32 opcode) {
//     *out << "TAIL: " << tc << "." << bc << "." << ic << ": " << OPCODE_StringShort(opcode) << std::endl;
// }



}
