#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

class Timer {
public:
  Timer()
  : begin(Clock::now())
  {
  }

  int ElapsedMilliseconds() const
  {
    auto duration = Clock::now() - begin;
    auto seconds = std::chrono::duration_cast<Second>(duration).count();
    return static_cast<int>(seconds * 1000);
  }

private:
  using Clock = std::chrono::high_resolution_clock;
  using Second = std::chrono::duration<double, std::ratio<1>>;
  std::chrono::time_point<Clock> begin;
};

inline void RuntimeError(const std::string& message)
{
  std::cout << "runtime error: " << message << std::endl;
  exit(1);
}

inline void ValidationError(const std::string& message)
{
  std::cout << "validation error: " << message << std::endl;
  exit(2);
}

inline void StoreBenchmarkTiming(const std::string& path, int time)
{
  std::ofstream stream(path);
  if (!stream) {
    RuntimeError("failed to store benchmark timing");
  }
  stream << time;
}

template <typename T>
void AssertEquals(T actual, T expected, const std::string& message)
{
  if (actual != expected) {
    std::ostringstream stream;
    stream << message << std::endl
           << "actual value " << actual << ", expected value " << expected
           << std::endl;
    ValidationError(stream.str());
  }
}

inline void AssertEqualsHex(uint64_t actual, uint64_t expected,
                            const std::string& message)
{
  if (actual != expected) {
    std::ostringstream stream;
    stream << message << std::endl
      << std::hex
      << "actual value " << actual << ", expected value " << expected
      << std::endl;
    ValidationError(stream.str());
  }
}

inline std::string JoinPath(std::string path1, std::string path2)
{
  if (!path1.empty() && (path1.back() == '/' || path1.back() == '\\'))
    path1 = path1.substr(0, path1.length() - 1);

  if (!path2.empty() && (path2[0] == '/' || path2[0] == '\\'))
    path2 = path2.substr(1, path2.length() - 1);

  return path1 + '/' + path2;
}

inline size_t GetLastSlashPos(const std::string& path)
{
  size_t pos1 = path.rfind('/');
  size_t pos2 = path.rfind('\\');

  if (pos1 == std::string::npos && pos2 == std::string::npos)
    return std::string::npos;
  else if (pos1 == std::string::npos)
    return pos2;
  else if (pos2 == std::string::npos)
    return pos1;
  else
    return std::max(pos1, pos2);
}

inline std::string GetFileName(const std::string& path)
{
  size_t slashPos = GetLastSlashPos(path);
  return (slashPos == std::string::npos) ? path : path.substr(slashPos + 1);
}

inline std::string GetDirectoryPath(const std::string& path)
{
  size_t slashPos = GetLastSlashPos(path);
  return (slashPos == std::string::npos) ? path : path.substr(0, slashPos);
}

inline std::string StripExtension(const std::string& path)
{
  size_t dotPos = path.rfind('.');
  if (dotPos == std::string::npos)
    return path;

  size_t slashPos = GetLastSlashPos(path);
  if (slashPos == std::string::npos)
    return path.substr(0, dotPos);

  return (dotPos < slashPos) ? path : path.substr(0, dotPos);
}

inline uint64_t CombineHashes(uint64_t hash1, uint64_t hash2)
{
  return hash1 ^ (hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2));
}
