// Copyright (c) 2023 Can Joshua Lehmann

#ifndef UNITTEST_HPP
#define UNITTEST_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <optional>
#include <functional>
#include <stdexcept>

namespace unittest {
  class AssertionError {
  private:
    const char* _expression;
    size_t _line;
    const char* _file;
  public:
    AssertionError(const char* expression, size_t line, const char* file):
      _expression(expression), _line(line), _file(file) {}
    
    const char* expression() const { return _expression; }
    size_t line() const { return _line; }
    const char* file() const { return _file; }
  };
  
  inline void assert(bool condition,
                     const char* expression,
                     size_t line,
                     const char* file) {
    if (!condition) {
      throw AssertionError(expression, line, file);
    }
  }
  
  struct Test {
    Test(const char* name, const std::function<void()>& body) {
      std::optional<AssertionError> error;
      try {
        body();
      } catch (const AssertionError& err) {
        error = err;
      }
      
      if (error.has_value()) {
        std::cout << "\e[1;31m[x]\e[0m ";
      } else {
        std::cout << "\e[32m[✓]\e[0m ";
      }
      std::cout << name << std::endl;
      
      if (error.has_value()) {
        std::cout << std::endl;
        std::cout << "Assertion failed: " << error.value().expression() << std::endl;
        std::cout << error.value().file() << " (" << error.value().line() << ")" << std::endl;
        std::cout << std::endl;
      }
    }
  };
};

#define unittest_assert(expr) unittest::assert(expr, #expr, __LINE__, __FILE__);

#endif
