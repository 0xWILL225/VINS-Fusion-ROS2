/*******************************************************
 * thin logger for VINS-Fusion runner integrations
 *******************************************************/

#pragma once

#include <atomic>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace vins_thin_logger {

enum class Level {
  kError = 0,
  kWarn = 1,
  kInfo = 2,
  kDebug = 3,
};

inline std::atomic<int>& verbosityStorage() {
  static std::atomic<int> verbosity{2};
  return verbosity;
}

inline int normalizeVerbosity(const int verbose) {
  if (verbose <= 0) {
    return 0;
  }
  if (verbose >= 3) {
    return 3;
  }
  return verbose;
}

inline void setVerbosity(const int verbose) {
  verbosityStorage().store(normalizeVerbosity(verbose));
}

inline bool shouldLog(const Level level) {
  return static_cast<int>(level) <= verbosityStorage().load();
}

inline const char* levelName(const Level level) {
  switch (level) {
    case Level::kError:
      return "ERROR";
    case Level::kWarn:
      return "WARN";
    case Level::kInfo:
      return "INFO";
    case Level::kDebug:
      return "DEBUG";
  }
  return "INFO";
}

inline std::ostream& streamFor(const Level level) {
  if (level == Level::kError || level == Level::kWarn) {
    return std::cerr;
  }
  return std::cout;
}

inline void write(const Level level, const std::string& message) {
  std::ostream& stream = streamFor(level);
  stream << "[VINS-Fusion][" << levelName(level) << "] " << message;
  if (message.empty() || message.back() != '\n') {
    stream << '\n';
  }
}

template <typename... Args>
inline void logf(const Level level, const char* format, Args... args) {
  if (!shouldLog(level)) {
    return;
  }

  const int message_size = std::snprintf(nullptr, 0, format, args...);
  if (message_size <= 0) {
    return;
  }

  std::vector<char> buffer(static_cast<size_t>(message_size) + 1U);
  std::snprintf(buffer.data(), buffer.size(), format, args...);
  write(level, std::string(buffer.data(), static_cast<size_t>(message_size)));
}

}  // namespace vins_thin_logger

#define VINS_LOG_STREAM(level, message)                                           \
  do {                                                                            \
    if (::vins_thin_logger::shouldLog(level)) {                                   \
      std::ostringstream vins_log_stream;                                         \
      vins_log_stream << message;                                                 \
      ::vins_thin_logger::write(level, vins_log_stream.str());                    \
    }                                                                             \
  } while (false)

#define VINS_LOG_ERROR(...) \
  ::vins_thin_logger::logf(::vins_thin_logger::Level::kError, __VA_ARGS__)
#define VINS_LOG_WARN(...) \
  ::vins_thin_logger::logf(::vins_thin_logger::Level::kWarn, __VA_ARGS__)
#define VINS_LOG_INFO(...) \
  ::vins_thin_logger::logf(::vins_thin_logger::Level::kInfo, __VA_ARGS__)
#define VINS_LOG_DEBUG(...) \
  ::vins_thin_logger::logf(::vins_thin_logger::Level::kDebug, __VA_ARGS__)

#define VINS_LOG_ERROR_STREAM(message) \
  VINS_LOG_STREAM(::vins_thin_logger::Level::kError, message)
#define VINS_LOG_WARN_STREAM(message) \
  VINS_LOG_STREAM(::vins_thin_logger::Level::kWarn, message)
#define VINS_LOG_INFO_STREAM(message) \
  VINS_LOG_STREAM(::vins_thin_logger::Level::kInfo, message)
#define VINS_LOG_DEBUG_STREAM(message) \
  VINS_LOG_STREAM(::vins_thin_logger::Level::kDebug, message)
