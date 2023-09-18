// Copyright (c) 2023 Can Joshua Lehmann

#include "../unittest.hpp"

#define assert(expr) unittest_assert(expr)

int main() {
  unittest::Test("Addition", [](){
    assert(1 + 2 == 3);
  });
  
  unittest::Test("Failed Test", [](){
    assert(0 == 1);
  });
  
  return 0;
}
