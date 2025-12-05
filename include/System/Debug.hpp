#pragma once

#include <sstream>
#include <string>
#include <iostream>

using string = std::string;


class Debug {
public:
  static void log(const string& debug, bool line = true) {
    std::cout << debug;
    if(line) std::cout << '\n';
  }

  template <typename T>
  static void log(const string& debug, const T& val, bool line = true) {
    std::ostringstream out;
    out << debug << ": " << val;
    std::cout << out.str();
    if(line) std::cout << '\n';
  }


  enum Error {
    Default,
    File
  };

  static void error(const string& debug, Error type = Default) {
    std::cout << "[Error]: ";
    switch(type) {
      case File:
        std::cout << "File missing or can't be loaded (" << debug << ")";
        break;
      default:
        std::cout << debug;
        break;
    }
    std::cout << '\n';
  }
};