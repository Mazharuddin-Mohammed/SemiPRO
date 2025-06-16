// Author: Dr. Mazharuddin Mohammed
#include "utils.hpp"
#include <yaml-cpp/yaml.h>

Logger& Logger::getInstance() {
  static Logger instance;
  return instance;
}

void Logger::initialize(const std::string& config_file) {
  YAML::Node config = YAML::LoadFile(config_file);
  std::string log_path = config["logging"]["path"].as<std::string>();
  log_file_.open(log_path, std::ios::app);
}

void Logger::log(const std::string& message) {
  if (log_file_.is_open()) {
    log_file_ << message << "\n";
    log_file_.flush();
  }
}