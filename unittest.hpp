// MIT License
// 
// Copyright (c) 2023 Can Joshua Lehmann
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef UNITTEST_HPP
#define UNITTEST_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <functional>
#include <stdexcept>
#include <optional>
#include <chrono>
#include <cmath>

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
  
  class Test {
  private:
    std::string _name;
    bool _is_timed = false;
    size_t _repeat = 1;
  public:
    Test(const char* name): _name(name) {}
    
    Test time(bool is_timed = true) && {
      _is_timed = is_timed;
      return std::move(*this);
    }
    
    Test repeat(size_t repeat) && {
      _repeat = repeat;
      return std::move(*this);
    }
  
  private:
    using Duration = std::chrono::high_resolution_clock::duration;
  
    struct Report {
      std::optional<AssertionError> error;
      Duration duration;
      
      Report() {}
      Report(Duration _duration): duration(_duration) {}
      Report(const AssertionError& _error): error(_error) {}
      
      bool is_error() const { return error.has_value(); }
      bool is_success() const { return !is_error(); }
    };
    
    template <class T>
    void write_duration(const T& duration,
                        std::ostream& stream) {
      #define in(unit) std::chrono::duration_cast<std::chrono::unit>(duration).count()
      
      if (in(seconds) > 0) {
        stream << in(seconds) << "s " << (in(milliseconds) % 1000) << "ms";
      } else if (in(milliseconds) > 0) {
        stream << in(milliseconds) << "ms";
      } else {
        stream << in(nanoseconds) << "ns";
      }
      
      #undef in
    }
    
    void write_duration_stats(const std::vector<Report>& reports, std::ostream& stream) {
      if (reports.size() == 1) {
        write_duration(reports[0].duration, stream);
      } else {
        bool is_first = true;
        size_t count = 0;
        Duration min, max, mean;
        for (const Report& report : reports) {
          if (report.is_success()) {
            if (is_first) {
              min = report.duration;
              max = report.duration;
              mean = report.duration;
              is_first = false;
            } else {
              if (report.duration < min) { min = report.duration; }
              if (report.duration > max) { max = report.duration; }
              mean += report.duration;
            }
            count++;
          }
        }
        
        mean /= count;
        
        double stddev = 0;
        for (const Report& report : reports) {
          if (report.is_success()) {
            double delta = std::chrono::duration<double>(report.duration - mean).count();
            stddev += delta * delta;
          }
        }
        stddev /= count - 1;
        stddev = std::sqrt(stddev);
        
        stream << "mean ";
        write_duration(mean, stream);
        stream << ", stddev ";
        write_duration(std::chrono::duration<double>(stddev), stream);
        stream << ", min ";
        write_duration(min, stream);
        stream << ", max ";
        write_duration(max, stream);
      }
    }
    
  public:
    void run(const std::function<void()>& body) && {
      size_t success_count = 0;
      std::vector<Report> reports;
      reports.reserve(_repeat);
      for (size_t iter = 0; iter < _repeat; iter++) {
        try {
          if (_is_timed) {
            std::chrono::high_resolution_clock clock;
            auto start = clock.now();
            body();
            auto stop = clock.now();
            reports.emplace_back(stop - start);
          } else {
            body();
            reports.emplace_back();
          }
          success_count++;
        } catch (const AssertionError& err) {
          reports.emplace_back(err);
        }
      }
      
      bool has_errors = success_count < reports.size();
      if (has_errors) {
        std::cout << "\e[1;31m[x]\e[0m ";
      } else {
        std::cout << "\e[32m[✓]\e[0m ";
      }
      std::cout << _name;
      if (_is_timed && success_count > 0) {
        std::cout << " (";
        write_duration_stats(reports, std::cout);
        std::cout << ")";
      }
      std::cout << std::endl;
      
      if (has_errors) {
        for (const Report& report : reports) {
          if (report.is_error()) {
            const AssertionError& error = report.error.value();
            std::cout << std::endl;
            std::cout << "Assertion failed: " << error.expression() << std::endl;
            std::cout << error.file() << " (" << error.line() << ")" << std::endl;
            std::cout << std::endl;
          }
        }
      }
    }
  };
};

#define unittest_assert(expr) unittest::assert(expr, #expr, __LINE__, __FILE__);

#endif
