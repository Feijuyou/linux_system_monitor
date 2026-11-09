#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <chrono>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
public:
    explicit Process(int pid);

    int Pid();
    std::string User();
    std::string Command();
    float CpuUtilization();
    std::string Ram();
    long int UpTime();

    // 刷新本进程的 CPU 使用率
    void RefreshCpu();

    // 用于排序
    bool operator<(Process const& other) const;

private:
    int pid_;

    float cpuUtilization_ = 0.0f;

    long previousTicks_ = -1;

    std::chrono::steady_clock::time_point previousTime_;
};


#endif