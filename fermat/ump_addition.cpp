
//64 bit add with carry.  use compiler intrinsics if available. 
#if defined(_M_X64) && (defined(_W32) || defined(_W64))    // MSVC Windows X64
#include <intrin.h>
#define add_carry_u64 _addcarry_u64
#elif defined(__GNUC__) && defined(__x86_64__)  //GCC X64
#include <x86intrin.h>
#define add_carry_u64 _addcarry_u64
#elif defined(__SIZEOF_INT128__)  //The compiler supports 128 bit arithmetic
static inline unsigned char add_carry_u64(unsigned char carry_in, uint64_t a, uint64_t b, unsigned long long* sum)
{
    unsigned __int128 x = static_cast<unsigned __int128>(a) + static_cast<unsigned __int128>(b) + carry_in;
    uint64_t low = x;
    unsigned char carry_out = (x >> 64) > 0 ? 1 : 0;
    *sum = low;
    return carry_out;
}
#else  //fallback
static inline unsigned char add_carry_u64(unsigned char carry_in, uint64_t a, uint64_t b, unsigned long long* sum)
{
    bool propagate = false;
    bool generate = false;
    unsigned char carry_out = 0;
    uint64_t x = 0;
    x = a + b;
    *sum = x + carry_in;
    propagate = x == (uint64_t)(-1);
    generate = x < a;
    carry_out = generate || (propagate && carry_in) ? 1 : 0;
    return carry_out;
}
#endif 

//32 bit add with carry. 
static inline unsigned char add_carry_u32(unsigned char carry_in, uint32_t a, uint32_t b, uint32_t* sum)
{
    uint64_t x = static_cast<uint64_t>(a) + static_cast<uint64_t>(b) + carry_in;
    unsigned char carry_out = (x >> 32) > 0 ? 1 : 0;
    *sum = (uint32_t)x;
    return carry_out;
}


namespace ump {

    static inline unsigned char add_carry(unsigned char carry_in, uint32_t a, uint32_t b, uint32_t* sum)
    {
        return add_carry_u32(carry_in, a, b, sum);
    }

    static inline unsigned char add_carry(unsigned char carry_in, uint64_t a, uint64_t b, uint64_t* sum)
    {
        return add_carry_u64(carry_in, a, b, (unsigned long long*)sum);
    }

    template<int BITS>
    Ump<BITS> Ump<BITS>::add(const Ump<BITS>& b) const
    {
        Ump result;
        unsigned char carry = 0;
        for (auto i = 0; i < LIMBS; i++)
        {
            carry = add_carry(carry, m_limbs[i], b.m_limbs[i], &result.m_limbs[i]);
        }

        return result;
    }

    template<int BITS>
    Ump<BITS> Ump<BITS>::add(int b) const
    {
        if (b < 0)
        {
            return sub(static_cast<uint32_t>(-b));
        }
        return add(static_cast<uint32_t>(b));

    }

    template<int BITS>
    Ump<BITS> Ump<BITS>::add(uint32_t b) const
    {
        Ump result = *this;
        result.m_limbs[0] = m_limbs[0] + b;
        uint8_t carry = result.m_limbs[0] < m_limbs[0] ? 1 : 0;
        for (int i = 1; carry && i < LIMBS; i++)
        {
            result.m_limbs[i] = m_limbs[i] + carry;
            carry = result.m_limbs[i] < m_limbs[i] ? 1 : 0;
        }
        return result;
    }

    template<int BITS>
    Ump<BITS> Ump<BITS>::add(uint64_t b) const
    {
        Ump result = *this;
        uint64_t x;
        bool carry;
        if (BITS_PER_WORD <= 32)
        {
            x = (static_cast<uint64_t>(m_limbs[1]) << 32) + m_limbs[0];
            x += b;
            result.m_limbs[0] = x;
            result.m_limbs[1] = x >> 32;
            carry = x < b;
            for (int i = 2; carry && i < LIMBS; i++)
            {
                result.m_limbs[i] = m_limbs[i] + (carry ? 1 : 0);
                carry = result.m_limbs[i] < m_limbs[i];
            }
        }
        else
        {
            result.m_limbs[0] = m_limbs[0] + b;
            carry = result.m_limbs[0] < m_limbs[0];
            for (int i = 1; carry && i < LIMBS; i++)
            {
                result.m_limbs[i] = m_limbs[i] + (carry ? 1 : 0);
                carry = result.m_limbs[i] < m_limbs[i];
            }
        }
        
        return result;
    }

    //same as add, but results are stored in the current object
    template<int BITS>
    void Ump<BITS>::increment(const Ump<BITS>& b)
    {
        unsigned char carry = 0;
        for (auto i = 0; i < LIMBS; i++)
        {
            carry = add_carry(carry, m_limbs[i], b.m_limbs[i], &m_limbs[i]);
        }
    }

    template<int BITS>
    void Ump<BITS>::increment(int b)
    {
        if (b < 0)
            decrement(static_cast<uint32_t>(-b));
        else
            increment(static_cast<uint32_t>(b));

    }

    template<int BITS>
    void Ump<BITS>::increment(uint32_t b)
    {
        uint32_t temp = m_limbs[0];
        m_limbs[0] += b;
        uint32_t carry = m_limbs[0] < temp ? 1 : 0;
        for (int i = 1; carry && i < LIMBS; i++)
        {
            temp = m_limbs[i];
            m_limbs[i] += carry;
            carry = m_limbs[i] < temp ? 1 : 0;
        }
    }

    template<int BITS>
    void Ump<BITS>::increment(uint64_t b)
    {
        *this = add(b);
    }

    template<int BITS>
    void Ump<BITS>::operator+=(const Ump<BITS>& b)
    {
        increment(b);
    }

    template<int BITS>
    void Ump<BITS>::operator+=(int b)
    {
        increment(b);
    }

    template<int BITS>
    void Ump<BITS>::operator+=(uint32_t b)
    {
        increment(b);
    }

    template<int BITS>
    void Ump<BITS>::operator+=(uint64_t b)
    {
        increment(b);
    }

    template<int BITS>
    Ump<BITS> operator + (const Ump<BITS>& lhs, const Ump<BITS>& rhs)
    {
        return lhs.add(rhs);
    }

    template<int BITS>
    Ump<BITS> operator + (const Ump<BITS>& lhs, int rhs)
    {
        return lhs.add(rhs);
    }

    template<int BITS>
    Ump<BITS> operator+(const Ump<BITS>& lhs, uint32_t rhs)
    {
        return lhs.add(rhs);
    }

    template<int BITS>
    Ump<BITS> operator+(const Ump<BITS>& lhs, uint64_t rhs)
    {
        return lhs.add(rhs);
    }
       
}
