/**
 * @file calc-graph.cpp
 * @date 5-Nov-2019
 */

#include "orx.h"
#include "orxSolver.h"


/** Misc defines
 */
#define orxSOLVER_KZ_CONFIG_INPUT_FIELDS_LIST        "Inputs"
#define orxSOLVER_KZ_CONFIG_LIST_FIELDS_LIST         "Lists"
#define orxSOLVER_KZ_CONFIG_FIELD_DEFAULTS_LIST      "Defaults"
#define orxSOLVER_KZ_CONFIG_KEEP_IN_CACHE            "KeepInCache"

#define orxSOLVER_KZ_LITERAL_INFINITY                "infinity"
#define orxSOLVER_KZ_LITERAL_PI                      "pi"

#define orxSOLVER_KZ_FUNC_ABS                        "ABS"
#define orxSOLVER_KZ_FUNC_ROUND                      "ROUND"
#define orxSOLVER_KZ_FUNC_CEIL                       "CEIL"
#define orxSOLVER_KZ_FUNC_FLOOR                      "FLOOR"
#define orxSOLVER_KZ_FUNC_TRUNCATE                   "TRUNCATE"
#define orxSOLVER_KZ_FUNC_CLAMP                      "CLAMP"
#define orxSOLVER_KZ_FUNC_ROOT                       "ROOT"

#define orxSOLVER_KZ_FUNC_LOG                        "LOG"
#define orxSOLVER_KZ_FUNC_LN                         "LOG"

#define orxSOLVER_KZ_FUNC_SIN                        "SIN"
#define orxSOLVER_KZ_FUNC_COS                        "COS"
#define orxSOLVER_KZ_FUNC_TAN                        "TAN"
#define orxSOLVER_KZ_FUNC_ARCSIN                     "ARCSIN"
#define orxSOLVER_KZ_FUNC_ARCCOS                     "ARCCOS"
#define orxSOLVER_KZ_FUNC_ARCTAN                     "ARCTAN"
#define orxSOLVER_KZ_FUNC_SINH                       "SINH"
#define orxSOLVER_KZ_FUNC_COSH                       "COSH"
#define orxSOLVER_KZ_FUNC_TANH                       "TANH"
#define orxSOLVER_KZ_FUNC_ARCSINH                    "ARCSINH"
#define orxSOLVER_KZ_FUNC_ARCCOSH                    "ARCCOSH"
#define orxSOLVER_KZ_FUNC_ARCTANH                    "ARCTANH"

#define orxSOLVER_KZ_FUNC_LIST_MAP                   "MAP"
#define orxSOLVER_KZ_FUNC_LIST_SUM                   "SUM"
#define orxSOLVER_KZ_FUNC_LIST_MULT                  "MULT"
#define orxSOLVER_KZ_FUNC_LIST_MAX                   "MAX"
#define orxSOLVER_KZ_FUNC_LIST_MIN                   "MIN"
#define orxSOLVER_KZ_FUNC_LIST_MEAN                  "MEAN"
#define orxSOLVER_KZ_FUNC_LIST_AVG                   "AVG"
#define orxSOLVER_KZ_FUNC_LIST_MEDIAN                "MEDIAN"
#define orxSOLVER_KZ_FUNC_LIST_MODE                  "MODE"
#define orxSOLVER_KZ_FUNC_LIST_RANGE                 "RANGE"

#define orxSOLVER_KZ_FUNC_VARIADIC_MAX               "MAX"
#define orxSOLVER_KZ_FUNC_VARIADIC_MIN               "MIN"
#define orxSOLVER_KZ_FUNC_VARIADIC_MEAN              "MEAN"
#define orxSOLVER_KZ_FUNC_VARIADIC_AVG               "AVG"
#define orxSOLVER_KZ_FUNC_VARIADIC_MEDIAN            "MEDIAN"
#define orxSOLVER_KZ_FUNC_VARIADIC_MODE              "MODE"
#define orxSOLVER_KZ_FUNC_VARIADIC_RANGE             "RANGE"

#define orxSOLVER_KC_GROUP_OPEN                      '('
#define orxSOLVER_KC_GROUP_CLOSE                     ')'
#define orxSOLVER_KC_OP_PLUS                         '+'
#define orxSOLVER_KC_OP_MINUS                        '-'
#define orxSOLVER_KC_OP_MULT                         '*'
#define orxSOLVER_KC_OP_DIV                          '/'
#define orxSOLVER_KC_OP_EXP                          '^'
#define orxSOLVER_KC_OP_LIST_PROPERTY                '.'

#define orxSOLVER_KC_ARG_SEPERATOR                   ','
#define orxSOLVER_KC_OPT_ARG_SPECIFIER               ':'

/*
 * This is a basic code template to quickly and easily get started with a project or tutorial.
 */


orxSTATUS orxFASTCALL orxHashTable_StringSet(orxHASHTABLE *_pstHashTable, orxSTRING _strKey, void *_pData)
{
    orxU64 key = (orxU64) orxString_GetID(_strKey);
    return orxHashTable_Set(_pstHashTable, key, _pData);
}

void* orxFASTCALL orxHashTable_StringGet(orxHASHTABLE *_pstHashTable, orxSTRING _strKey)
{
    orxU64 key = (orxU64) orxString_GetID(_strKey);
    return orxHashTable_Get(_pstHashTable, key);
}

orxSTATUS orxFASTCALL orxHashTable_StringAdd(orxHASHTABLE *_pstHashTable, orxSTRING _strKey, void *_pData)
{
    orxU64 key = (orxU64) orxString_GetID(_strKey);
    return orxHashTable_Add(_pstHashTable, key, _pData);
}

void** orxFASTCALL orxHashTable_StringRetrieve(orxHASHTABLE *_pstHashTable, orxSTRING _strKey)
{
    orxU64 key = (orxU64) orxString_GetID(_strKey);
    return orxHashTable_Retrieve(_pstHashTable, key);
}

orxSTATUS orxFASTCALL orxHashTable_StringRemove(orxHASHTABLE *_pstHashTable, orxSTRING _strKey)
{
    orxU64 key = (orxU64) orxString_GetID(_strKey);
    return orxHashTable_Remove(_pstHashTable, key);
}

typedef enum __CALC_OP_t
{
    // Basic math operations
    CALC_OP_PLUS = 0,
    CALC_OP_MINUS,
    CALC_OP_MULT,
    CALC_OP_DIV,
    CALC_OP_EXPONENT,

    // More complex functions
    CALC_OP_MIN,
    CALC_OP_MAX,
    CALC_OP_SUM,

    // No-op
    CALC_OP_NONE = orxENUM_NONE
} CALC_OP;

orxHASHTABLE *variables;

// Strait translation of the algorithm in dataflow_graph_proto-6.rb
// NOTE: make sure to always have a working version of this, iterate on it, and then
//       you can abstract it into it's own .h and .c files

orxFLOAT values[17] = {
    10,      // base_hp (input)
    0,       // damage_hp (input)
    orxNULL, // max_hp ( MIN(max_hp_cap, max_hp_1) )
    orxNULL, // max_hp_1 (max_hp_1_1 + max_hp_1_2)
    orxNULL, // max_hp_1_1 (base_hp * base_mult_hp)
    orxNULL, // max_hp_1_2 (equipment_mod_hp * equipment_mod_mult_hp)
    orxNULL, // max_hp_cap (temporary input)
    orxNULL, // base_mult_hp (temporary input)
    orxNULL, // equipment_mod_hp (temporary input)
    orxNULL, // equipment_mod_mult_hp (temporary input)
    orxNULL, // total_hp (total_hp_1 - damage_hp)
    orxNULL, // total_hp_1 (max_hp + total_hp_1_1)
    orxNULL, // total_hp_1_1 (temporary_mod_hp * temporary_mod_mult_hp)
    orxNULL, // temporary_mod_hp (temporary input)
    orxNULL, // temporary_mod_mult_hp (temporary input)
    orxNULL, // total_hp_2 ( MAX(0, damage_hp) )
    0        // total_hp_2_1 (0)
};
// need to track undefined values separately, since orxNULL == 0 and 0 is a perfectly valid value
/* NOTE:
  Rule: all derived values and inputs are marked as undefined initially, and any constants are
        marked as defined. After assignment, either of an input or through the algorithm, they
        get marked as defined
*/
orxBOOL defined_values[17] = {
    orxTRUE,  // base_hp (input)
    orxTRUE,  // damage_hp (input)
    orxFALSE, // max_hp ( MIN(max_hp_cap, max_hp_1) )
    orxFALSE, // max_hp_1 (max_hp_1_1 + max_hp_1_2)
    orxFALSE, // max_hp_1_1 (base_hp * base_mult_hp)
    orxFALSE, // max_hp_1_2 (equipment_mod_hp * equipment_mod_mult_hp)
    orxFALSE, // max_hp_cap (temporary input)
    orxFALSE, // base_mult_hp (temporary input)
    orxFALSE, // equipment_mod_hp (temporary input)
    orxFALSE, // equipment_mod_mult_hp (temporary input)
    orxFALSE, // total_hp (total_hp_1 - damage_hp)
    orxFALSE, // total_hp_1 (max_hp + total_hp_1_1)
    orxFALSE, // total_hp_1_1 (temporary_mod_hp * temporary_mod_mult_hp)
    orxFALSE, // temporary_mod_hp (temporary input)
    orxFALSE, // temporary_mod_mult_hp (temporary input)
    orxFALSE, // total_hp_2 ( MAX(0, damage_hp) )
    orxTRUE   // total_hp_2_1 (0) (constant)
};


orxFLOAT defaults[17] = {
    0,            // base_hp (input)
    0,            // damage_hp (input)
    orxNULL,      // max_hp ( MIN(max_hp_cap, max_hp_1) )
    orxNULL,      // max_hp_1 (max_hp_1_1 + max_hp_1_2)
    orxNULL,      // max_hp_1_1 (base_hp * base_mult_hp)
    orxNULL,      // max_hp_1_2 (equipment_mod_hp * equipment_mod_mult_hp)
    orxFLOAT_MAX, // max_hp_cap (temporary input)
    1,            // base_mult_hp (temporary input)
    0,            // equipment_mod_hp (temporary input)
    1,            // equipment_mod_mult_hp (temporary input)
    orxNULL,      // total_hp (total_hp_1 - damage_hp)
    orxNULL,      // total_hp_1 (max_hp + total_hp_1_1)
    orxNULL,      // total_hp_1_1 (temporary_mod_hp * temporary_mod_mult_hp)
    0,            // temporary_mod_hp (temporary input)
    1,            // temporary_mod_mult_hp (temporary input)
    orxNULL,      // total_hp_2 ( MAX(total_hp_2_1, damage_hp) )
    0             // total_hp_2_1 (0)
};
// same thing with defined defaults here
orxBOOL defined_defaults[17] = {
    orxTRUE,  // base_hp (input)
    orxTRUE,  // damage_hp (input)
    orxFALSE, // max_hp ( MIN(max_hp_cap, max_hp_1) )
    orxFALSE, // max_hp_1 (max_hp_1_1 + max_hp_1_2)
    orxFALSE, // max_hp_1_1 (base_hp * base_mult_hp)
    orxFALSE, // max_hp_1_2 (equipment_mod_hp * equipment_mod_mult_hp)
    orxTRUE,  // max_hp_cap (temporary input)
    orxTRUE,  // base_mult_hp (temporary input)
    orxTRUE,  // equipment_mod_hp (temporary input)
    orxTRUE,  // equipment_mod_mult_hp (temporary input)
    orxFALSE, // total_hp (total_hp_1 - damage_hp)
    orxFALSE, // total_hp_1 (max_hp + total_hp_1_1)
    orxFALSE, // total_hp_1_1 (temporary_mod_hp * temporary_mod_mult_hp)
    orxTRUE,  // temporary_mod_hp (temporary input)
    orxTRUE,  // temporary_mod_mult_hp (temporary input)
    orxFALSE, // total_hp_2 ( MAX(total_hp_2_1, damage_hp) )
    orxTRUE   // total_hp_2_1 (0)
};

CALC_OP operations[17] = {
    CALC_OP_NONE,  // base_hp (input)
    CALC_OP_NONE,  // damage_hp (input)
    CALC_OP_MIN,   // max_hp ( MIN(max_hp_cap, max_hp_1) )
    CALC_OP_PLUS,  // max_hp_1 (max_hp_1_1 + max_hp_1_2)
    CALC_OP_MULT,  // max_hp_1_1 (base_hp * base_mult_hp)
    CALC_OP_MULT,  // max_hp_1_2 (equipment_mod_hp * equipment_mod_mult_hp)
    CALC_OP_NONE,  // max_hp_cap (temporary input)
    CALC_OP_NONE,  // base_mult_hp (temporary input)
    CALC_OP_NONE,  // equipment_mod_hp (temporary input)
    CALC_OP_NONE,  // equipment_mod_mult_hp (temporary input)
    CALC_OP_MINUS, // total_hp (total_hp_1 - damage_hp)
    CALC_OP_PLUS,  // total_hp_1 (max_hp + total_hp_1_1)
    CALC_OP_MULT,  // total_hp_1_1 (temporary_mod_hp * temporary_mod_mult_hp)
    CALC_OP_NONE,  // temporary_mod_hp (temporary input)
    CALC_OP_NONE,  // temporary_mod_mult_hp (temporary input)
    CALC_OP_MAX,   // total_hp_2 ( MAX(total_hp_2_1, damage_hp) )
    CALC_OP_NONE   // total_hp_2_1 (0)
};

orxBOOL dirty_values[17] = {
    orxTRUE,  // base_hp (input)
    orxTRUE,  // damage_hp (input)
    orxFALSE, // max_hp ( MIN(max_hp_cap, max_hp_1) )
    orxFALSE, // max_hp_1 (max_hp_1_1 + max_hp_1_2)
    orxFALSE, // max_hp_1_1 (base_hp * base_mult_hp)
    orxFALSE, // max_hp_1_2 (equipment_mod_hp * equipment_mod_mult_hp)
    orxTRUE,  // max_hp_cap (temporary input)
    orxTRUE,  // base_mult_hp (temporary input)
    orxTRUE,  // equipment_mod_hp (temporary input)
    orxTRUE,  // equipment_mod_mult_hp (temporary input)
    orxFALSE, // total_hp (total_hp_1 - damage_hp)
    orxFALSE, // total_hp_1 (max_hp + total_hp_1_1)
    orxFALSE, // total_hp_1_1 (temporary_mod_hp * temporary_mod_mult_hp)
    orxTRUE,  // temporary_mod_hp (temporary input)
    orxTRUE,  // temporary_mod_mult_hp (temporary input)
    orxFALSE, // total_hp_2 ( MAX(total_hp_2_1, damage_hp) )
    orxFALSE  // total_hp_2_1 (0)
};

orxU32 forward_list[17][2] = {
    {4},  // base_hp (input)
    {15}, // damage_hp (input)
    {11}, // max_hp ( MIN(max_hp_cap, max_hp_1) )
    {2},  // max_hp_1 (max_hp_1_1 + max_hp_1_2)
    {3},  // max_hp_1_1 (base_hp * base_mult_hp)
    {3},  // max_hp_1_2 (equipment_mod_hp * equipment_mod_mult_hp)
    {2},  // max_hp_cap (temporary input)
    {4},  // base_mult_hp (temporary input)
    {5},  // equipment_mod_hp (temporary input)
    {5},  // equipment_mod_mult_hp (temporary input)
    {},   // total_hp (total_hp_1 - total_hp_2)
    {10}, // total_hp_1 (max_hp + total_hp_1_1)
    {11}, // total_hp_1_1 (temporary_mod_hp * temporary_mod_mult_hp)
    {12}, // temporary_mod_hp (temporary input)
    {12}, // temporary_mod_mult_hp (temporary input)
    {10}, // total_hp_2 ( MAX(total_hp_2_1, damage_hp) )
    {15}  // total_hp_2_1 (0)
};

orxU32 backward_list[17][2] = {
    {},       // base_hp (input)
    {},       // damage_hp (input)
    {6, 3},   // max_hp ( MIN(max_hp_cap, max_hp_1) )
    {4, 5},   // max_hp_1 (max_hp_1_1 + max_hp_1_2)
    {0, 7},   // max_hp_1_1 (base_hp * base_mult_hp)
    {8, 9},   // max_hp_1_2 (equipment_mod_hp * equipment_mod_mult_hp)
    {},       // max_hp_cap (temporary input)
    {},       // base_mult_hp (temporary input)
    {},       // equipment_mod_hp (temporary input)
    {},       // equipment_mod_mult_hp (temporary input)
    {11, 15}, // total_hp (total_hp_1 - total_hp_2)
    {2, 12},  // total_hp_1 (max_hp + total_hp_1_1)
    {13, 14}, // total_hp_1_1 (temporary_mod_hp * temporary_mod_mult_hp)
    {},       // temporary_mod_hp (temporary input)
    {},       // temporary_mod_mult_hp (temporary input)
    {16, 1},  // total_hp_2 ( MAX(total_hp_2_1, damage_hp) )
    {},       // total_hp_2_1 (0)
};

orxU32 forward_list_sizes[17] = {
    1,  // base_hp (input)
    1,  // damage_hp (input)
    1,  // max_hp ( MIN(max_hp_cap, max_hp_1) )
    1,  // max_hp_1 (max_hp_1_1 + max_hp_1_2)
    1,  // max_hp_1_1 (base_hp * base_mult_hp)
    1,  // max_hp_1_2 (equipment_mod_hp * equipment_mod_mult_hp)
    1,  // max_hp_cap (temporary input)
    1,  // base_mult_hp (temporary input)
    1,  // equipment_mod_hp (temporary input)
    1,  // equipment_mod_mult_hp (temporary input)
    0,  // total_hp (total_hp_1 - total_hp_2)
    1,  // total_hp_1 (max_hp + total_hp_1_1)
    1,  // total_hp_1_1 (temporary_mod_hp * temporary_mod_mult_hp)
    1,  // temporary_mod_hp (temporary input)
    1,  // temporary_mod_mult_hp (temporary input)
    1,  // total_hp_2 ( MAX(total_hp_2_1, damage_hp) )
    1   // total_hp_2_1 (0)
};

orxU32 backward_list_sizes[17] = {
    0,  // base_hp (input)
    0,  // damage_hp (input)
    2,  // max_hp ( MIN(max_hp_cap, max_hp_1) )
    2,  // max_hp_1 (max_hp_1_1 + max_hp_1_2)
    2,  // max_hp_1_1 (base_hp * base_mult_hp)
    2,  // max_hp_1_2 (equipment_mod_hp * equipment_mod_mult_hp)
    0,  // max_hp_cap (temporary input)
    0,  // base_mult_hp (temporary input)
    0,  // equipment_mod_hp (temporary input)
    0,  // equipment_mod_mult_hp (temporary input)
    2,  // total_hp (total_hp_1 - total_hp_2)
    2,  // total_hp_1 (max_hp + total_hp_1_1)
    2,  // total_hp_1_1 (temporary_mod_hp * temporary_mod_mult_hp)
    0,  // temporary_mod_hp (temporary input)
    0,  // temporary_mod_mult_hp (temporary input)
    2,  // total_hp_2 ( MAX(total_hp_2_1, damage_hp) )
    0,  // total_hp_2_1 (0)
};

orxFLOAT get_var(orxSTRING var) {
    orxU64 var_idx = (orxU64) orxHashTable_StringGet(variables, var);
    if (defined_values[var_idx] != orxNULL) {
        return values[var_idx];
    }
    return defaults[var_idx];
}

void set_var(orxSTRING var, orxFLOAT value) {
    orxU64 var_idx = (orxU64) orxHashTable_StringGet(variables, var);
    if (var_idx != orxNULL) {
      values[var_idx] = value;
      defined_values[var_idx] = orxTRUE;
      dirty_values[var_idx] = orxTRUE;
    }
}

void set_var_default(orxSTRING var, orxFLOAT value) {
    orxU64 var_idx = (orxU64) orxHashTable_StringGet(variables, var);
    if (var_idx != orxNULL) {
      defaults[var_idx] = value;
      defined_defaults[var_idx] = orxTRUE;
      dirty_values[var_idx] = orxTRUE;
    }
}

void add_equation(const orxSTRING var, const orxSTRING equation) {
    orxLOG("%s: \"%s\"", var, equation);
    parse_and_print_tree(equation, 1);
}

void log_values() {
    orxLOG("base_hp: %f", get_var((orxSTRING) "base_hp"));
    orxLOG("damage_hp: %f", get_var((orxSTRING) "damage_hp"));
    orxLOG("max_hp: %f", get_var((orxSTRING) "max_hp"));
    orxLOG("max_hp_cap: %f", get_var((orxSTRING) "max_hp_cap"));

    orxLOG("base_mult_hp: %f", get_var((orxSTRING) "base_mult_hp"));
    orxLOG("equipment_mod_hp: %f", get_var((orxSTRING) "equipment_mod_hp"));
    orxLOG("equipment_mod_mult_hp: %f", get_var((orxSTRING) "equipment_mod_mult_hp"));
    orxLOG("total_hp: %f", get_var((orxSTRING) "total_hp"));
    orxLOG("temporary_mod_hp: %f", get_var((orxSTRING) "temporary_mod_hp"));
    orxLOG("temporary_mod_mult_hp: %f", get_var((orxSTRING) "temporary_mod_mult_hp"));
}

void log_section(const orxSTRING section) {
    if (orxConfig_PushSection(section) == orxSTATUS_SUCCESS) {
        orxU32 num_keys = orxConfig_GetKeyCount();
        for (orxU32 i=0; i < num_keys; i++) {
            const orxSTRING key = orxConfig_GetKey(i);
            if (orxConfig_IsList(key)) {
                orxU32 list_size = orxConfig_GetListCount(key);
                for (orxU32 j=0; j < (list_size); j++) {
                    orxLOG("%s[%d]: \"%s\"", key, j, orxConfig_GetListString(key, j));
                }
            } else if (orxConfig_IsRandomValue(key)) {
                orxLOG("%s: RANDOM", key);
            } else {
                add_equation(key, orxConfig_GetString(key));
            }
        }
        orxConfig_PopSection();
    }
}




orxFLOAT orxFASTCALL do_op(CALC_OP op, orxU32 *list, orxU32 list_size)
{
    orxU32 first_val_idx = list[0];
    orxFLOAT result = values[first_val_idx];
    if (defined_values[first_val_idx] != orxTRUE) {
        result = defaults[first_val_idx];
    }
    //orxLOG("DO OP");
    for(orxU32 i=0; i<list_size; i++) {
        if (i != 0) {
            orxU32 val_idx = list[i];
            orxFLOAT val = values[val_idx];
            if (defined_values[val_idx] != orxTRUE) {
                val = defaults[val_idx];
            }
            switch (op) {
                case CALC_OP_PLUS:
                    //orxLOG("PLUS: %f + %f -> %f", result, val, (result + val));
                    result += val;
                    break;
                case CALC_OP_MINUS:
                    //orxLOG("MINUS: %f - %f -> %f", result, val, (result - val));
                    result -= val;
                    break;
                case CALC_OP_MULT:
                    //orxLOG("MULT: %f * %f -> %f", result, val, (result * val));
                    result *= val;
                    break;
                case CALC_OP_DIV:
                    //orxLOG("DIV: %f / %f -> %f", result, val, (result / val));
                    result /= val;
                    break;

                case CALC_OP_MIN:
                    //orxLOG("MIN: %f > %f -> %f", result, val, (result > val ? val : result));
                    result = result > val ? val : result;
                    break;
                case CALC_OP_MAX:
                    //orxLOG("MAX: %f < %f -> %f", result, val, (result < val ? val : result));
                    result = result < val ? val : result;
                    break;
                case CALC_OP_SUM:
                    //orxLOG("SUM");
                    result = do_op(CALC_OP_PLUS, list, list_size);
                    break;
                default:
                    //orxLOG("DEFAULT");
                    break;
            }
        }
    }
    return result;
}


void evaluate()
{
    orxBOOL evaluating = orxTRUE;

    while (evaluating == orxTRUE) {
        //orxLOG("WHILE LOOP BEGIN");
        evaluating = orxFALSE;
        for (orxU32 i=0; i<17; i++) {
            //orxLOG("FOR LOOP BEGIN %d", i);
            if (dirty_values[i] == orxTRUE) {
                //orxLOG("DIRTY VALUE FOUND AT %d", i);
                evaluating = orxTRUE;

                orxU32 eval_list[17];
                for(orxU32 j=0; j<17; j++) {
                    eval_list[j] = orxFALSE;
                }
                //orxLOG("eval_list = [%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d]", eval_list[0], eval_list[1], eval_list[2], eval_list[3], eval_list[4], eval_list[5], eval_list[6], eval_list[7], eval_list[8], eval_list[9], eval_list[10], eval_list[11], eval_list[12], eval_list[13], eval_list[14], eval_list[15], eval_list[16]);

                // TODO: something is off in here...
                if (forward_list_sizes[i] > 0) {
                    for (orxU32 j=0; j<forward_list_sizes[i]; j++) {
                        //orxLOG("CHEKING DEPENDENT VAR AT %d", j);
                        orxU32 dependent_var_idx = forward_list[i][j];
                        orxBOOL should_eval = orxTRUE;
                        // check for eval of dependent var
                        if (backward_list_sizes[dependent_var_idx] > 0) {
                            for (orxU32 k=0; k<backward_list_sizes[dependent_var_idx]; k++) {
                                //orxLOG("CHECKING DEPENDENCY AT %d", k);
                                orxU32 dependency_idx = backward_list[dependent_var_idx][k];
                                if (!(defined_values[dependency_idx] == orxTRUE || defined_defaults[dependency_idx] == orxTRUE)) {
                                    //orxLOG("VAR AT %d SHOULD NOT BE EVALED", j);
                                    should_eval = orxFALSE;
                                }
                                //orxLOG("FINISHED CHECKING DEPENDENCY AT %d", k);
                            }
                        }
                        else {
                            //orxLOG("VAR AT %d SHOULD NOT BE EVALED", j);
                            should_eval = orxFALSE;
                        }
                        //orxLOG("FINISHED CHECKING DEPENDENT VAR AT %d", j);
                        // eval dependent var if possible
                        if (should_eval == orxTRUE) {
                            //orxLOG("SHOULD EVAL VAR AT %d", j);
                            eval_list[j] = orxTRUE;
                        }
                    }
                }
                //orxLOG("eval_list = [%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d]", eval_list[0], eval_list[1], eval_list[2], eval_list[3], eval_list[4], eval_list[5], eval_list[6], eval_list[7], eval_list[8], eval_list[9], eval_list[10], eval_list[11], eval_list[12], eval_list[13], eval_list[14], eval_list[15], eval_list[16]);
                for (orxU32 j=0; j<17; j++) {
                    if (eval_list[j] == orxTRUE) {
                        orxU32 dependent_var_idx = forward_list[i][j];
                        //orxLOG("EVALUATING DEPENDENT VAR AT %d", j);
                        values[dependent_var_idx] = do_op(operations[dependent_var_idx], backward_list[dependent_var_idx], backward_list_sizes[dependent_var_idx]);
                        dirty_values[dependent_var_idx] = orxTRUE;
                        defined_values[dependent_var_idx] = orxTRUE;
                        //orxLOG("FINISHED EVALUATING DEPENDENT VAR AT %d", j);
                    }
                }
                //orxLOG("%d NO LONGER DIRTY", i);
                dirty_values[i] = orxFALSE;
            }
            //orxLOG("FOR LOOP END %d", i);
        }
        //orxLOG("WHILE LOOP END");
    }
}


typedef struct {
    orxU32 capacity;
    orxU32 size;
    orxU32* values;
} dynamicArray;

void* reallocate(void* previous, size_t oldSize, size_t newSize) {
    if (newSize == 0) {
        free(previous);
        return NULL;
    }

    return realloc(previous, newSize);
}
#define ALLOCATE(type, count) \
    (type*)reallocate(NULL, 0, sizeof(type) * (count))

#define FREE(type, pointer) \
    reallocate(pointer, sizeof(type), 0)

#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(previous, type, oldCount, count) \
    (type*)reallocate(previous, sizeof(type) * (oldCount), \
        sizeof(type) * (count))

#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * (oldCount), 0)

void dynamicArray_init(dynamicArray* array) {
    array->capacity = 4;
    array->size = 0;
    array->values = (orxU32*) malloc(sizeof(orxU32) * array->capacity);
}

void writeDynamicArray(dynamicArray* array, orxU32 value) {
    // free if total capacity is less than the size after adding the value
    if (array->capacity < array->size + 1) {
        orxU32 oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        orxLOG("Grow capacity from %d to %d", oldCapacity, array->capacity);
        array->values = GROW_ARRAY(array->values, orxU32, oldCapacity, array->capacity);
    }

    // add the value
    array->values[array->size] = value;
    array->size++;
}

void freeDynamicArray(dynamicArray* array) {
    FREE_ARRAY(orxU32, array->values, array->capacity);
    dynamicArray_init(array);
}

void printDynamicArray(dynamicArray* array) {
    orxLOG("array first: %d, last: %d, size: %d, capacity: %d", array->values[0], array->values[array->size - 1], array->size, array->capacity);
}
#undef FREE_ARRAY
#undef GROW_ARRAY
#undef GROW_CAPACITY
#undef FREE
#undef ALLOCATE


/*void dynamicArray_resize(dynamicArray* array) {
    array->capacity *= 2;
    array->values = realloc(array->values, sizeof(orxU32) * array->capacity);
}

orxU32 dynamicArray_get(dynamicArray* array, orxU32 index) {
    return array->values[index];
}
orxU32 dynamicArray_set(dynamicArray* array, orxU32 index, orxU32 value) {
    return array->values[index] = value;
}
orxSTATUS dynamicArray_append(dynamicArray** array, orxU32 value) {
    orxU32 current_size = *array != orxNULL ? array[0]->size : 0;
    orxU32 new_size = current_size + 1;

    if (current_size & new_size == 0) {
        void* tmp = realloc(*array, sizeof **array + (current_size + new_size) * sizeof array[0]->value[0]);
        if (tmp == orxNULL) { return orxSTATUS_FAILURE; }
        *array = tmp;
    }
    array[0]->array[current_size] = value;
    array[0]->size = new_size;
    return orxSTATUS_SUCCESS;
}
orxU32 dynamicArray_remove(dynamicArray* array)

void dynamicArray_free(dynamicArray* array) {
    array->capacity = 0;
    array->size = 0;
    free(array->values);
}*/

void print_config_section(const orxSTRING section) {
    //orxOBJECT *object;
    //object = orxObject_CreateFromConfig(section);
    //orxStructure_LogAll();
    orxLOG("Section: %s\torigin: %s", section, orxConfig_GetOrigin(section));
    //orxLOG("Object: %s", orxStructure_GetIDString(orxStructure_GetID(orxOBJECT_GET_STRUCTURE(object, OBJECT))));
    if (orxConfig_PushSection(section) == orxSTATUS_SUCCESS) {
        orxU32 num_keys = orxConfig_GetKeyCount();
        for (orxU32 i=0; i < num_keys; i++) {
            const orxSTRING key = orxConfig_GetKey(i);
            if (orxConfig_IsList(key)) {
                orxU32 list_size = orxConfig_GetListCount(key);
                for (orxU32 j=0; j < (list_size); j++) {
                    const orxSTRING list_string = orxConfig_GetListString(key, j);
                    orxLOG("%s[%d]: \"%s\"", key, j, list_string);
                    if (orxConfig_HasSection(list_string) == orxTRUE) {
                        print_config_section(list_string);
                    }
                }
            } else if (orxConfig_IsRandomValue(key)) {
                orxLOG("%s: RANDOM", key);
            } else if (orxConfig_IsCommandValue(key)) {
                orxLOG("%s: %f", key, orxConfig_GetFloat(key));
            } else {
                const orxSTRING value = orxConfig_GetString(key);
                orxLOG("%s: \"%s\"", key, value);
                if (orxConfig_HasSection(value) == orxTRUE) {
                    print_config_section(value);
                }
            }
        }
        // on Child, if this returns something, orxConfig_GetKeyCount doesn't return parent sections keys
        // -- result: it doesn't return those keys
        if (orxString_Compare(section, "RaiderSword") == 0) {
            orxLOG("RaiderSword defense: \"%s\"", orxConfig_GetString("defense"));
        }
        if (orxString_Compare(section, "BlightedRaiderSword") == 0) {
            orxLOG("BlightedRaiderSword defense: %d", orxConfig_GetS32("defense"));
        }
        orxConfig_PopSection();
    }
}

void rotate_print_config(orxU32 number) {
    switch(number % 17) {
        case 0: print_config_section("Parent"); break;
        case 1: print_config_section("Child"); break;
        case 2: print_config_section("Grandchild"); break;
        case 3: print_config_section("G_Grandchild"); break;
        case 4: print_config_section("G2_Grandchild"); break;
        case 5: print_config_section("G3_Grandchild"); break;
        case 6: print_config_section("G4_Grandchild"); break;
        case 7: print_config_section("Sword"); break;
        case 8: print_config_section("Shield"); break;
        case 9: print_config_section("RaiderSword"); break;
        case 10: print_config_section("DefenderBlade"); break;
        case 11: print_config_section("BlightedRaiderSword"); break;
        case 12: print_config_section("BastardSword"); break;
        case 13: print_config_section("SpikedShield"); break;
        case 14: print_config_section("DozerShield"); break;
        case 15: print_config_section("TowerShield"); break;
        case 16: print_config_section("HarbingerOfPain"); break;
    }
}

orxBOOL orxConfig_IsFloat(orxSTRING _zKey) {
    const orxSTRING zValue = orxConfig_GetString(_zKey);
    const orxSTRING zRemainder;
    orxFLOAT fValue;
    if(orxString_ToFloat(zValue, &fValue, &zRemainder) == orxSTATUS_SUCCESS) {
        return orxTRUE;
    }
    return orxFALSE;
}

orxSTATUS orxFASTCALL orxObject_PushRuntimeSection(orxOBJECT *_pstObject) {
  orxCHAR zRuntimeSection[20] = {};
  orxString_NPrint(zRuntimeSection, sizeof(zRuntimeSection) - 1, "0x%016llx", orxStructure_GetGUID(_pstObject));
  orxConfig_SetParent(zRuntimeSection, orxObject_GetName(_pstObject));
  return orxConfig_PushSection(zRuntimeSection);
}
/*
orxHASHTABLE* orxObject_GetSolverRuntime(orxOBJECT *_pstObject) {
  orxHASHTABLE *pstRuntime;

  orxObject_PushRuntimeSection(_pstObject);

  orxS32 s32VarsCount = orxConfig_GetListCount("RuntimeVariables");
  pstRuntime = orxHashTable_Create(s32VarsCount, orxHASHTABLE_KU32_FLAG_NOT_EXPANDABLE, orxMEMORY_TYPE_MAIN);

  for(orxU32 i = 0; i < s32VarsCount; i++) {
    const orxSTRING zKey = orxConfig_GetListString("RuntimeVariables", i);
    const orxSTRINGID u32KeyId = orxString_GetID(zKey);
    orxHashTable_Add(pstRuntime, u32KeyId, (void*) value);
  }

  orxConfig_PopSection();
  return pstRuntime;
}*/





/** Init function, it is called when all orx's modules have been initialized
 */
orxSTATUS orxFASTCALL Init()
{
    // Display a small hint in console
    orxLOG("\n* This template project creates a viewport/camera couple and an object"
    "\n* You can play with the config parameters in ../data/config/calc-graph.ini"
    "\n* After changing them, relaunch the executable to see the changes.");
    /*
    orxLOG("orxU64 = %d", sizeof(orxU64) );
    orxLOG("orxU32 = %d", sizeof(orxU32) );
    orxLOG("int = %d", sizeof(int) );
    orxLOG("long = %d", sizeof(long) );
    orxLOG("long long = %d", sizeof(long long) );
    orxLOG("orxSTRINGID = %d", sizeof(orxSTRINGID) );
    orxLOG("orxFLOAT = %d", sizeof(orxFLOAT) );
    orxLOG("orxDOUBLE = %d", sizeof(orxDOUBLE) );
    orxLOG("INFINITY = %f", INFINITY);
    orxLOG("-INFINITY = %f", -INFINITY);
    #ifdef NAN
    orxLOG("NAN = %f", NAN);
    orxLOG("-NAN = %f", -NAN);
    #endif
    orxLOG("INFINITY + INFINITY = %f", INFINITY + INFINITY);
    orxLOG("INFINITY - INFINITY = %f", INFINITY - INFINITY);
    orxLOG("INFINITY * INFINITY = %f", INFINITY * INFINITY);
    orxLOG("INFINITY / INFINITY = %f", INFINITY / INFINITY);
    orxLOG("INFINITY / 39000000.0 = %f", INFINITY / 39000000.0);
    orxLOG("INFINITY ^ INFINITY = %f", orxMath_Pow(INFINITY, INFINITY));
    orxLOG("root4(3111696) = %lf", orxMath_Pow(3111696, 1/4.0));
    orxLOG("PI = %.24f", M_PI);
    orxLOG("PI2 = %.24f", (double)3.14159265358979323846);
    orxLOG("e = %.24f", M_E);

    #define CHECK_END(string_start, string_end, string_length)       (((string_end) != (string_start)) && ((string_start) != orxCHAR_NULL) && ((string_end) == &((string_start)[string_length])))

    const orxSTRING test_convert = "y - 1.61803398874989484820 * 14 + x";
    const orxCHAR* start_phi = &test_convert[4];
    const orxCHAR* start_14 = &test_convert[29];
    int length_phi = 22;
    int length_14 = 2;
    orxCHAR* end_phi;
    orxCHAR* end_14;

    orxDOUBLE phi = strtod(start_phi, &end_phi);
    orxDOUBLE fourteen = strtod(start_14, &end_14);

    orxLOG("start_phi = %s", start_phi);
    orxLOG("start_14 = %s", start_14);
    orxLOG("end_phi = %s", end_phi);
    orxLOG("end_14 = %s", end_14);
    orxLOG("phi = %.22f", phi);
    orxLOG("fourteen = %.22f", fourteen);
    orxLOG("Check phi = %s", CHECK_END(start_phi, end_phi, length_phi) ? "true" : "false" );
    orxLOG("Check fourteen = %s", CHECK_END(start_14, end_14, length_14) ? "true" : "false" );
    #undef CHECK_END
    */
    parse_and_print_tree("4 + 12 * 3 ^ 12 ^ 2", 1);
    parse_and_print_tree("4 + x * 3 ^ x ^ 2", 1);
    parse_and_print_tree("2^3^2", 1);
    parse_and_print_tree("this + is - a * test / of ^ the - (emergency - broadcast) * system", 1);
    // (+ 4 (* x (^ (^ 3 x) 2 ) ) )
    parse_and_print_tree("2 ^4 * 3 - 4 - 4 / 3 - 1 - -1 * 2", 1);





    // Create the viewport
    orxViewport_CreateFromConfig("Viewport");

    // Create the object
    orxObject_CreateFromConfig("Object");

    variables = orxHashTable_Create(1024, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);


    orxHashTable_StringSet(variables, (orxSTRING) "base_hp", (void *) 0);
    orxHashTable_StringSet(variables, (orxSTRING) "damage_hp", (void *) 1);
    orxHashTable_StringSet(variables, (orxSTRING) "max_hp", (void *) 2);
    orxHashTable_StringSet(variables, (orxSTRING) "max_hp_cap", (void *) 6);

    orxHashTable_StringSet(variables, (orxSTRING) "base_mult_hp", (void *) 7);
    orxHashTable_StringSet(variables, (orxSTRING) "equipment_mod_hp", (void *) 8);
    orxHashTable_StringSet(variables, (orxSTRING) "equipment_mod_mult_hp", (void *) 9);
    orxHashTable_StringSet(variables, (orxSTRING) "total_hp", (void *) 10);
    orxHashTable_StringSet(variables, (orxSTRING) "temporary_mod_hp", (void *) 13);
    orxHashTable_StringSet(variables, (orxSTRING) "temporary_mod_mult_hp", (void *) 14);


    // Log section
    //log_section("TST_Expressions");
    //log_section("TST_Variables");
    //log_section("TST_Actual");
    //parser_parseConfigSection("TST_Actual");

    orxSolver_printConfigByTopology("TST_Variables");



    rotate_print_config(0);
    /*for(orxU32 i = 0; i < 17; i++) {
        rotate_print_config(i);
    }*/

    /*dynamicArray* array = (dynamicArray*) malloc(sizeof(dynamicArray));
    dynamicArray_init(array);
    for (int i = 0; i < 1000; i++) {
        writeDynamicArray(array, i);
        if (i % 100 == 0) {printDynamicArray(array);}
    }
    freeDynamicArray(array);*/


    //if (orxConfig_PushSection("StatGraph") == orxSTATUS_SUCCESS) {
        /*orxLOG("Inputs: \"%s\"", orxConfig_GetString("Inputs"));
        orxLOG("w: \"%s\"", orxConfig_GetString("w"));k
        orxLOG("x: \"%s\"", orxConfig_GetString("x"));
        orxLOG("y: \"%s\"", orxConfig_GetString("y"));
        orxLOG("z: \"%s\"", orxConfig_GetString("z"));
        orxLOG("wah: \"%s\"", orxConfig_GetString("wah"));*/
    //}


    /*orxLOG("\n\nBEFORE values: [");
    for(int i=0; i<17; i++) {
        orxFLOAT val = values[i];
        if (defined_values[i] != orxTRUE) {
            val = defaults[i];
        }
        orxLOG("%f", val);
        //if ((orxU32)val == orxNULL) { orxLOG("(orxNULL)"); }
        if (i != 16) {
          orxLOG(", ");
        }
    }
    orxLOG("]\n\n");*/

    ///////////////////////////////////////////////////////////////////////////////////////
    /*orxLOG("BEFORE:");
    log_values();

    evaluate();

    orxLOG("AFTER:");
    log_values();


    orxLOG("base_hp -> 10, damage_hp -> 6, base_mult_hp -> 1, equipment_mod_hp -> 4, equipment_mod_mult_hp -> 1, temporary_mod_hp -> 3, temporary_mod_mult_hp -> 1");
    set_var((orxSTRING) "base_hp", (orxFLOAT) 10);
    set_var((orxSTRING) "damage_hp", (orxFLOAT) 6);
    set_var((orxSTRING) "base_mult_hp", (orxFLOAT) 1);
    set_var((orxSTRING) "equipment_mod_hp", (orxFLOAT) 4);
    set_var((orxSTRING) "equipment_mod_mult_hp", (orxFLOAT) 1);
    set_var((orxSTRING) "temporary_mod_hp", (orxFLOAT) 3);
    set_var((orxSTRING) "temporary_mod_mult_hp", (orxFLOAT) 1);
    evaluate();
    log_values();


    orxLOG("base_hp -> 10, damage_hp -> -4, base_mult_hp -> 1, equipment_mod_hp -> 4, equipment_mod_mult_hp -> 1, temporary_mod_hp -> 3, temporary_mod_mult_hp -> 1");
    set_var((orxSTRING) "base_hp", (orxFLOAT) 10);
    set_var((orxSTRING) "damage_hp", (orxFLOAT) -4);
    set_var((orxSTRING) "base_mult_hp", (orxFLOAT) 1);
    set_var((orxSTRING) "equipment_mod_hp", (orxFLOAT) 4);
    set_var((orxSTRING) "equipment_mod_mult_hp", (orxFLOAT) 1);
    set_var((orxSTRING) "temporary_mod_hp", (orxFLOAT) 3);
    set_var((orxSTRING) "temporary_mod_mult_hp", (orxFLOAT) 1);
    evaluate();
    log_values();


    orxLOG("base_hp -> 10, damage_hp -> 6, base_mult_hp -> 1.5, equipment_mod_hp -> 4, equipment_mod_mult_hp -> 1.75, temporary_mod_hp -> 3, temporary_mod_mult_hp -> 1.25");
    set_var((orxSTRING) "base_hp", (orxFLOAT) 10);
    set_var((orxSTRING) "damage_hp", (orxFLOAT) 6);
    set_var((orxSTRING) "base_mult_hp", (orxFLOAT) 1.5);
    set_var((orxSTRING) "equipment_mod_hp", (orxFLOAT) 4);
    set_var((orxSTRING) "equipment_mod_mult_hp", (orxFLOAT) 1.75);
    set_var((orxSTRING) "temporary_mod_hp", (orxFLOAT) 3);
    set_var((orxSTRING) "temporary_mod_mult_hp", (orxFLOAT) 1.25);
    evaluate();
    log_values();*/

    ///////////////////////////////////////////////////////////////////////////////////////

    /*orxLOG("\n\nAFTER values: [");
    for(int i=0; i<17; i++) {
        orxFLOAT val = values[i];
        if (defined_values[i] != orxTRUE) {
            val = defaults[i];
        }
        orxLOG("%f", val);
        //if ((orxU32)val == orxNULL) { orxLOG("(orxNULL)"); }
        if (i != 16) {
          orxLOG(", ");
        }
    }
    orxLOG("]\n\n");*/



    /*orxLOG("\n\nBEFORE values: [");
    for(int i=0; i<17; i++) {
        orxLOG("%d", defined_values[i]);
        if ((defined_values[i]) == orxNULL) { orxLOG("(orxNULL)"); }
        if (i != 16) {
          orxLOG(", ");
        }
    }
    orxLOG("]\n\n");

    evaluate();

    orxLOG("\n\nAFTER values: [");
    for(int i=0; i<17; i++) {
        orxLOG("%d", defined_values[i]);

        if ((defined_values[i]) == orxNULL) { orxLOG("(orxNULL)"); }
        if (i != 16) {
          orxLOG(", ");
        }
    }
    orxLOG("]\n\n");*/





    orxHashTable_Delete(variables);

    // Done!
    return orxSTATUS_SUCCESS;
}

/** Run function, it is called every clock cycle
 */
orxSTATUS orxFASTCALL Run()
{
    orxSTATUS eResult = orxSTATUS_SUCCESS;

    // Should quit?
    if(orxInput_IsActive("Quit"))
    {
        // Update result
        eResult = orxSTATUS_FAILURE;
    }

    // Done!
    return eResult;
}

/** Exit function, it is called before exiting from orx
 */
void orxFASTCALL Exit()
{
    // Let Orx clean all our mess automatically. :)
}

/** Bootstrap function, it is called before config is initialized, allowing for early resource storage definitions
 */
orxSTATUS orxFASTCALL Bootstrap()
{
    // Add a config storage to find the initial config file
    orxResource_AddStorage(orxCONFIG_KZ_RESOURCE_GROUP, "../data/config", orxFALSE);

    // Return orxSTATUS_FAILURE to prevent orx from loading the default config file
    return orxSTATUS_SUCCESS;
}

/** Main function
 */
int main(int argc, char **argv)
{
    // Set the bootstrap function to provide at least one resource storage before loading any config files
    orxConfig_SetBootstrap(Bootstrap);

    // Execute our game
    orx_Execute(argc, argv, Init, Run, Exit);

    // Done!
    return EXIT_SUCCESS;
}
