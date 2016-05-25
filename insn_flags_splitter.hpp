#ifndef BAP_PIN_TRACER_INSNS_FLAGS_SPLITTER_HPP
#define BAP_PIN_TRACER_INSNS_FLAGS_SPLITTER_HPP

#include <algorithm>
#include <iterator>
#include <boost/range.hpp>
#include <boost/bind.hpp>
#include "flags_splitter.hpp"
namespace bap {

typedef boost::unordered_map<std::string, insns> map_type;

map_type make_map() {
    map_type map(FLAGS_MAX);
    for(int i = 0; i < INSN_MAX; ++i) {
        map[insns_names[i]] = static_cast<insns>(i);
    }
    return map;
}

const map_type m_map(make_map());

struct insn_flags_splitter : flags_splitter {
    flags_type read(const std::string& dis,
                    const std::string& name,
                    const bytes_type& data) {
        std::vector<flags> items;
        std::string insn = insn_name(dis);
        items.reserve(FLAGS_MAX);
        if (m_map.count(insn)) {
            insns i = m_map.at(insn);
            for(int j = 0; j < FLAGS_MAX; ++j) {
                switch(affects[i][j]) {
                case TST:
                case T_M:
                case T_P: items.push_back(static_cast<flags>(j)); break;
                default: break;
                }
            }
        }
        return make(name, data, items);
    }

    flags_type write(const std::string& dis,
                     const std::string& name,
                     const bytes_type& data) {
        std::vector<flags> items;
        std::string insn = insn_name(dis);
        items.reserve(FLAGS_MAX);
        if (m_map.count(insn)) {
            insns i = m_map.at(insn);
            for(int j = 0; j < FLAGS_MAX; ++j) {
                switch(affects[i][j]) {
                case CLR:
                case SET:
                case AH:
                case MOD:
                case POP:
                case UND:
                case M_U: items.push_back(static_cast<flags>(j)); break;
                default: break;
                }
            }
        }
        return make(name, data, items);
    }

private:
    flags_type make(const std::string& name,
                    const bytes_type& data,
                    const std::vector<flags> items) {
        flags_type flags;
        flags.reserve(std::distance(boost::begin(items),
                                    boost::end(items)) + 1);
        flag f = {name, bytes_type(data), data.size()*8};
        flags.push_back(f);
        
        std::transform(boost::begin(items),
                       boost::end(items),
                       std::back_inserter(flags),
                       boost::bind(&get_flag_2, _1, data));
        return flags;
    }

};

} //namespace bap

#endif //BAP_PIN_TRACER_INSNS_FLAGS_SPLITTER_HPP
