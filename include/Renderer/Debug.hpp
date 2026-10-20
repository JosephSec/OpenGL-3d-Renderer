#pragma once

#include <Renderer/API.hpp>

#include <iostream>


namespace Renderer {
  static void LogError(const std::string &_msg, bool _handled = false) {
    std::cout << _msg << '\n';
    if(_handled == false) {
      std::cin.get();
      throw;
    }
  }
}