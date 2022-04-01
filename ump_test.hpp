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
		bool add_test();
		bool add_test_ui();
		bool subtract_test();
		bool multiply_test_ui();
		bool mod_inv_test();
		bool montgomery_square_test();
		bool double_and_reduce_test();
		void generate_test_vectors_a(int vector_size);
		void generate_test_vectors_b(int vector_size);

		std::vector<Ump<1024>> a, b, c;
		std::vector<boost::multiprecision::uint1024_t> aa, bb, cc;

	};

	boost::multiprecision::uint1024_t ump_to_boost_uint1024_t(Ump<1024>);
	Ump<1024> boost_uint1024_t_to_ump(boost::multiprecision::uint1024_t);
}

#endif
