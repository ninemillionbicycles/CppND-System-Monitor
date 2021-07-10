#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
    public:
    float Utilization();  // DONE: See src/processor.cpp
    
    // DONE: Declare any necessary private members
    private:
    long active_;
    long total_;
    long prev_active_;
    long prev_total_;
    float utilization_;
};

#endif