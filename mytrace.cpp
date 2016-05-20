#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <list>
#include <cctype>
#include <algorithm>
#include "pin.H"
#include "text_tracer.hpp"

typedef bap::tracer<ADDRINT, THREADID> tracer_type;
namespace trace {

std::string register_name(REG reg) {
    std::string name(REG_StringShort(reg));
    std::transform(name.begin(), name.end(), name.begin(),
                   ::toupper);
    return name;
}

void update_context(tracer_type* tracer, const CONTEXT*);
void code_exec(tracer_type* tracer, BOOL cond, const CONTEXT* ctx,
               const char* dis, ADDRINT addr, UINT32 size,
               THREADID tid) {
    update_context(tracer, ctx);
    if (cond) {
        tracer_type::data_type bytes(size);
        PIN_SafeCopy(&bytes[0], (const void*) addr, size);
        tracer->code_exec(dis, addr, bytes, tid);
    }
}

std::list<REG> m_regs;

void register_read(tracer_type* tracer, const CONTEXT *ctxt,
                   UINT32 values_count, ...) {
    va_list va;
    va_start(va, values_count);
    REG reg = REG_INVALID();
    for (UINT32 i=0; i < values_count; ++i) {
        reg = static_cast<REG>(va_arg(va, UINT32));
        if (REG_valid(reg)) {
            reg = REG_FullRegName(reg);
            UINT32 size = REG_Size(reg);
            tracer_type::data_type data(size);
            PIN_GetContextRegval(ctxt, reg,
                                 reinterpret_cast<UINT8*>(&data[0]));
            tracer->register_read(register_name(reg), data);
        }
    }
    va_end(va);
}

void remember_register_write(tracer_type*, UINT32 values_count, ...) {
    va_list va;
    va_start(va, values_count);
    for (UINT32 i=0; i < values_count; ++i) {
        m_regs.push_back(static_cast<REG>(va_arg(va, UINT32)));
    }
    va_end(va);
}

void register_write(tracer_type* tracer,
                    const CONTEXT *ctxt, REG reg) {
    if (REG_valid(reg)) {
        reg = REG_FullRegName(reg);
        UINT32 size = REG_Size(reg);
        tracer_type::data_type data(size);
        PIN_GetContextRegval(ctxt, reg,
                             reinterpret_cast<UINT8*>(&data[0]));
        tracer->register_write(register_name(reg), data);
    }
}

std::list< std::pair<ADDRINT, UINT32> > m_mems;

void memory_load(tracer_type* tracer, UINT32 values_count, ...) {
    va_list va;
    va_start(va, values_count);
    for (UINT32 i=0; i < values_count; ++i) {
        ADDRINT addr = va_arg(va, ADDRINT);
        UINT32 size = va_arg(va, UINT32);
        tracer_type::data_type data(size);
        PIN_SafeCopy(static_cast<VOID*>(&data[0]),
                     reinterpret_cast<VOID*>(addr), size);
        tracer->memory_load(addr, data);
    }
    va_end(va);
}

void remember_memory_store(tracer_type*, UINT32 values_count, ...) {
    va_list va;
    va_start(va, values_count);
    for (UINT32 i=0; i < values_count; ++i) {
        ADDRINT addr = va_arg(va, ADDRINT);
        UINT32 size = va_arg(va, UINT32);
        m_mems.push_back(std::make_pair(addr, size));
    }
    va_end(va);
}

void memory_store(tracer_type* tracer, ADDRINT addr, UINT32 size) {
    tracer_type::data_type data(size);
    PIN_SafeCopy(static_cast<VOID*>(&data[0]),
                 reinterpret_cast<VOID*>(addr), size);
    tracer->memory_store(addr, data);
}

void update_context(tracer_type* tracer, const CONTEXT * ctxt) {
    for (std::list<REG>::const_iterator r = m_regs.begin();
         r != m_regs.end(); ++r) {
        register_write(tracer, ctxt, *r);
    }
    m_regs.clear();
    for (std::list< std::pair<ADDRINT, UINT32> >::const_iterator m =
             m_mems.begin(); m != m_mems.end(); ++m) {
        memory_store(tracer, m->first, m->second);
    }
    m_mems.clear();
}

} //namespace trace

VOID instruction_regs(INS ins, VOID* ptr) {
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
                             IARG_PTR, ptr,
                             IARG_CONTEXT,
                             IARG_UINT32, rd_count,
                             IARG_IARGLIST, regs_rd,
                             IARG_END);
    INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
                             (AFUNPTR)(trace::remember_register_write),
                             IARG_PTR, ptr,
                             IARG_UINT32, wr_count,
                             IARG_IARGLIST, regs_wr,
                             IARG_END);
    IARGLIST_Free(regs_rd);
    IARGLIST_Free(regs_wr);
}

VOID instruction_mem(INS ins, VOID* ptr) {
    IARGLIST mem_ld = IARGLIST_Alloc();
    IARGLIST mem_st = IARGLIST_Alloc();
    UINT32 ld_count = 0;
    UINT32 st_count = 0;

    for (UINT32 i = 0, I = INS_MemoryOperandCount(ins); i < I; ++i) {
        if (INS_MemoryOperandIsRead(ins, i)) {
            IARGLIST_AddArguments(
                mem_ld,
                IARG_MEMORYOP_EA, i,
                IARG_UINT32, INS_MemoryReadSize(ins),
                IARG_END);
            ++ld_count;
        }

        if (INS_MemoryOperandIsWritten(ins, i)) {
            IARGLIST_AddArguments(
                mem_st,
                IARG_MEMORYOP_EA, i,
                IARG_UINT32, INS_MemoryWriteSize(ins),
                IARG_END);
            ++st_count;
        }
    }

    INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
                             (AFUNPTR)(trace::memory_load),
                             IARG_PTR, ptr,
                             IARG_UINT32, ld_count,
                             IARG_IARGLIST, mem_ld,
                             IARG_END);

    INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
                             (AFUNPTR)(trace::remember_memory_store),
                             IARG_PTR, ptr,
                             IARG_UINT32, st_count,
                             IARG_IARGLIST, mem_st,
                             IARG_END);
    IARGLIST_Free(mem_ld);
    IARGLIST_Free(mem_st);
}

VOID instruction(INS ins, VOID* ptr) {
    std::string *dis = new std::string(INS_Disassemble(ins));
    if (INS_HasRealRep(ins)) {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)(trace::code_exec),
                       IARG_PTR, ptr,
                       IARG_FIRST_REP_ITERATION,
                       IARG_CONTEXT,
                       IARG_PTR, dis->c_str(),
                       IARG_INST_PTR,
                       IARG_UINT32, INS_Size(ins),
                       IARG_THREAD_ID,
                       IARG_END);
    } else {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)(trace::code_exec),
                       IARG_PTR, ptr,
                       IARG_BOOL, true,
                       IARG_CONTEXT,
                       IARG_PTR, dis->c_str(),
                       IARG_INST_PTR,
                       IARG_UINT32, INS_Size(ins),
                       IARG_THREAD_ID,
                       IARG_END);
    }
    instruction_regs(ins, ptr);
    instruction_mem(ins, ptr);
}

VOID fini(INT32 code, VOID* ptr) {
    tracer_type *tracer = static_cast<tracer_type*>(ptr);
    delete tracer;
}

KNOB<string> tracefile(KNOB_MODE_WRITEONCE, "pintool",
                       "o", "trace.txt",
                       "Trace file to output to.");

KNOB<string> format(KNOB_MODE_WRITEONCE, "pintool",
                    "fmt", "text",
                    "Trace output format (text | frames).");


INT32 usage() {
    PIN_ERROR( "This Pintool trace "
               "instructions memory and registers usage\n"
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

int main(int argc, char *argv[]) {
    PIN_InitSymbols();
    if (PIN_Init(argc, argv)) return usage();

    tracer_type *tracer =
        new bap::text_tracer<ADDRINT, THREADID>(tracefile.Value());

    INS_AddInstrumentFunction(instruction,
                              static_cast<VOID*>(tracer));
    PIN_AddFiniFunction(fini,
                        static_cast<VOID*>(tracer));

    // Never returns
    PIN_StartProgram();
    
    return 0;
}
