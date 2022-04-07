#include "fermat_prime.hpp"
#include <cstdint>
#include <omp.h>

    std::vector<bool> Fermat_prime::fermat_run()
    {
        std::vector<bool> results(m_offsets.size(), false);
#pragma omp parallel for
        for (int i=0; i< (int)m_offsets.size(); i++)
        {
            bool is_prime = ump::fermat_test_base_2(m_base_int, m_offsets[i]);
            results[i] = is_prime;
        }
        return results;
    }

    void Fermat_prime::set_base_int(std::string base_big_int)
    {
        m_base_int.assign(base_big_int);
    }


    void Fermat_prime::set_offsets(const std::vector<uint64_t>& offsets)
    {
        m_offsets = offsets;
    }



