#include <ctime>
#include <sstream>

struct date_printer {
    std::time_t datetime_;

    date_printer(std::time_t t) : datetime_(t) {}
};

std::ostream &operator<<(std::ostream &os, const date_printer &prn) {
    auto printed_date = gmtime(&prn.datetime_);
    char buf[40];
    strftime(buf, 40, "%c", printed_date);
    os << buf;
    return os;
}