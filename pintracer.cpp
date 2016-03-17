#include <iostream>
#include <fstream>
#include "pin.H"

std::ofstream trace;

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "tracefile", "trace.frame", "specify trace output file name");

INT32 Usage() {
    std::cerr << "This tool store executable trace in bap-trames format"
              << std::endl
              << KNOB_BASE::StringKnobSummary()
              << std::endl;
    return -1;
}

VOID Fini(INT32 code, VOID *v) {
    trace.close();
}

int main(int argc, char* argv[]) {
   if (PIN_Init(argc, argv)) return Usage();
   trace.open(KnobOutputFile.Value().c_str(), std::ofstream::out);
   PIN_StartProgram();
   return 0;
}
