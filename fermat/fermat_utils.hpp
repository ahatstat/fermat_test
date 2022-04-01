#ifndef UMP_FERMAT_UTILS_HPP
#define UMP_FERMAT_UTILS_HPP
//math functions used with fermat testing

#include <stdint.h>
#if __cplusplus >= 202002L || _HAS_CXX20
#include <bit>
#endif

namespace ump {

	uint32_t mod_inverse_32(uint32_t d);
	uint64_t mod_inverse_64(uint64_t d);
	template< class T > int count_leading_zeros(T x)
	{
#if __cplusplus >= 202002L || _HAS_CXX20
		return std::countl_zero(x);
#else
		int bits = sizeof(T) * 8;
		//hackers delight 5-13 modified for variable word size
		T y;
		int n, c;
		n = bits;
		c = bits / 2;
		do 
		{
			y = x >> c;
			if (y != 0)
			{
				n = n - c;
				x = y;
			}
			c = c >> 1;
		} while (c != 0);
		return n - x;
#endif

	}
		
}

#endif
