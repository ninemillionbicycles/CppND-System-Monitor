#ifndef PROCESS_H
#define PROCESS_H

#include <string>

#include <unistd.h>

/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  Process(int pid);
  int Pid();
  std::string User();
  std::string Command();
  float CpuUtilization();
  std::string Ram();
  long int UpTime();
  bool operator<(Process const& a) const;

 private:
  int pid_;
  long utime_, stime_, cutime_, cstime_, starttime_, uptime_, activetime_;
  std::string user_, command_, ram_;
  float utilization_;
};

#endif