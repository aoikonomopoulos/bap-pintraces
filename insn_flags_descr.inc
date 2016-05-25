#if defined(BAP_PINTRACER_DECLARE_FLAGS)
#define entry(name, pos, size, mask) name
#elif defined(BAP_PINTRACER_DECLARE_FLAGS_DESCR)
#define entry(name, pos, size, mask) {#name, pos, size, mask}
#else
#error "no action defined"
#endif

entry(ID, 21, 1, 0x1),
entry(VIP, 20, 1, 0x1),
entry(VIF, 19, 1, 0x1),
entry(AC, 18, 1, 0x1),
entry(VM, 17, 1, 0x1),
entry(RF, 16, 1, 0x1),
entry(NT, 14, 1, 0x1),
entry(IOPL, 12, 2, 0x3),
entry(OF, 11, 1, 0x1),
entry(DF, 10, 1, 0x1),
entry(IF, 9, 1, 0x1),
entry(TF, 8, 1, 0x1),
entry(SF, 7, 1, 0x1),
entry(ZF, 6, 1, 0x1),
entry(AF, 4, 1, 0x1),
entry(PF, 2, 1, 0x1),
entry(CF, 0, 1, 0x1),

#undef entry
