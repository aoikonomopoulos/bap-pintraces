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

std::list<REG> m_regs;
std::ostream& operator<< (std::ostream& out, const std::vector<char>& bytes) {
    std::string sep = "";
    for (std::vector<char>::const_iterator b = bytes.begin(); b != bytes.end(); ++b) {
        out << sep << std::hex << std::setfill('0') << std::setw(2)
            << (static_cast<int>(*b) & 0xFF);
        sep = " ";
    }
    return out;
}

void start(const char* dis, ADDRINT addr, UINT32 size, THREADID tid) {
    std::vector<char> rawbytes(size);
    PIN_SafeCopy(&rawbytes[0], (const void*) addr, size);
    out << "insn(" << tid << "): [" << rawbytes << "] -> " << dis << std::endl;
    
}

namespace reg {
void written(const CONTEXT*, REG);
}//

void finish(const CONTEXT * ctxt) {
    for (std::list<REG>::const_iterator r = m_regs.begin();
         r != m_regs.end(); ++r) {
        reg::written(ctxt, *r);
    }
    m_regs.clear();
}

namespace mem {
void read() {

}

void write() {

}
}

namespace reg {

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
    } else {
        out << REG_StringShort(r) << " " << r << " <= ?" << std::endl;
    }

}

}

} //namespace trace

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID*) {
    std::string *dis = new std::string(INS_Disassemble(ins));

    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)(trace::finish),
                   IARG_CONTEXT,
                   IARG_END);

    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)(trace::start),
                   IARG_PTR, dis->c_str(),
                   IARG_INST_PTR,
                   IARG_UINT32, INS_Size(ins),
                   IARG_THREAD_ID,
                   IARG_END);
    
    for (UINT32 i = 0, I = INS_OperandCount(ins); i < I; ++i) {
        if (INS_OperandIsReg(ins, i) && INS_OperandRead(ins, i)) {
            INS_InsertCall(ins, IPOINT_BEFORE,
                           (AFUNPTR)(trace::reg::read),
                           IARG_CONTEXT,
                           IARG_UINT32, INS_OperandReg(ins, i),
                           IARG_END);
        }

        if (INS_OperandIsMemory(ins, i) ||
            INS_OperandIsAddressGenerator(ins, i)) {
            INS_InsertCall(ins, IPOINT_BEFORE,
                           (AFUNPTR)(trace::reg::read),
                           IARG_CONTEXT,
                           IARG_UINT32, INS_OperandMemoryBaseReg(ins, i),
                           IARG_END);
            INS_InsertCall(ins, IPOINT_BEFORE,
                           (AFUNPTR)(trace::reg::read),
                           IARG_CONTEXT,
                           IARG_UINT32, INS_OperandMemoryIndexReg(ins, i),
                           IARG_END);
        }

        if (INS_OperandIsReg(ins, i) && INS_OperandWritten(ins, i)) {
            INS_InsertCall(ins, IPOINT_BEFORE,
                           (AFUNPTR)(trace::reg::write),
                           IARG_UINT32, INS_OperandReg(ins, i),
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
