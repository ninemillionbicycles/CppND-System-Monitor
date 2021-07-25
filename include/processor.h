#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  float Utilization();

 private:
  long active_;
  long total_;
  long prev_active_;
  long prev_total_;
  float utilization_;
};

#endif