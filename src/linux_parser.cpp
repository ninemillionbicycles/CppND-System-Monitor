#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <iostream> // TODO Delete later

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
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

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
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
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// DONE: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string line;
  int input[2]; // memory_total, memory_free
  string key;
  long memory_total, memory_free;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    for(int i = 0; i < 2; i++) {
      std::getline(filestream, line);
      std::istringstream linestream(line);
      linestream >> key >> input[i];
    }
  }
  memory_total = input[0];
  memory_free = input[1];
  return 1.0 * (memory_total - memory_free) / memory_total;
}

// DONE: Read and return the system uptime
long LinuxParser::UpTime() {
  long uptime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  return uptime;
}

// DONE: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  return ActiveJiffies() + IdleJiffies();
}

// TODO: Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) { 
  vector<string> input = CpuUtilization(pid);
  long active = std::stol(input[ProcessData::kUtime_]) + std::stol(input[ProcessData::kStime_]) +
                    std::stol(input[ProcessData::kCutime_]) + std::stol(input[ProcessData::kCstime_]);
  return active; 
}

// DONE: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { 
  vector<string> input = CpuUtilization();
  // active = user + nice + system + irq + softirq + steal
  long active = std::stol(input[CPUStates::kUser_]) + std::stol(input[CPUStates::kNice_]) + 
                std::stol(input[CPUStates::kSystem_]) + std::stol(input[CPUStates::kIRQ_]) + 
                std::stol(input[CPUStates::kSoftIRQ_]) + std::stol(input[CPUStates::kSteal_]);
  return active; 
}

// DONE: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> input = CpuUtilization();
  // idle = idle + iowait;
  long idle = std::stol(input[CPUStates::kIdle_]) + std::stol(input[CPUStates::kIOwait_]);
  return idle; 
}

// DONE: Read and return CPU utilization
// Return vector contains the following elements: name, user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
vector<string> LinuxParser::CpuUtilization() { 
  string line, input;
  vector<string> result;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> input; // Ignore the first entry "cpu"
    while (linestream >> input) {
      result.push_back(input);
    }
  }
  return result;
}

// NEW: Read and return CPU utilization data of a process
// Return vector contains the following elements: #14: utime, #15: stime, #16: cutime, #17: cstime, #22: starttime
vector<string> LinuxParser::CpuUtilization(int pid) { 
  string line, input;
  vector<string> result;
  int counter = 1; // IDs in /proc/[pid]/stat start with 1
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> input) {
      if (counter == 14 || counter == 15 || counter == 16 || counter == 17 || counter == 22) {
        result.push_back(input);
      }
      counter++;
    }
  }
  return result;
}

// DONE: Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  return Pids().size(); 
}

// DONE: Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  string line, key, value;
  int counter = 0;
  for(auto pid : Pids()) {
    std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
    if (filestream.is_open()) {
      while (std::getline(filestream, line)) {
        std::replace(line.begin(), line.end(), ':', ' ');
        std::istringstream linestream(line);
        while (linestream >> key >> value) {
          if (key == "State") {
            if (value == "R") { counter++; }
            else { break; }
          }
        }
      }
    }
  }
  return counter; 
}

// DONE: Read and return the command associated with a process
string LinuxParser::Command(int pid) { 
  string line;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
  }
  return line; 
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) { 
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "Uid") {
          // result = value;
          break;
        }
      }
      break;
    }
  }
  return value;
}

// TODO: Read and return the user associated with a process -> NEXT STEP!
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) { 
  // Uid(pid);
  return "User"; 
}

// DONE: Read and return the uptime of a process
long LinuxParser::UpTime(int pid) { return UpTime() - StartTime(pid) / CLK_TCK; }

// NEW: Read and return the time when the process started in clock ticks
long LinuxParser::StartTime(int pid) {
  vector<string> input = CpuUtilization(pid);
  return std::stol(input[ProcessData::kStarttime_]);
}
