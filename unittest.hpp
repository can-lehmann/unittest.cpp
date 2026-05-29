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
#include <cstdlib>

namespace unittest {
  class AssertionError {
  private:
    const char* _expression;
    size_t _line;
    const char* _file;
    std::string _message;
  public:
    AssertionError(const char* expression,
                   size_t line,
                   const char* file,
                   const std::string& message = ""):
      _expression(expression), _line(line), _file(file), _message(message) {}
    
    const char* expression() const { return _expression; }
    size_t line() const { return _line; }
    const char* file() const { return _file; }
    const std::string& message() const { return _message; }
  };
  
  inline void _assert(bool condition,
                      const char* expression,
                      size_t line,
                      const char* file) {
    if (!condition) {
      throw AssertionError(expression, line, file);
    }
  }

  class Suite;
  class Test;

  class Suite {
  private:
    size_t _num_errors = 0;
    size_t _num_successes = 0;
    size_t _num_skipped = 0;
    std::string _filter;
    bool _exit_on_failure = false;
    bool _aborted = false;

  public:
    Suite() {}

    Suite(int argc, char** argv) {
      for (int i = 1; i < argc; i++) {
        std::string arg(argv[i]);
        if (arg == "--help" || arg == "-h") {
          std::cout << "Usage: " << argv[0] << " [options]\n"
                    << "Options:\n"
                    << "  -k <keyword>  only run tests whose name contains <keyword>\n"
                    << "  -x            stop after first failure\n"
                    << "  -h, --help    show this help message\n";
          std::exit(0);
        } else if (arg == "-x") {
          _exit_on_failure = true;
        } else if (arg == "-k") {
          if (i + 1 >= argc) {
            std::cerr << "unittest: -k requires an argument\n";
            std::exit(1);
          }
          _filter = argv[++i];
        } else {
          std::cerr << "unittest: unrecognized argument: " << arg << "\n";
          std::cerr << "Run with --help for usage.\n";
          std::exit(1);
        }
      }
    }

    const std::string& filter() const { return _filter; }
    bool should_abort() const { return _aborted; }

    void report_outcome(const std::string& name, bool has_errors) {
      if (has_errors) {
        _num_errors++;
        if (_exit_on_failure) {
          _aborted = true;
        }
      } else {
        _num_successes++;
      }
    }

    void report_skipped() {
      _num_skipped++;
    }

    int finish() {
      if (_num_errors) {
        std::cout << "\e[1;31m" << _num_errors << " failed,\e[0m ";
      }
      std::cout << "\e[32m" << _num_successes << " passed\e[0m";
      if (_num_skipped) {
        std::cout << ", \e[33m" << _num_skipped << " skipped\e[0m";
      }
      std::cout << "\n";
      if (_num_errors) {
        return -1;
      }
      return 0;
    }

    inline Test test(const std::string& name);
    inline Test test(const char* name);
  };

  template <class Self>
  class BaseTest {
  private:
    std::string _name;
    Suite* _suite = nullptr;
    bool _is_timed = false;
    size_t _repeat = 1;
  public:
    BaseTest(const std::string& name): _name(name) {}
    BaseTest(const char* name): _name(name) {}

    BaseTest(const std::string& name, Suite& suite): _name(name), _suite(&suite) {}
    BaseTest(const char* name, Suite& suite): _name(name), _suite(&suite) {}
    
    const std::string& name() const { return _name; }
    bool is_timed() const { return _is_timed; }
    size_t repeat() const { return _repeat; }

    Self time(bool is_timed = true) && {
      _is_timed = is_timed;
      return std::move(*((Self*) this));
    }
    
    Self repeat(size_t repeat) && {
      _repeat = repeat;
      return std::move(*((Self*) this));
    }

    Self suite(Suite& suite) && {
      _suite = &suite;
      return std::move(*((Self*) this));
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
    bool run(const std::function<void()>& body) {
      if (_suite) {
        if (_suite->should_abort()) {
          _suite->report_skipped();
          return false;
        }
        const std::string& filter = _suite->filter();
        if (!filter.empty() && _name.find(filter) == std::string::npos) {
          _suite->report_skipped();
          return false;
        }
      }

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
            if (!error.message().empty()) {
              std::cout << error.message() << std::endl;
            }
            std::cout << error.file() << " (" << error.line() << ")" << std::endl;
            std::cout << std::endl;
          }
        }
      }

      if (_suite) {
        _suite->report_outcome(_name, has_errors);
      }

      return has_errors;
    }
  };

  class Test: public BaseTest<Test> {
  public:
    using BaseTest::BaseTest;
  };

  inline Test Suite::test(const std::string& name) {
    return Test(name, *this);
  }

  inline Test Suite::test(const char* name) {
    return Test(name, *this);
  }
};

#define unittest_assert(expr) unittest::_assert(expr, #expr, __LINE__, __FILE__);

#endif
