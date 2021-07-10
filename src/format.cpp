#include <string>
#include <sstream>
#include <iomanip>

#include "format.h"

#define HOUR 3600
#define MIN 60

using std::string;

// DONE: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) { 
    int hour = seconds / HOUR; // Integer division to get hour value from input
    int second = seconds % HOUR; // Get the remaining minutes and seconds in seconds
    int minute = second / MIN; // Integer division to get minute value from the remainder
    second = second % MIN; // // Get the remaining seconds
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << hour << ":";
    ss << std::setw(2) << std::setfill('0') << minute << ":";
    ss << std::setw(2) << std::setfill('0') << second;
    return ss.str();
}