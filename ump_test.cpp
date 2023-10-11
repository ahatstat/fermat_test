#include "ump_test.hpp"
#include <chrono>
namespace ump {

    bool Ump_test::conversion_test()
    {
        bool pass = true;
        Ump<1024> a1, b1;
        boost::multiprecision::uint1024_t aa1 = 0, bb1 = 0;
        b1 = boost_uint1024_t_to_ump(bb1);
        aa1 = ump_to_boost_uint1024_t(a1);
        pass = (aa1 == bb1);
        pass = pass && (a1 == b1);
        bb1 = bb1 - 1;
        a1 = a1 - 1;
        a1.clear_extra_words();
        b1 = boost_uint1024_t_to_ump(bb1);
        aa1 = ump_to_boost_uint1024_t(a1);
        pass = pass && (aa1 == bb1);
        pass = pass && (a1 == b1);
        a1.random();
        aa1 = ump_to_boost_uint1024_t(a1);
        b1 = boost_uint1024_t_to_ump(aa1);
        bb1 = ump_to_boost_uint1024_t(b1);
        pass = pass && (a1 == b1);
        pass = pass && (aa1 == bb1);
        std::string s1 = "1";
        std::string s2 = "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
        std::string s3 = "1234567890123456789";
        std::string s4 = "0xeed0ea40daaec1031b6c8172b9c3714846a8784736de503369e58e9c25499cb5a034c76ec59511778affe2150ae1e07623d5418a6c2132303a22fe599add9e12ad6b5434b5fd21a84befce066758dc418832b01fce21d6be1b4519e3f3d5b9bff3effd9ba963847ffb95c8c88ea3b854bfa576e6d63badc99cbc114adb27122";
        std::string s5 = "0xfe0584cba13b76ba4d468a779f158f2f6d2a4ba21e19383cff0ab2918db49f4a52ac1550fcdc3a600efedf7c47883a8f61425118dc7679b50291fafe0115315406c855e074ed3c78bbaf1d3ad74ea046a682900a01e10b810b795381c0c971222d88b8b27ad13de26129110c97be2aaf5807a90c7151ee383c01808f94faf5df";
        std::string s6 = "1234567890123456789999999999999999987654321";
        a1.assign(s1);
        aa1.assign(s1);
        b1 = boost_uint1024_t_to_ump(aa1);
        pass = pass && (a1 == b1);
        if (a1 != b1) std::cout << a1.to_str() << std::endl << b1.to_str() << std::endl;
        a1.assign(s2);
        aa1.assign(s2);
        b1 = boost_uint1024_t_to_ump(aa1);
        pass = pass && (a1 == b1);
        if (a1 != b1) std::cout << a1.to_str() << std::endl << b1.to_str() << std::endl;
        a1.assign(s3);
        aa1.assign(s3);
        b1 = boost_uint1024_t_to_ump(aa1);
        pass = pass && (a1 == b1);
        if (a1 != b1) std::cout << a1.to_str() << std::endl << b1.to_str() << std::endl;
        a1.assign(s4);
        aa1.assign(s4);
        b1 = boost_uint1024_t_to_ump(aa1);
        pass = pass && (a1 == b1);
        if (a1 != b1) std::cout << a1.to_str() << std::endl << b1.to_str() << std::endl;
        a1.assign(s5);
        aa1.assign(s5);
        b1 = boost_uint1024_t_to_ump(aa1);
        pass = pass && (a1 == b1);
        if (a1 != b1) std::cout << a1.to_str() << std::endl << b1.to_str() << std::endl;
        a1.assign(s6);
        aa1.assign(s6);
        b1 = boost_uint1024_t_to_ump(aa1);
        pass = pass && (a1 == b1);
        if (a1 != b1) std::cout << a1.to_str() << std::endl << b1.to_str() << std::endl;
        a1.assign(s5);
        uint32_t low_word = static_cast<uint32_t>(a1.m_limbs[0]);
        uint32_t casted = static_cast<uint32_t>(a1);
        pass = pass && (low_word == casted);
        a1.assign(s2);
        int casted_int = static_cast<int>(a1);
        pass = pass && (casted_int == -1);

        return pass;
    }

    bool Ump_test::ctz_test()
    {
        static constexpr int sample_size = 100000;
        bool pass = true;

        uint8_t b = 64 + 4;
        int clz = count_leading_zeros(b);
        pass = pass && clz == 1;
        pass = pass && count_trailing_zeros(b) == 2;

        generate_test_vectors_a(sample_size);
        generate_test_vectors_b(sample_size);

        c.resize(sample_size);
        cc.resize(sample_size);
        auto start = std::chrono::steady_clock::now();
        for (auto i = 0; i < sample_size; i++)
        {
            a[i] = a[i] >> (i % 1023);
            c[i] = count_trailing_zeros(a[i]);
        }
        auto end = std::chrono::steady_clock::now();
        auto ump_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        start = std::chrono::steady_clock::now();
        for (auto i = 0; i < sample_size; i++)
        {
            aa[i] = aa[i] >> (i % 1023);
            if (aa[i] == 0)
                cc[i] = 1024;
            else
                cc[i] = boost::multiprecision::lsb(aa[i]);
        }
        end = std::chrono::steady_clock::now();
        auto boost_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        for (auto i = 0; i < sample_size; i++)
        {
            boost::multiprecision::uint1024_t dd = ump_to_boost_uint1024_t(c[i]);
            if (dd != cc[i])
            {
                std::cout << "CTZ Test failed." << std::endl << "Got " << dd << std::endl << "Expected " << cc[i] << std::endl;
                pass = false;
            }
        }
        std::cout << "Count trailing zeros performance Ump " << ump_elapsed.count() * 1.0e6 / sample_size << "[us] Boost " <<
            boost_elapsed.count() * 1.0e6 / sample_size << "[us] " << (double)ump_elapsed.count() / boost_elapsed.count()
            << std::endl;

        return pass;
    }

    bool Ump_test::add_test()
    {
        static constexpr int sample_size = 100000;
        bool pass = true;
       
        generate_test_vectors_a(sample_size);
        generate_test_vectors_b(sample_size);
        
        c.resize(sample_size);
        cc.resize(sample_size);
        auto start = std::chrono::steady_clock::now();
        for (auto i = 0; i < sample_size; i++)
        {
            c[i] = a[i] + b[i];
        }
        auto end = std::chrono::steady_clock::now();
        auto ump_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        start = std::chrono::steady_clock::now();
        for (auto i = 0; i < sample_size; i++)
        {
            cc[i] = aa[i] + bb[i];
        }
        end = std::chrono::steady_clock::now();
        auto boost_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        for (auto i = 0; i < sample_size; i++)
        {
            boost::multiprecision::uint1024_t dd = ump_to_boost_uint1024_t(c[i]);
            if (dd != cc[i])
            {
                std::cout << std::hex << "Add Test failed." << std::endl << "Got " << dd << std::endl << "Expected " << cc[i] << std::endl;
                pass = false;
            }
        }
        std::cout << "Add performance Ump " << ump_elapsed.count() * 1.0e6 / sample_size << "[us] Boost " <<
            boost_elapsed.count() * 1.0e6 / sample_size << "[us] " << (double)ump_elapsed.count() / boost_elapsed.count()
            << std::endl;
        
        return pass;
    }

    bool Ump_test::add_test_ui()
    {
        static constexpr int sample_size = 100000;
        bool pass = true;
       
        generate_test_vectors_a(sample_size);
        generate_test_vectors_b(sample_size);
        
        c.resize(sample_size);
        cc.resize(sample_size);
        auto start = std::chrono::steady_clock::now();
        for (auto i = 0; i < sample_size; i++)
        {
            limb_t word = b[i].m_limbs[0];
            c[i] = a[i] + word;
        }
        auto end = std::chrono::steady_clock::now();
        auto ump_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        start = std::chrono::steady_clock::now();
        for (auto i = 0; i < sample_size; i++)
        {
            limb_t word = b[i].m_limbs[0];
            cc[i] = aa[i] + word;
        }
        end = std::chrono::steady_clock::now();
        auto boost_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        for (auto i = 0; i < sample_size; i++)
        {
            boost::multiprecision::uint1024_t dd = ump_to_boost_uint1024_t(c[i]);
            if (dd != cc[i])
            {
                std::cout << std::hex << "Add UI Test failed." << std::endl << a[i].to_str() << std::endl <<
                    " + " << b[i].m_limbs[0] << std::endl << " = " << c[i].to_str() << std::endl <<
                    "Expected " << cc[i] << std::endl;                
                pass = false;
            }
        }
        std::cout << "Add UI performance Ump " << ump_elapsed.count() * 1.0e6 / sample_size << "[us] Boost " <<
            boost_elapsed.count() * 1.0e6 / sample_size << "[us] " << (double)ump_elapsed.count() / boost_elapsed.count()
            << std::endl;
        return pass;
    }

    bool Ump_test::subtract_test()
    {
        static constexpr int sample_size = 100000;
        bool pass = true;
        
        generate_test_vectors_a(sample_size);
        generate_test_vectors_b(sample_size);
        
        c.resize(sample_size);
        cc.resize(sample_size);
        auto start = std::chrono::steady_clock::now();
        for (auto i = 0; i < sample_size; i++)
        {
            c[i] = a[i] - b[i];
        }
        auto end = std::chrono::steady_clock::now();
        auto ump_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        start = std::chrono::steady_clock::now();
        for (auto i = 0; i < sample_size; i++)
        {
            cc[i] = aa[i] - bb[i];
        }
        end = std::chrono::steady_clock::now();
        auto boost_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        for (auto i = 0; i < sample_size; i++)
        {
            boost::multiprecision::uint1024_t dd = ump_to_boost_uint1024_t(c[i]);
            if (dd != cc[i])
            {
                std::cout << std::hex << "Subtract Test failed." << std::endl << "Got " << dd << std::endl << "Expected " << cc[i] << std::endl;
                pass = false;
            }
        }
        std::cout << "Subtract performance Ump " << ump_elapsed.count() * 1.0e6 / sample_size << "[us] Boost " <<
            boost_elapsed.count() * 1.0e6 / sample_size << "[us] " << (double)ump_elapsed.count() / boost_elapsed.count()
            << std::endl;

        return pass;
    }

    bool Ump_test::negate_test()
    {
        static constexpr int sample_size = 100000;
        bool pass = true;

        generate_test_vectors_a(sample_size);
        generate_test_vectors_b(sample_size);

        c.resize(sample_size);
        cc.resize(sample_size);
        auto start = std::chrono::steady_clock::now();
        for (auto i = 0; i < sample_size; i++)
        {
            c[i] = -b[i];
        }
        auto end = std::chrono::steady_clock::now();
        auto ump_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        start = std::chrono::steady_clock::now();
        for (auto i = 0; i < sample_size; i++)
        {
            cc[i] = 0-bb[i];
        }
        end = std::chrono::steady_clock::now();
        auto boost_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        for (auto i = 0; i < sample_size; i++)
        {
            boost::multiprecision::uint1024_t dd = ump_to_boost_uint1024_t(c[i]);
            if (dd != cc[i])
            {
                std::cout << std::hex << "Subtract Test failed." << std::endl << "Got " << dd << std::endl << "Expected " << cc[i] << std::endl;
                pass = false;
            }
        }
        std::cout << "Negate performance Ump " << ump_elapsed.count() * 1.0e6 / sample_size << "[us] Boost " <<
            boost_elapsed.count() * 1.0e6 / sample_size << "[us] " << (double)ump_elapsed.count() / boost_elapsed.count()
            << std::endl;

        return pass;
    }

    //bignum multiplied by a single word
    bool Ump_test::multiply_test_ui()
    {
        static constexpr int sample_size = 100000;
        bool pass = true;
        
        generate_test_vectors_a(sample_size);
        generate_test_vectors_b(sample_size);
        
        c.resize(sample_size);
        cc.resize(sample_size);
        auto start = std::chrono::steady_clock::now();
        for (auto i = 0; i < sample_size; i++)
        {
            limb_t word = b[i].m_limbs[0];
            c[i] = a[i] * word;
        }
        auto end = std::chrono::steady_clock::now();
        auto ump_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        start = std::chrono::steady_clock::now();
        for (auto i = 0; i < sample_size; i++)
        {
            limb_t word = b[i].m_limbs[0];
            cc[i] = aa[i] * word;
        }
        end = std::chrono::steady_clock::now();
        auto boost_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        for (auto i = 0; i < sample_size; i++)
        {
            boost::multiprecision::uint1024_t dd = ump_to_boost_uint1024_t(c[i]);
            if (dd != cc[i])
            {
                std::cout << std::hex << "Multiply UI Test failed." << std::endl << a[i].to_str() << std::endl <<
                    " * " << b[i].m_limbs[0] << std::endl << " = " << c[i].to_str() << std::endl <<
                    "Expected " << cc[i] << std::endl;
                pass = false;
            }
        }
        std::cout << "Multiply UI performance Ump " << ump_elapsed.count() * 1.0e6 / sample_size << "[us] Boost " <<
            boost_elapsed.count() * 1.0e6 / sample_size << "[us] " << (double)ump_elapsed.count() / boost_elapsed.count()
            << std::endl;
        return pass;
    }

    bool Ump_test::multiply_test()
    {
        static constexpr int sample_size = 100000;
        bool pass = true;

        generate_test_vectors_a(sample_size);
        generate_test_vectors_b(sample_size);

        c.resize(sample_size);
        cc.resize(sample_size);
        auto start = std::chrono::steady_clock::now();
        for (auto i = 0; i < sample_size; i++)
        {
            c[i] = a[i] * b[i];
        }
        auto end = std::chrono::steady_clock::now();
        auto ump_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        start = std::chrono::steady_clock::now();
        for (auto i = 0; i < sample_size; i++)
        {
            cc[i] = aa[i] * bb[i];
        }
        end = std::chrono::steady_clock::now();
        auto boost_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        for (auto i = 0; i < sample_size; i++)
        {
            boost::multiprecision::uint1024_t dd = ump_to_boost_uint1024_t(c[i]);
            if (dd != cc[i])
            {
                std::cout << std::hex << "Multiply Test failed." << std::endl << a[i].to_str() << std::endl <<
                    " * " << b[i].m_limbs[0] << std::endl << " = " << c[i].to_str() << std::endl <<
                    "Expected " << cc[i] << std::endl;
                pass = false;
            }
        }
        std::cout << "Multiply performance Ump " << ump_elapsed.count() * 1.0e6 / sample_size << "[us] Boost " <<
            boost_elapsed.count() * 1.0e6 / sample_size << "[us] " << (double)ump_elapsed.count() / boost_elapsed.count()
            << std::endl;
        return pass;
    }

    //n bits * n bits = 2n bits
    bool Ump_test::multiply_full_test()
    {
        static constexpr int sample_size = 100000;
        bool pass = true;

        generate_test_vectors_a(sample_size);
        generate_test_vectors_b(sample_size);

        c.resize(sample_size);
        cc.resize(sample_size);
        d.resize(sample_size);
        d1.resize(sample_size);

        auto start = std::chrono::steady_clock::now();
        for (auto i = 0; i < sample_size; i++)
        {
            c[i] = a[i].multiply(d[i], b[i]);
        }
        auto end = std::chrono::steady_clock::now();
        auto ump_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        start = std::chrono::steady_clock::now();
        for (auto i = 0; i < sample_size; i++)
        {
            cc[i] = aa[i] * bb[i];
            boost::multiprecision::mpz_int wide = static_cast<boost::multiprecision::mpz_int>(aa[i]) *
                static_cast<boost::multiprecision::mpz_int>(bb[i]);
            d1[i] = static_cast<boost::multiprecision::uint1024_t>(wide >> 1024);
        }
        end = std::chrono::steady_clock::now();
        auto boost_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        for (auto i = 0; i < sample_size; i++)
        {
            boost::multiprecision::uint1024_t dd = ump_to_boost_uint1024_t(c[i]);
            if (dd != cc[i])
            {
                std::cout << std::hex << "Multiply Full Test failed for the low half." << std::endl << a[i].to_str() << std::endl <<
                    " * " << b[i].m_limbs[0] << std::endl << " = " << c[i].to_str() << std::endl <<
                    "Expected " << cc[i] << std::endl;
                pass = false;
            }
        }
        for (auto i = 0; i < sample_size; i++)
        {
            boost::multiprecision::uint1024_t dd = ump_to_boost_uint1024_t(d[i]);
            if (dd != d1[i])
            {
                std::cout << std::hex << "Multiply Full Test failed for the high half." << std::endl << a[i].to_str() << std::endl <<
                    " * " << b[i].m_limbs[0] << std::endl << " = " << d[i].to_str() << std::endl <<
                    "Expected " << d1[i] << std::endl;
                pass = false;
            }
        }
        std::cout << "Multiply Full performance Ump " << ump_elapsed.count() * 1.0e6 / sample_size << "[us] Boost " <<
            boost_elapsed.count() * 1.0e6 / sample_size << "[us] " << (double)ump_elapsed.count() / boost_elapsed.count()
            << std::endl;
        return pass;
    }

    //128 bit by 128 bit division
    bool Ump_test::divide_128_test()
    {
        static constexpr int sample_size = 100000;
        bool pass = true;

        generate_test_vectors_a(sample_size);
        generate_test_vectors_b(sample_size);

        c.resize(sample_size);
        cc.resize(sample_size);
        d.resize(sample_size);
        d1.resize(sample_size);

        auto start = std::chrono::steady_clock::now();
        for (auto i = 0; i < sample_size; i++)
        {
            a[i] = (a[i] << (1024 - 128)) >> (1024 - 128);
            b[i] = (b[i] << (1024 - 128)) >> (1024 - 128);
            Ump<128> a128, b128, q128, r128;
            a128.m_limbs[0] = a[i].m_limbs[0];
            a128.m_limbs[1] = a[i].m_limbs[1];
            b128.m_limbs[0] = b[i].m_limbs[0];
            b128.m_limbs[1] = b[i].m_limbs[1];
            if (b128 != 0)
                a128.divide(b128, q128, r128);
            c[i] = 0;
            c[i].m_limbs[0] = q128.m_limbs[0];
            c[i].m_limbs[1] = q128.m_limbs[1];
            d[i] = 0;
            d[i].m_limbs[0] = r128.m_limbs[0];
            d[i].m_limbs[1] = r128.m_limbs[1];

        }
        auto end = std::chrono::steady_clock::now();
        auto ump_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        start = std::chrono::steady_clock::now();
        for (auto i = 0; i < sample_size; i++)
        {
            boost::multiprecision::uint128_t aa128 = static_cast<boost::multiprecision::uint128_t>(aa[i]);
            boost::multiprecision::uint128_t bb128 = static_cast<boost::multiprecision::uint128_t>(bb[i]);
            boost::multiprecision::uint128_t q128, r128;
            if (bb128 != 0)
            {
                q128 = aa128 / bb128;
                r128 = aa128 % bb128;
            }
            cc[i] = static_cast<boost::multiprecision::uint1024_t>(q128);
            d1[i] = static_cast<boost::multiprecision::uint1024_t>(r128);
        }
        end = std::chrono::steady_clock::now();
        auto boost_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        for (auto i = 0; i < sample_size; i++)
        {
            boost::multiprecision::uint1024_t dd = ump_to_boost_uint1024_t(c[i]);
            if (dd != cc[i])
            {
                std::cout << std::hex << "Divide 128 Test failed for the quotient." << std::endl << a[i].to_str() << std::endl <<
                    " / " << b[i].to_str() << std::endl << " = " << c[i].to_str() << std::endl <<
                    "Expected " << cc[i] << std::endl;
                pass = false;
            }
        }
        for (auto i = 0; i < sample_size; i++)
        {
            boost::multiprecision::uint1024_t dd = ump_to_boost_uint1024_t(d[i]);
            if (dd != d1[i])
            {
                std::cout << std::hex << "Divide 128 Test failed for remainder." << std::endl << a[i].to_str() << std::endl <<
                    " % " << b[i].to_str() << std::endl << " = " << d[i].to_str() << std::endl <<
                    "Expected " << d1[i] << std::endl;
                pass = false;
            }
        }
        std::cout << "Divide 128 performance Ump " << ump_elapsed.count() * 1.0e6 / sample_size << "[us] Boost " <<
            boost_elapsed.count() * 1.0e6 / sample_size << "[us] " << (double)ump_elapsed.count() / boost_elapsed.count()
            << std::endl;
        return pass;
    }

    bool Ump_test::divide_test()
    {
        static constexpr int sample_size = 100000;
        bool pass = true;

        generate_test_vectors_a(sample_size);
        generate_test_vectors_b(sample_size);

        c.resize(sample_size);
        cc.resize(sample_size);
        d.resize(sample_size);
        d1.resize(sample_size);

        auto start = std::chrono::steady_clock::now();
        for (auto i = 0; i < sample_size; i++)
        {
            //add some sizze by making the denominator smaller
            if (i > 20)
            {
                b[i] = b[i] >> (7 * i % 1023);
                bb[i] = bb[i] >> (7 * i % 1023);
            }
            if (i == 50)
            {
                a[i] = b[i];
                aa[i] = bb[i];
            }
            
            if (b[i] != 0)
                a[i].divide(b[i], c[i], d[i]);
        }
        auto end = std::chrono::steady_clock::now();
        auto ump_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        start = std::chrono::steady_clock::now();
        for (auto i = 0; i < sample_size; i++)
        {
           
            if (bb[i] != 0)
            {
                cc[i] = aa[i] / bb[i];
                d1[i] = aa[i] % bb[i];
            }
        }
        end = std::chrono::steady_clock::now();
        auto boost_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        for (auto i = 0; i < sample_size; i++)
        {
            boost::multiprecision::uint1024_t dd = ump_to_boost_uint1024_t(c[i]);
            if (dd != cc[i])
            {
                std::cout << std::hex << "Divide Test failed the quotient." << std::endl << a[i].to_str() << std::endl <<
                    " / " << b[i].to_str() << std::endl << " = " << c[i].to_str() << std::endl <<
                    "Expected " << cc[i] << std::endl;
                pass = false;
            }
        }
        for (auto i = 0; i < sample_size; i++)
        {
            boost::multiprecision::uint1024_t dd = ump_to_boost_uint1024_t(d[i]);
            if (dd != d1[i])
            {
                std::cout << std::hex << "Divide Test failed for the remainder." << std::endl << a[i].to_str() << std::endl <<
                    " % " << b[i].to_str() << std::endl << " = " << d[i].to_str() << std::endl <<
                    "Expected " << d1[i] << std::endl;
                pass = false;
            }
        }
        std::cout << "Divide performance Ump " << ump_elapsed.count() * 1.0e6 / sample_size << "[us] Boost " <<
            boost_elapsed.count() * 1.0e6 / sample_size << "[us] " << (double)ump_elapsed.count() / boost_elapsed.count()
            << std::endl;
        return pass;
    }

    bool Ump_test::mod_inv_test()
    {
        static constexpr int sample_size = 1000;
        bool pass = true;
        
        generate_test_vectors_a(sample_size);
        generate_test_vectors_b(sample_size);
        
        c.resize(sample_size);
        cc.resize(sample_size);
        for (auto i = 0; i < sample_size; i++)
        {
            uint64_t word = b[i].m_limbs[0];
            //make it odd
            word = word | 1;
            uint64_t modinv = mod_inverse_64(word);
            uint64_t test = word * modinv;
            if (test != 1)
            {
                pass = false;
            }
        }
        
        return pass;
    }

    bool Ump_test::montgomery_square_test()
    {
        static constexpr int sample_size = 100;
        bool pass = true;
        
        generate_test_vectors_a(sample_size);
        generate_test_vectors_b(sample_size);
        
        c.resize(sample_size);
        cc.resize(sample_size);
        for (auto i = 0; i < sample_size; i++)
        {
            //b[i] is the modulus
            //make it odd
            b[i].m_limbs[0] |= 1;
            uint64_t m_primed = -mod_inverse_64(b[i].m_limbs[0]);
            Ump<1024> rmodm = b[i].R_mod_m();  //this is the equivalent of 1 in the montgomery domain mod m
            Ump<1024> temp = rmodm;
            montgomery_square(temp, b[i], m_primed); //one squared is one
            if (rmodm != temp)
            {
                pass = false;
            }
        }

        return pass;
    }

    bool Ump_test::double_and_reduce_test()
    {
        static constexpr int sample_size = 100;
        bool pass = true;
        generate_test_vectors_a(sample_size);
        generate_test_vectors_b(sample_size);
        c.resize(sample_size);
        cc.resize(sample_size);
        for (auto i = 0; i < sample_size; i++)
        {
            //b[i] is the modulus.  ensure it is non-zero
            if (bb[i] == 0) bb[i] = 1;
            //start with a[i] already reduced (less than the modulus)
            aa[i] = aa[i] % bb[i];
            a[i] = boost_uint1024_t_to_ump(aa[i]);
            b[i] = boost_uint1024_t_to_ump(bb[i]);
            c[i] = double_and_reduce(a[i], b[i], 1);
            boost::multiprecision::mpz_int x = (static_cast<boost::multiprecision::mpz_int>(aa[i]) * 2) % 
                static_cast<boost::multiprecision::mpz_int>(bb[i]);
            cc[i] = static_cast<boost::multiprecision::uint1024_t>(x);
            boost::multiprecision::uint1024_t dd = ump_to_boost_uint1024_t(c[i]);
            if (dd != cc[i])
            {
                pass = false;
            }
            c[i] = double_and_reduce(a[i], b[i], 4);
            x = (static_cast<boost::multiprecision::mpz_int>(aa[i]) * 16) %
                static_cast<boost::multiprecision::mpz_int>(bb[i]);
            cc[i] = static_cast<boost::multiprecision::uint1024_t>(x);
            dd = ump_to_boost_uint1024_t(c[i]);
            if (dd != cc[i])
            {
                pass = false;
            }
        }
        return pass;
    }

    bool Ump_test::powm_2_test()
    {
        static constexpr int sample_size = 100;
        bool pass = true;

        generate_test_vectors_a(sample_size);
        generate_test_vectors_b(sample_size);

        c.resize(sample_size);
        cc.resize(sample_size);
        for (auto i = 0; i < sample_size; i++)
        {
            //a[i] is the offset
            a[i].make_odd();
            a[i] += 1;  //a is even
            //b[i] is base big int
            b[i].make_odd();
            bb[i] = (bb[i] | 1) + a[i].m_limbs[0];
            c[i] = powm_2(b[i], a[i].m_limbs[0]); 
            boost::multiprecision::uint1024_t two = 2;
            cc[i] = boost::multiprecision::powm(two, bb[i] - 1, bb[i]);
            boost::multiprecision::uint1024_t dd = ump_to_boost_uint1024_t(c[i]);
            if (dd != cc[i])
            {
                pass = false;
            }
        }

        return pass;
    }

    void Ump_test::generate_test_vectors_a(int test_vector_size)
    {
        a = {};
        aa = {};
        Ump<1024> d;
        boost::multiprecision::uint1024_t dd;
        a.push_back(d);  //0
        dd = ump_to_boost_uint1024_t(d);
        aa.push_back(dd);
        d = d - 1;  //0xFFF...
        d.m_limbs[d.HIGH_WORD + 1] = 0;
        a.push_back(d);
        dd = ump_to_boost_uint1024_t(d);
        aa.push_back(dd);
        d.assign("1");
        a.push_back(d);
        dd = ump_to_boost_uint1024_t(d);
        aa.push_back(dd);
        d.assign("0xFFFFFFFFFFFFFFFF");
        a.push_back(d);
        dd = ump_to_boost_uint1024_t(d);
        aa.push_back(dd);
        d.assign("0");
        a.push_back(d);
        dd = ump_to_boost_uint1024_t(d);
        aa.push_back(dd);

        for (int i = a.size(); i < test_vector_size; i++)
        {
            Ump<1024> r;
            r.random();
            a.push_back(r);
            boost::multiprecision::uint1024_t rr;
            rr = ump_to_boost_uint1024_t(r);
            aa.push_back(rr);
            
        }
    }

    void Ump_test::generate_test_vectors_b(int test_vector_size)
    {
        b = {};
        bb = {};
        Ump<1024> d;
        boost::multiprecision::uint1024_t dd;
        b.push_back(d);  //0
        dd = ump_to_boost_uint1024_t(d);
        bb.push_back(dd);
        d.assign("1");
        b.push_back(d);
        dd = ump_to_boost_uint1024_t(d);
        bb.push_back(dd);
        d = d - 1;  //0xFFF...
        d.m_limbs[d.HIGH_WORD + 1] = 0;
        b.push_back(d);
        dd = ump_to_boost_uint1024_t(d);
        bb.push_back(dd);
        d.assign("0xFFFFFFFFFFFFFFFF");
        b.push_back(d);
        dd = ump_to_boost_uint1024_t(d);
        bb.push_back(dd);
        d.assign("1");
        b.push_back(d);
        dd = ump_to_boost_uint1024_t(d);
        bb.push_back(dd);

        for (int i = b.size(); i < test_vector_size; i++)
        {
            Ump<1024> r;
            r.random();
            b.push_back(r);
            boost::multiprecision::uint1024_t rr;
            rr = ump_to_boost_uint1024_t(r);
            bb.push_back(rr);
           
        }
    }

    boost::multiprecision::uint1024_t ump_to_boost_uint1024_t(Ump<1024> a)
    {
        boost::multiprecision::uint1024_t b, c;
        c.assign(0);
        for (int i = 0; i <= a.HIGH_WORD; i++)
        {
            uint64_t limb = a.m_limbs[i];
            b.assign(limb);
            b = b << (i * BITS_PER_WORD);
            c += b;
        }
        return c;
    }

    Ump<1024> boost_uint1024_t_to_ump(boost::multiprecision::uint1024_t a)
    {
        Ump<1024> c;
        limb_t mask = -1;
        for (int i = 0; i <= c.HIGH_WORD; i++)
        {
            //get the lowest word
            uint64_t limb = static_cast<uint64_t>(a) & mask;
            c.m_limbs[i] = limb;
            a = a >> sizeof(mask)*8;
        }
        return c;

    }



}
