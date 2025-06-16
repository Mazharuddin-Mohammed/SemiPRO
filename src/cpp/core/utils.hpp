#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <fstream>

class Logger {
public:
  static Logger& getInstance();
  void initialize(const std::string& config_file);
  void log(const std::string& message);

private:
  Logger() = default;
  std::ofstream log_file_;
};

#endif // UTILS_HPP