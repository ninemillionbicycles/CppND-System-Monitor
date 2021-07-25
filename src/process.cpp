#include <cassert>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) : pid_(pid), uptime_(0), activetime_(0) {
  // Initialize process data
  vector<string> input = LinuxParser::CpuUtilization(pid_);
  if(input.size() == 5) { // Check that process data could be retrieved
    utime_ = std::stol(input[LinuxParser::ProcessData::kUtime_]);
    stime_ = std::stol(input[LinuxParser::ProcessData::kStime_]);
    cutime_ = std::stol(input[LinuxParser::ProcessData::kCutime_]);
    cstime_ = std::stol(input[LinuxParser::ProcessData::kCstime_]);
    starttime_ = std::stol(input[LinuxParser::ProcessData::kStarttime_]);
    
    uptime_ = LinuxParser::UpTime() - (starttime_ / sysconf(_SC_CLK_TCK)); // Uptime of process in seconds
    activetime_ = utime_ + stime_ + cutime_ + cstime_; // Active time of process in clock ticks
  }

  if (uptime_ > 0) {
    utilization_ = 1.0 * (activetime_ / sysconf(_SC_CLK_TCK)) / uptime_;
  }
  else {
    utilization_ = 0.0;
  }
  
  user_ = LinuxParser::User(pid_);
  command_ = LinuxParser::Command(pid_);
  ram_ = to_string(LinuxParser::Ram(pid_));
}

// Return this process's ID
int Process::Pid() { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() { return utilization_; }

// Return the command that generated this process
string Process::Command() { return command_; }

// Return this process's memory utilization in megabyte
string Process::Ram() { return ram_; }

// Return the user (name) that generated this process
string Process::User() { return user_; }

// Return the age of this process (in seconds)
long int Process::UpTime() { return uptime_; }

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
  return (utilization_ > a.utilization_);
}