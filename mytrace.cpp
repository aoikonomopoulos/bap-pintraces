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

std::list<REG> m_regs;
std::list< std::pair<ADDRINT, UINT32> > m_mems;

void start(BOOL cond, const char* dis, ADDRINT addr, UINT32 size, THREADID tid) {
    if (cond) {
        bap::tracer<ADDRINT, THREADID>::data_type bytes(size);
        PIN_SafeCopy(&bytes[0], (const void*) addr, size);
        tracer->code_exec(dis, addr, bytes, tid);
    }
}

namespace reg {
void written(const CONTEXT*, REG);
}//

namespace mem {
void stored(ADDRINT addr, UINT32 size);
}

void finish(BOOL cond, const CONTEXT * ctxt) {
    for (std::list<REG>::const_iterator r = m_regs.begin();
         r != m_regs.end(); ++r) {
        reg::written(ctxt, *r);
    }
    m_regs.clear();
    for (std::list< std::pair<ADDRINT, UINT32> >::const_iterator m =
             m_mems.begin(); m != m_mems.end(); ++m) {
        mem::stored(m->first, m->second);
    }
    m_mems.clear();
}

namespace mem {
void load(ADDRINT addr, UINT32 size) {
    bap::tracer<ADDRINT, THREADID>::data_type data(size);
    PIN_SafeCopy(static_cast<VOID*>(&data[0]), reinterpret_cast<VOID*>(addr), size);
    tracer->memory_load(addr, data);
}

void store(ADDRINT addr, UINT32 size) {
    m_mems.push_back(std::make_pair(addr, size));
}

void stored(ADDRINT addr, UINT32 size) {
    bap::tracer<ADDRINT, THREADID>::data_type data(size);
    PIN_SafeCopy(static_cast<VOID*>(&data[0]), reinterpret_cast<VOID*>(addr), size);
    tracer->memory_store(addr, data);
}
}

namespace reg {

void read(const CONTEXT * ctxt, REG r) {
    if (REG_valid(r)) {
        REG reg = REG_FullRegName(r);
        UINT32 size = REG_Size(reg);
        bap::tracer<ADDRINT, THREADID>::data_type data(size);
        PIN_GetContextRegval(ctxt, reg, reinterpret_cast<UINT8*>(&data[0]));
        tracer->register_read(REG_StringShort(reg), data);
    }
}

void write(REG r) {
    m_regs.push_back(r);
}

void written(const CONTEXT *ctxt, REG r) {
    if (REG_valid(r)) {
        REG reg = REG_FullRegName(r);
        UINT32 size = REG_Size(reg);
        bap::tracer<ADDRINT, THREADID>::data_type data(size);
        PIN_GetContextRegval(ctxt, reg, reinterpret_cast<UINT8*>(&data[0]));
        tracer->register_write(REG_StringShort(reg), data);
    }
}

}

} //namespace trace

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID*) {
    std::string *dis = new std::string(INS_Disassemble(ins));

    if (INS_HasRealRep(ins)) {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)(trace::finish),
                       IARG_FIRST_REP_ITERATION,
                       IARG_CONTEXT,
                       IARG_END);

        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)(trace::start),
                       IARG_FIRST_REP_ITERATION,
                       IARG_PTR, dis->c_str(),
                       IARG_INST_PTR,
                       IARG_UINT32, INS_Size(ins),
                       IARG_THREAD_ID,
                       IARG_END);
    } else {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)(trace::finish),
                       IARG_BOOL, true,
                       IARG_CONTEXT,
                       IARG_END);

        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)(trace::start),
                       IARG_BOOL, true,
                       IARG_PTR, dis->c_str(),
                       IARG_INST_PTR,
                       IARG_UINT32, INS_Size(ins),
                       IARG_THREAD_ID,
                       IARG_END);
    }
    for (UINT32 i = 0, I = INS_OperandCount(ins); i < I; ++i) {
        if (INS_OperandIsReg(ins, i) && INS_OperandRead(ins, i)) {
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
                           (AFUNPTR)(trace::reg::read),
                           IARG_CONTEXT,
                           IARG_UINT32, INS_OperandReg(ins, i),
                           IARG_END);
        }

        if (INS_OperandIsMemory(ins, i) ||
            INS_OperandIsAddressGenerator(ins, i)) {
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
                           (AFUNPTR)(trace::reg::read),
                           IARG_CONTEXT,
                           IARG_UINT32, INS_OperandMemoryBaseReg(ins, i),
                           IARG_END);
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
                           (AFUNPTR)(trace::reg::read),
                           IARG_CONTEXT,
                           IARG_UINT32, INS_OperandMemoryIndexReg(ins, i),
                           IARG_END);
        }

        if (INS_OperandIsReg(ins, i) && INS_OperandWritten(ins, i)) {
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
                           (AFUNPTR)(trace::reg::write),
                           IARG_UINT32, INS_OperandReg(ins, i),
                           IARG_END);
        }

    }

    for (UINT32 i = 0, I = INS_MemoryOperandCount(ins); i < I; ++i) {
        if (INS_MemoryOperandIsRead(ins, i)) {
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
                                     (AFUNPTR)(trace::mem::load),
                                     IARG_MEMORYOP_EA, i,
                                     IARG_UINT32, INS_MemoryReadSize(ins),
                                     IARG_END);
        }

        if (INS_MemoryOperandIsWritten(ins, i)) {
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
                                     (AFUNPTR)(trace::mem::store),
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
