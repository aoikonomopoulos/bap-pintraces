#ifndef BAP_PIN_TOOL_REG_HPP
#define BAP_PIN_TOOL_REG_HPP

#include <list>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include "pin.H"
#include "tracer.hpp"


typedef bap::tracer<ADDRINT, THREADID> tracer_type;
namespace tool { namespace reg {

static bool enable_rip = false;
std::string register_name(REG reg) {
    switch(reg) {
    case REG_SEG_FS_BASE: return "FS_BASE";
    case REG_SEG_GS_BASE: return "GS_BASE";
    default:
        std::string name(REG_StringShort(reg));
        boost::algorithm::to_upper(name);
        return name;
    }
}

typedef std::list<std::pair<const char*, REG> > regs_type;
regs_type m_regs;

void read(const char* dis,
          tracer_type* tracer,
          const CONTEXT *ctxt,
          UINT32 values_count, ...) {
    va_list va;
    va_start(va, values_count);
    REG reg = REG_INVALID();
    for (UINT32 i=0; i < values_count; ++i) {
        reg = static_cast<REG>(va_arg(va, UINT32));
        if (REG_valid(reg)) {
            reg = REG_FullRegName(reg);
            if (reg == REG_INST_PTR && !enable_rip) continue;
            UINT32 size = REG_Size(reg);
            bap::bytes_type data(size);
            PIN_GetContextRegval(ctxt, reg,
                                 reinterpret_cast<UINT8*>(&data[0]));
            if(REG_is_flags(reg)) {
                bap::flags_type flags =
                    tracer->split().read(dis, register_name(reg), data);
                for(bap::flags_type::iterator i = flags.begin();
                    i < flags.end(); ++i) {
                    tracer->save().register_read(i->name, i->data,
                                                 i->bitsize);
                }
            } else {
                tracer->save().register_read(register_name(reg), data);
            }
        }
    }
    va_end(va);
}

void remember_write(const char* dis,
                    tracer_type*,
                    UINT32 values_count, ...) {
    va_list va;
    va_start(va, values_count);
    for (UINT32 i=0; i < values_count; ++i) {
        REG reg = static_cast<REG>(va_arg(va, UINT32));
        m_regs.push_back(std::make_pair(dis, reg));
    }
    va_end(va);
}

void special_write(const char* dis,
           tracer_type* tracer,
           const CONTEXT *ctxt, REG reg) {
    if (REG_valid(reg)) {
        reg = REG_FullRegName(reg);
        if (reg == REG_INST_PTR && !enable_rip) return;
        UINT32 size = REG_Size(reg);
        bap::bytes_type data(size);
        PIN_GetContextRegval(ctxt, reg,
                             reinterpret_cast<UINT8*>(&data[0]));
        if(REG_is_flags(reg)) {
            bap::flags_type flags =
                tracer->split().write(dis, register_name(reg), data);
            for(bap::flags_type::iterator i = flags.begin();
                i < flags.end(); ++i) {
                tracer->save().register_write(i->name, i->data,
                                              i->bitsize);
            }
        } else {
            tracer->save().register_write(register_name(reg), data);
        }
    }
}

void write(const char* dis,
           tracer_type* tracer,
           const CONTEXT *ctxt,
           UINT32 values_count, ...) {
    va_list va;
    va_start(va, values_count);
    REG reg = REG_INVALID();
    for (UINT32 i=0; i < values_count; ++i) {
        reg = static_cast<REG>(va_arg(va, UINT32));
        special_write(dis, tracer, ctxt, reg);
    }
    va_end(va);
}

bool write_if_zf(ADDRINT rflags,
                 const char* dis,
                 tracer_type* tracer,
                 const CONTEXT *ctxt,
                 UINT32 values_count, ...) {
    if (rflags & 0x40) {
        va_list va;
        va_start(va, values_count);
        REG reg = REG_INVALID();
        for (UINT32 i=0; i < values_count; ++i) {
            reg = static_cast<REG>(va_arg(va, UINT32));
            special_write(dis, tracer, ctxt, reg);
        }
        va_end(va);
        return false;
    }
    return true;
}

void update_context(tracer_type* tracer, const CONTEXT * ctxt) {
    std::for_each(
        boost::begin(m_regs),
        boost::end(m_regs),
        boost::bind(special_write,
                    boost::bind(&regs_type::value_type::first, _1),
                    tracer,
                    ctxt,
                    boost::bind(&regs_type::value_type::second, _1)));
    m_regs.clear();
}

VOID instruction_default(const char* dis, INS ins, VOID* ptr) {
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
            REG seg = INS_OperandMemorySegmentReg(ins, i);
            REG base = INS_OperandMemoryBaseReg(ins, i);
            REG index = INS_OperandMemoryIndexReg(ins, i);
            IARGLIST_AddArguments(
                regs_rd,
                IARG_UINT32, (seg == REG_SEG_FS ? REG_SEG_FS_BASE :
                              seg == REG_SEG_GS ? REG_SEG_GS_BASE :
                              REG_INVALID()),
                IARG_UINT32, base,
                IARG_UINT32, index,
                IARG_END);
            rd_count += 3;
            if (INS_RegWContain(ins, base)) {
                IARGLIST_AddArguments(
                    regs_wr,
                    IARG_UINT32, base,
                    IARG_END);
                ++wr_count;
            }

            if (INS_RegWContain(ins, index)) {
                IARGLIST_AddArguments(
                    regs_wr,
                    IARG_UINT32, index,
                    IARG_END);
                ++wr_count;
            }
        }

        if (INS_OperandIsReg(ins, i) && INS_OperandWritten(ins, i)) {
            REG reg = INS_OperandReg(ins, i);
            IARGLIST_AddArguments(
                regs_wr,
                IARG_UINT32, reg,
                IARG_END);
            ++wr_count;

            if (REG_is_gr16(reg) || REG_is_gr8(reg)) {
                IARGLIST_AddArguments(
                    regs_rd,
                    IARG_UINT32, reg,
                    IARG_END);
                ++rd_count;
            }
        }
    }

    INS_InsertCall(ins, IPOINT_BEFORE,
                   (AFUNPTR)(read),
                   IARG_PTR, dis,
                   IARG_PTR, ptr,
                   IARG_CONST_CONTEXT,
                   IARG_UINT32, rd_count,
                   IARG_IARGLIST, regs_rd,
                   IARG_END);
    INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
                             (AFUNPTR)(remember_write),
                             IARG_PTR, dis,
                             IARG_PTR, ptr,
                             IARG_UINT32, wr_count,
                             IARG_IARGLIST, regs_wr,
                             IARG_END);
    IARGLIST_Free(regs_rd);
    IARGLIST_Free(regs_wr);
}

VOID instruction_cmpxchg(const char* dis, INS ins, VOID* ptr) {
    IARGLIST regs_rd = IARGLIST_Alloc();
    IARGLIST regs_if_wr = IARGLIST_Alloc();
    IARGLIST regs_then_wr = IARGLIST_Alloc();
    UINT32 rd_count = 0;
    UINT32 if_wr_count = 0;
    UINT32 then_wr_count = 0;

    if ( INS_OperandIsReg(ins, 0) ) {
        IARGLIST_AddArguments(
            regs_rd,
            IARG_UINT32, INS_OperandReg(ins, 0),
            IARG_END);
        ++rd_count;

        IARGLIST_AddArguments(
            regs_then_wr,
            IARG_UINT32, INS_OperandReg(ins, 0),
            IARG_END);
        ++then_wr_count;
    } else if (INS_OperandIsMemory(ins, 0) ||
               INS_OperandIsAddressGenerator(ins, 0)) {
        REG seg = INS_OperandMemorySegmentReg(ins, 0);
        REG base = INS_OperandMemoryBaseReg(ins, 0);
        REG index = INS_OperandMemoryIndexReg(ins, 0);
        IARGLIST_AddArguments(
            regs_rd,
            IARG_UINT32, (seg == REG_SEG_FS ? REG_SEG_FS_BASE :
                          seg == REG_SEG_GS ? REG_SEG_GS_BASE :
                          REG_INVALID()),
            IARG_UINT32, base,
            IARG_UINT32, index,
            IARG_END);
        rd_count += 3;
    }

    IARGLIST_AddArguments(
        regs_rd,
        IARG_UINT32, INS_OperandReg(ins, 1),
        IARG_UINT32, INS_OperandReg(ins, 2),
        IARG_END);
    rd_count += 2;


    IARGLIST_AddArguments(
        regs_if_wr,
        IARG_UINT32, INS_OperandReg(ins, 3),
        IARG_END);
    ++if_wr_count;

    IARGLIST_AddArguments(
        regs_then_wr,
        IARG_UINT32, INS_OperandReg(ins, 1),
        IARG_UINT32, INS_OperandReg(ins, 3),
        IARG_END);
    then_wr_count+=2;

    INS_InsertCall(ins, IPOINT_BEFORE,
                   (AFUNPTR)(read),
                   IARG_PTR, dis,
                   IARG_PTR, ptr,
                   IARG_CONST_CONTEXT,
                   IARG_UINT32, rd_count,
                   IARG_IARGLIST, regs_rd,
                   IARG_END);

    INS_InsertIfCall(ins, IPOINT_AFTER,
                     (AFUNPTR)(write_if_zf),
                     IARG_REG_VALUE, INS_OperandReg(ins, 3),
                     IARG_PTR, dis,
                     IARG_PTR, ptr,
                     IARG_CONST_CONTEXT,
                     IARG_UINT32, if_wr_count,
                     IARG_IARGLIST, regs_if_wr,
                     IARG_END);

    INS_InsertThenCall(ins, IPOINT_AFTER,
                       (AFUNPTR)(write),
                       IARG_PTR, dis,
                       IARG_PTR, ptr,
                       IARG_CONST_CONTEXT,
                       IARG_UINT32, then_wr_count,
                       IARG_IARGLIST, regs_then_wr,
                       IARG_END);

    IARGLIST_Free(regs_rd);
    IARGLIST_Free(regs_if_wr);
    IARGLIST_Free(regs_then_wr);

}

VOID instruction(const char* dis, INS ins, VOID* ptr) {
    switch (INS_Opcode(ins)) {
    case   XED_ICLASS_CMPXCHG: instruction_cmpxchg(dis, ins, ptr); break;
    default: instruction_default(dis, ins, ptr);
    }
}


}} //namespace tool::reg
#endif //BAP_PIN_TOOL_HPP
