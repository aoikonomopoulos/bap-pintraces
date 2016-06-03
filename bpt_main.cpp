#include <fstream>
#include <iostream>
#include <pin.H>
#include "bpt.hpp"
#include "bpt_writer_text.hpp"
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

VOID fini(INT32 code, VOID* ptr) {
    bpt::visitor* out = static_cast<bpt::visitor*>(ptr);
    bpt::fini(code, out);
    delete out;
}

VOID trace(TRACE trace, VOID* ptr) {
    bpt::visitor* out = static_cast<bpt::visitor*>(ptr);
    bpt::trace(trace, out);
}

int main(int argc, char *argv[]) {
    PIN_InitSymbols();
    if (PIN_Init(argc, argv)) return usage();

    bpt::visitor* out = new bpt::writer_text(tracefile.Value().c_str());

    TRACE_AddInstrumentFunction(trace, static_cast<VOID*>(out));

    PIN_AddFiniFunction(fini, static_cast<VOID*>(out));

    PIN_StartProgram();
    return 0;
}
