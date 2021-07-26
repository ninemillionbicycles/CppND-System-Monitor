#include <dirent.h>
#include <unistd.h>
#include <cassert>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// Read and return operating system from filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// Read and return kernel from filesystem
string LinuxParser::Kernel() {
  string os;
  string kernel;
  string version;
  string line;
  std::ifstream filestream(kProcDirectory + kVersionFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// Read and return process ids from filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.emplace_back(pid);
      }
    }
  }
  closedir(directory);
  assert(pids.size() > 0);
  return pids;
}

// Read and return the system level memory utilization
float LinuxParser::MemoryUtilization() {
  string line;
  string key;
  int input[2];  // memory_total, memory_free
  long memory_total;
  long memory_free;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    for (int i = 0; i < 2; i++) {
      std::getline(filestream, line);
      std::istringstream linestream(line);
      linestream >> key >> input[i];
    }
  }
  memory_total = input[0];
  memory_free = input[1];
  return 1.0 * (memory_total - memory_free) / memory_total;
}

// Read and return the system uptime in seconds
long LinuxParser::UpTime() {
  long uptime;
  string line;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  return uptime;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return ActiveJiffies() + IdleJiffies(); }

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  vector<string> input = CpuUtilization();
  // active = user + nice + system + irq + softirq + steal
  long active = std::stol(input[CPUStates::kUser_]) +
                std::stol(input[CPUStates::kNice_]) +
                std::stol(input[CPUStates::kSystem_]) +
                std::stol(input[CPUStates::kIRQ_]) +
                std::stol(input[CPUStates::kSoftIRQ_]) +
                std::stol(input[CPUStates::kSteal_]);
  return active;
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> input = CpuUtilization();
  long idle = std::stol(input[CPUStates::kIdle_]) +
              std::stol(input[CPUStates::kIOwait_]);
  return idle;
}

// Read and return the system's CPU utilization
// Return vector contains the following elements: name, user, nice, system,
// idle, iowait, irq, softirq, steal, guest, guest_nice;
vector<string> LinuxParser::CpuUtilization() {
  string line;
  string input;
  vector<string> result;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> input;  // Ignore the first entry "cpu"
    while (linestream >> input) {
      result.emplace_back(input);
    }
  }
  assert(result.size() == 10);
  return result;
}

// Read and return CPU utilization data of a process
// Return vector contains the following elements: utime, stime, cutime, cstime,
// starttime
vector<string> LinuxParser::CpuUtilization(int pid) {
  string line;
  string input;
  vector<string> result;
  int counter = 0;  // IDs in /proc/[pid]/stat start with 1
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    while (linestream >> input) {
      counter++;
      // #14: utime, #15: stime, #16: cutime, #17: cstime, #22: starttime
      if (counter == 14 || counter == 15 || counter == 16 || counter == 17 ||
          counter == 22) {
        result.emplace_back(input);
      }
    }
  }
  return result;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() { return Pids().size(); }

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line;
  string key;
  string value;
  int counter = 0;
  for (auto pid : Pids()) {
    std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
    if (filestream.is_open()) {
      while (std::getline(filestream, line)) {
        std::replace(line.begin(), line.end(), ':', ' ');
        std::istringstream linestream(line);
        while (linestream >> key >> value) {
          if (key == "State") {
            if (value == "R") {
              counter++;
            } else {
              break;
            }
          }
        }
      }
    }
  }
  return counter;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string line;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
  }

  return (line.substr(0, 40) + "...");
}

// Read and return the memory used by a process in megabyte
int LinuxParser::Ram(int pid) {
  string line;
  string key;
  string value;
  int result;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), '\t', ' ');
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "VmRSS") {  // VmSize gives the virtual memory size, VmRSS
                             // gives the physical memory size
        result = std::stoi(value) / 1024;
      }
    }
  }
  return result;
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), '\t', ' ');
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "Uid") {
        break;
      }
    }
  }
  return value;
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string uid = Uid(pid);
  string line;
  string key;
  string value;
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), 'x', ' ');
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (value == uid) {
        break;
      }
    }
  }
  return key.substr(0, 5);
}