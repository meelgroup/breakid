/******************************************
Copyright (c) 2010-2019 Jo Devriendt
Copyright (c) 2010-2019 Bart Bogaerts
Copyright (c) 2019-2024 Mate Soos

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

#pragma once

#if defined _WIN32
    #define DLL_PUBLIC __declspec(dllexport)
#else
    #define DLL_PUBLIC __attribute__ ((visibility ("default")))
    #define DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#endif

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <cstdint>

namespace BID {

struct PrivateData;

class BLit
{
    uint32_t x;
    constexpr explicit BLit(uint32_t i) : x(i) { }
public:
    constexpr BLit() : x((0xffffffffU >> 3)<<1) {}   // (lit_Undef)
    constexpr explicit BLit(uint32_t var, bool is_inverted) :
        x((var<<1) | (uint32_t)is_inverted)
    {}

    constexpr const uint32_t& toInt() const { return x; }
    constexpr BLit  operator~() const { return BLit(x ^ 1); }
    constexpr BLit  operator^(const bool b) const { return BLit(x ^ (uint32_t)b); }
    constexpr BLit& operator^=(const bool b) {
        x ^= (uint32_t)b;
        return *this;
    }
    constexpr bool sign() const { return x & 1; }
    constexpr uint32_t  var() const { return x >> 1; }
    constexpr BLit  unsign() const { return BLit(x & ~1U); }
    constexpr bool operator==(const BLit& p) const { return x == p.x; }
    constexpr bool operator!= (const BLit& p) const { return x != p.x; }

    constexpr bool operator <  (const BLit& p) const { return x < p.x; }
    constexpr bool operator >  (const BLit& p) const { return x > p.x; }
    constexpr bool operator >=  (const BLit& p) const { return x >= p.x; }
    constexpr static BLit toBLit(uint32_t data) { return BLit(data);
    }
};

constexpr BLit BLit_Undef = BLit::toBLit((0xffffffffU >> 3)<<1);

DLL_PUBLIC inline std::ostream& operator<<(std::ostream& os, const BLit lit)
{

    os << (lit.sign() ? "-" : "") << (lit.var() + 1);
    return os;
}

struct BreakID {
    DLL_PUBLIC BreakID();
    DLL_PUBLIC ~BreakID();

    DLL_PUBLIC static std::string get_sha1_version();

    //configuration functions
    DLL_PUBLIC void set_useMatrixDetection(bool val);
    DLL_PUBLIC void set_useBinaryClauses(bool val);
    DLL_PUBLIC void set_useShatterTranslation(bool val);
    DLL_PUBLIC void set_useFullTranslation(bool val);
    DLL_PUBLIC void set_symBreakingFormLength(int val);
    DLL_PUBLIC void set_verbosity(uint32_t val);
    DLL_PUBLIC void set_steps_lim(int64_t val);

    //Dynamic CNF
    DLL_PUBLIC void start_dynamic_cnf(uint32_t nVars);
    DLL_PUBLIC void add_clause(BID::BLit* start, size_t num);
    DLL_PUBLIC void end_dynamic_cnf();

    //Main functions
    DLL_PUBLIC void detect_subgroups();
    DLL_PUBLIC void break_symm();

    //Print info
    DLL_PUBLIC void print_subgroups(std::ostream& out, const char* prefix = "c ");
    DLL_PUBLIC void print_symm_break_stats(const char* prefix = "c ");
    DLL_PUBLIC void print_perms_and_matrices(std::ostream& out, const char* prefix = "c ");
    DLL_PUBLIC void print_generators(std::ostream& out, const char* prefix = "c ");

    //Get info
    DLL_PUBLIC uint32_t get_num_generators();
    DLL_PUBLIC uint32_t get_num_break_cls();
    DLL_PUBLIC uint32_t get_num_aux_vars();
    DLL_PUBLIC int64_t get_steps_remain() const;
    DLL_PUBLIC uint64_t get_num_subgroups() const;
    DLL_PUBLIC std::vector<std::vector<BID::BLit>> get_brk_cls();
    DLL_PUBLIC void get_perms(std::vector<std::unordered_map<BLit, BLit> >* out);

private:
    BID::PrivateData* dat = NULL;
};

}

namespace std {
  template <> struct hash<BID::BLit> {
    std::size_t operator()(const BID::BLit& k) const {
      return k.toInt();
    }
  };
}
