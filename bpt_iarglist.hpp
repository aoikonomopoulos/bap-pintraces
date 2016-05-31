#ifndef BPT_IARG_LIST
#define BPT_IARG_LIST

#include "pin.H"

namespace bpt {

struct iarg_list {
    explicit iarg_list() : list(IARGLIST_Alloc()) {}
private:
    IARGLIST list;
}

}


#endif //BPT_IARG_LIST
