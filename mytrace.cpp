/*
 *  This file contains an ISA-portable PIN tool for tracing memory accesses.
 */
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <list>
#include "pin.H"
#include "text_tracer.hpp"

bap::tracer<ADDRINT, THREADID> *tracer =
    new bap::text_tracer<ADDRINT, THREADID>("trace.txt");

namespace trace {

void flush(const CONTEXT * ctxt);
void code_exec(BOOL cond, const CONTEXT* ctx,
               const char* dis, ADDRINT addr, UINT32 size,
               THREADID tid) {
    flush(ctx);
    if (cond) {
        bap::tracer<ADDRINT, THREADID>::data_type bytes(size);
        PIN_SafeCopy(&bytes[0], (const void*) addr, size);
        tracer->code_exec(dis, addr, bytes, tid);
    }
}

std::list<REG> m_regs;

void register_read(const CONTEXT *ctxt, UINT32 values_count, ...) {
    va_list va;
    va_start(va, values_count);
    REG reg = REG_INVALID();
    for (UINT32 i=0; i < values_count; ++i) {
        reg = static_cast<REG>(va_arg(va, UINT32));
        if (REG_valid(reg)) {
            reg = REG_FullRegName(reg);
            UINT32 size = REG_Size(reg);
            bap::tracer<ADDRINT, THREADID>::data_type data(size);
            PIN_GetContextRegval(ctxt, reg,
                                 reinterpret_cast<UINT8*>(&data[0]));
            tracer->register_read(REG_StringShort(reg), data);
        }
    }
    va_end(va);
}

void remember_register_write(UINT32 values_count, ...) {
    va_list va;
    va_start(va, values_count);
    for (UINT32 i=0; i < values_count; ++i) {
        m_regs.push_back(static_cast<REG>(va_arg(va, UINT32)));
    }
    va_end(va);
}

void register_write(const CONTEXT *ctxt, REG reg) {
    if (REG_valid(reg)) {
        reg = REG_FullRegName(reg);
        UINT32 size = REG_Size(reg);
        bap::tracer<ADDRINT, THREADID>::data_type data(size);
        PIN_GetContextRegval(ctxt, reg,
                             reinterpret_cast<UINT8*>(&data[0]));
        tracer->register_write(REG_StringShort(reg), data);
    }
}

std::list< std::pair<ADDRINT, UINT32> > m_mems;

void memory_load(ADDRINT addr, UINT32 size) {
    bap::tracer<ADDRINT, THREADID>::data_type data(size);
    PIN_SafeCopy(static_cast<VOID*>(&data[0]),
                 reinterpret_cast<VOID*>(addr), size);
    tracer->memory_load(addr, data);
}

void remember_memory_store(ADDRINT addr, UINT32 size) {
    m_mems.push_back(std::make_pair(addr, size));
}

void memory_store(ADDRINT addr, UINT32 size) {
    bap::tracer<ADDRINT, THREADID>::data_type data(size);
    PIN_SafeCopy(static_cast<VOID*>(&data[0]),
                 reinterpret_cast<VOID*>(addr), size);
    tracer->memory_store(addr, data);
}

void flush(const CONTEXT * ctxt) {
    for (std::list<REG>::const_iterator r = m_regs.begin();
         r != m_regs.end(); ++r) {
        register_write(ctxt, *r);
    }
    m_regs.clear();
    for (std::list< std::pair<ADDRINT, UINT32> >::const_iterator m =
             m_mems.begin(); m != m_mems.end(); ++m) {
        memory_store(m->first, m->second);
    }
    m_mems.clear();
}

} //namespace trace

VOID instruction_regs(INS ins) {
    IARGLIST regs_rd = IARGLIST_Alloc();
    IARGLIST regs_wr = IARGLIST_Alloc();
    UINT32 rd_count = 0;
    UINT32 wr_count = 0;
    for (UINT32 i = 0, I = INS_OperandCount(ins); i < I; ++i) {
        if (INS_OperandIsReg(ins, i) && INS_OperandRead(ins, i)) {
            IARGLIST_AddArguments(
                regs_rd,
                IARG_UINT32, INS_OperandReg(ins, i),
                IARG_END);
            ++rd_count;
        }

        if (INS_OperandIsMemory(ins, i) ||
            INS_OperandIsAddressGenerator(ins, i)) {
            IARGLIST_AddArguments(
                regs_rd,
                IARG_UINT32, INS_OperandMemoryBaseReg(ins, i),
                IARG_UINT32, INS_OperandMemoryIndexReg(ins, i),
                IARG_END);
            ++rd_count;
        }

        if (INS_OperandIsReg(ins, i) && INS_OperandWritten(ins, i)) {
            IARGLIST_AddArguments(
                regs_wr,
                IARG_UINT32, INS_OperandReg(ins, i),
                IARG_END);
            ++wr_count;
        }
    }
    INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
                             (AFUNPTR)(trace::register_read),
                             IARG_CONTEXT,
                             IARG_UINT32, rd_count,
                             IARG_IARGLIST, regs_rd,
                             IARG_END);
    INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
                             (AFUNPTR)(trace::remember_register_write),
                             IARG_UINT32, wr_count,
                             IARG_IARGLIST, regs_wr,
                             IARG_END);
    IARGLIST_Free(regs_rd);
    IARGLIST_Free(regs_wr);
}

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID*) {
    std::string *dis = new std::string(INS_Disassemble(ins));
    if (INS_HasRealRep(ins)) {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)(trace::code_exec),
                       IARG_FIRST_REP_ITERATION,
                       IARG_CONTEXT,
                       IARG_PTR, dis->c_str(),
                       IARG_INST_PTR,
                       IARG_UINT32, INS_Size(ins),
                       IARG_THREAD_ID,
                       IARG_END);
    } else {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)(trace::code_exec),
                       IARG_BOOL, true,
                       IARG_CONTEXT,
                       IARG_PTR, dis->c_str(),
                       IARG_INST_PTR,
                       IARG_UINT32, INS_Size(ins),
                       IARG_THREAD_ID,
                       IARG_END);
    }
    instruction_regs(ins);
    for (UINT32 i = 0, I = INS_MemoryOperandCount(ins); i < I; ++i) {
        if (INS_MemoryOperandIsRead(ins, i)) {
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
                                     (AFUNPTR)(trace::memory_load),
                                     IARG_MEMORYOP_EA, i,
                                     IARG_UINT32, INS_MemoryReadSize(ins),
                                     IARG_END);
        }

        if (INS_MemoryOperandIsWritten(ins, i)) {
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
                                     (AFUNPTR)(trace::remember_memory_store),
                                     IARG_MEMORYOP_EA, i,
                                     IARG_UINT32, INS_MemoryWriteSize(ins),
                                     IARG_END);
        }
    }
}

VOID Fini(INT32 code, VOID*) {
    delete tracer;
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */
   
INT32 Usage() {
    PIN_ERROR( "This Pintool prints a trace of memory addresses\n" 
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[]) {
    PIN_InitSymbols();
    if (PIN_Init(argc, argv)) return Usage();

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();
    
    return 0;
}
