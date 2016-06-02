#ifndef BPT_INSPECTION_HPP
#define BPT_INSPECTION_HPP

#include <pin.H>

namespace bpt {

class args_list;
void inspect_inst_regs(INS ins, args_list& reads, args_list& writes);
void inspect_inst_mem(INS ins, args_list& loads, args_list& stores);

} //namespace pbt

#endif //BPT_INSPECTION_HPP
