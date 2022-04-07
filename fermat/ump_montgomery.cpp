#include "ump.hpp"

namespace ump {

        //montgomery square - optimized to take advantage of symmetry of squaring operation
        //Fermat testing spends most of its time inside this function
        //returns xxR^-1
        template<int BITS> void montgomery_square(Ump<BITS>& x, const Ump<BITS>& m, limb_t m_primed)
        {
            //square
            Ump<BITS> result;
            std::array<limb_t, (Ump<BITS>::LIMBS) * 2> Accumulator{};
            limb_t sq, sq_upper, yy;

            for (int i = 0; i <= Ump<BITS>::HIGH_WORD; i++)
            {
                
                //the current term
                limb_t w = x.m_limbs[i];
                //the square term
                sq = mulq(sq_upper, w, w);
                //multiply the remaining upper portion of x by the current word (y = x * w)
                //intialize the carry in to half the upper square value.  this will get doubled again in the following loop.
                limb_t mul_carry_in = sq_upper / 2;
                unsigned char carry = sq_upper % 2; //initialize the add carry to the LSB of the upper square term
                unsigned char accumulator_carry = add_carry(0, Accumulator[2 * i], sq, &Accumulator[2 * i]);
                for (int j = 1; j < Ump<BITS>::LIMBS - i; j++)
                {
                    //mulitply y = x * xi excluding the square term
                    yy = mul_carry(mul_carry_in, x.m_limbs[i + j], w, mul_carry_in);
                    //y = y * 2 and save the carry out
                    unsigned char upper_bit = yy >> (BITS_PER_WORD - 1);
                    yy = yy*2 + carry;
                    carry = upper_bit;
                    //accumulate
                    accumulator_carry = add_carry(accumulator_carry, Accumulator[2 * i + j], yy, &Accumulator[2 * i + j]);
                }
                Accumulator[i + Ump<BITS>::LIMBS] = carry + accumulator_carry;
            }
          
            //reduce
            limb_t carry = 0;
            unsigned char carry2 = 0;
            for (int i = 0; i <= Ump<BITS>::HIGH_WORD; i++)
            {
                limb_t u = Accumulator[i] * m_primed;
                for (int j = 0; j < Ump<BITS>::LIMBS; j++)
                {
                    limb_t muj = mul_carry(carry, m.m_limbs[j], u, carry);
                    carry2 = add_carry(carry2, Accumulator[i + j], muj, &Accumulator[i + j]);
                }
                //continue carrying
                for (int j = Ump<BITS>::LIMBS; carry2 && j < (int)Accumulator.size() - i; j++)
                {
                    carry2 = add_carry(carry2, Accumulator[i + j], 0, &Accumulator[i + j]);
                }
            }
            
            //move back to x
            for (int i = 0; i < Ump<BITS>::LIMBS; i++)
            {
                x.m_limbs[i] = Accumulator[i + Ump<BITS>::HIGH_WORD + 1];
            }

            //adjust
            if (x >= m)
            {
                x -= m;
            }
        }

        //returns true if 2^(m-1) mod m == 1, false otherwise
        //m_primed and rmodm are precalculated values.  See hac 14.94.  
        //R^2 mod m is not needed because with base 2 is trivial to calculate 2*R mod m given R mod m
        template<int BITS>  bool fermat_test_base_2(const Ump<BITS>& base_m, uint64_t offset)
        {
            const Ump<BITS>& m = base_m + offset;
            
            //precalculation of some constants based on the modulus
            const limb_t m_primed = -mod_inverse_64(m.m_limbs[0]);

            //initialize the product to R mod m, the equivalent of 1 in the montgomery domain
            Ump<BITS> A = m.R_mod_m();
            
            //Ump<BITS> exp = m - 1;  //m is odd, so m-1 only changes the lowest bit of m from 1 to 0. 

            //perform the first few iterations without montgomery multiplication - we are multiplying by small powers of 2
            int word = Ump<BITS>::HIGH_WORD;
            const int top_bits_window = 4;
            int shift = BITS_PER_WORD - top_bits_window;
            limb_t mask = static_cast<limb_t>((1 << top_bits_window) - 1) << shift;
            int top_bits = (m.m_limbs[word] & mask) >> shift;
            A = double_and_reduce(A, m, top_bits);
            //Go through each bit of the exponent.  We assume all words except the lower three match the base big int
            for (int i = BITS - top_bits_window - 1; i >= 1; i--)
            {
                //square
                montgomery_square(A, m, m_primed);
                word = i / BITS_PER_WORD;
                int bit = i % BITS_PER_WORD;
                mask = 1ull << bit;
                bool bit_is_set = (m.m_limbs[word] & mask) != 0;
                if (bit_is_set)
                {
                    //multiply by the base (2) if the exponent bit is set
                    A = double_and_reduce(A, m, 1);
                }
                
            }
            //the final iteration happens here. the exponent m-1 lowest bit is always 0 so we never need to double and reduce after squaring
            //The final squaring can be avoided if we check the if the current value is +1 or -1 in the montgomery domain 
            bool pass = false;
            if (A == m.R_mod_m() || A == (m - m.R_mod_m()))
            {
                pass = true;
            }    
            //A = montgomery_square_2(A, m, m_primed);
            //convert back from montgomery domain
            //A = montgomery_reduce(A, m, m_primed);
            return pass;

        }

        //returns 2^(m-1) mod m
        //m_primed and rmodm are precalculated values.  See hac 14.94.  
        //R^2 mod m is not needed because with base 2 is trivial to calculate 2*R mod m given R mod m
        template<int BITS>
        Ump<BITS> powm_2(const Ump<BITS>& base_m, uint64_t offset)
        {
            const Ump<BITS>& m = base_m + offset;

            //precalculation of some constants based on the modulus
            const limb_t m_primed = -mod_inverse_64(m.m_limbs[0]);

            //initialize the product to R mod m, the equivalent of 1 in the montgomery domain
            Ump<BITS> A = m.R_mod_m();

            //perform the first few iterations without montgomery multiplication - we are multiplying by small powers of 2
            int word = Ump<BITS>::HIGH_WORD;
            const int top_bits_window = 4;
            int shift = BITS_PER_WORD - top_bits_window;
            limb_t mask = static_cast<limb_t>((1 << top_bits_window) - 1) << shift;
            int top_bits = (m.m_limbs[word] & mask) >> shift;
            A = double_and_reduce(A, m, top_bits);
            //Go through each bit of the exponent.  We assume all words except the lower three match the base big int
            for (int i = BITS - top_bits_window - 1; i >= 1; i--)
            {
                //square
                montgomery_square(A, m, m_primed);
                word = i / BITS_PER_WORD;
                int bit = i % BITS_PER_WORD;
                mask = 1ull << bit;
                bool bit_is_set = (m.m_limbs[word] & mask) != 0;
                if (bit_is_set)
                {
                    //multiply by the base (2) if the exponent bit is set
                    A = double_and_reduce(A, m, 1);
                }

            }
            //the final iteration happens here. the exponent m-1 lowest bit is always 0 so we never need to double and reduce after squaring
            montgomery_square(A, m, m_primed);
            //convert back from montgomery domain
            montgomery_reduce(A, m, m_primed);
            return A;
        }

        //reduce x to xR^-1 mod m
        //HAC 14.32
        template<int BITS> void montgomery_reduce(Ump<BITS>& A, const Ump<BITS>& m, limb_t m_primed)
        {
            for (int i = 0; i <= Ump<BITS>::HIGH_WORD; i++)
            {
                limb_t u = A.m_limbs[0] * m_primed;
                A += m * u;
                A >>= BITS_PER_WORD;
            }
            if (A >= m)
            {
                A -= m;
            }
        }
        
        // return 2 * x mod m given x and m using shift and subtract.
        //For this to be efficient, m must be similar magnitude (within a few bits) of x. 
        template<int BITS>  Ump<BITS> double_and_reduce(const Ump<BITS>& x, const Ump<BITS>& m, int shift)
        {
            Ump<BITS> A = x << shift;
            while (A >= m) 
            {
                A -= m;
            }
            return A;
        }


}
