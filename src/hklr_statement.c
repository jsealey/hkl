#include <assert.h>
#include <stdio.h>

#include "hkl_alloc.h"
#include "hklr_statement.h"
#include "hklr.h"

extern void hklr_statement_puts(HklrExpression* expr);
extern void hklr_statement_assign(HklrExpression* lhs, HklrExpression* rhs);
extern void hklr_statement_if(HklrExpression* expr, HklList* list);
extern void hklr_statement_while(HklrExpression* expr, HklList* list);

HklrStatement* hklr_statement_new(HklStatementType type, ...)
{
  assert(type != HKL_STMT_NONE);

  HklrStatement* stmt = hkl_alloc_object(HklrStatement);
  stmt->type = type;

  va_list argp;
  va_start(argp, type);

  switch (type)
  {
    case HKL_STMT_PUTS:
      // puts requires 1 expression
      stmt->arg[0].expression = va_arg(argp, HklrExpression*);
      break;

    case HKL_STMT_ASSIGN:

      stmt->arg[0].expression = va_arg(argp, HklrExpression*);
      stmt->arg[1].expression = va_arg(argp, HklrExpression*);
      break;

    case HKL_STMT_INIT:
      stmt->arg[0].flags = va_arg(argp, HklFlag);
      stmt->arg[1].string = va_arg(argp, HklString*);
      stmt->arg[2].expression = va_arg(argp, HklrExpression*);
      break;

    case HKL_STMT_IF:
    case HKL_STMT_WHILE:

      stmt->arg[0].expression = va_arg(argp, HklrExpression*);
      stmt->arg[1].list = va_arg(argp, HklList*);
      break;


    default:
      break;
  }

  va_end(argp);

  return stmt;
}

void hklr_statement_exec(HklrStatement* stmt)
{
  assert(stmt != NULL);

  switch (stmt->type)
  {

    case HKL_STMT_PUTS:
      hklr_statement_puts(stmt->arg[0].expression);
    break;

    case HKL_STMT_HKLR:

      // Print runtime information
      fprintf(stdout, "Ops:             %zu\n", HKLR.ops);
      fprintf(stdout, "Objects Created: %zu\n", HKLR.gc_created);
      fprintf(stdout, "Objects Freed:   %zu\n", HKLR.gc_freed);
      fprintf(stdout, "Object Cycles:   %zu\n", HKLR.gc_rootsize);
      fprintf(stdout, "GC Runs:         %zu\n", HKLR.gc_runs);
      fprintf(stdout, "Scope Level:     %zu\n", HKLR.scope_level);
      fprintf(stdout, "Globals:         %zu\n", HKLR.globals->length);
      fprintf(stdout, "Locals:          %zu\n", ((HklScope*) HKLR.scopes->tail->data)->locals->length);
      fprintf(stdout, "Upvals:          %zu\n", ((HklScope*) HKLR.scopes->tail->data)->upvals->length);
      fflush(stdout);
      break;

    case HKL_STMT_INIT:
    {
      // This is very much a hack and needs to be made ALOT better
      // this doesn't actually play nice with flags, but will
      // do for now to create globals and locals
      if (stmt->arg[0].flags & HKL_FLAG_GLOBAL)
        hklr_global_insert(stmt->arg[1].string, hklr_object_new(HKL_TYPE_NIL, stmt->arg[0].flags));
      else
        hklr_local_insert(stmt->arg[1].string, hklr_object_new(HKL_TYPE_NIL, stmt->arg[0].flags));

      HklrExpression* temp = hklr_expression_new(HKL_EXPR_ID, stmt->arg[1].string);
      hklr_statement_assign(temp, stmt->arg[2].expression);
      hklr_expression_free(temp);
    }
    break;

    case HKL_STMT_ASSIGN:
      hklr_statement_assign(stmt->arg[0].expression, stmt->arg[1].expression);
    break; // HKL_STMT_ASSIGN

    case HKL_STMT_IF:
      hklr_statement_if(stmt->arg[0].expression, stmt->arg[1].list);
      break;

    case HKL_STMT_WHILE:
      hklr_statement_while(stmt->arg[0].expression, stmt->arg[1].list);
      break;

    default:
      break;
  }

  // Increment numbers of completed operations
  HKLR.ops++;
}

// helper function to free all items in a statement list
static void hklr_statement_free_list(void* stmt, void* data) {

  hklr_statement_free((HklrStatement*) stmt);
}

void hklr_statement_free(HklrStatement* stmt)
{
  assert(stmt != NULL);

  switch (stmt->type)
  {
    case HKL_STMT_PUTS:
      // Free the expression
      hklr_expression_free(stmt->arg[0].expression);
      break;

    case HKL_STMT_INIT:
      hklr_expression_free(stmt->arg[2].expression);
      break;

    case HKL_STMT_ASSIGN:
      hklr_expression_free(stmt->arg[0].expression);
      hklr_expression_free(stmt->arg[1].expression);
      break;

    case HKL_STMT_IF:
    case HKL_STMT_WHILE:
      hklr_expression_free(stmt->arg[0].expression);
      hkl_list_traverse(stmt->arg[1].list, hklr_statement_free_list, NULL);
      hkl_list_free(stmt->arg[1].list);
      break;

    default:
    break;
  }

  hkl_free_object(stmt);
}
