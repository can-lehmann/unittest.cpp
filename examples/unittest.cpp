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
