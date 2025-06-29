#include "utils.h"

std::vector<std::string> split_string(const std::string &input) {
    std::vector<std::string> result;
    std::istringstream stream(input);
    std::string token;
  
    while (stream >> token) {
      result.push_back(token);
    }
    return result;
  }