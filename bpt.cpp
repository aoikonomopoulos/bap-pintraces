#include <iostream>
#include <fstream>
#include "bpt.hpp"

namespace bpt {

struct trace_context {
    UINT32 tc;
    UINT32 bc;
    UINT32 ic;
    void clear() { bc = 0; ic = 0; }
};

VOID handle_next(saver* out, UINT32 tc, UINT32 bc, UINT32 ic, UINT32 opcode) {
    *out << tc << "." << bc << "." << ic << ": " << OPCODE_StringShort(opcode) << std::endl;
}

VOID handle_tail(saver* out, UINT32 tc, UINT32 bc, UINT32 ic, UINT32 opcode) {
    *out << "TAIL: " << tc << "." << bc << "." << ic << ": " << OPCODE_StringShort(opcode) << std::endl;
}


void process_next(trace_context& ctx, saver* out, INS ins) {
    *out << ctx.tc << '.' << ctx.bc << '.' << ctx.ic << ": " << INS_Disassemble(ins) << std::endl;
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)(handle_next),
                   IARG_PTR, out,
                   IARG_UINT32, ctx.tc,
                   IARG_UINT32, ctx.bc,
                   IARG_UINT32, ctx.ic,
                   IARG_UINT32, INS_Opcode(ins),
                   IARG_END);
}

void process_tail(trace_context& ctx, saver* out, INS ins) {
    *out << ctx.tc << '.' << ctx.bc << '.' << ctx.ic << ": " << INS_Disassemble(ins) << std::endl;
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)(handle_tail),
                   IARG_PTR, out,
                   IARG_UINT32, ctx.tc,
                   IARG_UINT32, ctx.bc,
                   IARG_UINT32, ctx.ic,
                   IARG_UINT32, INS_Opcode(ins),
                   IARG_END);
}

void process_block(trace_context& ctx, saver* out, BBL b) {
    ctx.ic = 0;

    INS ins = BBL_InsHead(b);
    //counting all instruction except last one, which is single exit
    for(UINT32 i = 1, I = BBL_NumIns(b);
        i < I; ++i, ins = INS_Next(ins)) {
        process_next(ctx, out, ins);
        ++ctx.ic;
    }
    
    process_tail(ctx, out, BBL_InsTail(b));
    ++ctx.ic;
}

VOID trace(TRACE trace, saver* out) {
    static trace_context ctx;
    ctx.clear();
    for(BBL b = TRACE_BblHead(trace); BBL_Valid(b); b = BBL_Next(b)) {
        process_block(ctx, out, b);
        ++ctx.bc;
    }
    ++ctx.tc;
}

VOID fini(INT32, saver*) {}

}
