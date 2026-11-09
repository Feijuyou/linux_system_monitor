#include "processor.h"
#include "processor.h"
#include "linux_parser.h"
#include <fstream>
#include <cstdint>
#include <exception>
#include <string>
#include <vector>
// TODO: Return the aggregate CPU utilization
float Processor::Utilization() { 
    std::vector<std::string> cpu_times = LinuxParser::CpuUtilization();
    if (cpu_times.size() < 8) {
        // Not enough data to calculate CPU utilization
        return 0.0f;
    }

    std::uint64_t total =0;
    std::uint64_t idle = 0;

    try {
        // Convert the CPU times from strings to integers
        for(std::size_t i = 0; i < 8; ++i) {
            std::uint64_t value = std::stoull(cpu_times[i]);
            total += value;
            if (i == 3 || i==4) { // The 4th value is the idle time
                idle += value;
            }
        }
    } catch (const std::exception&) {
        // Handle conversion errors
        return 0.0f;
    }

    if(!hasPrevious_) {
        // First time calculation, store the values and return 0
        previousTotal_ = total;
        previousIdle_ = idle;
        hasPrevious_ = true;
        return 0.0f;
    }

    if(total < previousTotal_ || idle < previousIdle_) {
        // Handle potential overflow or reset
        previousTotal_ = total;
        previousIdle_ = idle;
        return 0.0f;
    }

    std::uint64_t totalDiff = total - previousTotal_;
    std::uint64_t idleDiff = idle - previousIdle_;

    // 更新上一次采样数据
    previousTotal_ = total;
    previousIdle_ = idle;

    // 判断数据是否合法
    if (totalDiff == 0 || idleDiff > totalDiff) {
        return 0.0f;
    }

    // 计算 CPU 使用率
    float utilization =
        static_cast<float>(totalDiff - idleDiff) / totalDiff;

    return utilization;
 }