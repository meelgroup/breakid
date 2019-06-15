/******************************************
Copyright (c) 2014, Mate Soos

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
***********************************************/

#ifndef BOSPH_SOLVERTYPESMINI_H_
#define BOSPH_SOLVERTYPESMINI_H_


#include <cstdint>
#include <iostream>

namespace BID {

class BLit
{
    uint32_t x;
    explicit BLit(uint32_t i) : x(i) { }
public:
    BLit() : x((0xffffffffU >> 3)<<1) {}   // (lit_Undef)
    explicit BLit(uint32_t var, bool is_inverted) :
        x((var<<1) | (uint32_t)is_inverted)
    {}

    const uint32_t& toInt() const { // Guarantees small, positive integers suitable for array indexing.
        return x;
    }
    BLit  operator~() const {
        return BLit(x ^ 1);
    }
    BLit  operator^(const bool b) const {
        return BLit(x ^ (uint32_t)b);
    }
    BLit& operator^=(const bool b) {
        x ^= (uint32_t)b;
        return *this;
    }
    bool sign() const {
        return x & 1;
    }
    uint32_t  var() const {
        return x >> 1;
    }
    BLit  unsign() const {
        return BLit(x & ~1U);
    }
    bool operator==(const BLit& p) const {
        return x == p.x;
    }
    bool operator!= (const BLit& p) const {
        return x != p.x;
    }
    /**
    @brief ONLY to be used for ordering such as: a, b, ~b, etc.
    */
    bool operator <  (const BLit& p) const {
        return x < p.x;     // '<' guarantees that p, ~p are adjacent in the ordering.
    }
    bool operator >  (const BLit& p) const {
        return x > p.x;
    }
    bool operator >=  (const BLit& p) const {
        return x >= p.x;
    }
    static BLit toBLit(uint32_t data)
    {
        return BLit(data);
    }
};

static const BLit BLit_Undef = BLit::toBLit((0xffffffffU >> 3)<<1);

inline std::ostream& operator<<(std::ostream& os, const BLit lit)
{

    os << (lit.sign() ? "-" : "") << (lit.var() + 1);
    return os;
}

}

namespace std {

  template <>
  struct hash<BID::BLit>
  {
    std::size_t operator()(const BID::BLit& k) const
    {
      return k.toInt();
    }
  };

}

#endif //__SOLVERTYPESMINI_H__
