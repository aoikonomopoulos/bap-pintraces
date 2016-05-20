#ifndef BAP_PIN_TRACER_HPP
#define BAP_PIN_TRACER_HPP
#include <vector>
namespace bap {

template <typename addr_type, typename thread>
struct tracer {
    typedef std::vector<char> data_type;
    virtual void code_exec(const std::string&, addr_type, const data_type&, thread) = 0;
    virtual void memory_load(addr_type, const data_type&) = 0;
    virtual void memory_store(addr_type, const data_type&) = 0;
    virtual void register_read(const std::string&, const data_type&) = 0;
    virtual void register_write(const std::string&, const data_type&) = 0;
    virtual ~tracer() {}
protected:
    tracer() {};
private:
    tracer(const tracer&);
    tracer& operator=(const tracer&);
};

} //namespace bap

#endif //BAP_PIN_TRACER_HPP
