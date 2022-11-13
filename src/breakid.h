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


const char* get_sha1_version(BreakID* bid);

//configuration functions
BID_DLL_PUBLIC void set_useMatrixDetection(BreakID* bid, bool val);
BID_DLL_PUBLIC void set_useBinaryClauses(BreakID* bid, bool val);
BID_DLL_PUBLIC void set_useShatterTranslation(BreakID* bid, bool val);
BID_DLL_PUBLIC void set_useFullTranslation(BreakID* bid, bool val);
BID_DLL_PUBLIC void set_symBreakingFormLength(BreakID* bid, int val);
BID_DLL_PUBLIC void set_verbosity(BreakID* bid, uint32_t val);
BID_DLL_PUBLIC void set_steps_lim(BreakID* bid, int64_t val);

//Dynamic CNF
BID_DLL_PUBLIC void start_dynamic_cnf(BreakID* bid, uint32_t nVars);
BID_DLL_PUBLIC void add_clause(BreakID* bid, int* start, size_t num);
BID_DLL_PUBLIC void end_dynamic_cnf(BreakID* bid);

//Main functions
BID_DLL_PUBLIC void detect_subgroups(BreakID* bid);
BID_DLL_PUBLIC void break_symm(BreakID* bid);

//Print info
BID_DLL_PUBLIC void print_graph(BreakID* bid);
BID_DLL_PUBLIC void print_subgroups(BreakID* bid, char* out);
BID_DLL_PUBLIC void print_symm_break_stats(BreakID* bid);
BID_DLL_PUBLIC void print_perms_and_matrices(BreakID* bid, char* out);
BID_DLL_PUBLIC void print_generators(BreakID* bid, char* out);

//Get info
BID_DLL_PUBLIC uint32_t get_num_generators(BreakID* bid);
BID_DLL_PUBLIC uint32_t get_num_break_cls(BreakID* bid);
BID_DLL_PUBLIC uint32_t get_num_aux_vars(BreakID* bid);
BID_DLL_PUBLIC int64_t get_steps_remain(BreakID* bid);
BID_DLL_PUBLIC uint64_t get_num_subgroups(BreakID* bid);
BID_DLL_PUBLIC int** get_brk_cls(BreakID* bid);
BID_DLL_PUBLIC void free_brk_cls(BreakID* bid, int** cls);

#ifdef __cplusplus
} // end extern c
#endif
