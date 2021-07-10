#include "processor.h"
#include "linux_parser.h"
#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;

// DONE: Return the aggregate CPU utilization
float Processor::Utilization() { 
    total_ = LinuxParser::Jiffies();
    active_ = LinuxParser::ActiveJiffies();
    utilization_ = 1.0 * (active_ - prev_active_) / (total_ - prev_total_);
    prev_total_ = total_;
    prev_active_ = active_;
    return utilization_;
}