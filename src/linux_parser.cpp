#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>
#include "linux_parser.h"
#include <fstream>
#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <pwd.h>
#include <sstream>


using std::stof;
using std::string;
using std::to_string;
using std::vector;

namespace {

bool ReadProcessTimes(int pid, long& cpuTicks, long& startTicks) {
    std::ifstream file(
        LinuxParser::kProcDirectory +
        std::to_string(pid) +
        LinuxParser::kStatFilename
    );

    std::string line;
    if (!std::getline(file, line)) return false;

    // 进程名可能包含空格和括号
    auto pos = line.rfind(')');
    if (pos == std::string::npos) return false;

    std::istringstream stream(line.substr(pos + 1));

    std::string value;
    long utime = 0;
    long stime = 0;

    try {
        // 右括号后，从第3个字段 state 开始
        for (int field = 3; field <= 22; ++field) {
            if (!(stream >> value)) return false;

            if (field == 14) utime = std::stol(value);
            if (field == 15) stime = std::stol(value);
            if (field == 22) startTicks = std::stol(value);
        }
    } catch (const std::exception&) {
        return false;
    }

    cpuTicks = utime + stime;
    return true;
}

}  // namespace

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
  string os, version, kernel;
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

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  std::ifstream filestream("/proc/meminfo");
  if(!filestream.is_open()) {
    return 0.0;
  }
  std::string line;
  std::string key;
  
  long long memTotal = 0;
  long long memavailable = 0;
  long long value = 0;
  while(std::getline(filestream, line)) {
    std::istringstream linestream(line);
    linestream >> key >> value;
    if(key == "MemTotal:") {
      memTotal = value;
    } 
    else if(key == "MemAvailable:") {
      memavailable = value;
    }
  }

  if(memTotal <= 0 || memavailable < 0 || memavailable > memTotal) {
    return 0.0f;
  }
  float utilization = static_cast<float>(memTotal - memavailable)/(memTotal);
  return utilization;
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {  std::ifstream file(kProcDirectory + kUptimeFilename);

    double seconds = 0;
    if (!(file >> seconds)) return 0;

    return static_cast<long>(seconds); }

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return 0; }

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) { long cpuTicks = 0;
    long startTicks = 0;

    if (!ReadProcessTimes(pid, cpuTicks, startTicks)) {
        return -1;
    }

    return cpuTicks;}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { return 0; }

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { return 0; }

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  
std::ifstream filestream(kProcDirectory + kStatFilename);
  std::string line;
  
  if(!std::getline(filestream, line)) {
    return {};
  }
  
  std::istringstream linestream(line); // Create a string stream from the line
  std::string cpu;

  linestream >> cpu; // Skip the "cpu" label

  if(cpu != "cpu") {
    return {};
  }
  std::vector<std::string> cpu_Times;
  std::string value;
  while(linestream >> value) {
    cpu_Times.push_back(value);
  }
  return cpu_Times;

}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() { return 0; }

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() { return 0; }

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) { 
  std::string path =
        kProcDirectory + std::to_string(pid) + kCmdlineFilename;

    std::ifstream file(path, std::ios::binary);
    std::string command;

    if (file) {
        std::ostringstream buffer;
        buffer << file.rdbuf();
        command = buffer.str();

        // cmdline 的参数使用 '\0' 分隔
        std::replace(command.begin(), command.end(), '\0', ' ');

        while (!command.empty() && command.back() == ' ') {
            command.pop_back();
        }
    }

    // 内核线程等进程可能没有 cmdline
    if (command.empty()) {
        std::ifstream comm(
            kProcDirectory + std::to_string(pid) + "/comm"
        );
        std::getline(comm, command);
    }

    return command;
   }

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) { std::ifstream file(
        kProcDirectory + std::to_string(pid) + kStatusFilename
    );

    std::string key;
    long memoryKB = 0;

    while (file >> key) {
        if (key == "VmRSS:") {
            if (file >> memoryKB) {
                std::ostringstream result;
                result << std::fixed << std::setprecision(1)
                       << memoryKB / 1024.0;
                return result.str();
            }
            break;
        }
        std::string rest;
        std::getline(file, rest);
 }
 return "0"; 
}
// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {  std::ifstream file(
        kProcDirectory + std::to_string(pid) + kStatusFilename
    );

    std::string key, uid;

    while (file >> key) {
        if (key == "Uid:") {
            if (file >> uid) return uid;
            break;
        }
        std::string rest;
        std::getline(file, rest);
    }

    return "";
}

std::string LinuxParser::User(int pid) {
    std::string uid = Uid(pid);
    if (uid.empty()) return "?";

    try {
        uid_t id = static_cast<uid_t>(std::stoul(uid));
        struct passwd* entry = getpwuid(id);
        if (entry != nullptr) return entry->pw_name;
    } catch (...) {
        return "?";
    }

    return uid; }

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function


// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) {  long cpuTicks = 0;
    long startTicks = 0;

    if (!ReadProcessTimes(pid, cpuTicks, startTicks)) {
        return 0;
    }

    long ticksPerSecond = sysconf(_SC_CLK_TCK);
    if (ticksPerSecond <= 0) return 0;

    long age = UpTime() - startTicks / ticksPerSecond;
    return std::max(0L, age);
  }


