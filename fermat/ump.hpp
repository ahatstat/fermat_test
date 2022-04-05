#ifndef UMP_HPP
#define UMP_HPP

//unsigned multiprecision integer class
//The size of the integer in bits is selectable via template

#include <cstdint>
#include <string>
#include <array>
#include <iostream>
#include "fermat_utils.hpp"

namespace ump {

		using limb_t = uint64_t;
		static constexpr int BYTES_PER_WORD = sizeof(limb_t);
		static constexpr int BITS_PER_WORD = BYTES_PER_WORD * 8;

		template<int BITS> class Ump
		{
			static_assert(BITS > 0, "The big int must have at least one bit.");
		public:
			
			//LIMBS is the number of machine words used to store the big int
			//we allocate one extra word to handle overflow and normalization in division algorithm
			static constexpr int EXTRA_WORDS = 1;
			static constexpr int HIGH_WORD = (BITS + BITS_PER_WORD - 1) / BITS_PER_WORD - 1; //round up 
			static constexpr int LIMBS = HIGH_WORD + 1 + EXTRA_WORDS;  //extra word(s) for overflow
			//the least significant word is stored in array element 0
			std::array<limb_t, LIMBS> m_limbs;
			
			Ump(uint32_t);
			Ump(uint64_t);
			Ump(int);
			Ump();
			Ump(std::string);
			Ump add(const Ump&) const;
			Ump add(int) const;
			Ump add(uint32_t) const;
			Ump add(uint64_t) const;
			Ump sub(const Ump&) const;
			Ump sub(int) const;
			Ump sub(uint32_t) const;
			Ump sub(uint64_t) const;
			void increment(const Ump&);
			void increment(int);
			void increment(uint32_t);
			void increment(uint64_t);
			void operator += (const Ump&);
			void operator += (int);
			void operator += (uint32_t);
			void operator += (uint64_t);
			void decrement(const Ump&);
			void decrement(int);
			void decrement(uint32_t);
			void decrement(uint64_t);
			void operator -= (const Ump&);
			void operator -= (int);
			void operator -= (uint32_t);
			void operator -= (uint64_t);
			Ump operator << (int) const;
			void operator <<= (int);
			Ump operator >> (int) const;
			void operator >>= (int);
			Ump operator ~ () const;
			Ump multiply(limb_t) const;
			Ump multiply(const Ump&) const;
			Ump multiply(Ump&, const Ump&) const;
			void operator *= (limb_t);
			void operator *= (const Ump&);
			void assign(std::string);
			void random();
			Ump modinv(const Ump&) const;
			void divide(const Ump& divisor, Ump& quotient, Ump& remainder) const;	
			Ump R_mod_m() const;
			//void remainder(const Ump& divisor, Ump& remainder) const;
			int compare(const Ump&) const;
			std::string to_str() const;
			void clear_extra_words();
			void make_odd();
			bool is_prime();

		};
		//template<> void Ump<128>::divide(const Ump& divisor, Ump& quotient, Ump& remainder) const;
		
		template<int BITS>   Ump<BITS> operator + (const Ump<BITS>& lhs, const Ump<BITS>& rhs);
		template<int BITS>   Ump<BITS> operator + (const Ump<BITS>& lhs, int rhs);
		template<int BITS>   Ump<BITS> operator + (const Ump<BITS>& lhs, uint32_t rhs);
		template<int BITS>   Ump<BITS> operator + (const Ump<BITS>& lhs, uint64_t rhs);
		template<int BITS>   Ump<BITS> operator - (const Ump<BITS>& lhs, const Ump<BITS>& rhs);
		template<int BITS>   Ump<BITS> operator - (const Ump<BITS>& lhs, int rhs);
		template<int BITS>   Ump<BITS> operator - (const Ump<BITS>& lhs, uint32_t rhs);
		template<int BITS>   Ump<BITS> operator * (const Ump<BITS>& lhs, typename Ump<BITS>::limb_t);
		template<int BITS>   Ump<BITS> operator * (const Ump<BITS>& lhs, const Ump<BITS>& rhs);
		template<int BITS>  Ump<BITS> operator / (const Ump<BITS>& lhs, const Ump<BITS>& rhs);
		template<int BITS>  Ump<BITS> operator % (const Ump<BITS>& lhs, const Ump<BITS>& rhs);
		template<int BITS>   bool operator > (const Ump<BITS>& lhs, const Ump<BITS>& rhs);
		template<int BITS>   bool operator > (const Ump<BITS>& lhs, int rhs);
		template<int BITS>   bool operator < (const Ump<BITS>& lhs, const Ump<BITS>& rhs);
		template<int BITS>   bool operator < (const Ump<BITS>& lhs, int rhs);
		template<int BITS>   bool operator == (const Ump<BITS>& lhs, const Ump<BITS>& rhs);
		template<int BITS>   bool operator == (const Ump<BITS>& lhs, int rhs);
		template<int BITS>   bool operator >= (const Ump<BITS>& lhs, const Ump<BITS>& rhs);
		template<int BITS>   bool operator >= (const Ump<BITS>& lhs, int rhs);
		template<int BITS>   bool operator <= (const Ump<BITS>& lhs, const Ump<BITS>& rhs);
		template<int BITS>   bool operator <= (const Ump<BITS>& lhs, int rhs);
		template<int BITS>   bool operator != (const Ump<BITS>& lhs, const Ump<BITS>& rhs);
		template<int BITS>   bool operator != (const Ump<BITS>& lhs, int rhs);
		template<int BITS>  Ump<BITS> montgomery_square(const Ump<BITS>& x, const Ump<BITS>& m, limb_t m_primed);
		template<int BITS>  bool powm_2(const Ump<BITS>& m, uint64_t offset);
		template<int BITS>  Ump<BITS> double_and_reduce(const Ump<BITS>& x, const Ump<BITS>& m, int shift);
}
#include "ump.cpp"
#endif
