
//divide 128 bit by a 64 bit number and return the 64 bit quotient and remainders 
#ifdef __SIZEOF_INT128__     // GNU C
static inline uint64_t div128by64(uint64_t& remainder, uint64_t high_dividend, uint64_t low_dividend, uint64_t divisor)
{
    uint64_t quotient = 0;
    unsigned __int128 dividend = ((unsigned __int128)high_dividend << 64) | low_dividend;
    quotient = dividend / divisor;
    remainder = dividend % divisor;
    return quotient;
}

#elif defined(_M_X64) || defined(_M_ARM64)     // MSVC
#include <intrin.h>
static inline uint64_t div128by64(uint64_t& remainder, uint64_t high_dividend, uint64_t low_dividend, uint64_t divisor)
{
    uint64_t quotient = 0;
    quotient = _udiv128(high_dividend, low_dividend, divisor, &remainder);
    return quotient;
}

#else  //fallback
//TODO: implement fallback
static inline uint64_t div128by64(uint64_t& remainder, uint64_t high_dividend, uint64_t low_dividend, uint64_t divisor)
{
    uint64_t quotient = 0;
    remainder = 0;
    return quotient;
}
#endif
    
namespace ump {
    
    //special case of 128 by 128 bit division which can be optimized if the compiler supports 128 bit integers
    //quotient = *this/divisor.  remainder = *this - divisor * quotient.
    template<>
    inline void Ump<128>::divide(const Ump& divisor, Ump& quotient, Ump& remainder) const
    {
        remainder = 0;
        quotient = 0;
        if (divisor == 0)
        {
            std::cout << "Ump: Division by zero." << std::endl;
            return;
        }
#ifdef __SIZEOF_INT128__     // GNU C
        unsigned __int128 dividend = ((unsigned __int128)m_limbs[1] << 64) | m_limbs[0];
        unsigned __int128 divis = ((unsigned __int128)divisor.m_limbs[1] << 64) | divisor.m_limbs[0];
        unsigned __int128 q = dividend / divis;
        unsigned __int128 rem = dividend % divis;
        remainder.m_limbs[1] = rem >> 64;
        remainder.m_limbs[0] = rem;
        quotient.m_limbs[1] = q >> 64;
        quotient.m_limbs[0] = q;

#else
            
        //hackers delight 9-5 modified for 128 bit
        //u is dividend.  v is divisor. q quotient. r is remainder
        if (*this < divisor)
        {
            remainder = *this;
            return;
        }
        uint64_t k, n;
        if (divisor.m_limbs[1] == 0) //the divisor (and thus the remainder) fits in one word.
        {
            if (m_limbs[1] < divisor.m_limbs[0])  // the quotient will fit in one word
            {
                quotient.m_limbs[0] = div128by64(remainder.m_limbs[0], m_limbs[1], m_limbs[0], divisor.m_limbs[0]);
                return;
            }
            else  //the quotient will fit in two words. 
            {
                //break into halves
                quotient.m_limbs[1] = div128by64(k, 0, m_limbs[1], divisor.m_limbs[0]);
                quotient.m_limbs[0] = div128by64(remainder.m_limbs[0], k, m_limbs[0], divisor.m_limbs[0]);
                return;
            }
        }
        //the divisor is larger than one word.  The quotient will fit into one word and the remainder requires up to 2 words.
        n = count_leading_zeros(divisor.m_limbs[1]);  // 0 <= n <= 63
        //normalize the divisor so its MSB is one
        Ump<128> v = divisor;
        v = v << n;
        Ump<128> u = *this;
        //divide the numerator by 2 to ensure no overflow is possible
        u = u >> 1;
        quotient.m_limbs[0] = div128by64(k, u.m_limbs[1], u.m_limbs[0], v.m_limbs[1]);
        //undo the normalization and divide by 2
        quotient.m_limbs[0] = quotient.m_limbs[0] >> (63 - n) ; //the quotient is now correct or too large by one
        quotient.m_limbs[0]--; //the quotient is now correct or too small by one
        remainder = *this - quotient * divisor;
        //apply correction
        if (remainder >= divisor)
        {
            quotient.m_limbs[0]++;
            remainder -= divisor;
        }
#endif

    }
        //normalized multiprecision integer division.  Return both quotient and remainder.  Reference HAC chapter 14 algorithm 14.20.
        template<int BITS>
        void Ump<BITS>::divide(const Ump<BITS>& divisor, Ump<BITS>& quotient, Ump<BITS>& remainder) const
        {
            const bool divide_debug = false;  //enable printfs
            if (divisor == 0)
            {
                if (divide_debug) printf("division by zero detected.\n");
                quotient = 0;
                remainder = 0;
                return;
            }
            if (divisor == 1)
            {
                quotient = *this;
                remainder = 0;
                return;
            }
            
            int cmp = compare(divisor);
            if (cmp == 0)
            {
                //the dividend and divisor are equal
                quotient = 1;
                remainder = 0;
                return;
            }
            if (cmp == -1)
            {
                //the divisor is larger than the dividend
                quotient = 0;
                remainder = *this;
                return;
            }

            quotient = 0;
            Ump<BITS> y = divisor;
            Ump<BITS> x = *this;
           
            //t is the highest non-0 word of the divisor
            int t;
            for (t = Ump<BITS>::LIMBS - 1; t >= 0; t--)
            {
                if (divisor.m_limbs[t] > 0)
                    break;
            }
            //normalize by shifting both dividend and divisor left until the MSB of the first divisor word is 1.
            //this can overflow the dividend so we have to use the extra word.
            int normalize_shift = count_leading_zeros(divisor.m_limbs[t]); 
            y <<= normalize_shift;
            x <<= normalize_shift;
            
            //n is the highest non-0 word of the dividend
            int n;
            for (n = Ump<BITS>::LIMBS - 1; n >= 0; n--)
            {
                if (x.m_limbs[n] > 0)
                    break;
            }

            if (divide_debug) printf("n %i t %i normalize_shift %i \n", n, t, normalize_shift);
            //after normalization, this loop should execute max once
            //step 2
            Ump<BITS> temp;
            temp = y << (BITS_PER_WORD * (n - t));
            while (x >= temp)
            {
                quotient.m_limbs[n - t] += 1;
                x -= temp;
            }

            //step 3
            for (auto i = n; i > t; i--)  //t can be 0.  i is >= 1;
            {
                int j = i - t - 1;  //the index of the current quotient word.  j >= 0;
                limb_t xi = x.m_limbs[i];
                if (divide_debug) printf("3.1 i %i j %i\n", i, j);
                //3.1
                if (xi == y.m_limbs[t])
                {
                    //with what test vector can i get inside this if?
                    quotient.m_limbs[j] = -1;
                    if (divide_debug) printf("3.1a\n");
                }
                else
                {
                    //3.1b
                    //perform double precision division using the upper words
                    uint64_t k;
                    quotient.m_limbs[j] = div128by64(k, xi, x.m_limbs[i - 1], y.m_limbs[t]);
                }
                //3.2
                //determine if the estimate for qy is greater than x.  this requires a triple precision comparison.
                Ump<2 * BITS_PER_WORD> y_3, qy_estimate, x_comp;
                y_3.m_limbs[1] = y.m_limbs[t];
                y_3.m_limbs[0] = t > 0 ? y.m_limbs[t - 1] : 0;
                qy_estimate = y_3 * quotient.m_limbs[j];
                x_comp.m_limbs[2] = xi;
                x_comp.m_limbs[1] = x.m_limbs[i - 1];
                x_comp.m_limbs[0] = i >= 2 ? x.m_limbs[i - 2] : 0;
               //3.2 correction loop
                while (qy_estimate > x_comp)
                {
                    //update the estimate
                    quotient.m_limbs[j]--;
                    qy_estimate -= y_3;
                }
                //3.3 subtract q*y from x, where q is the current single precision quotient word we are working on and y is the full precision y. 
                limb_t multiplication_carry = 0;  //carry for the multiply
                unsigned char borrow = 0; //borrow for the subtraction
                //3.3 multiply and subtract in one loop to minimize need for intermediate storage
                for (auto k = 0; k <= t; k++)
                {
                    limb_t qy = mul_carry(multiplication_carry, quotient.m_limbs[j], y.m_limbs[k], multiplication_carry);
                    borrow = sub_borrow(borrow, x.m_limbs[j + k], qy, &x.m_limbs[j + k]);
                }
                //handle carries to the final word
                borrow = sub_borrow(borrow, x.m_limbs[j + t + 1], multiplication_carry, &x.m_limbs[j + t + 1]);
                //3.4 check if the previous subtraction of qy overflowed.  if so add back one y
                if (borrow)
                {
                    //with what test vector can i get inside this if?
                    //printf("3.4 correction\n");
                    unsigned char addition_carry = 0;
                    for (auto k = 0; k <= t; k++)
                    {
                        int x_index = j + k;
                        addition_carry = add_carry(addition_carry, x.m_limbs[x_index], y.m_limbs[k], &x.m_limbs[x_index]);
                    }
                    //handle carries to the final word
                    int x_index = j + t + 1;
                    x.m_limbs[x_index] += addition_carry;

                    //decrement the quotient word
                    quotient.m_limbs[j]--;
                }
            }
            remainder = x >> normalize_shift;  //denormalize the remainder
            return;
        }

        //Calculate R mod m where m is the modulus (this), R is 2^BITS 
        template<int BITS>
        Ump<BITS> Ump<BITS>::R_mod_m() const
        {
            if (*this == 1)
                return 0;
            Ump<BITS> r, q, m, rem;
            //set r to 2^bits - 1 to avoid overflow.
            r -= 1;
            r.clear_extra_words();
            m = *this;
            r.divide(m, q, rem);
            //apply correction
            return rem + 1;
        }

}
