#include "aqcc.h"

AST *optimize_ast_constant_detail(AST *ast, Env *env)
{
    if (ast == NULL) return ast;

    switch (ast->kind) {
        case AST_ADD:
        case AST_SUB:
        case AST_MUL:
        case AST_DIV:
        case AST_REM:
        case AST_LSHIFT:
        case AST_RSHIFT:
        case AST_LT:
        case AST_LTE:
        case AST_EQ:
        case AST_AND:
        case AST_XOR:
        case AST_OR:
        case AST_LAND:
        case AST_LOR:
        case AST_NEQ: {
            ast->lhs = optimize_ast_constant_detail(ast->lhs, env),
            ast->rhs = optimize_ast_constant_detail(ast->rhs, env);
            if (ast->lhs->kind != AST_INT || ast->rhs->kind != AST_INT)
                return ast;
            // TODO: feature work: long
            int ret = 0;
            switch (ast->kind) {
                case AST_ADD:
                    ret = ast->lhs->ival + ast->rhs->ival;
                    break;
                case AST_SUB:
                    ret = ast->lhs->ival - ast->rhs->ival;
                    break;
                case AST_MUL:
                    ret = ast->lhs->ival * ast->rhs->ival;
                    break;
                case AST_DIV:
                    ret = ast->lhs->ival / ast->rhs->ival;
                    break;
                case AST_REM:
                    ret = ast->lhs->ival % ast->rhs->ival;
                    break;
                case AST_LSHIFT:
                    ret = ast->lhs->ival << ast->rhs->ival;
                    break;
                case AST_RSHIFT:
                    ret = ast->lhs->ival >> ast->rhs->ival;
                    break;
                case AST_LT:
                    ret = ast->lhs->ival < ast->rhs->ival;
                    break;
                case AST_LTE:
                    ret = ast->lhs->ival <= ast->rhs->ival;
                    break;
                case AST_EQ:
                    ret = ast->lhs->ival == ast->rhs->ival;
                    break;
                case AST_AND:
                    ret = ast->lhs->ival & ast->rhs->ival;
                    break;
                case AST_XOR:
                    ret = ast->lhs->ival ^ ast->rhs->ival;
                    break;
                case AST_OR:
                    ret = ast->lhs->ival | ast->rhs->ival;
                    break;
                case AST_LAND:
                    ret = ast->lhs->ival && ast->rhs->ival;
                    break;
                case AST_LOR:
                    ret = ast->lhs->ival || ast->rhs->ival;
                    break;
                case AST_NEQ:
                    ret = ast->lhs->ival != ast->rhs->ival;
                    break;
                default:
                    assert(0);
            }

            return new_int_ast(ret);
        }

        case AST_ASSIGN:
        case AST_LVAR_DECL_INIT:
        case AST_GVAR_DECL_INIT:
        case AST_ENUM_VAR_DECL_INIT:
            ast->lhs = optimize_ast_constant_detail(ast->lhs, env);
            ast->rhs = optimize_ast_constant_detail(ast->rhs, env);
            return ast;

        case AST_COMPL:
        case AST_UNARY_MINUS:
        case AST_NOT: {
            ast->lhs = optimize_ast_constant_detail(ast->lhs, env);
            if (ast->lhs->kind != AST_INT) return ast;
            int ret = 0;
            switch (ast->kind) {
                case AST_COMPL:
                    ret = ~ast->lhs->ival;
                    break;
                case AST_UNARY_MINUS:
                    ret = -ast->lhs->ival;
                    break;
                case AST_NOT:
                    ret = !ast->lhs->ival;
                    break;
                default:
                    assert(0);
            }
            return new_int_ast(ret);
        }

        case AST_EXPR_STMT:
        case AST_RETURN:
        case AST_PREINC:
        case AST_POSTINC:
        case AST_PREDEC:
        case AST_POSTDEC:
        case AST_ADDR:
        case AST_INDIR:
        case AST_CAST:
        case AST_LVALUE2RVALUE:
            ast->lhs = optimize_ast_constant_detail(ast->lhs, env);
            return ast;

        case AST_ARY2PTR:
            ast->ary = optimize_ast_constant_detail(ast->ary, env);
            return ast;

        case AST_COND:
            ast->cond = optimize_ast_constant_detail(ast->cond, env);
            ast->then = optimize_ast_constant_detail(ast->then, env);
            ast->els = optimize_ast_constant_detail(ast->els, env);
            return ast;

        case AST_EXPR_LIST:
            for (int i = 0; i < vector_size(ast->exprs); i++)
                vector_set(ast->exprs, i,
                           optimize_ast_constant_detail(
                               (AST *)vector_get(ast->exprs, i), env));
            return ast;

        case AST_VAR: {
            int *ival = lookup_enum_value(env, ast->varname);
            if (!ival) return ast;
            return new_int_ast(*ival);
        }

        case AST_DECL_LIST:
            for (int i = 0; i < vector_size(ast->decls); i++)
                vector_set(ast->decls, i,
                           optimize_ast_constant_detail(
                               (AST *)vector_get(ast->decls, i), env));
            return ast;

        case AST_FUNCCALL:
            for (int i = 0; i < vector_size(ast->args); i++)
                vector_set(ast->args, i,
                           optimize_ast_constant_detail(
                               (AST *)vector_get(ast->args, i), env));
            return ast;

        case AST_FUNCDEF:
            ast->body = optimize_ast_constant_detail(ast->body, env);
            return ast;

        case AST_COMPOUND:
            for (int i = 0; i < vector_size(ast->stmts); i++)
                vector_set(ast->stmts, i,
                           optimize_ast_constant_detail(
                               (AST *)vector_get(ast->stmts, i), env));
            return ast;

        case AST_IF:
            ast->cond = optimize_ast_constant_detail(ast->cond, env);
            ast->then = optimize_ast_constant_detail(ast->then, env);
            ast->els = optimize_ast_constant_detail(ast->els, env);
            return ast;

        case AST_LABEL:
            ast->label_stmt =
                optimize_ast_constant_detail(ast->label_stmt, env);
            return ast;

        case AST_CASE:
            ast->lhs = optimize_ast_constant_detail(ast->lhs, env);
            ast->rhs = optimize_ast_constant_detail(ast->rhs, env);
            return ast;

        case AST_DEFAULT:
            ast->lhs = optimize_ast_constant_detail(ast->lhs, env);
            return ast;

        case AST_SWITCH:
            ast->target = optimize_ast_constant_detail(ast->target, env);
            ast->switch_body =
                optimize_ast_constant_detail(ast->switch_body, env);
            return ast;

        case AST_DOWHILE:
            ast->cond = optimize_ast_constant_detail(ast->cond, env);
            ast->then = optimize_ast_constant_detail(ast->then, env);
            return ast;

        case AST_FOR:
            ast->initer = optimize_ast_constant_detail(ast->initer, env);
            ast->midcond = optimize_ast_constant_detail(ast->midcond, env);
            ast->iterer = optimize_ast_constant_detail(ast->iterer, env);
            ast->for_body = optimize_ast_constant_detail(ast->for_body, env);
            return ast;

        case AST_MEMBER_REF:
            ast->stsrc = optimize_ast_constant_detail(ast->stsrc, env);
            return ast;
    }

    return ast;
}

AST *optimize_ast_constant(AST *ast, Env *env)
{
    return optimize_ast_constant_detail(ast, env);
}

void optimize_asts_constant(Vector *asts, Env *env)
{
    for (int i = 0; i < vector_size(asts); i++)
        vector_set(asts, i,
                   optimize_ast_constant((AST *)vector_get(asts, i), env));
}

int get_using_register(Code *code)
{
    if (code == NULL) return -1;
    switch (code->kind) {
        case REG_AL:
        case REG_DIL:
        case REG_SIL:
        case REG_DL:
        case REG_CL:
        case REG_R8B:
        case REG_R9B:
        case REG_R10B:
        case REG_R11B:
        case REG_R12B:
        case REG_R13B:
        case REG_R14B:
        case REG_R15B:
        case REG_BPL:
        case REG_SPL:
        case REG_AX:
        case REG_DI:
        case REG_SI:
        case REG_DX:
        case REG_CX:
        case REG_R8W:
        case REG_R9W:
        case REG_R10W:
        case REG_R11W:
        case REG_R12W:
        case REG_R13W:
        case REG_R14W:
        case REG_R15W:
        case REG_BP:
        case REG_SP:
        case REG_EAX:
        case REG_EDI:
        case REG_ESI:
        case REG_EDX:
        case REG_ECX:
        case REG_R8D:
        case REG_R9D:
        case REG_R10D:
        case REG_R11D:
        case REG_R12D:
        case REG_R13D:
        case REG_R14D:
        case REG_R15D:
        case REG_EBP:
        case REG_ESP:
        case REG_RAX:
        case REG_RDI:
        case REG_RSI:
        case REG_RDX:
        case REG_RCX:
        case REG_R8:
        case REG_R9:
        case REG_R10:
        case REG_R11:
        case REG_R12:
        case REG_R13:
        case REG_R14:
        case REG_R15:
        case REG_RBP:
        case REG_RSP:
        case REG_RIP:
            return code->kind;
        case CD_ADDR_OF:
            return get_using_register(code->lhs);
        case CD_ADDR_OF_LABEL:
            return get_using_register(code->lhs);
    }

    return -1;
}

int is_register_code(Code *code)
{
    if (code == NULL) return 0;
    return code->kind & (REG_8 | REG_16 | REG_32 | REG_64);
}

Vector *optimize_code_detail_eliminate(Vector *block)
{
    Vector *nblock = new_vector();
    int used_reg_flag = 0;
    for (int i = vector_size(block) - 1; i >= 0; i--) {
        Code *code = (Code *)vector_get(block, i);

        switch (code->kind) {
            case INST_LEA:
            case INST_MOV: {
                if (!is_register_code(code->rhs) ||
                    used_reg_flag & (1 << (code->rhs->kind & 31)))
                    vector_push_back(nblock, code);
                if (is_register_code(code->rhs))
                    used_reg_flag &= ~(1 << (code->rhs->kind & 31));
            } break;

            default:
                vector_push_back(nblock, code);
                break;
        }

        for (int i = 0; i < vector_size(code->read_dep); i++) {
            int reg = get_using_register(vector_get(code->read_dep, i));
            if (reg != -1) used_reg_flag |= 1 << (reg & 31);
        }
    }

    // reverse nblock
    int size = vector_size(nblock);
    for (int i = 0; i < size / 2; i++) {
        Code *lhs = (Code *)vector_get(nblock, i),
             *rhs = (Code *)vector_get(nblock, size - i - 1);
        vector_set(nblock, i, rhs);
        vector_set(nblock, size - i - 1, lhs);
    }

    return nblock;
}

Vector *optimize_code_detail(Vector *block)
{
    return optimize_code_detail_eliminate(block);
}

Vector *optimize_code(Vector *code)
{
    Vector *ncodes = new_vector();
    for (int i = 0; i < vector_size(code); i++) {
        char *str = vector_get(code, i);
        if (str != NULL) {  // not marker of basic block
            vector_push_back(ncodes, str);
            continue;
        }

        // create basic block
        Vector *block = new_vector();
        for (i++; i < vector_size(code); i++) {
            char *str = vector_get(code, i);
            if (str == NULL) break;
            vector_push_back(block, str);
        }
        if (i >= vector_size(code)) error("no marker for basic block's end");
        // optimize the block
        vector_push_back_vector(ncodes, optimize_code_detail(block));
    }
    return ncodes;
}
