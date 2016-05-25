#if defined(BAP_PINTRACER_DECLARE_INSN)
#define tline(insn, ...) insn
#elif defined(BAP_PIN_TRACER_DECLARE_INSN_FLAGS)
#define tline(insn, ...) { __VA_ARGS__ }
#elif defined(BAP_PINTRACER_DECLARE_INSN_NAMES)
#define tline(insn, ...) #insn
#else
#error "no action defined"
#endif

#define tbwdq(insn, ...) \
        tline(insn, __VA_ARGS__), \
        tline(insn ## B, __VA_ARGS__), \
        tline(insn ## W, __VA_ARGS__), \
        tline(insn ## D, __VA_ARGS__), \
        tline(insn ## Q, __VA_ARGS__)

#define tbwd_(insn, ...) \
        tline(insn, __VA_ARGS__), \
        tline(insn ## B, __VA_ARGS__), \
        tline(insn ## W, __VA_ARGS__), \
        tline(insn ## D, __VA_ARGS__)

#define t__dq(insn, ...) \
        tline(insn, __VA_ARGS__), \
        tline(insn ## D, __VA_ARGS__), \
        tline(insn ## Q, __VA_ARGS__)

#define t__cc(insn, ...) \
  tline(  insn ## O,   N,   N,   N,   N,   N,   N,   N,    N, TST,   N,   N,   N,   N,   N,   N,   N,   N), \
  tline( insn ## NO,   N,   N,   N,   N,   N,   N,   N,    N, TST,   N,   N,   N,   N,   N,   N,   N,   N), \
  tline(  insn ## B,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N,   N,   N,   N, TST), \
  tline(  insn ## C,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N,   N,   N,   N, TST), \
  tline(insn ## NAE,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N,   N,   N,   N, TST), \
  tline( insn ## NB,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N,   N,   N,   N, TST), \
  tline( insn ## NC,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N,   N,   N,   N, TST), \
  tline( insn ## AE,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N,   N,   N,   N, TST), \
  tline(  insn ## Z,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, TST,   N,   N,   N), \
  tline(  insn ## E,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, TST,   N,   N,   N), \
  tline( insn ## NZ,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, TST,   N,   N,   N), \
  tline( insn ## ZE,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, TST,   N,   N,   N), \
  tline( insn ## BE,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, TST,   N,   N, TST), \
  tline( insn ## NA,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, TST,   N,   N, TST), \
  tline(insn ## NBE,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, TST,   N,   N, TST), \
  tline(  insn ## A,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, TST,   N,   N, TST), \
  tline(  insn ## S,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N, TST,   N,   N,   N,   N), \
  tline( insn ## NS,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N, TST,   N,   N,   N,   N), \
  tline(  insn ## P,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N,   N,   N, TST,   N), \
  tline( insn ## PE,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N,   N,   N, TST,   N), \
  tline( insn ## PN,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N,   N,   N, TST,   N), \
  tline( insn ## PO,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N,   N,   N, TST,   N), \
  tline(  insn ## L,   N,   N,   N,   N,   N,   N,   N,    N, TST,   N,   N,   N, TST,   N,   N,   N,   N), \
  tline(insn ## NGE,   N,   N,   N,   N,   N,   N,   N,    N, TST,   N,   N,   N, TST,   N,   N,   N,   N), \
  tline( insn ## NL,   N,   N,   N,   N,   N,   N,   N,    N, TST,   N,   N,   N, TST,   N,   N,   N,   N), \
  tline( insn ## GE,   N,   N,   N,   N,   N,   N,   N,    N, TST,   N,   N,   N, TST,   N,   N,   N,   N), \
  tline( insn ## LE,   N,   N,   N,   N,   N,   N,   N,    N, TST,   N,   N,   N, TST, TST,   N,   N,   N), \
  tline( insn ## NG,   N,   N,   N,   N,   N,   N,   N,    N, TST,   N,   N,   N, TST, TST,   N,   N,   N), \
  tline(insn ## NLE,   N,   N,   N,   N,   N,   N,   N,    N, TST,   N,   N,   N, TST, TST,   N,   N,   N), \
  tline(  insn ## G,   N,   N,   N,   N,   N,   N,   N,    N, TST,   N,   N,   N, TST, TST,   N,   N,   N)

//       INSN,  ID, VIP, VIF,  AC,  VM,  RF,  NT, IOPL,  OF,  DF,  IF,  TF,  SF,  ZF,  AF,  PF,  CF
tline(    AAA,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, UND, UND, T_M, UND, MOD),
tline(    AAS,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, UND, UND, T_M, UND, MOD),
tline(    AAD,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, MOD, MOD, UND, MOD, UND),
tline(    AAM,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, MOD, MOD, UND, MOD, UND),
tline(    ADC,   N,   N,   N,   N,   N,   N,   N,    N, MOD,   N,   N,   N, MOD, MOD, MOD, MOD, T_M),
tline(    ADD,   N,   N,   N,   N,   N,   N,   N,    N, MOD,   N,   N,   N, MOD, MOD, MOD, MOD, MOD),
tline(    AND,   N,   N,   N,   N,   N,   N,   N,    N, CLR,   N,   N,   N, MOD, MOD, UND, MOD, CLR),
tline(   ARPL,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, MOD,   N,   N,   N),
tline(    BSF,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, UND, MOD, UND, UND, UND),
tline(    BSR,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, UND, MOD, UND, UND, UND),
tline(     BT,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, UND, UND, UND, UND, MOD),
tline(    BTC,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, UND, UND, UND, UND, MOD),
tline(    BTR,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, UND, UND, UND, UND, MOD),
tline(    BTS,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, UND, UND, UND, UND, MOD),
tline(   BZHI,   N,   N,   N,   N,   N,   N,   N,    N, CLR,   N,   N,   N, MOD, MOD, UND, UND, MOD),
tline(    CLC,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N,   N,   N,   N, CLR),
tline(    CLD,   N,   N,   N,   N,   N,   N,   N,    N,   N, CLR,   N,   N,   N,   N,   N,   N,   N),
tline(    CLI,   N,   N, MOD,   N,   N,   N,   N,  TST,   N,   N, MOD,   N,   N,   N,   N,   N,   N),
tline(    CMC,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N,   N,   N,   N, MOD),
t__cc(   CMOV,   N,   N,   N,   N,   N,   N,   N,    N, TST,   N,   N,   N, TST, TST,   N, TST, TST),
tline(    CMP,   N,   N,   N,   N,   N,   N,   N,    N, MOD,   N,   N,   N, MOD, MOD, MOD, MOD, MOD),

//       INSN,  ID, VIP, VIF,  AC,  VM,  RF,  NT, IOPL,  OF,  DF,  IF,  TF,  SF,  ZF,  AF,  PF,  CF
tbwdq(   CMPS,   N,   N,   N,   N,   N,   N,   N,    N, MOD, TST,   N,   N, MOD, MOD, MOD, MOD, MOD),
tline(CMPXCHG,   N,   N,   N,   N,   N,   N,   N,    N, MOD,   N,   N,   N, MOD, MOD, MOD, MOD, MOD),
tline(CMPXCHG8B, N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, MOD,   N,   N,   N),
tline(CMPXCHG16B,N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, MOD,   N,   N,   N),
tline( COMISD,   N,   N,   N,   N,   N,   N,   N,    N, CLR,   N,   N,   N, CLR, MOD, CLR, MOD, MOD),
tline( COMISS,   N,   N,   N,   N,   N,   N,   N,    N, CLR,   N,   N,   N, CLR, MOD, CLR, MOD, MOD),
tline(    DAA,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, MOD, MOD, T_M, MOD, T_M),
tline(    DAS,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, MOD, MOD, T_M, MOD, T_M),
tline(    DEC,   N,   N,   N,   N,   N,   N,   N,    N, MOD,   N,   N,   N, MOD, MOD, MOD, MOD,   N),
tline(    DIV,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, UND, UND, UND, UND, UND),
tline( FCMOVB,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N,   N,   N,   N, TST),
tline(FCMOVBE,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, TST,   N,   N, TST),
tline( FCMOVE,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, TST,   N,   N,   N),
tline(FCMOVNB,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N,   N,   N,   N, TST),
tline(FCMOVNBE,  N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, TST,   N,   N, TST),
tline(FCMOVNE,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, TST,   N,   N,   N),
tline(FCMOVNU,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N,   N,   N, TST,   N),
tline( FCMOVU,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N,   N,   N, TST,   N),
tline(  FCOMI,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, MOD,   N, MOD, MOD),
tline( FCOMIP,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, MOD,   N, MOD, MOD),
tline( FUCOMI,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, MOD,   N, MOD, MOD),
tline(FUCOMIP,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, MOD,   N, MOD, MOD),
tline(   IDIV,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, UND, UND, UND, UND, UND),
tline(   IMUL,   N,   N,   N,   N,   N,   N,   N,    N, MOD,   N,   N,   N, UND, UND, UND, UND, MOD),
tline(    INC,   N,   N,   N,   N,   N,   N,   N,    N, MOD,   N,   N,   N, MOD, MOD, MOD, MOD,   N),
tline(     IN,   N,   N,   N,   N,   N,   N,   N,  TST,   N,   N,   N,   N,   N,   N,   N,   N,   N),
tbwd_(  INS,   N,   N,   N,   N,   N,   N,   N,  TST,   N, TST,   N,   N,   N,   N,   N,   N,   N),
tline(    INT,   N,   N, MOD, MOD, T_M, CLR, MOD,  TST,   N,   N, MOD, CLR,   N,   N,   N,   N,   N),
tline(   INTO,   N,   N,   N, MOD, T_M, CLR, MOD,  TST, TST,   N, MOD, MOD,   N,   N,   N,   N,   N),
t__dq(   IRET, POP, POP, POP, POP, T_P, POP, T_P,  T_P, POP, POP, POP, POP, POP, POP, POP, POP, POP),
t__cc(      J,   N,   N,   N,   N,   N,   N,   N,    N, TST,   N,   N,   N, TST, TST,   N, TST, TST),
tline(    LAR,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, MOD,   N,   N,   N),
tbwdq(   LODS,   N,   N,   N,   N,   N,   N,   N,    N,   N, TST,   N,   N,   N,   N,   N,   N,   N),
tline(  LOOPE,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, TST,   N,   N,   N),
tline( LOOPNE,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, TST,   N,   N,   N),
tline(    LSL,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, MOD,   N,   N,   N),
tline(  LZCNT,   N,   N,   N,   N,   N,   N,   N,    N, UND,   N,   N,   N, UND, MOD, UND, UND, MOD),
tbwdq(   MOVS,   N,   N,   N,   N,   N,   N,   N,    N,   N, TST,   N,   N,   N,   N,   N,   N,   N),
tline(    MUL,   N,   N,   N,   N,   N,   N,   N,    N, MOD,   N,   N,   N, UND, UND, UND, UND, MOD),
tline(    NEG,   N,   N,   N,   N,   N,   N,   N,    N, MOD,   N,   N,   N, MOD, MOD, MOD, MOD, MOD),
tline(     OR,   N,   N,   N,   N,   N,   N,   N,    N, CLR,   N,   N,   N, MOD, MOD, UND, MOD, CLR),
tline(    OUT,   N,   N,   N,   N,   N,   N,   N,  TST,   N,   N,   N,   N,   N,   N,   N,   N,   N),
tbwd_(   OUTS,   N,   N,   N,   N,   N,   N,   N,  TST,   N, TST,   N,   N,   N,   N,   N,   N,   N),
tline( POPCNT,   N,   N,   N,   N,   N,   N,   N,    N, CLR,   N,   N,   N, CLR, MOD, CLR, CLR, CLR),
t__dq(   POPF, POP, TST, MOD, POP, TST, CLR, POP,  T_P, POP, POP, POP, POP, POP, POP, POP, POP, POP),

//       INSN,  ID, VIP, VIF,  AC,  VM,  RF,  NT, IOPL,  OF,  DF,  IF,  TF,  SF,  ZF,  AF,  PF,  CF
tline(    RCL,   N,   N,   N,   N,   N,   N,   N,    N, M_U,   N,   N,   N,   N,   N,   N,   N, T_M),
tline(    RCR,   N,   N,   N,   N,   N,   N,   N,    N, M_U,   N,   N,   N,   N,   N,   N,   N, T_M),
tline(    ROL,   N,   N,   N,   N,   N,   N,   N,    N, M_U,   N,   N,   N,   N,   N,   N,   N, T_M),
tline(    ROR,   N,   N,   N,   N,   N,   N,   N,    N, M_U,   N,   N,   N,   N,   N,   N,   N, T_M),
tline(    RSM, MOD, MOD, MOD, MOD, MOD, MOD, MOD,  MOD, MOD, MOD, MOD, MOD, MOD, MOD, MOD, MOD, MOD),
tline(   SAHF,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,  AH,  AH,  AH,  AH,  AH),
tline(    SAL,   N,   N,   N,   N,   N,   N,   N,    N, M_U,   N,   N,   N, MOD, MOD, UND, MOD, MOD),
tline(    SAR,   N,   N,   N,   N,   N,   N,   N,    N, M_U,   N,   N,   N, MOD, MOD, UND, MOD, MOD),
tline(    SBB,   N,   N,   N,   N,   N,   N,   N,    N, MOD,   N,   N,   N, MOD, MOD, MOD, MOD, T_M),
tbwdq(   SCAS,   N,   N,   N,   N,   N,   N,   N,    N, MOD, TST,   N,   N, MOD, MOD, MOD, MOD, MOD),
t__cc(    SET,   N,   N,   N,   N,   N,   N,   N,    N, TST,   N,   N,   N, TST, TST,   N, TST, TST),
tline(   SHLD,   N,   N,   N,   N,   N,   N,   N,    N, M_U,   N,   N,   N, MOD, MOD, UND, MOD, MOD),
tline(   SHRD,   N,   N,   N,   N,   N,   N,   N,    N, M_U,   N,   N,   N, MOD, MOD, UND, MOD, MOD),
tline(    SHR,   N,   N,   N,   N,   N,   N,   N,    N, M_U,   N,   N,   N, MOD, MOD, UND, MOD, MOD),
tline(    STC,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N,   N,   N,   N, SET),
tline(    STD,   N,   N,   N,   N,   N,   N,   N,    N,   N, SET,   N,   N,   N,   N,   N,   N,   N),
tline(    STI,   N,   N, MOD,   N,   N,   N,   N,  TST,   N,   N, MOD,   N,   N,   N,   N,   N,   N),
tbwdq(   STOS,   N,   N,   N,   N,   N,   N,   N,    N,   N, TST,   N,   N,   N,   N,   N,   N,   N),
tline(    SUB,   N,   N,   N,   N,   N,   N,   N,    N, MOD,   N,   N,   N, MOD, MOD, MOD, MOD, MOD),
tline(SYSCALL, MOD, MOD, MOD, MOD, CLR, CLR, MOD,  MOD, MOD, MOD, MOD, MOD, MOD, MOD, MOD, MOD, MOD),
tline(SYSENTER,  N,   N,   N,   N, CLR, CLR,    N,   N,   N,   N, CLR,   N,   N,   N,   N,   N,   N),
tline( SYSRET, MOD, MOD, MOD, MOD, UND, CLR, MOD,  MOD, MOD, MOD, MOD, MOD, MOD, MOD, MOD, MOD, MOD),
tline(   TEST,   N,   N,   N,   N,   N,   N,   N,    N, CLR,   N,   N,   N, MOD, MOD, UND, MOD, CLR),
tline(UCOMISD,   N,   N,   N,   N,   N,   N,   N,    N, CLR,   N,   N,   N, CLR, MOD, CLR, MOD, MOD),
tline(UCOMISS,   N,   N,   N,   N,   N,   N,   N,    N, CLR,   N,   N,   N, CLR, MOD, CLR, MOD, MOD),

//       INSN,  ID, VIP, VIF,  AC,  VM,  RF,  NT, IOPL,  OF,  DF,  IF,  TF,  SF,  ZF,  AF,  PF,  CF
tline(   VERR,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, MOD,   N,   N,   N),
tline(   VERW,   N,   N,   N,   N,   N,   N,   N,    N,   N,   N,   N,   N,   N, MOD,   N,   N,   N),
tline(   XADD,   N,   N,   N,   N,   N,   N,   N,    N, MOD,   N,   N,   N, MOD, MOD, MOD, MOD, MOD),
tline(    XOR,   N,   N,   N,   N,   N,   N,   N,    N, CLR,   N,   N,   N, MOD, MOD, UND, MOD, CLR),

#undef tline
#undef tbwdq
#undef tbwd_
#undef t__dq
#undef t__cc
