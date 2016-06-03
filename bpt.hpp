#ifndef BPT_HPP
#define BPT_HPP

#include <pin.H>
#include "bpt_fwd.hpp"

namespace bpt {

VOID trace(TRACE, visitor*);
VOID fini(INT32 code, visitor*);

}

#endif //BPT_HPP
