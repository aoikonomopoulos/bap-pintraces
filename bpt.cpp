#include "pin.H"
#include "bpt_tool.hpp"

KNOB<string> tracefile(KNOB_MODE_WRITEONCE, "pintool",
                       "o", "trace.frames",
                       "Trace file to output to.");

KNOB<string> format(KNOB_MODE_WRITEONCE, "pintool",
                    "fmt", "frames",
                    "Trace output format (text | frames).");

KNOB<string> split(KNOB_MODE_WRITEONCE, "pintool",
                   "split-flags", "insn",
                   "Split flags to bits and trace it "
                   "as independed bits. Valid values:\n"
                   "\t none - disable splitting \n"
                   "\t arch - grow flags size to GR size\n"
                   "\t full - split all flags bits \n"
                   "\t insn - trace only "
                   "instruction used flags bits.");

KNOB<bool> rflags(KNOB_MODE_WRITEONCE, "pintool",
                   "enable-rflags", "false",
                  "Enable trace RFLAGS register on split");

KNOB<bool> rip(KNOB_MODE_WRITEONCE, "pintool",
               "enable-rip", "false",
               "Enable trace rIP register");

KNOB<bool> uflags(KNOB_MODE_WRITEONCE, "pintool",
                  "enable-undefined-flags", "false",
                  "Enable trace undefined rflags values");


INT32 usage() {
    PIN_ERROR( "This Pintool trace "
               "instructions memory and registers usage\n"
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

static void modload(IMG img, VOID *arg)
{
    tracer_type *tracer = static_cast<tracer_type *>(arg);
    const string &name = IMG_Name(img);

    tracer->save().modload(name, IMG_LowAddress(img), IMG_HighAddress(img));
}

int main(int argc, char *argv[]) {
    PIN_InitSymbols();
    if (PIN_Init(argc, argv)) return usage();
    tracer_type *tracer = 0;
    try {
        tracer = new tracer_type(format.Value(),
                                 tracefile.Value(),
                                 split.Value(),
                                 rflags.Value(),
                                 uflags.Value());
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit(0);
    } catch (...) {
        std::cerr << "unexpected exception" << std::endl;
        exit(0);
    }

    bpt::tool::reg::enable_rip = rip.Value();
    INS_AddInstrumentFunction(bpt::tool::instruction,
                              static_cast<VOID*>(tracer));
    PIN_AddFiniFunction(bpt::tool::fini,
                        static_cast<VOID*>(tracer));
    IMG_AddInstrumentFunction(modload,
                              static_cast<VOID*>(tracer));
    // Never returns
    PIN_StartProgram();
    
    return 0;
}
