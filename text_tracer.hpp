#ifndef BAP_PIN_TEXT_TRACER_HPP
#define BAP_PIN_TEXT_TRACER_HPP

#include <iomanip>
#include <fstream>
#include <numeric>
#include <algorithm>
#include <functional>
#include "tracer.hpp"

namespace bap {

namespace {

struct ios_flag_saver {
    explicit ios_flag_saver(std::ostream& os)
        : os_(os), f_(os.flags()) {}
    ~ios_flag_saver() { os_.flags(f_); }
private:
    std::ostream& os_;
    std::ios::fmtflags f_;
};

struct code_printer : std::binary_function<const std::string&,
                                           char,
                                           std::string> {
    code_printer(std::ostream& o, const std::string& s)
        : out(o), sep(s) {}
    std::string operator()(const std::string& init, char b) {
        ios_flag_saver s(out);
        out << init << std::hex << std::uppercase << std::setfill('0') << std::setw(2)
            << static_cast<int>(static_cast<unsigned char>(b));
        return sep;
    }
private:
    std::ostream& out;
    const std::string& sep;
};

std::ostream& print_code(std::ostream& out,
                         const std::vector<char>& bytes,
                         const std::string& sep) {
    std::accumulate(bytes.begin(), bytes.end(),
                    std::string(""), code_printer(out, sep));
    return out;
}

std::ostream& print_data(std::ostream& out,
                         const std::vector<char>& data) {

    std::vector<char>::const_reverse_iterator it =
        std::find_if(data.rbegin(), data.rend(),
                     std::bind1st(std::not_equal_to<char>(), 0));
    if (it != data.rend()) {
        out << "0x";
        std::accumulate(it, data.rend(),
                        std::string(""), code_printer(out, ""));
    } else {
        out << "0x0";
    }
    out << ":" << (data.size()*8);
    return out;
}

template <typename T>
std::ostream& print_addr(std::ostream& out, const T& addr) {
    ios_flag_saver s(out);
    out << "0x" << std::hex << std::uppercase << addr
        << std::dec << ':' << sizeof(addr)*8;
    return out;
}

}

template <typename addr_type, typename thread>
struct text_tracer : tracer<addr_type, thread> {
    typedef typename tracer<addr_type, thread>::data_type data_type;
    explicit text_tracer(const char* path)
        : out(path, std::ofstream::out) {}

    void code_exec(const std::string& dis,
                   addr_type addr,
                   const data_type& bytes,
                   thread tid) {
        out << "ins(" << tid << ", ";
        print_addr(out, addr);
        out << ") : [";
        print_code(out, bytes, ", ");
        out << "] -> " << dis << std::endl;
    }

    void memory_load(addr_type addr, const data_type& data) {
        memory_io(" => ", addr, data);
    }

    void memory_store(addr_type addr, const data_type& data) {
        memory_io(" <= ", addr, data);
    }

    void register_read(const std::string& name,
                       const data_type& data) {
        register_io( " => ", name, data);
    }

    void register_write(const std::string& name,
                        const data_type& data) {
        register_io( " <= ", name, data);
    }

    ~text_tracer() {
        out.close();
    }
private:
    void memory_io(const std::string& dir, addr_type addr,
                const data_type& data) {
        print_addr(out, addr);
        out << dir;
        print_data(out, data);
        out << std::endl;
    }

    void register_io(const std::string& dir, const std::string name,
                     const data_type& data) {
        out << name << dir;
        print_data(out, data);
        out << std::endl;
    }

private:
    std::ofstream out;
};

}

#endif //BAP_PIN_TEXT_TRACER_HPP
