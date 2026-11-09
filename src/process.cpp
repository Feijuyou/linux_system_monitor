#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include "linux_parser.h"
#include <chrono>
#include "process.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) : pid_(pid) {}

// 获取 PID
int Process::Pid() {
    return pid_;
}

// 获取用户名
std::string Process::User() {
    return LinuxParser::User(pid_);
}

// 获取启动命令
std::string Process::Command() {
    return LinuxParser::Command(pid_);
}

// 获取内存占用
std::string Process::Ram() {
    return LinuxParser::Ram(pid_);
}

// 获取进程运行时间
long int Process::UpTime() {
    return LinuxParser::UpTime(pid_);
}

// 返回上一次计算的 CPU 使用率
float Process::CpuUtilization() {
    return cpuUtilization_;
}

// 更新 CPU 使用率
void Process::RefreshCpu() {
    long ticks = LinuxParser::ActiveJiffies(pid_);//它会去读取 /proc/1234/stat，获取这个进程累计消耗的 CPU 时间。
    if (ticks < 0) {
        cpuUtilization_ = 0.0f;
        previousTicks_ = -1;
        return;
    }

    auto now = std::chrono::steady_clock::now();

    if (previousTicks_ >= 0 && ticks >= previousTicks_) {

        double elapsed =
            std::chrono::duration<double>(
                now - previousTime_
            ).count();   //这次采样cpu的时间

        long hz = sysconf(_SC_CLK_TCK);

        if (elapsed > 0 && hz > 0) {

            double usedSeconds =
                static_cast<double>(ticks - previousTicks_) / hz;//进程消耗的cpu时间，单位是秒

            cpuUtilization_ =
                static_cast<float>(usedSeconds / elapsed); //cpu使用率
        }
    } else {
        cpuUtilization_ = 0.0f;
    }

    previousTicks_ = ticks;
    previousTime_ = now;
}

// CPU 使用率低的进程排在前面（供比较使用）
bool Process::operator<(Process const& other) const {
    return cpuUtilization_ < other.cpuUtilization_;
}