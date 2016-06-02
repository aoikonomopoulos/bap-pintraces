#ifndef BPT_FWD_HPP
#define BPT_FWD_HPP

#include <vector>

#include <boost/shared_ptr.hpp>

namespace bpt {

class event;
class operation_event;
class read_event;
class write_event;
class read_flags_event;
class write_flags_event;
class load_event;
class store_event;

class writer;

typedef boost::shared_ptr<event> event_ptr;
typedef std::vector<char> bytes_type;

}// namespace bpt;

#endif //BPT_FWD_HPP
