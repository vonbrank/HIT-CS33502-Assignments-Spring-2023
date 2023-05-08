#include "node.h"
#include <stdio.h>
#include <stdlib.h>
#include "./utils/string.h"
#include "utils/hashmap.h"
#include "analyser.h"

HashMap symbol_table;      // HashMap<SymbolAttr*>
HashMap struct_type_table; // HashMap<StructAttr*>

SpecifierType current_analysing_specifier_type = -1;
StructAttr *current_analysing_specifier_attr = NULL;

extern int printAnalysingLog;

void analyse(NodePtr root)
{

    if (printAnalysingLog)
        printf("analysing root...\n");

    symbol_table = hashmap_new();
    struct_type_table = hashmap_new();

    if (root == NULL)
        return;

    // printf("%s\n", root->name);

    NodePtr ext_def_list = root->sonList[0];
    analyse_ext_def_list(ext_def_list);
    // printf("%s\n", ext_def_list->name);
}

void analyse_ext_def_list(NodePtr root)
{
    if (printAnalysingLog)
        printf("analysing ext_def_list...\n");

    if (root == NULL)
        return;

    NodePtr ext_def = root->sonList[0];
    NodePtr ext_def_list = root->sonList[1];
    analyse_ext_def(ext_def);
    analyse_ext_def_list(ext_def_list);
}

void analyse_ext_def(NodePtr root)
{
    if (printAnalysingLog)
        printf("analysing ext_def...\n");

    NodePtr specifier = root->sonList[0];
    NodePtr dec_or_fun = root->sonList[1];
    SpecifierAttr specifier_attr = analyse_specifier(specifier);
    if (string_equals(string_new(dec_or_fun->name), string_new("ExtDecList")))
    {
        analyse_ext_dec_list(dec_or_fun, specifier_attr);
    }
    else if (string_equals(string_new(dec_or_fun->name), string_new("FunDec")))
    {
        analyse_fun_dec(dec_or_fun, specifier_attr);
        NodePtr comp_st = root->sonList[2];

        ReturnType return_type;
        if (specifier_attr.type == SPECIFIER_TYPE_INT)
        {
            return_type = RETURN_TYPE_INT;
        }
        else if (specifier_attr.type == SPECIFIER_TYPE_FLOAT)
        {
            return_type = RETURN_TYPE_FLOAT;
        }
        else if (specifier_attr.type == SPECIFIER_TYPE_STRUCT)
        {
            return_type = RETURN_TYPE_STRUCT;
        }
        CompStInh comp_st_inh;
        comp_st_inh.return_type = return_type;

        analyse_comp_st(comp_st, comp_st_inh);
    }
}

SpecifierAttr analyse_specifier(NodePtr root)
{
    if (printAnalysingLog)
        printf("analysing specifier...\n");

    NodePtr son = root->sonList[0];

    SpecifierAttr specifier_attr;

    if (string_equals(string_new(son->name), string_new("TYPE")))
    {
        if (string_equals(string_new(son->valueText), string_new("int")))
        {
            specifier_attr.type = SPECIFIER_TYPE_INT;
            return specifier_attr;
        }
        specifier_attr.type = SPECIFIER_TYPE_FLOAT;
        return specifier_attr;
    }
    String struct_name = analyse_struct_specifier(son);
    specifier_attr.type = SPECIFIER_TYPE_STRUCT;
    specifier_attr.struct_name = struct_name;
    return specifier_attr;
}

// return struct specifier name
String analyse_struct_specifier(NodePtr root)
{
    if (printAnalysingLog)
        printf("analysing struct specifier...\n");

    NodePtr tag = root->sonList[1];
    if (string_equals(string_new(tag->name), string_new("Tag")))
    {
        NodePtr id = tag->sonList[0];
        String struct_name = string_new(id->valueText);

        StructAttr *struct_attr = hashmap_find(struct_type_table, struct_name);

        if (struct_attr == NULL)
        {
            printf("Error type 17 at Line %d: Undefined structure \"%s\".\n", id->lineNumber, struct_name.str);
        }

        return struct_name;
    }

    NodePtr opt_tag = root->sonList[1];
    NodePtr def_list = root->sonList[3];

    String struct_name;

    if (opt_tag == NULL)
    {
        struct_name = string_from_rand();
    }
    else
    {
        NodePtr id = opt_tag->sonList[0];
        struct_name = string_new(id->valueText);
    }

    current_analysing_specifier_attr = hashmap_find(struct_type_table, struct_name);

    if (current_analysing_specifier_attr != NULL)
    {
        printf("Error type 16 at Line %d: Duplicated name \"%s\".\n", root->lineNumber, struct_name.str);
        return struct_name;
    }

    current_analysing_specifier_type = SPECIFIER_TYPE_STRUCT;
    current_analysing_specifier_attr = (StructAttr *)malloc(sizeof(StructAttr));
    current_analysing_specifier_attr->name = struct_name;
    current_analysing_specifier_attr->struct_fields_table = hashmap_new();

    analyse_def_list(def_list);

    hashmap_insert(struct_type_table, struct_name, current_analysing_specifier_attr);

    current_analysing_specifier_attr = NULL;
    current_analysing_specifier_type = -1;

    return struct_name;
}

void analyse_ext_dec_list(NodePtr root, SpecifierAttr specifier_attr)
{
    if (printAnalysingLog)
        printf("analysing ext_dec_list...\n");

    if (root->sonNumber == 1)
    {
        analyse_var_dec(root->sonList[0], specifier_attr);
    }
    else if (root->sonNumber == 3)
    {
        analyse_var_dec(root->sonList[0], specifier_attr);
        analyse_ext_dec_list(root->sonList[2], specifier_attr);
    }
}

void analyse_fun_dec(NodePtr root, SpecifierAttr specifier_attr)
{
    if (printAnalysingLog)
        printf("analysing fun_dec...\n");

    NodePtr id = root->sonList[0];

    SymbolAttr *symbol_attr = hashmap_find(symbol_table, string_new(id->valueText));

    if (symbol_attr == NULL)
    {

        symbol_attr = (SymbolAttr *)malloc(sizeof(SymbolAttr));
        symbol_attr->type = SYMBOL_TYPE_FUNC;
        symbol_attr->func_args = vector_new();

        if (specifier_attr.type == SPECIFIER_TYPE_INT)
        {
            symbol_attr->func_return_type = FUNC_RETURN_TYPE_INT;
        }
        else if (specifier_attr.type == SPECIFIER_TYPE_FLOAT)
        {
            symbol_attr->func_return_type = FUNC_RETURN_TYPE_FLOAT;
        }
        else if (specifier_attr.type == SPECIFIER_TYPE_STRUCT)
        {
            symbol_attr->func_return_type = FUNC_RETURN_TYPE_STRUCT;
        }

        if (root->sonNumber == 4)
        {
            NodePtr var_dec_list = root->sonList[2];
            symbol_attr->func_args = analyse_var_dec_list(var_dec_list);
        }

        hashmap_insert(symbol_table, string_new(id->valueText), symbol_attr);
    }
    else
    {
        printf("Error type 4 at Line %d: Redefined function \"%s\".\n", id->lineNumber, id->valueText);
    }
}

// Vector<FuncArgAttr*>
Vector analyse_var_dec_list(NodePtr root)
{
    if (root->sonNumber == 1)
    {
        NodePtr var_dec = root->sonList[0];
        FuncArgAttr *func_arg_attr = analyse_param_dec(var_dec);
        Vector fun_args = vector_new();
        fun_args = vector_push(fun_args, func_arg_attr);
        return fun_args;
    }

    NodePtr var_dec = root->sonList[0];
    NodePtr var_dec_list = root->sonList[2];
    FuncArgAttr *func_arg_attr = analyse_param_dec(var_dec);
    Vector fun_args = analyse_var_dec_list(var_dec_list);
    fun_args = vector_push(fun_args, func_arg_attr);
    return fun_args;
}

FuncArgAttr *analyse_param_dec(NodePtr root)
{
    FuncArgAttr *func_arg_attr = (FuncArgAttr *)malloc(sizeof(FuncArgAttr));

    NodePtr specifier = root->sonList[0];
    NodePtr var_dec = root->sonList[1];

    SpecifierAttr specifier_attr = analyse_specifier(specifier);
    String var_dec_name = analyse_var_dec(var_dec, specifier_attr);
    func_arg_attr->name = var_dec_name;

    // printf("fuck specifier_attr = %d\n", specifier_attr.type);

    if (specifier_attr.type == SPECIFIER_TYPE_INT)
    {
        func_arg_attr->type = FUNC_ARG_TYPE_INT;
    }
    else if (specifier_attr.type == SPECIFIER_TYPE_FLOAT)
    {
        func_arg_attr->type = FUNC_ARG_TYPE_FLOAT;
    }
    else if (specifier_attr.type == SPECIFIER_TYPE_STRUCT)
    {
        func_arg_attr->type = FUNC_ARG_TYPE_STRUCT;
    }

    return func_arg_attr;
}

String analyse_var_dec(NodePtr root, SpecifierAttr specifier_attr)
{
    if (printAnalysingLog)
        printf("analysing var_dec...\n");

    NodePtr son_0 = root->sonList[0];

    if (string_equals(string_new(son_0->name), string_new("ID")))
    {

        String var_dec_name = string_new(son_0->valueText);
        if (current_analysing_specifier_type == SPECIFIER_TYPE_STRUCT)
        {
            StructFieldAttr *struct_field_attr = hashmap_find(current_analysing_specifier_attr->struct_fields_table, string_new(son_0->valueText));

            if (struct_field_attr != NULL)
            {
                printf("Error type 15 at Line %d: Redefined field \"%s\".\n", son_0->lineNumber, son_0->valueText);
            }
            else
            {
                struct_field_attr = (StructFieldAttr *)malloc(sizeof(StructFieldAttr));
                struct_field_attr->name = var_dec_name;

                if (specifier_attr.type == SPECIFIER_TYPE_INT)
                {
                    struct_field_attr->type = STRUCT_FIELD_TYPE_INT;
                }
                else if (specifier_attr.type == SPECIFIER_TYPE_FLOAT)
                {
                    struct_field_attr->type = STRUCT_FIELD_TYPE_FLOAT;
                }
                else if (specifier_attr.type == SPECIFIER_TYPE_STRUCT)
                {
                    struct_field_attr->type = STRUCT_FIELD_TYPE_ARRAY;
                }

                hashmap_insert(current_analysing_specifier_attr->struct_fields_table, var_dec_name, struct_field_attr);
            }
        }
        else
        {
            SymbolAttr *symbol_attr = hashmap_find(symbol_table, string_new(son_0->valueText));

            if (symbol_attr != NULL)
            {
                printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", son_0->lineNumber, son_0->valueText);
            }
            else
            {
                symbol_attr = (SymbolAttr *)malloc(sizeof(SymbolAttr));

                if (specifier_attr.type == SPECIFIER_TYPE_INT)
                {
                    symbol_attr->type = SYMBOL_TYPE_INT;
                }
                else if (specifier_attr.type == SPECIFIER_TYPE_FLOAT)
                {
                    symbol_attr->type = SYMBOL_TYPE_FLOAT;
                }
                else if (specifier_attr.type == SPECIFIER_TYPE_STRUCT)
                {
                    symbol_attr->type = SYMBOL_TYPE_STRUCT;
                    symbol_attr->struct_name = specifier_attr.struct_name;
                }

                hashmap_insert(symbol_table, string_new(son_0->valueText), symbol_attr);
            }
        }
        return var_dec_name;
    }
    else if (string_equals(string_new(son_0->name), string_new("VarDec")))
    {
        return analyse_array_var_dec(root, specifier_attr);
    }

    return string_new("");
}

String analyse_array_var_dec(NodePtr root, SpecifierAttr specifier_attr)
{
    if (printAnalysingLog)
        printf("analysing array_var_dec...\n");

    NodePtr son_0 = root->sonList[0];

    if (string_equals(string_new(son_0->name), string_new("ID")))
    {

        SymbolAttr *symbol_attr = hashmap_find(symbol_table, string_new(son_0->valueText));

        String var_dec_name = string_new("");

        if (symbol_attr != NULL)
        {
            var_dec_name = string_new(son_0->valueText);
            printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", son_0->lineNumber, son_0->valueText);
        }
        else
        {
            symbol_attr = (SymbolAttr *)malloc(sizeof(SymbolAttr));
            symbol_attr->type = SYMBOL_TYPE_ARRAY;
            var_dec_name = string_new(son_0->valueText);
            hashmap_insert(symbol_table, string_new(son_0->valueText), symbol_attr);
        }
        return var_dec_name;
    }
    else if (string_equals(string_new(son_0->name), string_new("VarDec")))
    {
        return analyse_array_var_dec(son_0, specifier_attr);
    }

    return string_new("");
}

void analyse_comp_st(NodePtr root, CompStInh inh)
{
    if (printAnalysingLog)
        printf("analysing comp_st...\n");

    NodePtr def_list = root->sonList[1];
    NodePtr stmt_list = root->sonList[2];
    analyse_def_list(def_list);
    analyse_stmt_list(stmt_list, inh.return_type);
}

void analyse_def_list(NodePtr root)
{
    if (printAnalysingLog)
        printf("analysing def_list...\n");
    if (root == NULL)
        return;
    // printf("def_list son number = %d\n", root->sonNumber);
    NodePtr def = root->sonList[0];
    NodePtr def_list = root->sonList[1];
    analyse_def(def);
    analyse_def_list(def_list);
}

void analyse_def(NodePtr root)
{
    if (printAnalysingLog)
        printf("analysing def...\n");

    NodePtr specifier = root->sonList[0];
    NodePtr dec_list = root->sonList[1];
    SpecifierAttr specifier_attr = analyse_specifier(specifier);
    analyse_dec_list(dec_list, specifier_attr);
}

void analyse_dec_list(NodePtr root, SpecifierAttr specifier_attr)
{
    if (printAnalysingLog)
        printf("analysing dec_list...\n");

    if (root->sonNumber == 1)
    {
        analyse_dec(root->sonList[0], specifier_attr);
    }
    else if (root->sonNumber == 3)
    {
        analyse_dec(root->sonList[0], specifier_attr);
        analyse_dec_list(root->sonList[2], specifier_attr);
    }
}

void analyse_dec(NodePtr root, SpecifierAttr specifier_attr)
{
    if (printAnalysingLog)
        printf("analysing dec...\n");

    if (root->sonNumber == 1)
    {
        analyse_var_dec(root->sonList[0], specifier_attr);
    }
    else if (root->sonNumber == 3)
    {
        analyse_var_dec(root->sonList[0], specifier_attr);
        ExpAttr exp_attr = analyse_exp(root->sonList[2]);

        if (exp_attr.type != EXP_TYPE_ANY)
        {
            int both_int = exp_attr.type == EXP_TYPE_INT && specifier_attr.type == SPECIFIER_TYPE_INT;
            int both_float = exp_attr.type == EXP_TYPE_FLOAT && specifier_attr.type == SPECIFIER_TYPE_FLOAT;

            if (!(both_int || both_float))
            {
                printf("Error type 5 at Line %d: Type mismatched for assignment.\n", root->lineNumber);
            }
        }
    }
}

void analyse_stmt_list(NodePtr root, ReturnType return_type)
{
    if (printAnalysingLog)
        printf("analysing stmt_list...\n");
    if (root == NULL)
        return;

    NodePtr stmt = root->sonList[0];
    NodePtr stmt_list = root->sonList[1];
    analyse_stmt(stmt, return_type);
    analyse_stmt_list(stmt_list, return_type);
}

void analyse_stmt(NodePtr root, ReturnType return_type)
{
    if (printAnalysingLog)
        printf("analysing stmt...\n");

    if (root->sonNumber == 1)
    {
        NodePtr comp_st = root->sonList[0];
        CompStInh comp_st_inh;
        comp_st_inh.return_type = return_type;
        analyse_comp_st(comp_st, comp_st_inh);
    }
    else
    {
        NodePtr son_0 = root->sonList[0];
        NodePtr son_1 = root->sonList[1];
        NodePtr son_2 = root->sonList[2];

        if (string_equals(string_new(son_0->name), string_new("Exp")))
        {
            analyse_exp(son_0);
        }
        else if (string_equals(string_new(son_0->name), string_new("RETURN")) && string_equals(string_new(son_1->name), string_new("Exp")))
        {
            ExpAttr exp_attr = analyse_exp(son_1);

            if (exp_attr.type != EXP_TYPE_ANY)
            {
                int both_int = exp_attr.type == EXP_TYPE_INT && return_type == RETURN_TYPE_INT;
                int both_float = exp_attr.type == EXP_TYPE_FLOAT && return_type == RETURN_TYPE_FLOAT;

                if (!(both_int || both_float))
                {
                    printf("Error type 8 at Line %d: Type mismatched for return.\n", son_0->lineNumber);
                }
            }
        }
        // 如果 son_2 是 Exp，那必然是 IF 或 WHILE
        else if (string_equals(string_new(son_2->name), string_new("Exp")))
        {
            analyse_exp(son_2);
            NodePtr stmt = root->sonList[4];
            analyse_stmt(stmt, return_type);

            if (root->sonNumber == 7)
            {
                NodePtr else_stmt = root->sonList[6];
                analyse_stmt(else_stmt, return_type);
            }
        }
    }
}

ExpAttr analyse_exp(NodePtr root)
{
    if (printAnalysingLog)
        printf("analysing exp...\n");

    if (root->sonNumber == 1)
    {
        NodePtr son = root->sonList[0];
        ExpAttr exp_attr;
        exp_attr.can_be_left_value = 0;
        exp_attr.is_id = 0;
        if (string_equals(string_new(son->name), string_new("ID")))
        {
            SymbolAttr *symbol_attr = hashmap_find(symbol_table, string_new(son->valueText));

            exp_attr.can_be_left_value = 1;

            if (symbol_attr == NULL)
            {
                exp_attr.type = EXP_TYPE_ANY;
                printf("Error type 1 at Line %d: Undefined variable \"%s\".\n", son->lineNumber, son->valueText);
            }
            else
            {
                if (symbol_attr->type == SYMBOL_TYPE_INT)
                {
                    exp_attr.type = EXP_TYPE_INT;
                }
                else if (symbol_attr->type == SYMBOL_TYPE_FLOAT)
                {
                    exp_attr.type = EXP_TYPE_FLOAT;
                }
                else if (symbol_attr->type == SYMBOL_TYPE_ARRAY)
                {
                    exp_attr.type = EXP_TYPE_ARRAY;
                }
                else
                {
                    // TODO 处理 Struct 的 情况
                    exp_attr.type = EXP_TYPE_STRUCT;
                    exp_attr.struct_name = symbol_attr->struct_name;
                }
                exp_attr.is_id = 1;
                exp_attr.id_name = string_new(son->valueText);
            }
        }
        else if (string_equals(string_new(son->name), string_new("INT")))
        {
            exp_attr.type = EXP_TYPE_INT;
        }
        else if (string_equals(string_new(son->name), string_new("FLOAT")))
        {
            exp_attr.type = EXP_TYPE_FLOAT;
        }
        return exp_attr;
    }
    else if (root->sonNumber == 2)
    {
        // NOT Exp 或 MINUS Exp 都不是左值
        NodePtr exp = root->sonList[1];
        ExpAttr exp_attr = analyse_exp(exp);
        exp_attr.can_be_left_value = 0;
        return exp_attr;
    }
    else if (root->sonNumber >= 3)
    {
        NodePtr son_0 = root->sonList[0];
        NodePtr son_1 = root->sonList[1];
        NodePtr son_2 = root->sonList[2];

        ExpAttr exp_attr;
        exp_attr.can_be_left_value = 0;
        exp_attr.type = EXP_TYPE_ANY;
        exp_attr.is_id = 0;
        if (string_equals(string_new(son_1->name), string_new("LB")))
        {
            // TODO 处理数组引用
            exp_attr.can_be_left_value = 1;

            ExpAttr previous_exp_attr = analyse_exp(son_0);

            if (previous_exp_attr.is_id && previous_exp_attr.type != EXP_TYPE_ARRAY)
            {
                printf("Error type 10 at Line %d: \"%s\" is not an array.\n", son_1->lineNumber, previous_exp_attr.id_name.str);
            }

            ExpAttr middle_exp_attr = analyse_exp(son_2);

            if (middle_exp_attr.type != EXP_TYPE_INT)
            {
                printf("Error type 12 at Line %d: value between \"[\" and \"]\" is not an integer\n", son_1->lineNumber);
            }
        }
        else if (string_equals(string_new(son_1->name), string_new("DOT")))
        {
            if (printAnalysingLog)
                printf("fuck test log\n");
            // TODO 处理结构体引用

            exp_attr.can_be_left_value = 1;

            ExpAttr previous_exp_attr = analyse_exp(son_0);

            if (previous_exp_attr.type != EXP_TYPE_STRUCT)
            {
                printf("Error type 13 at Line %d: Illegal use of \".\".\n", son_1->lineNumber);
            }
            else
            {
                String struct_name = previous_exp_attr.struct_name;
                StructAttr *struct_attr = hashmap_find(struct_type_table, struct_name);
                String field_name = string_new(son_2->valueText);

                StructFieldAttr *struct_field_attr = hashmap_find(struct_attr->struct_fields_table, field_name);

                if (struct_field_attr == NULL)
                {
                    printf("Error type 14 at Line %d: Non-existent field \"%s\".\n", son_1->lineNumber, field_name.str);
                }
                else
                {
                    if (struct_field_attr->type == STRUCT_FIELD_TYPE_INT)
                    {
                        exp_attr.type = EXP_TYPE_INT;
                    }
                    else if (struct_field_attr->type == STRUCT_FIELD_TYPE_FLOAT)
                    {
                        exp_attr.type = EXP_TYPE_FLOAT;
                    }
                    else if (struct_field_attr->type == STRUCT_FIELD_TYPE_STRUCT)
                    {
                        exp_attr.type = EXP_TYPE_STRUCT;
                    }
                    else if (struct_field_attr->type == STRUCT_FIELD_TYPE_ARRAY)
                    {
                        exp_attr.type = EXP_TYPE_ARRAY;
                    }
                }
            }
        }
        else if (string_equals(string_new(son_0->name), string_new("LP")))
        {
            NodePtr exp = root->sonList[1];
            ExpAttr exp_attr = analyse_exp(exp);
            return exp_attr;
        }
        else if (string_equals(string_new(son_0->name), string_new("Exp")) && string_equals(string_new(son_2->name), string_new("Exp")))
        {
            ExpAttr exp_attr_left = analyse_exp(son_0);
            ExpAttr exp_attr_right = analyse_exp(son_2);

            if (string_equals(string_new(son_1->name), string_new("ASSIGNOP")) && exp_attr_left.can_be_left_value == 0)
            {
                printf("Error type 6 at Line %d:  The left-hand side of an assignment must be a variable.\n", son_0->lineNumber);
                exp_attr.type = EXP_TYPE_ANY;
            }
            else if (exp_attr_left.type == exp_attr_right.type)
            {
                exp_attr.type = exp_attr_left.type;
            }
            else if (exp_attr_left.type != EXP_TYPE_ANY && exp_attr_right.type != EXP_TYPE_ANY)
            {

                if (string_equals(string_new(son_1->name), string_new("ASSIGNOP")))
                {
                    printf("Error type 5 at Line %d: Type mismatched for assignment.\n", son_0->lineNumber);
                }
                else
                {
                    printf("Error type 7 at Line %d:  Type mismatched for operands.\n", son_0->lineNumber);
                }

                exp_attr.type = EXP_TYPE_ANY;
            }
        }
        else if (string_equals(string_new(son_0->name), string_new("ID")))
        {
            SymbolAttr *symbol_attr = hashmap_find(symbol_table, string_new(son_0->valueText));

            if (symbol_attr == NULL)
            {
                printf("Error type 2 at Line %d: Undefined function \"%s\".\n", son_0->lineNumber, son_0->valueText);
                exp_attr.type = EXP_TYPE_ANY;
            }
            else if (symbol_attr->type != SYMBOL_TYPE_FUNC)
            {

                printf("Error type 11 at Line %d: \"%s\" is not a function.\n", son_0->lineNumber, son_0->valueText);
                exp_attr.type = EXP_TYPE_ANY;
            }
            else
            {
                Vector fun_paras = analyse_exp_args(son_2);

                Vector fun_args = symbol_attr->func_args;

                int can_call = 1;

                if (fun_args.size != fun_paras.size)
                {
                    can_call = 0;
                }
                else
                {
                    for (int i = 0; i < fun_paras.size; i++)
                    {
                        ExpAttr *exp_attr = fun_paras.ptr[i];
                        FuncArgAttr *func_arg_attr = fun_args.ptr[i];

                        int both_int = exp_attr->type == EXP_TYPE_INT && func_arg_attr->type == FUNC_ARG_TYPE_INT;
                        int both_float = exp_attr->type == EXP_TYPE_FLOAT && func_arg_attr->type == FUNC_ARG_TYPE_FLOAT;
                        // TODO 处理 struct 的情况

                        if (!(both_int || both_float))
                        {
                            can_call = 0;
                            break;
                        }
                    }
                }

                if (can_call == 0)
                {
                    printf("Error type 9 at Line %d: Function signature mismatched.\n", son_2->lineNumber);
                }
            }
        }

        return exp_attr;
    }

    ExpAttr exp_attr;
    exp_attr.can_be_left_value = 0;
    exp_attr.type = EXP_TYPE_ANY;
    exp_attr.is_id = 0;
}

// Vector<ExpAttr*>
Vector analyse_exp_args(NodePtr root)
{

    NodePtr exp_arg = root->sonList[0];
    ExpAttr exp_attr = analyse_exp(exp_arg);
    ExpAttr *exp_attr_ptr = (ExpAttr *)malloc(sizeof(ExpAttr));
    exp_attr_ptr->can_be_left_value = exp_attr.can_be_left_value;
    exp_attr_ptr->is_id = exp_attr.is_id;
    if (exp_attr.is_id)
    {
        exp_attr_ptr->id_name = string_clone(exp_attr.id_name);
    }
    exp_attr_ptr->type = exp_attr.type;

    if (root->sonNumber == 1)
    {
        Vector fun_paras = vector_new();
        fun_paras = vector_push(fun_paras, exp_attr_ptr);
        return fun_paras;
    }

    NodePtr exp_args = root->sonList[2];
    Vector fun_paras = analyse_exp_args(exp_args);
    fun_paras = vector_push(fun_paras, exp_attr_ptr);

    return fun_paras;
}