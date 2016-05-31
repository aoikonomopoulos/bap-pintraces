#ifndef BPT_INSN_FLAGS_HPP
#define BPT_INSN_FLAGS_HPP

#include <boost/unordered_map.hpp>

namespace bpt {

/*Instruction affects on flag*/
enum affect {
    CLR, /*The flag is always cleared to 0.*/
    SET, /*The flag is always set to 1.*/
    AH, /*The flag is loaded with value from AH register.*/
    MOD, /*The flag is modified, depending on the results of the instruction.*/
    POP, /*The flag is loaded with value popped off of the stack.*/
    TST, /*The flag is tested.*/
    UND, /*The effect on the flag is undefined.*/
    T_M, /*The flag is tested and modified.*/
    T_P, /*The flag is tested and popped.*/
    M_U, /*The flag is modified or undefined.*/
    N /*The flag is not affected by the instruction.*/
};

/*RFLAGS register flags*/
enum flags {
#define BPT_DECLARE_FLAGS
#include "bpt_insn_flags_descr.inc"
#undef BPT_DECLARE_FLAGS
    FLAGS_MAX
};

/*Flag description*/
static const struct flags_descr {
    const char* name;
    int pos;
    int size;
    int mask;
} descr [] = {
#define BPT_DECLARE_FLAGS_DESCR
#include "bpt_insn_flags_descr.inc"
#undef BPT_DECLARE_FLAGS_DESCR
};

/*Instuctions affected of RFLAGS*/
enum insns {
#define BPT_DECLARE_INSN
#include "bpt_insn_flags.inc"
#undef BPT_DECLARE_INSN
INSN_MAX
};

/*Instuctions names affected of RFLAGS*/
static const char* insns_names[] = {
#define BPT_DECLARE_INSN_NAMES
#include "bpt_insn_flags.inc"
#undef BPT_DECLARE_INSN_NAMES
};

/*Instuctions affects to each flag*/
static const affect affects[][FLAGS_MAX] = {
#define BPT_DECLARE_INSN_FLAGS
#include "bpt_insn_flags.inc"
#undef BPT_DECLARE_INSN_FLAGS
};



} //namespace bpt

#endif //BPT_INSN_FLAGS_HPP
