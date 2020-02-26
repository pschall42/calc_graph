#ifndef _orxSOLVER_H_
#define _orxSOLVER_H_

#ifdef __cplusplus
extern "C" {
#endif

// Implementation based in part off of the Crafting Interpreters book found here: https://craftinginterpreters.com/

#define M_PHI 1.61803398874989484820

#define orxSOLVER_KZ_CONFIG_INPUT_FIELDS_LIST        "Inputs"
#define orxSOLVER_KZ_CONFIG_LIST_FIELDS_LIST         "Lists"
#define orxSOLVER_KZ_CONFIG_FIELD_DEFAULTS_LIST      "Defaults"

#include "orx.h"

typedef enum __orxSOLVER_MODE_t {
	orxSOLVER_MODE_SOLVE = 0,
	orxSOLVER_MODE_UPDATE
} orxSOLVER_MODE;

typedef enum __orxSOLVER_VAR_TYPE_t {
    orxSOLVER_VAR_TYPE_STRING = 0,
    orxSOLVER_VAR_TYPE_DOUBLE,
    orxSOLVER_VAR_TYPE_BOOL,
    orxSOLVER_VAR_TYPE_VECTOR,

    orxSOLVER_VAR_TYPE_NUMBER,

    orxSOLVER_VAR_TYPE_NONE = orxENUM_NONE
} orxSOLVER_VAR_TYPE;

typedef struct __orxSOLVER_VAR_t {
    union {
        orxVECTOR vValue;           /**< Vector value : 12 */
        const     orxSTRING zValue; /**< String value : 4 */
        orxDOUBLE dValue;           /**< Double value : 8 */
        orxBOOL   bValue;           /**< Bool value : 4 */
    };
    orxSOLVER_VAR_TYPE eType;       /**< Type : 16 */
} orxSOLVER_VAR;

void parse_and_print_tree(const orxSTRING equation, int line);
void parser_parseConfigSection(const orxSTRING section);

void orxSolver_printConfigByTopology(const orxSTRING section);

/*
typedef struct {} orxSolver;

// Debug the compiled code
orxStatus orxFASTCALL orxSolver_Decompile(orxSolver* solver);

// Bind the solver to a structure, which is set to solve either when the given event type is triggered, or solve whenever the given clock is updated. One must be present, but the other may be orxNULL to ignore.
orxStatus orxFASTCALL orxSolver_Bind(orxSolver *solver, orxStructure *pstStructure, orxEVENT_TYPE register_event_type, orxENUM register_event_id, orxCLOCK *_pstClock);

// Solve for a given structure
orxStatus orxFASTCALL orxSolver_Solve(orxSolver* solver, orxStructure* pstStructure);

*/

#ifdef __cplusplus
}
#endif

#endif /* _orxSOLVER_H_ */
