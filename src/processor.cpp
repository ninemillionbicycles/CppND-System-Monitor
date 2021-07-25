#include "processor.h"
#include <iostream>
#include <string>
#include <vector>
#include "linux_parser.h"

using std::string;
using std::vector;

// Return the aggregate CPU utilization
float Processor::Utilization() {
  total_ = LinuxParser::Jiffies();
  active_ = LinuxParser::ActiveJiffies();
  utilization_ = 1.0 * (active_ - prev_active_) / (total_ - prev_total_);
  prev_total_ = total_;
  prev_active_ = active_;
  return utilization_;
}