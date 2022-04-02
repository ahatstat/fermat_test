# fermat_test
Header only c++ bignum library tuned for fermat primality testing.  No dependencies, portable, no assembly.

## Application
Useful for verifying probable primality with a cpu following a sieve/search of a pseudo random region with a gpu. 
The application is designed to test for primality on inputs with no fewer than 16 leading zero bits.  Inputs with large number of leading zero bits will run very slowly. 

## Usage
Copy the fermat folder and its contents to your project.  
#include `fermat/ump.hpp`
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
The test application requires GMP or MPIR (Windows) and OpenSSL. 




