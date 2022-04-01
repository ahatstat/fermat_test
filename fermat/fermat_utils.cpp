#include "fermat_utils.hpp"
namespace ump {

    //find the modular mutiplicative inverse of d mod 2^32
    //using newtown's method.  See Hackers' Delight 10-16
    //d must be odd
    uint32_t mod_inverse_32(uint32_t d)
    {
        uint32_t xn, t;
        //initialize the estimate so that it is correct to 4 bits
        xn = d * d + d - 1;
        //for 32 bits the solution should converge after 3 iterations max (7 multiplies total)
        for (auto i = 0; i < 3; i++)
        {
            t = d * xn;
            xn = xn * (2 - t);
        }
        return xn;

    }

    //64 bit mod inverse.  Same algorithm as 32 bits but requires one more iteration
    uint64_t mod_inverse_64(uint64_t d)
    {
        uint64_t xn, t;
        //initialize the estimate so that it is correct to 4 bits
        xn = d * d + d - 1;
        //for 64 bits the solution should converge after 4 iterations max
        for (auto i = 0; i < 4; i++)
        {
            t = d * xn;
            xn = xn * (2 - t);
        }
        return xn;
    }



}
