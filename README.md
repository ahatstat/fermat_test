# fermat_test
Fixed width unsigned multiple precision bignum library tuned for fermat primality testing.  Easy to use, fast, portable, header-only c++ library with no dependencies.

## Usage
Copy the fermat folder and its contents to your project.  
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




