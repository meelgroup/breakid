/* Wrapper for BreakID
 *
 *
 */

#include "breakid_c.h"
#include "breakid.hpp"
#include "constants.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <cassert>
#include <limits.h>

using namespace BID;

template<typename T>
std::vector<T> wrap(const T* vals, size_t num_vals)
{
    return std::vector<T>(vals, vals + num_vals);
}

template<typename Dest, typename T>
Dest unwrap(const std::vector<T>& vec)
{
    return Dest {toc(vec.data()), vec.size()};
}

#define NOEXCEPT_START noexcept { try {
#define NOEXCEPT_END } catch(...) { \
    std::cerr << "ERROR: exception thrown past FFI boundary" << std::endl;\
    std::exit(-1);\
} }

extern "C"
{
    //create and destroy
    DLL_PUBLIC BreakID* breakid_new(void) NOEXCEPT_START {
            return new BreakID();
    } NOEXCEPT_END

    DLL_PUBLIC void breakid_del(BreakID* breakid) NOEXCEPT_START {
            delete breakid;
    } NOEXCEPT_END


    //configuration functions
    DLL_PUBLIC void breakid_set_useMatrixDetection(BreakID* bid, bool val) NOEXCEPT_START {
            bid->set_useMatrixDetection(val);
    } NOEXCEPT_END

    DLL_PUBLIC void breakid_set_useBinaryClauses(BreakID* bid, bool val) NOEXCEPT_START {
            bid->set_useBinaryClauses(val);
    } NOEXCEPT_END

    DLL_PUBLIC void breakid_set_useShatterTranslation(BreakID* bid, bool val) NOEXCEPT_START {
            bid->set_useShatterTranslation(val);
    } NOEXCEPT_END

    DLL_PUBLIC void breakid_set_useFullTranslation(BreakID* bid, bool val) NOEXCEPT_START {
            bid->set_useFullTranslation(val);
    } NOEXCEPT_END

    DLL_PUBLIC void breakid_set_symBreakingFormLength(BreakID* bid, int val) NOEXCEPT_START {
            bid->set_symBreakingFormLength(val);
    } NOEXCEPT_END

    DLL_PUBLIC void breakid_set_verbosity(BreakID* bid, uint32_t val) NOEXCEPT_START {
            bid->set_verbosity(val);
    } NOEXCEPT_END

    DLL_PUBLIC void breakid_set_steps_lim(BreakID* bid, int64_t val) NOEXCEPT_START {
            bid->set_steps_lim(val);
    } NOEXCEPT_END


    //Print info
    DLL_PUBLIC void breakid_print_subgroups(BreakID* bid) NOEXCEPT_START {
        bid->print_subgroups(std::cout);
    } NOEXCEPT_END

    DLL_PUBLIC void breakid_print_symm_break_stats(BreakID* bid) NOEXCEPT_START {
        bid->print_symm_break_stats();
    } NOEXCEPT_END

    DLL_PUBLIC void breakid_print_perms_and_matrices(BreakID* bid) NOEXCEPT_START {
        bid->print_perms_and_matrices(std::cout);
    } NOEXCEPT_END

    DLL_PUBLIC void breakid_print_generators(BreakID* bid) NOEXCEPT_START {
        bid->print_generators(std::cout);
    } NOEXCEPT_END


    //Get info
    DLL_PUBLIC uint32_t breakid_get_num_generators(BreakID* bid) NOEXCEPT_START {
        return bid->get_num_generators();
    } NOEXCEPT_END

    DLL_PUBLIC uint32_t breakid_get_num_break_cls(BreakID* bid) NOEXCEPT_START {
        return bid->get_num_break_cls();
    } NOEXCEPT_END

    DLL_PUBLIC uint32_t breakid_get_num_aux_vars(BreakID* bid) NOEXCEPT_START {
        return bid->get_num_aux_vars();
    } NOEXCEPT_END

    DLL_PUBLIC int64_t  breakid_get_steps_remain(BreakID* bid) NOEXCEPT_START {
        return bid->get_steps_remain();
    } NOEXCEPT_END

    DLL_PUBLIC uint64_t breakid_get_num_subgroups(BreakID* bid) NOEXCEPT_START {
        return bid->get_num_subgroups();
    } NOEXCEPT_END

    //Dynamic CNF
    DLL_PUBLIC void breakid_start_dynamic_cnf(BreakID* bid, uint32_t nVars) NOEXCEPT_START {
        bid->start_dynamic_cnf(nVars);
    } NOEXCEPT_END

    DLL_PUBLIC void breakid_add_clause(BreakID* bid, int* start, size_t num) NOEXCEPT_START {
        bid->add_clause((BID::BLit*)start, num);
    } NOEXCEPT_END

    DLL_PUBLIC void breakid_end_dynamic_cnf(BreakID* bid) NOEXCEPT_START {
        bid->end_dynamic_cnf();
    } NOEXCEPT_END

    //Main functions
    DLL_PUBLIC void breakid_detect_subgroups(BreakID* bid) NOEXCEPT_START {
        bid->detect_subgroups();
    } NOEXCEPT_END

    DLL_PUBLIC void breakid_break_symm(BreakID* bid) NOEXCEPT_START {
        bid->break_symm();
    } NOEXCEPT_END

    DLL_PUBLIC unsigned* breakid_get_brk_cls(BreakID* bid, int* num) NOEXCEPT_START {
       auto brk = bid->get_brk_cls();
       *num = brk.size();
       size_t total_sz = 0;
       for (auto cl: brk) {
           total_sz += cl.size()+1;
       }
       unsigned* cls_ptr = (unsigned*) malloc(total_sz * sizeof(int));
       unsigned* at = cls_ptr;
       for (auto cl: brk) {
            for(auto lit: cl) {
                *at = lit.toInt();
                at++;
            }
            *at = UINT_MAX;
            at++;
        }
       return at;
    } NOEXCEPT_END
}
