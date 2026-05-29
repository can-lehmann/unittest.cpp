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

Pass `argc` and `argv` to enable command-line options:

```cpp
int main(int argc, char** argv) {
  unittest::Suite suite(argc, argv);
  // ...
  return suite.finish();
}
```

- `-k <keyword>` (only run tests whose name contains `<keyword>`)
- `-x` (stop after the first failure)
- `-s` (disable output capturing; by default, `stdout`/`stderr` from passing tests is suppressed and only shown for failing ones)
- `-h`, `--help` (show usage)

## License

This project is licensed under the MIT license.
See [LICENSE.txt](LICENSE.txt) for more details.
