#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "node.h"
#include "utils/vector.h"
#include "utils/string.h"
#include "utils/hashmap.h"

typedef enum _func_arg_type
{
    FUNC_ARG_TYPE_INT,
    FUNC_ARG_TYPE_FLOAT,
    FUNC_ARG_TYPE_STRUCT,
} FuncArgType;

typedef struct _func_arg_attr
{
    FuncArgType type;
    String name;
} FuncArgAttr;

typedef enum _func_return_type
{
    FUNC_RETURN_TYPE_INT,
    FUNC_RETURN_TYPE_FLOAT,
    FUNC_RETURN_TYPE_STRUCT,
} FuncReturnType;

typedef enum _symbol_type
{
    SYMBOL_TYPE_INT,
    SYMBOL_TYPE_FLOAT,
    SYMBOL_TYPE_STRUCT,
    SYMBOL_TYPE_ARRAY,
    SYMBOL_TYPE_FUNC,
} SymbolType;

typedef enum _specifier_type
{
    SPECIFIER_TYPE_INT,
    SPECIFIER_TYPE_FLOAT,
    SPECIFIER_TYPE_STRUCT,
} SpecifierType;

typedef struct _symbol_attr
{
    SymbolType type;

    // 仅当 type == SYMBOL_TYPE_FUNC 时有意义
    FuncReturnType func_return_type;
    Vector func_args; // Vector<FuncArgAttr*>

    // 仅当 type == SYMBOL_TYPE_STRUCT 时有意义
    String struct_name;

    // 仅当 type == SYMBOL_TYPE_ARRAY 时有意义
    SpecifierType array_type;

} SymbolAttr;

typedef struct _specifier_attr
{
    SpecifierType type;

    // 仅当 type == SPECIFIER_TYPE_STRUCT 时有意义
    String struct_name;
} SpecifierAttr;

typedef enum _exp_type
{
    EXP_TYPE_INT,
    EXP_TYPE_FLOAT,
    EXP_TYPE_ARRAY,
    EXP_TYPE_STRUCT,
    EXP_TYPE_ANY,
} ExpType;

typedef struct _exp_attr
{
    ExpType type;
    int can_be_left_value;

    int is_id;
    // is_id == 1 时有效
    String id_name;

    // type == EXP_TYPE_STRUCT 时有效
    String struct_name;

    int is_rel_exp;
    // is_rel_exp == 1 时有效
    int temp_var_number_left;
    int temp_var_number_right;
    String rel_op_text;

    int temp_var_number;

    int is_array_field;
    // type == EXP_TYPE_ARRAY 时候有效
    SpecifierType array_type;
} ExpAttr;

// typedef enum _stmt_type
// {
//     STMT_TYPE_UNIT,
//     STMT_TYPE_INT,
//     STMT_TYPE_FLOAT,
//     STMT_TYPE_ANY,
// } StmtType;

// typedef struct _stmt_list_attr {
//     StmtType return_type;
//     int return_line_number;
// } StmtListAttr;

typedef enum _return_type
{
    RETURN_TYPE_INT,
    RETURN_TYPE_FLOAT,
    RETURN_TYPE_STRUCT,
} ReturnType;
typedef struct _comp_st_inh
{
    ReturnType return_type;
} CompStInh;

typedef enum _struct_field_type
{
    STRUCT_FIELD_TYPE_INT,
    STRUCT_FIELD_TYPE_FLOAT,
    STRUCT_FIELD_TYPE_ARRAY,
    STRUCT_FIELD_TYPE_STRUCT,
} StructFieldType;

typedef struct _struct_field_attr
{
    StructFieldType type;
    String name;
} StructFieldAttr;

typedef struct _struct_attr
{
    HashMap struct_fields_table; // HashMap<*StructFieldAttr>
    String name;
} StructAttr;

void translate(NodePtr root);
void translate_ext_def_list(NodePtr root);
void translate_ext_def(NodePtr root);
SpecifierAttr translate_specifier(NodePtr root);
void translate_ext_dec_list(NodePtr root, SpecifierAttr specifier_attr);
void translate_fun_dec(NodePtr root, SpecifierAttr specifier_attr);
String translate_var_dec(NodePtr root, SpecifierAttr specifier_attr);
void translate_comp_st(NodePtr root, CompStInh inh);
void translate_def_list(NodePtr root);
void translate_def(NodePtr root);
void translate_dec_list(NodePtr root, SpecifierAttr specifier_attr);
void translate_dec(NodePtr root, SpecifierAttr specifier_attr);
ExpAttr translate_exp(NodePtr root);
FuncArgAttr *translate_param_dec(NodePtr root);
Vector translate_var_dec_list(NodePtr root);
String translate_array_var_dec(NodePtr root, SpecifierAttr specifier_attr, int array_size);
Vector translate_exp_args(NodePtr root);
String translate_struct_specifier(NodePtr root);
void translate_stmt(NodePtr root, ReturnType return_type);
void translate_stmt_list(NodePtr root, ReturnType return_type);

#endif