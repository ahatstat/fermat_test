


//https://stackoverflow.com/questions/28868367/getting-the-high-part-of-64-bit-integer-multiplication

#ifdef __SIZEOF_INT128__     // GNU C
    //multiply two 64 bit numbers and return the upper word
    static inline uint64_t mulhi64(uint64_t a, uint64_t b)
    {
        unsigned __int128 prod = a * (unsigned __int128)b;
        return prod >> 64;
    }
    //multiply two 64 bit numbers and return the upper and lower word
    static inline uint64_t mulq(uint64_t& high_product, uint64_t a, uint64_t b)
    {
        unsigned __int128 prod = a * (unsigned __int128)b;
        high_product = prod >> 64;
        return prod;
    }

#elif defined(_M_X64) || defined(_M_ARM64)     // MSVC
        // MSVC for x86-64 or AArch64
        // possibly also  || defined(_M_IA64) || defined(_WIN64)
        // but the docs only guarantee x86-64!  Don't use *just* _WIN64; it doesn't include AArch64 Android / Linux

       // https://docs.microsoft.com/en-gb/cpp/intrinsics/umulh
#include <intrin.h>
#define mulhi64 __umulh
static inline uint64_t mulq(uint64_t& high_product, uint64_t a, uint64_t b)
{
    high_product = mulhi64(a, b);
    return a * b;
}

#elif defined(_M_IA64) // || defined(_M_ARM)       // MSVC again
        // https://docs.microsoft.com/en-gb/cpp/intrinsics/umul128
        // incorrectly say that _umul128 is available for ARM
        // which would be weird because there's no single insn on AArch32
#include <intrin.h>
    static inline uint64_t mulhi64(uint64_t a, uint64_t b)
    {
        unsigned __int64 HighProduct;
        (void)_umul128(a, b, &HighProduct);
        return HighProduct;
    }
    static inline uint64_t mulq(uint64_t& high_product, uint64_t a, uint64_t b)
    {
        (void)_umul128(a, b, &high_product);
        return a * b;
    }

#else
    static inline uint64_t mulhi64(uint64_t a, uint64_t b)
    {
        uint64_t    a_lo = (uint32_t)a;
        uint64_t    a_hi = a >> 32;
        uint64_t    b_lo = (uint32_t)b;
        uint64_t    b_hi = b >> 32;

        uint64_t    a_x_b_hi = a_hi * b_hi;
        uint64_t    a_x_b_mid = a_hi * b_lo;
        uint64_t    b_x_a_mid = b_hi * a_lo;
        uint64_t    a_x_b_lo = a_lo * b_lo;

        uint64_t    carry_bit = ((uint64_t)(uint32_t)a_x_b_mid +
            (uint64_t)(uint32_t)b_x_a_mid +
            (a_x_b_lo >> 32)) >> 32;

        uint64_t    multhi = a_x_b_hi +
            (a_x_b_mid >> 32) + (b_x_a_mid >> 32) +
            carry_bit;

        return multhi;
    }
    static inline uint64_t mulq(uint64_t& high_product, uint64_t a, uint64_t b)
    {
        uint64_t    a_lo = (uint32_t)a;
        uint64_t    a_hi = a >> 32;
        uint64_t    b_lo = (uint32_t)b;
        uint64_t    b_hi = b >> 32;

        uint64_t    a_x_b_hi = a_hi * b_hi;
        uint64_t    a_x_b_mid = a_hi * b_lo;
        uint64_t    b_x_a_mid = b_hi * a_lo;
        uint64_t    a_x_b_lo = a_lo * b_lo;

        uint64_t    carry_bit = ((uint64_t)(uint32_t)a_x_b_mid +
            (uint64_t)(uint32_t)b_x_a_mid +
            (a_x_b_lo >> 32)) >> 32;

        uint64_t    multhi = a_x_b_hi +
            (a_x_b_mid >> 32) + (b_x_a_mid >> 32) +
            carry_bit;

        high_product = multhi;
        return a * b;
     }

#endif

namespace ump {

    //multiply with carry in.  returns the lower half of the multiplication.
    static inline uint64_t mul_carry(uint64_t& upper_product, uint64_t a, uint64_t b, uint64_t carry_in)
    {
#ifdef __SIZEOF_INT128__
        unsigned __int128 uv = (unsigned __int128)a * (unsigned __int128)b + carry_in;
        uint64_t low_word = uv;
        upper_product = uv >> 64;
        return low_word;
#else
        uint64_t low_word = a * b;
        uint64_t x = low_word + carry_in;
        upper_product = mulhi64(a, b);
        upper_product += (x < low_word) ? 1 : 0;
        return x;

#endif
    }

    //32 bit multiply with carry in
    static inline uint32_t mul_carry(uint32_t& upper_product, uint32_t a, uint32_t b, uint32_t carry_in)
    {
        uint64_t uv = (uint64_t)a * (uint64_t)b + carry_in;
        uint32_t low_word = (uint32_t)uv; 
        upper_product = uv >> 32;
        return low_word;
    }


    //low half of mutliplication
    template<int BITS>
    Ump<BITS> Ump<BITS>::multiply(const Ump<BITS>& y) const
    {
        Ump<BITS> result;
            
        for (int i = 0; i <= Ump<BITS>::HIGH_WORD; i++)
        {
            limb_t c = 0;
            unsigned char cc = 0;
            for (int j = 0; j <= Ump<BITS>::HIGH_WORD - i; j++)
            {
                 uint64_t u = mul_carry(c, m_limbs[j], y.m_limbs[i], c);
                 cc = add_carry(cc, result.m_limbs[i + j], u, &result.m_limbs[i + j]);
            }
        }
        return result;
    }

    //full width mutliplication.  return the lower half and a reference to the upper half.
    template<int BITS>
    Ump<BITS> Ump<BITS>::multiply(Ump& upper_result, const Ump& y) const
    {
        Ump<BITS> lower_result;
        upper_result = 0;

        for (int i = 0; i <= Ump<BITS>::HIGH_WORD; i++)
        {
            limb_t c = 0;
            unsigned char cc = 0;
            //lower half
            for (int j = 0; j <= Ump<BITS>::HIGH_WORD - i; j++)
            {
                uint64_t u = mul_carry(c, m_limbs[j], y.m_limbs[i], c);
                cc = add_carry(cc, lower_result.m_limbs[i + j], u, &lower_result.m_limbs[i + j]);
            }
            //upper half
            for (int j = Ump<BITS>::HIGH_WORD - i + 1; j <= Ump<BITS>::HIGH_WORD; j++)
            {
                uint64_t u = mul_carry(c, m_limbs[j], y.m_limbs[i], c);
                cc = add_carry(cc, upper_result.m_limbs[i + j - (Ump<BITS>::HIGH_WORD + 1)], u,
                    &upper_result.m_limbs[i + j - (Ump<BITS>::HIGH_WORD + 1)]);
            }
            upper_result.m_limbs[i] = c + cc;
        }
        return lower_result;
    }

        

    template<int BITS>
    Ump<BITS> operator * (const Ump<BITS>& lhs, const Ump<BITS>& rhs)
    {
        return lhs.multiply(rhs);
    }

    template<int BITS>
    void Ump<BITS>::operator*=(const Ump<BITS>& y)
    {
        *this = multiply(y);
    }


    //multiplication of a big uint by an unsigned integer.  
    template<int BITS>
    Ump<BITS> Ump<BITS>::multiply(limb_t x) const
    {
        Ump<BITS> result;
        limb_t c = 0;
        for (auto i = 0; i < LIMBS; i++)
        {
            if (BITS_PER_WORD <= 32)
            {
                uint64_t uv = static_cast<uint64_t>(m_limbs[i]) * x + c;
                result.m_limbs[i] = uv;  //store the lower bits
                c = uv >> 32;  //the upper bits is the carry
            }
            else
            {
                result.m_limbs[i] = mul_carry(c, m_limbs[i], x, c);
            }
                
        }
        return result;
    }

  


    template<int BITS>
    Ump<BITS> operator * (const Ump<BITS>& lhs, limb_t rhs)
    {
        return lhs.multiply(rhs);
    }

    template<int BITS>
    void Ump<BITS>::operator*=(limb_t x)
    {
        *this = multiply(x);

    }


        
}
