#ifndef FERMAT_PRIME_HPP
#define FERMAT_PRIME_HPP

#include <cstdint>
#include <vector>
#include "ump.hpp"

	class Fermat_prime
	{
	public:
		
		std::vector<bool> fermat_run();
		void set_base_int(std::string base_big_int);
		template <int BITS>
		void set_base_int(ump::Ump<BITS> a)
		{
			m_base_int = a;
		}
		void set_offsets(const std::vector<uint64_t>& offsets);

	private:
		ump::Ump<1024> m_base_int;
		std::vector<uint64_t> m_offsets;

	};

#endif
