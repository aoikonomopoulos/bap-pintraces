#include <list>
#include <set>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include "pin.H"
#include "text_tracer.hpp"
#include "frames_tracer.hpp"
#include "none_flags_splitter.hpp"
#include "arch_size_flags_splitter.hpp"
#include "full_flags_splitter.hpp"

struct tracer_type {
    tracer_type(bap::tracer<ADDRINT, THREADID>* saver,
                bap::flags_splitter* splitter)
        : saver_(saver)
        , splitter_(splitter) {}
    bap::tracer<ADDRINT, THREADID>& save() { return *saver_; }
    bap::flags_splitter& split() { return *splitter_; }
    ~tracer_type() {
        delete saver_;
        delete splitter_;
    }
private:
    bap::tracer<ADDRINT, THREADID>* saver_;
    bap::flags_splitter* splitter_;
};

namespace trace {

std::string register_name(REG reg) {
    std::string name(REG_StringShort(reg));
    boost::algorithm::to_upper(name);
    return name;
}

std::set<std::string> m_insns;
const char* disassemble(INS insn) {
    std::pair<std::set<std::string>::iterator, bool> ret =
        m_insns.insert(INS_Disassemble(insn));
    return ret.first->c_str();
}

void update_context(tracer_type* tracer, const CONTEXT*);
void code_exec(tracer_type* tracer, BOOL cond, const CONTEXT* ctx,
               const char* dis, ADDRINT addr, UINT32 size,
               THREADID tid) {
    update_context(tracer, ctx);
    if (cond) {
        bap::bytes_type bytes(size);
        PIN_SafeCopy(&bytes[0], (const void*) addr, size);
        tracer->save().code_exec(dis, addr, bytes, tid);
    }
}

typedef std::list<std::pair<const char*, REG> > regs_type;
regs_type m_regs;

void register_read(const char* dis,
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

void remember_register_write(const char* dis,
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

void register_write(const char* dis,
                    tracer_type* tracer,
                    const CONTEXT *ctxt, REG reg) {
    if (REG_valid(reg)) {
        reg = REG_FullRegName(reg);
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

typedef std::list< std::pair<ADDRINT, UINT32> > mems_type;
mems_type m_mems;

void memory_load(tracer_type* tracer, UINT32 values_count, ...) {
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
    bap::bytes_type data(size);
    PIN_SafeCopy(static_cast<VOID*>(&data[0]),
                 reinterpret_cast<VOID*>(addr), size);
    tracer->save().memory_store(addr, data);
}

void update_context(tracer_type* tracer, const CONTEXT * ctxt) {
    std::for_each(
        boost::begin(m_regs),
        boost::end(m_regs),
        boost::bind(register_write,
                    boost::bind(&regs_type::value_type::first, _1),
                    tracer,
                    ctxt,
                    boost::bind(&regs_type::value_type::second, _1)));
    m_regs.clear();

    std::for_each(
        boost::begin(m_mems),
        boost::end(m_mems),
        boost::bind(memory_store,
                    tracer,
                    boost::bind(&mems_type::value_type::first, _1),
                    boost::bind(&mems_type::value_type::second, _1)));
    m_mems.clear();
}

} //namespace trace

VOID instruction_regs(const char* dis, INS ins, VOID* ptr) {
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
                             IARG_PTR, dis,
                             IARG_PTR, ptr,
                             IARG_CONTEXT,
                             IARG_UINT32, rd_count,
                             IARG_IARGLIST, regs_rd,
                             IARG_END);
    INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
                             (AFUNPTR)(trace::remember_register_write),
                             IARG_PTR, dis,
                             IARG_PTR, ptr,
                             IARG_UINT32, wr_count,
                             IARG_IARGLIST, regs_wr,
                             IARG_END);
    IARGLIST_Free(regs_rd);
    IARGLIST_Free(regs_wr);
}

VOID instruction_mem(const char* dis, INS ins, VOID* ptr) {
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
    const char* dis = trace::disassemble(ins);
    if (INS_HasRealRep(ins)) {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)(trace::code_exec),
                       IARG_PTR, ptr,
                       IARG_FIRST_REP_ITERATION,
                       IARG_CONTEXT,
                       IARG_PTR, dis,
                       IARG_INST_PTR,
                       IARG_UINT32, INS_Size(ins),
                       IARG_THREAD_ID,
                       IARG_END);
    } else {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)(trace::code_exec),
                       IARG_PTR, ptr,
                       IARG_BOOL, true,
                       IARG_CONTEXT,
                       IARG_PTR, dis,
                       IARG_INST_PTR,
                       IARG_UINT32, INS_Size(ins),
                       IARG_THREAD_ID,
                       IARG_END);
    }
    instruction_regs(dis, ins, ptr);
    instruction_mem(dis, ins, ptr);
}

VOID fini(INT32 code, VOID* ptr) {
    tracer_type *tracer = static_cast<tracer_type*>(ptr);
    delete tracer;
}

KNOB<string> tracefile(KNOB_MODE_WRITEONCE, "pintool",
                       "o", "trace.frames",
                       "Trace file to output to.");

KNOB<string> format(KNOB_MODE_WRITEONCE, "pintool",
                    "fmt", "frames",
                    "Trace output format (text | frames).");

KNOB<string> split(KNOB_MODE_WRITEONCE, "pintool",
                   "split-flags", "none",
                   "Split flags to bits and trace it "
                   "as independed bits. Valid values:\n"
                   "\t none - disable splitting \n"
                   "\t arch - grow flags size to GR size\n"
                   "\t full - split all flags bits \n"
                   "\t insn - trace only "
                   "instruction used flags bits.");


INT32 usage() {
    PIN_ERROR( "This Pintool trace "
               "instructions memory and registers usage\n"
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

tracer_type* build_tracer() {
    std::string fmt = format.Value();
    std::string spl = split.Value();
    std::string path = tracefile.Value();
    bap::tracer<ADDRINT, THREADID> *tracer = 0;
    if (fmt == "text") {
        tracer = new bap::text_tracer<ADDRINT, THREADID>(path);
    } else if (fmt == "frames") {
        tracer = new bap::frames_tracer<ADDRINT, THREADID>(path);
    } else {
        std::cerr << "Unknown trace format " << fmt << std::endl;
        exit(0);
    }

    bap::flags_splitter *splitter = 0;
    if (spl == "none") {
        splitter = new bap::none_flags_splitter();
    } else if (spl == "arch") {
        splitter = new bap::arch_size_flags_splitter<ADDRINT>();
    } else if (spl == "full") {
        splitter = new bap::full_flags_splitter();
    } else {
        std::cerr << "Unknown flags_split option " << spl << std::endl;
        delete tracer;
        exit(0);
    }
    return new tracer_type(tracer, splitter);
}
        
int main(int argc, char *argv[]) {
    PIN_InitSymbols();
    if (PIN_Init(argc, argv)) return usage();

    tracer_type *tracer = build_tracer();

    INS_AddInstrumentFunction(instruction,
                              static_cast<VOID*>(tracer));
    PIN_AddFiniFunction(fini,
                        static_cast<VOID*>(tracer));

    // Never returns
    PIN_StartProgram();
    
    return 0;
}
