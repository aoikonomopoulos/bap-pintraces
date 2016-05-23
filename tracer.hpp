#ifndef BAP_PIN_TRACER_HPP
#define BAP_PIN_TRACER_HPP
#include <vector>
namespace bap {

typedef std::vector<char> bytes_type;

template <typename addr_type, typename thread>
struct tracer {
    virtual void code_exec(const std::string&,
                           addr_type, const bytes_type&, thread) = 0;
    virtual void memory_load(addr_type, const bytes_type&) = 0;
    virtual void memory_store(addr_type, const bytes_type&) = 0;
    virtual void register_read(const std::string&,
                               const bytes_type&,
                               int bitsize = 0) = 0;
    virtual void register_write(const std::string&,
                                const bytes_type&,
                                int bitsize = 0) = 0;
    virtual ~tracer() {}
protected:
    tracer() {};
private:
    tracer(const tracer&);
    tracer& operator=(const tracer&);
};

} //namespace bap

#endif //BAP_PIN_TRACER_HPP
