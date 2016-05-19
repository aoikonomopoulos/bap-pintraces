/*
 *  This file contains an ISA-portable PIN tool for tracing memory accesses.
 */
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <list>
#include "pin.H"


std::ofstream out("log.out", std::ofstream::out);

namespace trace {

std::ostream& operator<< (std::ostream& out, const std::vector<char>& bytes) {
    std::string sep = "";
    for (std::vector<char>::const_iterator b = bytes.begin(); b != bytes.end(); ++b) {
        out << sep << std::hex << std::setfill('0') << std::setw(2)
            << (static_cast<int>(*b) & 0xFF);
        sep = " ";
    }
    return out;
}

std::list<REG> m_regs;
std::list< std::pair<ADDRINT, UINT32> > m_mems;
std::ostream& operator<< (std::ostream& out, const std::pair<PIN_REGISTER, UINT32>& regval) {
    const char* b = reinterpret_cast<const char*>(&regval.first);
    UINT32 size = regval.second;
    for(UINT32 i = 0; i < size; ++i) {
        out << std::hex << std::setfill('0') << std::setw(2)
            << (static_cast<int>(b[size - i - 1]) & 0xFF);
    }
    out << ":" << std::dec << 8*size;
    return out;
}

void start(BOOL cond, const char* dis, ADDRINT addr, UINT32 size, THREADID tid) {
    //out << "start: " << (cond ? "true" : "false") << std::endl;
    if (cond) {
        std::vector<char> rawbytes(size);
        PIN_SafeCopy(&rawbytes[0], (const void*) addr, size);
        out << "insn(" << tid << "): [" << rawbytes << "] -> " << dis << std::endl;
    }
    
}

namespace reg {
void written(const CONTEXT*, REG);
}//

namespace mem {
void written(ADDRINT addr, UINT32 size);
}

void finish(BOOL cond, const CONTEXT * ctxt) {
    //out << "finish: " << (cond ? "true" : "false") << std::endl;
    for (std::list<REG>::const_iterator r = m_regs.begin();
         r != m_regs.end(); ++r) {
        reg::written(ctxt, *r);
    }
    m_regs.clear();
    for (std::list< std::pair<ADDRINT, UINT32> >::const_iterator m =
             m_mems.begin(); m != m_mems.end(); ++m) {
        mem::written(m->first, m->second);
    }
    m_mems.clear();
    if (cond) {
        out << std::endl;
    }
}

namespace mem {
void read(ADDRINT addr, UINT32 size) {
    PIN_REGISTER value;
    PIN_SafeCopy(static_cast<VOID*>(&value), reinterpret_cast<VOID*>(addr), size);
    out << std::hex << std::setfill('0') << addr << " => " << std::make_pair(value, size) << std::endl;
}

void write(ADDRINT addr, UINT32 size) {
    m_mems.push_back(std::make_pair(addr, size));
}

void written(ADDRINT addr, UINT32 size) {
    PIN_REGISTER value;
    PIN_SafeCopy(static_cast<VOID*>(&value), reinterpret_cast<VOID*>(addr), size);
    out << std::hex << std::setfill('0') << addr << " => " << std::make_pair(value, size) << std::endl;
}
}

namespace reg {

void read(const CONTEXT * ctxt, REG r) {
    if (REG_valid(r)) {
        REG reg = REG_FullRegName(r);
        UINT32 size = REG_Size(reg);
        PIN_REGISTER value;
        PIN_GetContextRegval(ctxt, reg, reinterpret_cast<UINT8*>(&value));
        out << REG_StringShort(reg) << " => " << std::make_pair(value, size) << std::endl;
    } else {
        out << REG_StringShort(r) << " " << r << " => ?" << std::endl;
    }
}

void write(REG r) {
    m_regs.push_back(r);
}

void written(const CONTEXT *ctxt, REG r) {
    if (REG_valid(r)) {
        REG reg = REG_FullRegName(r);
        UINT32 size = REG_Size(reg);
        PIN_REGISTER value;
        PIN_GetContextRegval(ctxt, reg, reinterpret_cast<UINT8*>(&value));
        out << REG_StringShort(reg) << " <= " << std::make_pair(value, size) << std::endl;
    } /*else {
        out << REG_StringShort(r) << " " << r << " <= ?" << std::endl;
        } */

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
                                     (AFUNPTR)(trace::mem::read),
                                     IARG_MEMORYOP_EA, i,
                                     IARG_UINT32, INS_MemoryReadSize(ins),
                                     IARG_END);
        }

        if (INS_MemoryOperandIsWritten(ins, i)) {
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
                                     (AFUNPTR)(trace::mem::write),
                                     IARG_MEMORYOP_EA, i,
                                     IARG_UINT32, INS_MemoryWriteSize(ins),
                                     IARG_END);
        }
    }
}

VOID Fini(INT32 code, VOID*) {
    out.close();
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
