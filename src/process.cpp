#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid): pid_ (pid) {
    utilization_ = CpuUtilization(); // Call CpuUtilization() such that utilization_ can be used for sorting
}

// DONE: Return this process's ID
int Process::Pid() { return pid_; }

// DONE: Return this process's CPU utilization
float Process::CpuUtilization() {
    float total = LinuxParser::UpTime(pid_); // Total process running time in seconds
    float active = LinuxParser::ActiveJiffies(pid_) / LinuxParser::CLK_TCK; // Active time of process in seconds
    float utilization = active / total;
    if(total == 0) { return 0.0; }
    else { return utilization; }
}

// DONE: Return the command that generated this process
string Process::Command() { return LinuxParser::Command(pid_); }

// TODO: Return this process's memory utilization
string Process::Ram() { return string(); }

// TODO: Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(pid_); }

// DONE: Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(pid_); }

// DONE: Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
    return (utilization_ > a.utilization_);
}