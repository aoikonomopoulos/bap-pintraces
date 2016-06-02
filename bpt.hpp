#ifndef BPT_HPP
#define BPT_HPP

#include <pin.H>
#include "bpt_fwd.hpp"

namespace bpt {

VOID trace(TRACE, writer*);
VOID fini(INT32 code, writer*);

}

#endif //BPT_HPP
