#include <typeinfo>
#include "resolveTypes.h"
#include "analysis.h"
#include "stmt.h"
#include "expr.h"


class FindReturn : public Traversal {
 public:
  bool found;
  FindReturn(void);
  void preProcessStmt(Stmt* stmt);
};

FindReturn::FindReturn() {
  found = false;
}

void FindReturn::preProcessStmt(Stmt* stmt) {
  if (dynamic_cast<ReturnStmt*>(stmt)) {
    found = true;
  }
}

ResolveTypes::ResolveTypes() {
  whichModules = MODULES_CODEGEN;
}


static bool types_match(Type* super, Type* sub) {
  if (sub == super) {
    return true;
  } else if (dynamic_cast<StructuralType*>(super) &&
             dynamic_cast<NilType*>(sub)) {
    return true;
  } else if (ClassType* superClass = dynamic_cast<ClassType*>(super)) {
    if (ClassType* subClass = dynamic_cast<ClassType*>(sub)) {
      if (subClass->parentClasses.in(superClass)) {
        return true;
      }
    }
  }
  return false;
}


static bool replaceTypeWithAnalysisType(Symbol* sym) {
  if (!analyzeAST) {
    // BLC: if analysis hasn't run, we can't use its result
    return false;
  }
  if (sym->type == NULL) {
    // BLC: if there's no type, apparently we can't call into analysis?
    return false;
  }
  if (typeid(*(sym->type)) == typeid(IndexType)) {
    // BLC: don't replace IndexTypes by what analysis has computed
    // yet... it doesn't seem accurate enough yet
    return false;
  }
  if (is_Scalar_Type(sym->type) || sym->type->astType == TYPE_USER) {
    // BLC (quoting from John's 05/10/05 log message): "analysis
    // defers to declared scalar types since unused scalars would
    // otherwise not have a type from analysis (we could handle these
    // cases specially, perhaps emitting a warning)"
    return false;
  }
  // otherwise, use what analysis gives us
  return true;
}


void ResolveTypes::processSymbol(Symbol* sym) {
  if (FnSymbol* fn = dynamic_cast<FnSymbol*>(sym)) {
    if (fn->retType == dtUnknown) {
      if (analyzeAST) {
        fn->retType = return_type_info(fn);
        if (checkAnalysisTypeinfo) {
          if (fn->retType == dtUnknown) {
            INT_FATAL(fn, "Analysis unable to determine return type of '%s'", fn->cname);
          }
        }
      } else {
        FindReturn* findReturn = new FindReturn();
        fn->body->traverse(findReturn, true);
        if (!findReturn->found) {
          fn->retType = dtVoid;
        } else {
          INT_FATAL(fn, "Analysis required to determine return type of '%s'", fn->cname);
        }
      }
    } else if (analyzeAST) {
      Type* analysisRetType = return_type_info(fn);
      if (!types_match(fn->retType, analysisRetType)) {
        if (checkAnalysisTypeinfo) {
          INT_WARNING(fn, "Analysis return type mismatch (%s/%s) of '%s'",
                      fn->retType->symbol->name,
                      analysisRetType->symbol->name,
                      fn->cname);
        }
        fn->retType = analysisRetType;
      } else {
        fn->retType = analysisRetType;
      }
    }
  } else if (sym->type == dtUnknown || replaceTypeWithAnalysisType(sym)) {
    if (analyzeAST) {
      sym->type = type_info(sym);
      if (checkAnalysisTypeinfo) {
        if (sym->type == dtUnknown) {
          INT_FATAL(sym, "Analysis unable to determine type of '%s'", sym->cname);
        }
      }
    } else {
      INT_FATAL(sym, "Analysis required to determine type of '%s'", sym->cname);
    }
  } else if (analyzeAST) {
    Type* analysisType = type_info(sym);
    if (!types_match(sym->type, analysisType)) {
      if (checkAnalysisTypeinfo) {
        INT_WARNING(sym, "Analysis type mismatch (%s/%s) of '%s'",
                    sym->type->symbol->name,
                    analysisType->symbol->name,
                    sym->cname);
        if (analysisType != dtUnknown) {
          sym->type = analysisType;
        }
      }
    } else {
      sym->type = analysisType;
    }
  }

  if (sym->type && 
      !dynamic_cast<TypeSymbol*>(sym) &&
      !dynamic_cast<FnSymbol*>(sym)
    ) {
    TypeSymbol* symType = dynamic_cast<TypeSymbol*>(sym->type->symbol);
    if (!type_is_used(symType)) {
      INT_FATAL(sym, "type_is_used assertion failure\n"
                "(likely to be due to dead code not being eliminated)");
    }
    if (symType == NULL) {
      INT_FATAL("null symType");
    }
  }

  /***
   ***  Hack: loops over sequences, types of index variables
   ***/
  if (dynamic_cast<VarSymbol*>(sym)) {
    if (sym->type == dtInteger) {
      if (ForLoopStmt* for_loop =
          dynamic_cast<ForLoopStmt*>(sym->defPoint->parentStmt)) {
        DefExpr* def_expr = for_loop->indices->first();
        if (def_expr->sym == sym) {
          if (SeqType* seq_type = dynamic_cast<SeqType*>(for_loop->domain->typeInfo())) {
            sym->type = seq_type->elementType;
          }
        }
      }
    }
  }
}

ResolveTupleTypes::ResolveTupleTypes() {
  whichModules = MODULES_CODEGEN;
}

void ResolveTupleTypes::processSymbol(Symbol* sym) {
  if (!analyzeAST)
    return;
  if (TypeSymbol *t = dynamic_cast<TypeSymbol*>(sym)) {
    if (TupleType *tt = dynamic_cast<TupleType*>(t->type)) {
      tt->components.clear();
      forv_Vec(VarSymbol, v, tt->fields) {
        Type *t = type_info(v);
        tt->components.add(t);
      }
    }
  }
}
