#if defined(declare)
#undef declare
#endif

#if defined(BAP_PINTRACER_DECLARE_INSN)
#define declare(insn, ...) insn
#elif defined(BAP_PIN_TRACER_DECLARE_INSN_FLAGS)
#define declare(insn, ...) { ## __VA_ARGS__ }
#elif defined(BAP_PINTRACER_DECLARE_INSN_NAMES)
#define declare(inst, ...) #inst
#else
#error "no action defined"
#endif


//         INSN,  ID, VIP, VIF,  AC,  VM,  RF,  NT, IOPL,  OF,  DF,  IF,  TF,  SF,  ZF,  AF,  PF,  CF
declare(    AAA,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, UND, UND, T_M, UND, MOD),
declare(    AAS,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, UND, UND, T_M, UND, MOD),
declare(    AAD,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, MOD, MOD, UND, MOD, UND),
declare(    AAM,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, MOD, MOD, UND, MOD, UND),
declare(    ADC,   N,   N,   N,   N,   N,   N,   N,    N, MOD,   N,   N,   N, MOD, MOD, MOD, MOD, T_M),
declare(    ADD,   N,   N,   N,   N,   N,   N,   N,    N, MOD,   N,   N,   N, MOD, MOD, MOD, MOD, MOD),
declare(    AND,   N,   N,   N,   N,   N,   N,   N,    N, CLR,   N,   N,   N, MOD, MOD, UND, MOD, CLR),
declare(   ARPL,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, MOD,   N,   N,   N),
declare(    BSF,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, UND, MOD, UND, UND, UND),
declare(    BSR,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, UND, MOD, UND, UND, UND),
declare(     BT,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, UND, UND, UND, UND, MOD),
declare(    BTC,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, UND, UND, UND, UND, MOD),
declare(    BTR,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, UND, UND, UND, UND, MOD),
declare(    BTS,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, UND, UND, UND, UND, MOD),
declare(   BZHI,   N,   N,   N,   N,   N,   N,   N,    N, CLR,   N,   N,   N, MOD, MOD, UND, UND, MOD),
declare(    CLC,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N,   N,   N,   N, CLR),
declare(    CLD,   N,   N,   N,   N,   N,   N,   N,    N,   N, CLR,   N,   N,   N,   N,   N,   N,   N),
declare(    CLI,   N,   N, MOD,   N,   N,   N,   N,  TST,   N,   N, MOD,   N,   N,   N,   N,   N,   N),
declare(    CMC,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N,   N,   N,   N, MOD),

