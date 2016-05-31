#ifndef BPT_IARG_LIST
#define BPT_IARG_LIST

#include <boost/noncopyable.hpp>

#include "pin.H"

namespace bpt {

struct iarg_list : boost::noncopyable {
    explicit iarg_list();
    ~iarg_list();

    iarg_list& operator()(IARG_TYPE);

    template <typename T>
    iarg_list& operator()(IARG_TYPE, T);

    template <typename T1, typename T2>
    iarg_list& operator()(IARG_TYPE, T1, IARG_TYPE, T2);

    IARGLIST value();
    UINT32 size();
private:
    IARGLIST list_;
    UINT32 size_;
};


iarg_list::iarg_list() : list_(IARGLIST_Alloc()), size_(0) {}
iarg_list::~iarg_list() { IARGLIST_Free(list_); }

iarg_list& iarg_list::operator()(IARG_TYPE typ) {
    IARGLIST_AddArguments(list_, typ, IARG_END);
    ++size_;
    return *this;
}

template <typename T>
iarg_list& iarg_list::operator()(IARG_TYPE typ, T val) {
    IARGLIST_AddArguments(list_, typ, val, IARG_END);
    ++size_;
    return *this;
}

template <typename T1, typename T2>
iarg_list& iarg_list::operator()(IARG_TYPE typ1, T1 val1,
                                 IARG_TYPE typ2, T2 val2) {
    IARGLIST_AddArguments(list_, typ1, val1, typ2, val2, IARG_END);
    size_ += 2;
    return *this;

}

UINT32 iarg_list::size() { return size_; }
IARGLIST iarg_list::value() { return list_; }

}


#endif //BPT_IARG_LIST
