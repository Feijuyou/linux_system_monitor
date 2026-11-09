#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "process.h"
#include "processor.h"
#include "system.h"
#include "linux_parser.h"
#include <algorithm>
#include <unordered_map>
#include <utility>
using std::set;
using std::size_t;
using std::string;
using std::vector;

// TODO: Return the system's CPU
Processor& System::Cpu() { return cpu_; }

// TODO: Return a container composed of the system's processes
vector<Process>& System::Processes() {  // 1. 保存上一轮的进程对象
    std::unordered_map<int, Process> previous; //创建一个临时容器

    for (auto& process : processes_) {
        int pid = process.Pid();
        previous.emplace(pid, std::move(process));
    }

    processes_.clear();

    // 2. 获取最新 PID 列表
    std::vector<int> pids = LinuxParser::Pids();

    // 3. 构建最新的进程列表
    for (int pid : pids) {

        auto it = previous.find(pid);

        if (it != previous.end()) {
            // 老进程：保留历史 CPU 采样数据
            processes_.push_back(std::move(it->second));
        } else {
            // 新进程：创建新的 Process 对象
            processes_.emplace_back(pid);
        }

        // 更新 CPU 使用率
        processes_.back().RefreshCpu();
    }

    // 4. CPU 占用率从高到低排序
    std::sort(processes_.begin(), processes_.end(),
        [](const Process& a, const Process& b) {
            return b < a;
        });

    // 5. 返回整个进程列表的引用
    return processes_; }

// TODO: Return the system's kernel identifier (string)
std::string System::Kernel() { return string(); }

// TODO: Return the system's memory utilization
float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }

// TODO: Return the operating system name
std::string System::OperatingSystem() { return string(); }

// TODO: Return the number of processes actively running on the system
int System::RunningProcesses() { return 0; }

// TODO: Return the total number of processes on the system
int System::TotalProcesses() { return 0; }

// TODO: Return the number of seconds since the system started running
long int System::UpTime() { return 0; }