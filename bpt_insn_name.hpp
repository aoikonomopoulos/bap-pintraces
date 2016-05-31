#ifndef BAP_PIN_INSN_NAME_HPP
#define BAP_PIN_INSN_NAME_HPP

#include <string>
#include <vector>
#include <algorithm>
#include <boost/algorithm/string.hpp>

namespace bpt {

namespace {

template<class input_iterator, class forward_iterator>
input_iterator find_first_not_of ( input_iterator first1,
                                   input_iterator last1,
                                   forward_iterator first2,
                                   forward_iterator last2) {
  while (first1 != last1) {
      if (last2 == std::find(first2, last2, *first1))
          return first1;
      ++first1;
  }
  return last1;
}

}

std::string insn_name(const std::string& dis) {
    static const char* prefix[] = {
        "LOCK", "REP", "REPE", "REPZ", "REPNE", "REPNZ"
    };

    std::vector<std::string> tokens;
    boost::split(tokens, dis, boost::is_any_of("\t "),
                 boost::token_compress_on);
    
    std::vector<std::string>::iterator it =
        find_first_not_of(boost::begin(tokens),
                          boost::end(tokens),
                          boost::begin(prefix),
                          boost::end(prefix));
    return it == boost::end(tokens) ? "" : *it;

}


} //namespace bpt

#endif //BAP_PIN_INSN_NAME_HPP
