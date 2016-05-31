#ifndef BPT_HPP
#define BPT_HPP

#include "pin.H"

namespace bpt {
struct saver;
VOID trace(TRACE, saver*);
VOID fini(INT32 code, saver*);

}



#endif //BPT_HPP
