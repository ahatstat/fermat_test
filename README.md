# fermat_test
Fixed width unsigned multiple precision bignum library tuned for fermat primality testing.  Easy to use, fast, portable, header-only c++ library with no dependencies.

## Cmake
Include the library in your project using FetchContent
```
include(FetchContent)
FetchContent_Declare(
    fermat_test
    GIT_REPOSITORY https://github.com/ahatstat/fermat_test.git
    GIT_TAG        main
)

FetchContent_MakeAvailable(fermat_test)
add_executable(ump_test ump.cpp)
target_include_directories(ump_test PRIVATE ${fermat_test_SOURCE_DIR}/fermat)
```
# Without Cmake
Copy the fermat folder and its contents to your project.  

## Example
```
#include "ump.hpp"
...
ump::uint1024_t a, b, c;
a.random();
b.assign("0x1234FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
c = a + b;
bool probale_prime = c.is_prime();
```
## Building the tests (cmake)
The test application compares results against Boost, GMP or MPIR (Windows) and OpenSSL.  These libraries are required to build and run the test application.  Using the ump bignum header only library does not require any other libraries.  

Configure, Build, Run the tests
```
cmake -S. -B build -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build
./FermatPrimeTest
```

Sample Results
```
found a 576 bit prime after 36 guesses.
0x0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000007 is probably prime.
0x000000000000000000000000000001ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff is probably prime.
0x000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000047b is not prime.
Count trailing zeros performance Ump 30[us] Boost 20[us] 1.5
Add performance Ump 20[us] Boost 30[us] 0.666667
Add UI performance Ump 20[us] Boost 30[us] 0.666667
Subtract performance Ump 20[us] Boost 30[us] 0.666667
Negate performance Ump 10[us] Boost 20[us] 0.5
Multiply UI performance Ump 30[us] Boost 30[us] 1
Multiply performance Ump 110[us] Boost 170[us] 0.647059
Multiply Full performance Ump 250[us] Boost 3190[us] 0.0783699
Divide 128 performance Ump 40[us] Boost 30[us] 1.33333
Divide performance Ump 500[us] Boost 1980[us] 0.252525
Found 30 primes out of 10000 tested. Expected about 28.1776.
8873.11 primality tests/second. (112.70us)
Benchmark (OpenSSL): 16806.72 primality tests/second. (59.50us)
All tests passed.
```




