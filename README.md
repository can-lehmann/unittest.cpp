# unittest.cpp

A unittesting framework for C++.

```cpp
using Test = unittest::Test;

Test("Addition").run([](){
  unittest_assert(1 + 2 == 3);
});

// Timed test
Test("Timed Addition").repeat(1000).time().run([](){
  unittest_assert(1 + 2 == 3);
});
```

## License

This project is licensed under the MIT license.
See [LICENSE.txt](LICENSE.txt) for more details.
