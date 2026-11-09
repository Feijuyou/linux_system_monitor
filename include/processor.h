#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <cstdint>

class Processor {
 public:
  float Utilization();  // TODO: See src/processor.cpp

  // TODO: Declare any necessary private members
 private:
 std::uint64_t previousTotal_ = 0;
 std::uint64_t previousIdle_ = 0;
 bool hasPrevious_ = false;
};

#endif