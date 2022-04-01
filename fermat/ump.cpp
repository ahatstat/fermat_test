#include "ump_addition.cpp"
#include "ump_subtraction.cpp"
#include "ump_multiplication.cpp"
#include "ump_division.cpp"
#include "ump_montgomery.cpp"
#include <random>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <string>
#include "ump.hpp"

namespace ump {

    template<int BITS>
    Ump<BITS>::Ump() : m_limbs{}
    { 
    }

    template<int BITS>
    Ump<BITS>::Ump(std::string s)
    {
        assign(s);
    }

    template<int BITS>
    Ump<BITS>::Ump(uint32_t init32) : m_limbs{}
    {
        m_limbs[0] = init32;
    }

    template<int BITS>
    Ump<BITS>::Ump(uint64_t init64) : m_limbs{}
    {
        if (BITS_PER_WORD == 32)
        {
            m_limbs[0] = init64;
            m_limbs[1] = init64 >> 32;
        }
        else
            m_limbs[0] = init64;

    }

    template<int BITS>
    Ump<BITS>::Ump(int init) : m_limbs{}
    {
        m_limbs[0] = init;
        if (init < 0)
        {
            for (auto i = 1; i < LIMBS; i++)
            {
                m_limbs[i] = ~(0u);
            }
        }
           
    }

    template<int BITS>
    Ump<BITS> Ump<BITS>::operator<<(int shift) const
    {
            
        Ump<BITS> result;
        if (shift == 0)
            return *this;
        if (shift < 0)
        {
            return (*this >> -shift);
        }
        int whole_word_shift = shift / BITS_PER_WORD;
        if (whole_word_shift >= LIMBS)
        {
            return 0;
        }
        int bit_shift = shift % BITS_PER_WORD;
        limb_t save_bits = 0;
        for (int i = whole_word_shift; i < LIMBS; i++)
        {
            limb_t mask1 = bit_shift > 0 ? save_bits >> (BITS_PER_WORD - bit_shift) : 0;
            limb_t mask2 = bit_shift > 0 ? (limb_t)~0 << (BITS_PER_WORD - bit_shift) : ~0;
            result.m_limbs[i] = ((m_limbs[i - whole_word_shift] << bit_shift) | mask1);
            save_bits = m_limbs[i - whole_word_shift] & mask2;
        }
        return result;
    }

    //left shift in place
    template<int BITS>
    void Ump<BITS>::operator<<=(int shift)
    {
        if (shift == 0)
            return;
        if (shift < 0)
        {
            return;
        }
        int whole_word_shift = shift / BITS_PER_WORD;
            
        const int bit_shift = shift % BITS_PER_WORD;
        const limb_t upper_bits_mask = bit_shift > 0 ? ~0 << (BITS_PER_WORD - bit_shift) : ~0;

        for (int i = LIMBS-1; i >= whole_word_shift; i--)
        {
            int source_word_index = i - whole_word_shift;
            limb_t upper_bits = (source_word_index - 1 >= 0) ? m_limbs[source_word_index - 1] & upper_bits_mask : 0;
            limb_t lower_bits = bit_shift > 0 ? upper_bits >> (BITS_PER_WORD - bit_shift) : 0;
            m_limbs[i] = ((m_limbs[source_word_index] << bit_shift) | lower_bits);
                
        }
        for (int i = 0; i < whole_word_shift && i < LIMBS; i++)
        {
            m_limbs[i] = 0;
        }
        return;
    }

    //right shift in place
    template<int BITS>
    void Ump<BITS>::operator>>=(int shift)
    {
        if (shift == 0)
            return;
        if (shift < 0)
        {
            return;
        }
        int whole_word_shift = shift / BITS_PER_WORD;

        const int bit_shift = shift % BITS_PER_WORD;
        const limb_t lower_bits_mask = bit_shift > 0 ? ~0 >> (BITS_PER_WORD - bit_shift) : ~0;

        for (int i = 0; i < LIMBS - whole_word_shift && i < LIMBS; i++)
        {
            int source_word_index = i + whole_word_shift;
            limb_t lower_bits = (source_word_index + 1 < LIMBS) ? m_limbs[source_word_index + 1] & lower_bits_mask : 0;
            limb_t upper_bits = bit_shift > 0 ? lower_bits << (BITS_PER_WORD - bit_shift) : 0;
            m_limbs[i] = ((m_limbs[source_word_index] >> bit_shift) | upper_bits);

        }
        for (int i = LIMBS - whole_word_shift; i < LIMBS && i >= 0; i++)
        {
            m_limbs[i] = 0;
        }
        return;
    }

    template<int BITS>
    Ump<BITS> Ump<BITS>::operator>>(int shift) const
    {
        Ump<BITS> result;
        if (shift == 0)
            return *this;
        if (shift < 0)
        {
            return (*this << -shift);
        }
        int whole_word_shift = shift / BITS_PER_WORD;
        if (whole_word_shift >= LIMBS)
        {
            return 0;
        }
        int bit_shift = shift % BITS_PER_WORD;
        limb_t save_bits = 0;
        for (int i = LIMBS - 1 - whole_word_shift; i >= 0; i--)
        {
            limb_t mask1 = bit_shift > 0 ? save_bits << (BITS_PER_WORD - bit_shift) : 0;
            limb_t mask2 = bit_shift > 0 ? ~0 >> (BITS_PER_WORD - bit_shift) : ~0;
            result.m_limbs[i] = ((m_limbs[i + whole_word_shift] >> bit_shift) | mask1);
            save_bits = m_limbs[i + whole_word_shift] & mask2;
        }
        return result;
    }

    template<int BITS>
    Ump<BITS> Ump<BITS>::operator~() const
    {
        Ump<BITS> result;
        for (auto i = 0; i < LIMBS; i++)
        {
            result.m_limbs[i] = ~m_limbs[i];
        }
        return result;
    }

    //return the modular inverse if it exists using the binary extended gcd algorithm 
    //Reference HAC chapter 14 algorithm 14.61
    //The modulus m must be odd
    template<int BITS>
    Ump<BITS> Ump<BITS>::modinv(const Ump<BITS>& m) const
    {
        //the modulus m must be odd and greater than 1 and the number to invert must be non zero plus other restrictions
        if (m.m_limbs[0] % 2 == 0 || m < 1 || *this == 0 )
            return 0;
        const bool debug = false;
        //const Ump<BITS> y = *this;
        const Ump<BITS> x = m;  //x == m is odd and > 0
        Ump<BITS> u = m;  //u, x >= 0
        Ump<BITS> v = *this;  //v, y >= 0
        //steps 1 and 2 don't happen if m is odd
        //step 3
        Ump<BITS> B = 0, D = 1;  //B and D can be negative, we must deal with the signs    
        int i = 0;
        if (debug)
        {
            char us[400], vs[400], Bs[400], Ds[400];
            u.to_cstr(us);
            v.to_cstr(vs);
            B.to_cstr(Bs);
            D.to_cstr(Ds);
            printf("iteration %i\nu=%s\nv=%s\nB=%s\nD=%s\n", i, us, vs, Bs, Ds);
        }
        //max iterations is 2 * (2 * 1024 + 2) = 4100
        while (u != 0 && !u.m_limbs[LIMBS - 1])  //if u goes negative, stop.  something is wrong
        {
            //step 4
            while (!(u.m_limbs[0] & 1))  //while u is even
            {
                u = u >> 1;
                if (B.m_limbs[0] & 1)  //if B is odd
                {
                    B = B - x;  //x is always odd so B should be even after this
                }
                    
                B = B >> 1;  //divide by 2
                //copy the top bit to preserve the sign
                B.m_limbs[LIMBS - 1] = B.m_limbs[LIMBS - 1] | ((B.m_limbs[LIMBS - 1] & (1u << 30)) << 1);
                if (debug)
                {
                    char us[400], vs[400], Bs[400], Ds[400];
                    u.to_cstr(us);
                    v.to_cstr(vs);
                    B.to_cstr(Bs);
                    D.to_cstr(Ds);
                    printf("4. u was even.  iteration %i\nu=%s\nv=%s\nB=%s\nD=%s\n", i, us, vs, Bs, Ds);
                }
                ++i;
            }
            //step 5
            while (!(v.m_limbs[0] & 1))  //while v is even
            {
                v = v >> 1;
                if (D.m_limbs[0] & 1)
                {
                    D = D - x;  //x is always odd.  D should be even after this.
                }
                D = D >> 1;  //divide by 2
                //copy the top bit to preserve the sign
                D.m_limbs[LIMBS - 1] = D.m_limbs[LIMBS - 1] | ((D.m_limbs[LIMBS - 1] & (1u << 30)) << 1);
                if (debug)
                {
                    char us[400], vs[400], Bs[400], Ds[400];
                    u.to_cstr(us);
                    v.to_cstr(vs);
                    B.to_cstr(Bs);
                    D.to_cstr(Ds);
                    printf("5. v was even.  iteration %i\nu=%s\nv=%s\nB=%s\nD=%s\n", i, us, vs, Bs, Ds);
                }
                ++i;
            }
            //step 6
            if (u >= v)
            {
                u -= v;
                B -= D;
            }
            else
            {
                v -= u;
                D -= B;
            }
            if (debug)
            {
                    
                char us[400], vs[400], Bs[400], Ds[400];
                u.to_cstr(us);
                v.to_cstr(vs);
                B.to_cstr(Bs);
                D.to_cstr(Ds);
                printf("6. u=%s\nv=%s\nB=%s\nD=%s\n", us, vs, Bs, Ds);
            }
                
        }
        //if the result is negative, add moduli
        while (D.m_limbs[LIMBS - 1] & (1u << 31))
            D += m;

        //if the result is larger than the modulus, subtract moduli
        while (D > m)
            D -= m;

        if (debug)
        {
            char Ds[400];
            char vs[400];
            D.to_cstr(Ds);
            v.to_cstr(vs);
            printf("result=%s\nv=%s\n", Ds, vs);
        }

        //the inverse modulus does not exist
        if (v != 1)
            return 0;

        return D;

    }


    //returns 1 if the integer is greater than the value to compare, 0 if equal, -1 if less than
    template<int BITS>
    int Ump<BITS>::compare(const Ump<BITS>& b) const
    {
          
        for (auto i = LIMBS - 1; i >= 0; i--)
        {
            if (m_limbs[i] > b.m_limbs[i])
                return 1;
            if (m_limbs[i] < b.m_limbs[i])
                return -1;
        }
        return 0;

    }

    template<int BITS>
    std::string Ump<BITS>::to_str() const
    {
        std::string s = "0x";

        for (auto i = LIMBS - 1; i >= 0; i--)
        {
            std::stringstream ss;
            ss << std::setfill('0') << std::setw(sizeof(limb_t) * 2)
                << std::hex << m_limbs[i];
            s += ss.str();
            //s += " ";
        }
              
        return s;
    }

    template<int BITS>
    void Ump<BITS>::clear_extra_words()
    {
        for (int i = HIGH_WORD+1; i < LIMBS; i++)
        {
            m_limbs[i] = 0;
        }
    }

    template<int BITS>
    void Ump<BITS>::make_odd()
    {
        m_limbs[0] |= 1;
    }

    template<int BITS>
    bool Ump<BITS>::is_prime()
    {
        if (m_limbs[0] % 2 == 0)
            return false;
        return powm_2(*this, 0);
    }

    template<int BITS>
    Ump<BITS> operator / (const Ump<BITS>& lhs, const Ump<BITS>& rhs)
    {
        Ump<BITS> q, r;
        lhs.divide(rhs, q, r);
        return q;
    }

    template<int BITS>
    Ump<BITS> operator % (const Ump<BITS>& lhs, const Ump<BITS>& rhs)
    {
        Ump<BITS> q, r;
        lhs.divide(rhs, q, r);
        return r;
    }

    template<int BITS>
    bool operator > (const Ump<BITS>& lhs, const Ump<BITS>& rhs)
    {
        return lhs.compare(rhs) == 1;
    }

    template<int BITS>   bool operator>(const Ump<BITS>& lhs, int rhs)
    {
        return lhs.compare(rhs) == 1;
    }

    template<int BITS>
        bool operator < (const Ump<BITS>& lhs, const Ump<BITS>& rhs)
    {
        return lhs.compare(rhs) == -1;
    }

    template<int BITS>   
    bool operator<(const Ump<BITS>& lhs, int rhs)
    {
        return lhs.compare(rhs) == -1;
    }

    template<int BITS>
    bool operator==(const Ump<BITS>& lhs, const Ump<BITS>& rhs)
    {
        return lhs.compare(rhs) == 0;
    }

    template<int BITS>   
    bool operator==(const Ump<BITS>& lhs, int rhs)
    {
        return lhs.compare(rhs) == 0;
    }

    template<int BITS>
    bool operator>=(const Ump<BITS>& lhs, const Ump<BITS>& rhs)
    {
        return lhs.compare(rhs) >= 0;
    }

    template<int BITS>   
    bool operator>=(const Ump<BITS>& lhs, int rhs)
    {
        return lhs.compare(rhs) >= 0;
    }

    template<int BITS>
    bool operator<=(const Ump<BITS>& lhs, const Ump<BITS>& rhs)
    {
        return lhs.compare(rhs) <= 0;
    }

    template<int BITS>   
    bool operator<=(const Ump<BITS>& lhs, int rhs)
    {
        return lhs.compare(rhs) <= 0;
    }

    template<int BITS>
    bool operator!=(const Ump<BITS>& lhs, const Ump<BITS>& rhs)
    {
        return lhs.compare(rhs) != 0;
    }

    template<int BITS>   
    bool operator!=(const Ump<BITS>& lhs, int rhs)
    {
        return lhs.compare(rhs) != 0;
    }

    template<int BITS>
    void Ump<BITS>::random()
    {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dis;
        for (int i = 0; i <= HIGH_WORD; i++)
        {
            m_limbs[i] = dis(gen);
        }
    }

    //import from a string
    template<int BITS>
    void Ump<BITS>::assign(std::string s)
    {
        for (int i = 0; i < LIMBS; i++)
        {
            m_limbs[i] = 0;
        }

        //to lower case
        std::transform(s.begin(), s.end(), s.begin(),
            [](unsigned char c) { return std::tolower(c); });
        if (s.size() == 0)
            return;
        //assume the string is decimal unless it starts with 0x
        if (s.size() > 2 && s[0] == '0' && s[1] == 'x')
        {
            //remove the 0x
            s.erase(0, 2);

            //if there are an odd number of characters, prepend a 0
            if (s.size() % 2 == 1)
            {
                s.insert(0, 1, '0');
            }
            int limb_index = 0;
            int byte_index = 0;
            //read hex string
            for (int i = s.size()-2; i >= 0; i-=2)
            {
                //process one byte at a time starting from the end of the string
                std::string byteString = s.substr(i, 2);
                limb_t mask = strtol(byteString.c_str(), NULL, 16);
                mask = mask << (byte_index * 8);
                m_limbs[limb_index] = m_limbs[limb_index] | mask;
                byte_index = (byte_index + 1) % BYTES_PER_WORD;
                limb_index += byte_index == 0 ? 1 :0;
                if (limb_index > HIGH_WORD)
                    break;
            }
        }
        else
        {
            //decimal string
            //read each digit, multiply by a power of 10 and accumulate
            Ump<BITS> ten_power = 1;
            for (int i = s.size() - 1; i >= 0; i--)
            {
                std::string digit_string = s.substr(i, 1);
                uint32_t digit = strtol(digit_string.c_str(), NULL, 10);
                Ump<BITS> t;
                t = ten_power * digit;
                increment(t);
                ten_power = ten_power * 10;
            }
        }

    }



}
