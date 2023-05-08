#include "node.h"
#include <stdio.h>
#include <stdlib.h>
#include "./utils/string.h"
#include "utils/hashmap.h"
#include "translator.h"

HashMap symbol_table;      // HashMap<SymbolAttr*>
HashMap struct_type_table; // HashMap<StructAttr*>

int label_count;
int temp_var_count;
HashMap local_var_to_temp_var;

SpecifierType current_analysing_specifier_type = -1;
StructAttr *current_analysing_specifier_attr = NULL;

// int translating_func_args = 0;

extern int printTranslatingLog;

void init()
{
    symbol_table = hashmap_new();
    struct_type_table = hashmap_new();

    SymbolAttr *read_func_symbol_attr = (SymbolAttr *)malloc(sizeof(SymbolAttr));
    read_func_symbol_attr->type = SYMBOL_TYPE_FUNC;
    read_func_symbol_attr->func_return_type = FUNC_RETURN_TYPE_INT;
    read_func_symbol_attr->func_args = vector_new();

    SymbolAttr *write_func_symbol_attr = (SymbolAttr *)malloc(sizeof(SymbolAttr));
    write_func_symbol_attr->type = SYMBOL_TYPE_FUNC;
    write_func_symbol_attr->func_return_type = FUNC_RETURN_TYPE_INT;

    Vector write_func_args = vector_new();

    FuncArgAttr *write_func_arg0_attr = (FuncArgAttr *)malloc(sizeof(FuncArgAttr));
    write_func_arg0_attr->name = string_new("n");
    write_func_arg0_attr->type = FUNC_ARG_TYPE_INT;
    write_func_args = vector_push(write_func_args, write_func_arg0_attr);

    write_func_symbol_attr->func_args = write_func_args;

    hashmap_insert(symbol_table, string_new("read"), read_func_symbol_attr);
    hashmap_insert(symbol_table, string_new("write"), write_func_symbol_attr);

    label_count = 0;
}

int reset_temp_var_count()
{
    temp_var_count = 0;
}

int get_next_temp_var_number()
{
    temp_var_count++;
    return temp_var_count;
}

int *get_next_temp_var_number_ptr()
{
    temp_var_count++;
    int *temp_var_number = (int *)malloc(sizeof(int));
    *temp_var_number = temp_var_count;
    return temp_var_number;
}

ExpAttr exp_attr_new()
{
    ExpAttr exp_attr;
    exp_attr.can_be_left_value = 0;
    exp_attr.type = EXP_TYPE_ANY;
    exp_attr.is_id = 0;
    exp_attr.is_rel_exp = 0;
    exp_attr.is_array_field = 0;
    return exp_attr;
}

ExpAttr *exp_attr_clone(ExpAttr exp_attr)
{
    ExpAttr *exp_attr_ptr = (ExpAttr *)malloc(sizeof(ExpAttr));
    exp_attr_ptr->can_be_left_value = exp_attr.can_be_left_value;
    exp_attr_ptr->is_id = exp_attr.is_id;
    exp_attr_ptr->temp_var_number = exp_attr.temp_var_number;
    exp_attr_ptr->is_rel_exp = exp_attr.is_rel_exp;
    return exp_attr_ptr;
}

void translate(NodePtr root)
{

    if (printTranslatingLog)
        printf("analysing root...\n");

    if (root == NULL)
        return;

    init();

    NodePtr ext_def_list = root->sonList[0];
    translate_ext_def_list(ext_def_list);
    // printf("%s\n", ext_def_list->name);
}

void translate_ext_def_list(NodePtr root)
{
    if (printTranslatingLog)
        printf("analysing ext_def_list...\n");

    if (root == NULL)
        return;

    NodePtr ext_def = root->sonList[0];
    NodePtr ext_def_list = root->sonList[1];
    translate_ext_def(ext_def);
    translate_ext_def_list(ext_def_list);
}

void translate_ext_def(NodePtr root)
{
    if (printTranslatingLog)
        printf("analysing ext_def...\n");

    NodePtr specifier = root->sonList[0];
    NodePtr dec_or_fun = root->sonList[1];
    SpecifierAttr specifier_attr = translate_specifier(specifier);
    if (string_equals(string_new(dec_or_fun->name), string_new("ExtDecList")))
    {
        translate_ext_dec_list(dec_or_fun, specifier_attr);
    }
    else if (string_equals(string_new(dec_or_fun->name), string_new("FunDec")))
    {

        // 重置局部变量符号表
        reset_temp_var_count();
        local_var_to_temp_var = hashmap_new();

        translate_fun_dec(dec_or_fun, specifier_attr);

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

        translate_comp_st(comp_st, comp_st_inh);
    }
}

SpecifierAttr translate_specifier(NodePtr root)
{
    if (printTranslatingLog)
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
    String struct_name = translate_struct_specifier(son);
    specifier_attr.type = SPECIFIER_TYPE_STRUCT;
    specifier_attr.struct_name = struct_name;
    return specifier_attr;
}

// return struct specifier name
String translate_struct_specifier(NodePtr root)
{
    if (printTranslatingLog)
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

    translate_def_list(def_list);

    hashmap_insert(struct_type_table, struct_name, current_analysing_specifier_attr);

    current_analysing_specifier_attr = NULL;
    current_analysing_specifier_type = -1;

    return struct_name;
}

void translate_ext_dec_list(NodePtr root, SpecifierAttr specifier_attr)
{
    if (printTranslatingLog)
        printf("analysing ext_dec_list...\n");

    if (root->sonNumber == 1)
    {
        translate_var_dec(root->sonList[0], specifier_attr);
    }
    else if (root->sonNumber == 3)
    {
        translate_var_dec(root->sonList[0], specifier_attr);
        translate_ext_dec_list(root->sonList[2], specifier_attr);
    }
}

void translate_fun_dec(NodePtr root, SpecifierAttr specifier_attr)
{
    if (printTranslatingLog)
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

        printf("FUNCTION %s :\n", id->valueText);
        if (root->sonNumber == 4)
        {
            NodePtr var_dec_list = root->sonList[2];
            symbol_attr->func_args = translate_var_dec_list(var_dec_list);
        }

        hashmap_insert(symbol_table, string_new(id->valueText), symbol_attr);
    }
    else
    {
        printf("Error type 4 at Line %d: Redefined function \"%s\".\n", id->lineNumber, id->valueText);
    }
}

// Vector<FuncArgAttr*>
Vector translate_var_dec_list(NodePtr root)
{
    if (printTranslatingLog)
        printf("analysing var_dec_list...\n");

    if (root->sonNumber == 1)
    {
        NodePtr var_dec = root->sonList[0];
        FuncArgAttr *func_arg_attr = translate_param_dec(var_dec);
        Vector fun_args = vector_new();
        fun_args = vector_push(fun_args, func_arg_attr);
        return fun_args;
    }

    NodePtr var_dec = root->sonList[0];
    NodePtr var_dec_list = root->sonList[2];
    FuncArgAttr *func_arg_attr = translate_param_dec(var_dec);
    Vector fun_args = translate_var_dec_list(var_dec_list);
    fun_args = vector_push(fun_args, func_arg_attr);
    return fun_args;
}

FuncArgAttr *translate_param_dec(NodePtr root)
{
    if (printTranslatingLog)
        printf("analysing param_dec...\n");

    FuncArgAttr *func_arg_attr = (FuncArgAttr *)malloc(sizeof(FuncArgAttr));

    NodePtr specifier = root->sonList[0];
    NodePtr var_dec = root->sonList[1];

    SpecifierAttr specifier_attr = translate_specifier(specifier);
    String var_dec_name = translate_var_dec(var_dec, specifier_attr);
    func_arg_attr->name = var_dec_name;

    int *param_temp_var_number = (int *)hashmap_find(local_var_to_temp_var, var_dec_name);
    if (param_temp_var_number != NULL)
        printf("PARAM t%d\n", *param_temp_var_number);

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

String translate_var_dec(NodePtr root, SpecifierAttr specifier_attr)
{
    if (printTranslatingLog)
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

                hashmap_insert(local_var_to_temp_var, string_new(son_0->valueText), get_next_temp_var_number_ptr());
            }
        }
        return var_dec_name;
    }
    else if (string_equals(string_new(son_0->name), string_new("VarDec")))
    {
        return translate_array_var_dec(root, specifier_attr, 1);
    }

    return string_new("");
}

String translate_array_var_dec(NodePtr root, SpecifierAttr specifier_attr, int array_size)
{
    if (printTranslatingLog)
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
            symbol_attr->array_type = specifier_attr.type;
            var_dec_name = string_new(son_0->valueText);
            hashmap_insert(symbol_table, string_new(son_0->valueText), symbol_attr);
            int *local_var_number = get_next_temp_var_number_ptr();

            printf("DEC t%d %d\n", *local_var_number, array_size * 4);

            hashmap_insert(local_var_to_temp_var, string_new(son_0->valueText), local_var_number);
        }
        return var_dec_name;
    }
    else if (string_equals(string_new(son_0->name), string_new("VarDec")))
    {

        NodePtr son_2 = root->sonList[2];

        return translate_array_var_dec(son_0, specifier_attr, array_size * atoi(son_2->valueText));
    }

    return string_new("");
}

void translate_comp_st(NodePtr root, CompStInh inh)
{
    if (printTranslatingLog)
        printf("analysing comp_st...\n");

    NodePtr def_list = root->sonList[1];
    NodePtr stmt_list = root->sonList[2];
    translate_def_list(def_list);
    translate_stmt_list(stmt_list, inh.return_type);
}

void translate_def_list(NodePtr root)
{
    if (printTranslatingLog)
        printf("analysing def_list...\n");
    if (root == NULL)
        return;
    // printf("def_list son number = %d\n", root->sonNumber);
    NodePtr def = root->sonList[0];
    NodePtr def_list = root->sonList[1];
    translate_def(def);
    translate_def_list(def_list);
}

void translate_def(NodePtr root)
{
    if (printTranslatingLog)
        printf("analysing def...\n");

    NodePtr specifier = root->sonList[0];
    NodePtr dec_list = root->sonList[1];
    SpecifierAttr specifier_attr = translate_specifier(specifier);
    translate_dec_list(dec_list, specifier_attr);
}

void translate_dec_list(NodePtr root, SpecifierAttr specifier_attr)
{
    if (printTranslatingLog)
        printf("analysing dec_list...\n");

    if (root->sonNumber == 1)
    {
        translate_dec(root->sonList[0], specifier_attr);
    }
    else if (root->sonNumber == 3)
    {
        translate_dec(root->sonList[0], specifier_attr);
        translate_dec_list(root->sonList[2], specifier_attr);
    }
}

void translate_dec(NodePtr root, SpecifierAttr specifier_attr)
{
    if (printTranslatingLog)
        printf("analysing dec...\n");

    if (root->sonNumber == 1)
    {
        translate_var_dec(root->sonList[0], specifier_attr);
    }
    else if (root->sonNumber == 3)
    {
        String var_dec_name = translate_var_dec(root->sonList[0], specifier_attr);
        ExpAttr exp_attr = translate_exp(root->sonList[2]);

        if (exp_attr.type != EXP_TYPE_ANY)
        {
            int both_int = exp_attr.type == EXP_TYPE_INT && specifier_attr.type == SPECIFIER_TYPE_INT;
            int both_float = exp_attr.type == EXP_TYPE_FLOAT && specifier_attr.type == SPECIFIER_TYPE_FLOAT;

            if (!(both_int || both_float))
            {
                printf("Error type 5 at Line %d: Type mismatched for assignment.\n", root->lineNumber);
            }
            else
            {
                int *temp_var_number = (int *)hashmap_find(local_var_to_temp_var, var_dec_name);
                printf("t%d := t%d\n", *temp_var_number, exp_attr.temp_var_number);
            }
        }
    }
}

void translate_stmt_list(NodePtr root, ReturnType return_type)
{
    if (printTranslatingLog)
        printf("analysing stmt_list...\n");
    if (root == NULL)
        return;

    NodePtr stmt = root->sonList[0];
    NodePtr stmt_list = root->sonList[1];
    translate_stmt(stmt, return_type);
    translate_stmt_list(stmt_list, return_type);
}

void translate_stmt(NodePtr root, ReturnType return_type)
{
    if (printTranslatingLog)
        printf("analysing stmt...\n");

    if (root->sonNumber == 1)
    {
        NodePtr comp_st = root->sonList[0];
        CompStInh comp_st_inh;
        comp_st_inh.return_type = return_type;
        translate_comp_st(comp_st, comp_st_inh);
    }
    else
    {
        NodePtr son_0 = root->sonList[0];
        NodePtr son_1 = root->sonList[1];
        NodePtr son_2 = root->sonList[2];

        if (string_equals(string_new(son_0->name), string_new("Exp")))
        {
            translate_exp(son_0);
        }
        else if (string_equals(string_new(son_0->name), string_new("RETURN")) && string_equals(string_new(son_1->name), string_new("Exp")))
        {
            ExpAttr exp_attr = translate_exp(son_1);

            if (exp_attr.type != EXP_TYPE_ANY)
            {
                int both_int = exp_attr.type == EXP_TYPE_INT && return_type == RETURN_TYPE_INT;
                int both_float = exp_attr.type == EXP_TYPE_FLOAT && return_type == RETURN_TYPE_FLOAT;

                if (!(both_int || both_float))
                {
                    printf("Error type 8 at Line %d: Type mismatched for return.\n", son_0->lineNumber);
                }
                printf("RETURN t%d\n", exp_attr.temp_var_number);
            }
        }
        // 如果 son_2 是 Exp，那必然是 IF 或 WHILE
        else if (string_equals(string_new(son_2->name), string_new("Exp")))
        {

            ExpAttr exp_attr = translate_exp(son_2);

            if (exp_attr.is_rel_exp)
                printf("IF t%d %s t%d ", exp_attr.temp_var_number_left, exp_attr.rel_op_text.str, exp_attr.temp_var_number_right);

            // IF LP Exp RP Stmt
            if (root->sonNumber == 5)
            {

                int label1 = ++label_count;
                int label2 = ++label_count;
                if (exp_attr.is_rel_exp)
                {
                    printf("GOTO label%d\n", label1);
                }
                if (exp_attr.is_rel_exp)
                {
                    printf("GOTO label%d\n", label2);
                }

                printf("LABEL label%d :\n", label1);
                NodePtr stmt = root->sonList[4];
                translate_stmt(stmt, return_type);

                printf("LABEL label%d :\n", label2);
            }

            // IF LP Exp RP Stmt ELSE Stmt
            if (root->sonNumber == 7)
            {

                int label1 = ++label_count;
                int label2 = ++label_count;
                if (exp_attr.is_rel_exp)
                {
                    printf("GOTO label%d\n", label1);
                }

                NodePtr else_stmt = root->sonList[6];
                translate_stmt(else_stmt, return_type);
                if (exp_attr.is_rel_exp)
                {
                    printf("GOTO label%d\n", label2);
                }

                printf("LABEL label%d :\n", label1);
                NodePtr stmt = root->sonList[4];
                translate_stmt(stmt, return_type);

                printf("LABEL label%d :\n", label2);
            }
        }
    }
}

ExpAttr translate_exp(NodePtr root)
{
    if (printTranslatingLog)
        printf("analysing exp...\n");

    if (root->sonNumber == 1)
    {
        NodePtr son = root->sonList[0];
        ExpAttr exp_attr = exp_attr_new();
        exp_attr.temp_var_number = get_next_temp_var_number();
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
                    exp_attr.array_type = symbol_attr->array_type;
                }
                else
                {
                    // TODO 处理 Struct 的 情况
                    exp_attr.type = EXP_TYPE_STRUCT;
                    exp_attr.struct_name = symbol_attr->struct_name;
                }
                exp_attr.is_id = 1;
                exp_attr.id_name = string_new(son->valueText);
                exp_attr.temp_var_number = *(int *)hashmap_find(local_var_to_temp_var, string_new(son->valueText));
            }
        }
        else
        {
            if (string_equals(string_new(son->name), string_new("INT")))
            {
                exp_attr.type = EXP_TYPE_INT;
            }
            else if (string_equals(string_new(son->name), string_new("FLOAT")))
            {
                exp_attr.type = EXP_TYPE_FLOAT;
            }
            printf("t%d := #%s\n", exp_attr.temp_var_number, son->valueText);
        }

        return exp_attr;
    }
    else if (root->sonNumber == 2)
    {
        // NOT Exp 或 MINUS Exp 都不是左值
        ExpAttr exp_attr = exp_attr_new();
        exp_attr.temp_var_number = get_next_temp_var_number();

        NodePtr exp = root->sonList[1];
        ExpAttr exp_exp_attr = translate_exp(exp);
        exp_attr.type = exp_exp_attr.type;

        if (string_equals(string_new(root->sonList[0]->name), string_new("MINUS")))
        {
            printf("t%d := #0 - t%d\n", exp_attr.temp_var_number, exp_exp_attr.temp_var_number);
        }
        else
        {
            printf("t%d := ~%d\n", exp_attr.temp_var_number, exp_exp_attr.temp_var_number);
        }

        return exp_attr;
    }
    else if (root->sonNumber >= 3)
    {
        NodePtr son_0 = root->sonList[0];
        NodePtr son_1 = root->sonList[1];
        NodePtr son_2 = root->sonList[2];

        ExpAttr exp_attr = exp_attr_new();
        exp_attr.temp_var_number = get_next_temp_var_number();
        if (string_equals(string_new(son_1->name), string_new("LB")))
        {
            // TODO 处理数组引用
            ExpAttr previous_exp_attr = translate_exp(son_0);

            exp_attr.can_be_left_value = 1;
            exp_attr.is_array_field = 1;

            int no_error = 1;

            if (previous_exp_attr.is_id && previous_exp_attr.type != EXP_TYPE_ARRAY)
            {
                no_error = 0;
                printf("Error type 10 at Line %d: \"%s\" is not an array.\n", son_1->lineNumber, previous_exp_attr.id_name.str);
            }

            ExpAttr middle_exp_attr = translate_exp(son_2);

            if (middle_exp_attr.type != EXP_TYPE_INT)
            {
                no_error = 0;
                printf("Error type 12 at Line %d: value between \"[\" and \"]\" is not an integer\n", son_1->lineNumber);
            }

            if (no_error)
            {
                SymbolAttr *symbol_attr = hashmap_find(symbol_table, previous_exp_attr.id_name);
                int *id_temp_var_number = hashmap_find(local_var_to_temp_var, previous_exp_attr.id_name);

                // printf("fuck %s\n", son_2->name);

                if (previous_exp_attr.array_type == SPECIFIER_TYPE_INT)
                {
                    exp_attr.type = EXP_TYPE_INT;
                }
                else if (previous_exp_attr.array_type == SPECIFIER_TYPE_FLOAT)
                {
                    exp_attr.type = EXP_TYPE_FLOAT;
                }
                else if (previous_exp_attr.array_type == SPECIFIER_TYPE_STRUCT)
                {
                    exp_attr.type = EXP_TYPE_STRUCT;
                }

                int offset_temp_var_id = get_next_temp_var_number();
                printf("t%d := t%d * #4\n", offset_temp_var_id, middle_exp_attr.temp_var_number);

                printf("t%d := t%d + &t%d\n", exp_attr.temp_var_number, offset_temp_var_id, *id_temp_var_number);
            }
        }
        else if (string_equals(string_new(son_1->name), string_new("DOT")))
        {
            if (printTranslatingLog)
                printf("fuck test log\n");
            // TODO 处理结构体引用

            exp_attr.can_be_left_value = 1;

            ExpAttr previous_exp_attr = translate_exp(son_0);

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
            ExpAttr exp_attr = translate_exp(exp);
            return exp_attr;
        }
        else if (string_equals(string_new(son_0->name), string_new("Exp")) && string_equals(string_new(son_2->name), string_new("Exp")))
        {
            ExpAttr exp_attr_left = translate_exp(son_0);
            ExpAttr exp_attr_right = translate_exp(son_2);

            if (string_equals(string_new(son_1->name), string_new("ASSIGNOP")))
            {
                if (exp_attr_left.can_be_left_value == 0)
                {

                    printf("Error type 6 at Line %d:  The left-hand side of an assignment must be a variable.\n", son_0->lineNumber);
                    exp_attr.type = EXP_TYPE_ANY;
                }
                else
                {
                    if (exp_attr_left.is_array_field)
                    {
                        printf("*t%d := t%d\n", exp_attr_left.temp_var_number, exp_attr_right.temp_var_number);
                    }
                    else
                    {
                        printf("t%d := t%d\n", exp_attr_left.temp_var_number, exp_attr_right.temp_var_number);
                    }
                }
            }
            else if (exp_attr_left.type == exp_attr_right.type)
            {
                exp_attr.type = exp_attr_left.type;

                if (string_equals(string_new(son_1->name), string_new("RELOP")))
                {
                    exp_attr.is_rel_exp = 1;
                    exp_attr.temp_var_number_left = exp_attr_left.temp_var_number;
                    exp_attr.temp_var_number_right = exp_attr_right.temp_var_number;
                    exp_attr.rel_op_text = string_new(son_1->valueText);
                }
                else
                {

                    String op = string_new("+");
                    if (string_equals(string_new(son_1->name), string_new("PLUS")))
                    {
                        op = string_new("+");
                    }
                    else if (string_equals(string_new(son_1->name), string_new("MINUS")))
                    {
                        op = string_new("-");
                    }
                    else if (string_equals(string_new(son_1->name), string_new("STAR")))
                    {
                        op = string_new("*");
                    }
                    else if (string_equals(string_new(son_1->name), string_new("DIV")))
                    {
                        op = string_new("/");
                    }

                    printf("t%d := %st%d %s %st%d\n", exp_attr.temp_var_number,
                           exp_attr_left.is_array_field ? "*" : "",
                           exp_attr_left.temp_var_number,
                           op.str,
                           exp_attr_right.is_array_field ? "*" : "",
                           exp_attr_right.temp_var_number);
                }
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
                Vector fun_paras = string_equals(string_new(son_2->name), string_new("Args")) ? translate_exp_args(son_2) : vector_new();

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
                else
                {

                    if (symbol_attr->func_return_type == FUNC_RETURN_TYPE_INT)
                    {
                        exp_attr.type = EXP_TYPE_INT;
                    }
                    else if (symbol_attr->func_return_type == FUNC_RETURN_TYPE_FLOAT)
                    {
                        exp_attr.type = EXP_TYPE_FLOAT;
                    }
                    else if (symbol_attr->func_return_type == FUNC_RETURN_TYPE_STRUCT)
                    {
                        exp_attr.type = EXP_TYPE_STRUCT;
                    }

                    if (string_equals(string_new(son_0->valueText), string_new("read")))
                    {
                        printf("READ t%d\n", exp_attr.temp_var_number);
                    }
                    else if (string_equals(string_new(son_0->valueText), string_new("write")))
                    {
                        ExpAttr *arg_0_attr = (ExpAttr *)(fun_paras.ptr[0]);
                        printf("WRITE t%d\n", arg_0_attr->temp_var_number);
                    }
                    else
                    {
                        for (int i = fun_paras.size - 1; i >= 0; i--)
                        {
                            ExpAttr *exp_attr = fun_paras.ptr[i];
                            printf("ARG t%d\n", exp_attr->temp_var_number);
                        }
                        printf("t%d := CALL %s\n", exp_attr.temp_var_number, son_0->valueText);
                    }
                }
            }
        }

        return exp_attr;
    }

    ExpAttr exp_attr = exp_attr_new();
    exp_attr.temp_var_number = get_next_temp_var_number();
    return exp_attr;
}

// Vector<ExpAttr*>
Vector translate_exp_args(NodePtr root)
{
    if (printTranslatingLog)
        printf("analysing exp_args...\n");

    NodePtr exp_arg = root->sonList[0];
    ExpAttr exp_attr = translate_exp(exp_arg);

    ExpAttr *exp_attr_ptr = exp_attr_clone(exp_attr);
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
    Vector fun_paras = translate_exp_args(exp_args);
    fun_paras = vector_push(fun_paras, exp_attr_ptr);

    return fun_paras;
}