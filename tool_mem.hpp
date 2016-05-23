#ifndef BAP_PIN_TOOL_MEM_HPP
#define BAP_PIN_TOOL_MEM_HPP

#include <list>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include "pin.H"
#include "tracer.hpp"


typedef bap::tracer<ADDRINT, THREADID> tracer_type;
namespace tool { namespace mem {

typedef std::list< std::pair<ADDRINT, UINT32> > mems_type;
mems_type m_mems;

void load(tracer_type* tracer, UINT32 values_count, ...) {
    va_list va;
    va_start(va, values_count);
    for (UINT32 i=0; i < values_count; ++i) {
        ADDRINT addr = va_arg(va, ADDRINT);
        UINT32 size = va_arg(va, UINT32);
        bap::bytes_type data(size);
        PIN_SafeCopy(static_cast<VOID*>(&data[0]),
                     reinterpret_cast<VOID*>(addr), size);
        tracer->save().memory_load(addr, data);
    }
    va_end(va);
}

void remember_store(tracer_type*, UINT32 values_count, ...) {
    va_list va;
    va_start(va, values_count);
    for (UINT32 i=0; i < values_count; ++i) {
        ADDRINT addr = va_arg(va, ADDRINT);
        UINT32 size = va_arg(va, UINT32);
        m_mems.push_back(std::make_pair(addr, size));
    }
    va_end(va);
}

void store(tracer_type* tracer, ADDRINT addr, UINT32 size) {
    bap::bytes_type data(size);
    PIN_SafeCopy(static_cast<VOID*>(&data[0]),
                 reinterpret_cast<VOID*>(addr), size);
    tracer->save().memory_store(addr, data);
}

void update_context(tracer_type* tracer, const CONTEXT * ctxt) {
    std::for_each(
        boost::begin(m_mems),
        boost::end(m_mems),
        boost::bind(store,
                    tracer,
                    boost::bind(&mems_type::value_type::first, _1),
                    boost::bind(&mems_type::value_type::second, _1)));
    m_mems.clear();
}

VOID instruction(const char* dis, INS ins, VOID* ptr) {
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
                             (AFUNPTR)(load),
                             IARG_PTR, ptr,
                             IARG_UINT32, ld_count,
                             IARG_IARGLIST, mem_ld,
                             IARG_END);

    INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
                             (AFUNPTR)(remember_store),
                             IARG_PTR, ptr,
                             IARG_UINT32, st_count,
                             IARG_IARGLIST, mem_st,
                             IARG_END);
    IARGLIST_Free(mem_ld);
    IARGLIST_Free(mem_st);
}

}} //namespace tool::mem
#endif //BAP_PIN_TOOL_HPP
