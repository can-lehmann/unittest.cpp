// Copyright (c) 2023 Can Joshua Lehmann

#include <chrono>
#include <thread>

#include "../unittest.hpp"

using Test = unittest::Test;
#define assert(expr) unittest_assert(expr)

int main() {
  Test("Addition").run([](){
    assert(1 + 2 == 3);
  });
  
  Test("Failed Test").run([](){
    assert(0 == 1);
  });
  
  Test("Timed Test").time().run([](){
    std::this_thread::sleep_for(std::chrono::milliseconds(123));
  });
  
  Test("Repeated Test").repeat(10).run([](){
    assert(true);
  });
  
  Test("Repeated Timed Test").repeat(10).time().run([](){
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  });
  
  return 0;
}
