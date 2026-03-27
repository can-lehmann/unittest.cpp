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

In order to get a summary of all tests and return a non-zero exit code if any test fails, you can use a `Suite`:

```cpp
unittest::Suite suite;

suite.test("Addition").run([](){
  assert(1 + 2 == 3);
});

return suite.finish();
```

## License

This project is licensed under the MIT license.
See [LICENSE.txt](LICENSE.txt) for more details.
