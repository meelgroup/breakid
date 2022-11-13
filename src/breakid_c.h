/* C header for BreakID
 *
 *
 */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
    #define NOEXCEPT noexcept

    namespace BID { struct BreakID; }
    using BID::BreakID;

    extern "C" {
#else
    // c stuff
    #include <stdbool.h>
    #define NOEXCEPT

    // forward declaration
    typedef struct BreakID BreakID;
#endif

#if defined _WIN32
    #define BID_DLL_PUBLIC __declspec(dllexport)
#else
    #define BID_DLL_PUBLIC __attribute__ ((visibility ("default")))
#endif


const char* get_sha1_version(BreakID* bid) NOEXCEPT;

//create and destroy
BID_DLL_PUBLIC BreakID* breakid_new() NOEXCEPT;
BID_DLL_PUBLIC void breakid_del(BreakID* breakid) NOEXCEPT;

//configuration functions
BID_DLL_PUBLIC void breakid_set_useMatrixDetection(BreakID* bid, bool val) NOEXCEPT;
BID_DLL_PUBLIC void breakid_set_useBinaryClauses(BreakID* bid, bool val) NOEXCEPT;
BID_DLL_PUBLIC void breakid_set_useShatterTranslation(BreakID* bid, bool val) NOEXCEPT;
BID_DLL_PUBLIC void breakid_set_useFullTranslation(BreakID* bid, bool val) NOEXCEPT;
BID_DLL_PUBLIC void breakid_set_symBreakingFormLength(BreakID* bid, int val) NOEXCEPT;
BID_DLL_PUBLIC void breakid_set_verbosity(BreakID* bid, uint32_t val) NOEXCEPT;
BID_DLL_PUBLIC void breakid_set_steps_lim(BreakID* bid, int64_t val) NOEXCEPT;

//Dynamic CNF
BID_DLL_PUBLIC void breakid_start_dynamic_cnf(BreakID* bid, uint32_t nVars) NOEXCEPT;
BID_DLL_PUBLIC void breakid_add_clause(BreakID* bid, int* start, size_t num) NOEXCEPT;
BID_DLL_PUBLIC void breakid_end_dynamic_cnf(BreakID* bid) NOEXCEPT;

//Main functions
BID_DLL_PUBLIC void breakid_detect_subgroups(BreakID* bid) NOEXCEPT;
BID_DLL_PUBLIC void breakid_break_symm(BreakID* bid) NOEXCEPT;

//Print info
BID_DLL_PUBLIC void breakid_print_subgroups(BreakID* bid) NOEXCEPT;
BID_DLL_PUBLIC void breakid_print_symm_break_stats(BreakID* bid) NOEXCEPT;
BID_DLL_PUBLIC void breakid_print_perms_and_matrices(BreakID* bid) NOEXCEPT;
BID_DLL_PUBLIC void breakid_print_generators(BreakID* bid) NOEXCEPT;

//Get info
BID_DLL_PUBLIC uint32_t breakid_get_num_generators(BreakID* bid) NOEXCEPT;
BID_DLL_PUBLIC uint32_t breakid_get_num_break_cls(BreakID* bid) NOEXCEPT;
BID_DLL_PUBLIC uint32_t breakid_get_num_aux_vars(BreakID* bid) NOEXCEPT;
BID_DLL_PUBLIC int64_t  breakid_get_steps_remain(BreakID* bid) NOEXCEPT;
BID_DLL_PUBLIC uint64_t breakid_get_num_subgroups(BreakID* bid) NOEXCEPT;

// Returns the number of clauses in `num_ret` and returns an int* that
// contains all clauses's literals with a 0 in between:
// lit1 lit2 lit3 0 lit1 lit2 0 ... where 1st clause contains lit1, lit2, lit3
BID_DLL_PUBLIC int*     breakid_get_brk_cls(BreakID* bid, int* num_ret) NOEXCEPT;



#ifdef __cplusplus
} // end extern c
#endif
