#ifndef UMP_TEST_HPP
#define UMP_TEST_HPP

//tests for the ump class

#include <gmp.h>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/random.hpp>
#include <boost/multiprecision/gmp.hpp>
//#include <boost/integer/mod_inverse.hpp>
#include <vector>
#include <iostream>
#include <cstdint>
#include "fermat/ump.hpp"

namespace ump {

	class Ump_test
	{
	public:
		bool conversion_test();
		bool ctz_test();
		bool add_test();
		bool add_test_ui();
		bool subtract_test();
		bool negate_test();
		bool multiply_test_ui();
		bool multiply_test();
		bool multiply_full_test();
		bool divide_128_test();
		bool divide_test();
		bool mod_inv_test();
		bool montgomery_square_test();
		bool double_and_reduce_test();
		bool powm_2_test();
		void generate_test_vectors_a(int vector_size);
		void generate_test_vectors_b(int vector_size);

		std::vector<Ump<1024>> a, b, c, d;
		std::vector<boost::multiprecision::uint1024_t> aa, bb, cc, d1;

	};

	boost::multiprecision::uint1024_t ump_to_boost_uint1024_t(Ump<1024>);
	Ump<1024> boost_uint1024_t_to_ump(boost::multiprecision::uint1024_t);
}

#endif
