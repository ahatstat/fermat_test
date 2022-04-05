# fermat_test
Header only c++ bignum library tuned for fermat primality testing.  Easy to use, no dependencies, no boost, portable, no assembly.

## Usage
Copy the fermat folder and its contents to your project.  
`#include fermat/ump.hpp`
```
int count = 0;
const int bits = 1024;
ump::Ump<bits> n;
bool prime = false;
while (!prime)
{
	n.random();
	n.make_odd();
	prime = n.is_prime();
	count++;
}
std::cout << "found a " << bits << " bit probable prime after " << count << " guesses." << std::endl;
```
## Building the tests (cmake)
The test application compares results against Boost, GMP or MPIR (Windows) and OpenSSL.  These libraries are required to build and run the test application.  Using the library does not require any libraries.   




