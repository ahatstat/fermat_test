#include <gmp.h>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/random.hpp>
#include <boost/multiprecision/gmp.hpp>
#include <vector>
#include <iostream>
#include <omp.h>
#include <openssl/bn.h>
#include "fermat/fermat_prime.hpp"
#include "ump_test.hpp"

//test program to compare ump fermat test performance against boost, gmp, and openssl libraries.

//boost fermat test base 2
bool primality_test_cpu_boost(boost::multiprecision::uint1024_t p)
{

	boost::multiprecision::mpz_int base = 2;
	boost::multiprecision::mpz_int result;
	boost::multiprecision::mpz_int p1 = static_cast<boost::multiprecision::mpz_int>(p);
	result = boost::multiprecision::powm(base, p1 - 1, p1);
	bool isPrime = (result == 1);
	return (isPrime);
}

//gmp fermat test base 2
bool primality_test_cpu_gmp(boost::multiprecision::uint1024_t p)
{

	boost::multiprecision::mpz_int base = 2;
	boost::multiprecision::mpz_int p1 = static_cast<boost::multiprecision::mpz_int>(p);
	mpz_t b, r, pp, pp1;
	mpz_init(b);
	mpz_init(r);
	mpz_init(pp);
	mpz_init(pp1);
	mpz_set(b, base.backend().data());
	mpz_set(pp, p1.backend().data());
	mpz_sub_ui(pp1, pp, 1);
	mpz_powm(r, b, pp1, pp);
	bool isPrime = mpz_cmp_ui(r, 1) == 0;
	mpz_clear(b);
	mpz_clear(r);
	mpz_clear(pp);
	return (isPrime);
}

//openssl fermat test base 2
bool primality_test_cpu_openssl(boost::multiprecision::uint1024_t p)
{

	BN_CTX* ctx;
	ctx = BN_CTX_new();
	if (!ctx)
	{
		std::cout << "Openssl error." << std::endl;
		return false;
	}
	BIGNUM *b, *r, *pp = nullptr, *pp1 = nullptr;
	b = BN_new();
	BN_set_word(b, 2);
	std::string p_str = p.str();
	BN_dec2bn(&pp, p_str.c_str());
	pp1 = BN_dup(pp);
	BN_sub_word(pp1, 1);
	r = BN_new();
	BN_mod_exp(r, b, pp1, pp, ctx);
	bool is_prime = BN_is_one(r);
	BN_CTX_free(ctx);
	BN_free(b);
	BN_free(r);
	BN_free(pp);
	BN_free(pp1);
	return is_prime;

}

bool fermat_performance_test()
{
	using namespace boost::multiprecision;
	using namespace boost::random;

	bool cpu_verify = true;
	bool pass = true;

	//ump::Ump<1024> pp("0x53bf18ac03f0adfb36fc4864b42013375ebdc0bb311f06636771e605ad731ca1383c7d9056522ed9bda4f608ef71498bc9c7dade6c56bf1534494e0ef371e79f09433e4c9e64624695a42d7920bd5022f449156d2f93f3be3a429159794ac9e49f69c706793ef249a284f9173a82379e62dffac42c0f53f155f65a784f31f42c");
	//set the base big int to some large value with the MSB set.
	ump::Ump<1024> pp("0");
	pp -= 0xFFFFFFFFFFFFFFFF;
	pp.clear_extra_words();
	//the base int must be odd
	pp.make_odd();
	//std::cout << pp.to_str() << std::endl;

	static constexpr int primality_test_batch_size = 1e4;
	uint64_t offset_start = 0xFFFFFFFFFFFFFE;
	//we generate a vector of even offsets to simulate prime candidates that are offset from the base big int
	std::vector<uint64_t> offsets;
	//generate an array of offsets for batch prime testing
	for (uint64_t i = offset_start; i < offset_start + primality_test_batch_size; i++)
	{
		offsets.push_back(i * 2);
	}
	
	std::vector<bool> primality_test_results;

	Fermat_prime fermat_test_ump;
	//set the base big int
	fermat_test_ump.set_base_int(pp);
	//set the array of offsets from the base int to test
	fermat_test_ump.set_offsets(offsets);
	auto start = std::chrono::steady_clock::now();
	//run the primality test and return the results
	primality_test_results = fermat_test_ump.fermat_run();
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	int primes_found = 0;
	//compare the test results with other library results
	auto benchmark_start = std::chrono::steady_clock::now();
#pragma omp parallel for
	for (auto i = 0; i < primality_test_batch_size; i++)
	{
		if (primality_test_results[i])
			primes_found++;
		if (cpu_verify)
		{
			boost::multiprecision::uint1024_t ppp(pp.to_str());
			//bool is_prime_cpu = primality_test_cpu_boost(ppp + offsets[i]);
			//bool is_prime_cpu = primality_test_cpu_gmp(ppp + offsets[i]);
			//openssl is the fastest on my hardware
			bool is_prime_cpu = primality_test_cpu_openssl(ppp + offsets[i]);
			if (is_prime_cpu != primality_test_results[i])
			{
				std::cout << "Primality test mismatch at offset " << i << ", " << offsets[i] << std::endl;
				pass = false;
			}
		}
	}
	auto benchmark_end = std::chrono::steady_clock::now();
	auto benchmark_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(benchmark_end - benchmark_start);


	double expected_primes = primality_test_batch_size * 2 / (1024 * 0.693147);
	std::stringstream ss;
	ss << "Found " << primes_found << " primes out of " << primality_test_batch_size << " tested. Expected about " << expected_primes << ". ";
	std::cout << ss.str() << std::endl;
	ss = {};
	ss << std::fixed << std::setprecision(2) << 1000.0 * primality_test_batch_size / elapsed.count() << " primality tests/second. (" << 1000.0 * elapsed.count() / primality_test_batch_size << "us)";
	std::cout << ss.str() << std::endl;
	if (cpu_verify)
	{
		ss = {};
		ss << "Benchmark: " << std::fixed << std::setprecision(2) << 1000.0 * primality_test_batch_size / benchmark_elapsed.count() << " primality tests/second. (" << 1000.0 * benchmark_elapsed.count() / primality_test_batch_size << "us)";
		std::cout << ss.str() << std::endl;
	}
	return pass;
}

	

int main()
{
	ump::Ump_test test;
	bool pass = true;
	if (!test.conversion_test())
	{
		std::cout << "Conversion test failed." << std::endl;
		pass = false;
	}
	if (!test.add_test())
	{
		std::cout << "Add test failed." << std::endl;
		pass = false;
	}
	if (!test.add_test_ui())
	{
		std::cout << "Add test ui failed." << std::endl;
		pass = false;
	}
	if (!test.subtract_test())
	{
		std::cout << "Subtract test failed." << std::endl;
		pass = false;
	}
	if (!test.multiply_test_ui())
	{
		std::cout << "Multiply test ui failed." << std::endl;
		pass = false;
	}
	if (!test.mod_inv_test())
	{
		std::cout << "Mod inv test failed." << std::endl;
		pass = false;
	}
	if (!test.montgomery_square_test())
	{
		std::cout << "Montgomery square test failed." << std::endl;
		pass = false;
	}
	if (!test.double_and_reduce_test())
	{
		std::cout << "Double and reduce test failed." << std::endl;
		pass = false;
	}
	if (!fermat_performance_test())
	{
		std::cout << "Fermat test failed." << std::endl;
		pass = false;
	}
	if (pass)
	{
		std::cout << "All tests passed." << std::endl;

	}
	return 0;
}
