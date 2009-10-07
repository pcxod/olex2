#include "expbuilder.h"

using namespace esdl::exparse;

ClassRegistry<olxstr> StringValue::functions;
LibraryRegistry StringValue::globals;

//......................................................................................................
bool exp_builder::needs_sorting(expression_tree* root)  {
  if( BuiltInsFactory::is_cmp(root->data) || BuiltInsFactory::is_arithmetic(root->data) )  {
    if( root->right != NULL && BuiltInsFactory::is_logical(root->right->data) )  {
      if( root->right->left == NULL )  // +/- number
        return false;
      return !root->right->priority;
    }
  }
  if( BuiltInsFactory::has_arithmetic_priority(root->data) )  {
    if( root->right != NULL && 
      (BuiltInsFactory::is_arithmetic(root->right->data) && 
      !BuiltInsFactory::has_arithmetic_priority(root->right->data)) )  
    {
      if( root->right->left == NULL )  // +/- number
        return false;
      return !root->right->priority;
    }
  }
  return false;
}
//......................................................................................................
expression_tree* exp_builder::sort_logical(expression_tree* root)  {
  // need to test that it is not just -number... root->right->left does it?
  if( root->parent == NULL && root->left == NULL && root->right != NULL && root->right->left != NULL )  {
    expression_tree* top = root->right;
    root->right = top->left;
    top->parent = root->parent;
    root->parent = top;
    top->left = root;
    return sort_logical(top);
  }
  if( needs_sorting(root) )  {
    if( root->parent != NULL && root->left == NULL )  {
      expression_tree* top = root->right;
      root->right = top->left;
      top->parent = root->parent->parent;
      top->left = root->parent;
      if( top->right != NULL )
        top->right = sort_logical(top->right);
      return top;
    }
    else  {
      expression_tree* top = root->right;
      root->right = top->left;
      top->left = root;
      top->parent = root->parent;
      root->parent = top;
      if( top->right != NULL )
        top->right = sort_logical(top->right);
      return top;
    }
  }
  if( root->right != NULL )  {
    expression_tree *original = root->right,
      *result = sort_logical(root->right);
    if( result != original )
      return result;
  }
  return root;
}
//......................................................................................................
IEvaluable* exp_builder::process_const_func(IEvaluable* func, IEvaluable* left, IEvaluable* right)  {
  if( (dynamic_cast<BuiltInsFactory::IConstFunc*>(func) != NULL || 
    dynamic_cast<BuiltInsFactory::IConstFunc2*>(func) != NULL) )
  {
    ANumberEvaluator* left_na = (left == NULL ? NULL : dynamic_cast<ANumberEvaluator*>(left));
    ANumberEvaluator* right_na = (right == NULL ? NULL : dynamic_cast<ANumberEvaluator*>(right));
    if( left == NULL ||
      (left_na != NULL && left_na->is_final()) &&
      (right == NULL || (right_na != NULL && right_na->is_final())) )  
    {
      IEvaluable* ce = func->_evaluate();
      delete func;
      return ce;
    }
  }
  return func;
}
//......................................................................................................
IEvaluable* exp_builder::evaluator_from_evator(expression_tree* root, IEvaluable* left)  {
  TPtrList<IEvaluable> args;
  bool all_const = true;
  for( int i=0; i < root->evator->args.Count(); i++ )  {
    args.Add( create_evaluator(root->evator->args[i]) );
    if( args.Last() == NULL )  {
      for( int j=0; j < args.Count()-2; j++ )
        delete args[j];
      throw TInvalidArgumentException(__OlxSourceInfo, "could not find appropriate evluable");
    }
    ANumberEvaluator* ne = dynamic_cast<ANumberEvaluator*>(args.Last());
    if( ne == NULL || !ne->is_final() )
      all_const = false;
  }
  IEvaluable* rv = NULL;
  if( root->parent != NULL && root->parent->data == '.' )  {
    if( left == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "invalid indirection");
    rv = left->find_method(root->evator->name, factory, args);
  }
  else  {
    rv = BuiltInsFactory::create(root->evator->name, args);
    if( rv == NULL )
      rv = scope.functions.create_from_name(factory, root->evator->name, args);
  }
  if( rv == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("undefined function: ") << root->evator->name);
    // need to test if the function result cannot be changed
  if( all_const )
    return process_const_func(rv, NULL);
  return rv;
}
//......................................................................................................
IEvaluable* exp_builder::locate_function(const olxstr& name, IEvaluable* left, IEvaluable* right)  {
  TPtrList<IEvaluable> args;
  args.Add(left);
  if( right != NULL )  args.Add(right);
  IEvaluable* rv = left->find_method(name, factory, args);
  if( rv == NULL )  {
    rv = BuiltInsFactory::create(name, left, right);
    rv = process_const_func(rv, left, right);
  }
  if( rv == NULL )  {
    if( left != NULL )  delete left;
    if( right != NULL )  delete right;
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("could not create specified evaluator: ") << name);
  }
  return rv;
}
//......................................................................................................
IEvaluable* exp_builder::create_evaluator(expression_tree* root)  {
  if( root->data == '.' )  {
    IEvaluable* left = NULL, *rv=NULL;
    if( root->left == NULL || (left=create_evaluator(root->left)) == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "invalid indirection");
    while( root->right != NULL && root->right->evator != NULL )  {
      rv = evaluator_from_evator(root->right, left);
      IEvaluable* eval = rv->_evaluate();
      if( rv->ref_cnt == 0 )  delete rv;
      if( left->ref_cnt == 0 )  delete left;
      left = rv = eval;
      root = root->right;
    }
    if( root->right != NULL )  {  // anything more left?
      IEvaluable* right = create_evaluator(root->right);
      return locate_function(root->data, rv, right);
    }
    return rv;
  }
  if( root->data == '=' )  {  // assignment
    if( root->right == NULL || root->left == NULL || root->evator != NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "invalid assignment operator");
    IEvaluable* val = create_evaluator(root->right);
    if ( !val->is_final() )  {
      IEvaluable* res = val->_evaluate();
      if( val->ref_cnt == 0 )
        delete val;
      val = res;
    }
    scope.add_var(root->left->data, val, true);
    return val;
  }
  if( root->left != NULL || root->right != NULL )  {
    IEvaluable *left = NULL, *right = NULL;
    try  {
      if( root->left == NULL )  {
        if( root->evator != NULL )
          left = evaluator_from_evator(root);
      }
      else
        left = create_evaluator(root->left);
      right = (root->right != NULL ? create_evaluator(root->right) : NULL);
    }
    catch(const TExceptionBase& exc)  {
      if( left != NULL )  delete left;
      if( right != NULL )  delete right;
      throw TFunctionFailedException(__OlxSourceInfo, exc, "could not find appropriate evlauable");
    }
    if( left == NULL && right != NULL )  {
      left = right;
      right = NULL;
    }
    return locate_function(root->data, left, right);
  }
  else  {
    if( root->evator != NULL )
      return evaluator_from_evator(root);
    else  {
      if( root->data.IsNumber() )  { // is number?
        if( root->data.IndexOf('.') != -1 )
          return new DoubleValue(root->data.ToDouble());
        else
          return new IntValue(root->data.ToInt());
      }
      else  {
        // check if a string
        if( (root->data.StartsFrom('\'') && root->data.EndsWith('\'')) ||
            root->data.StartsFrom('\"') && root->data.EndsWith('\"') )  
        {
          return new StringValue(root->data.Trim(root->data.CharAt(0)));
        }
        int ind = scope.consts.IndexOf(root->data);
        if( ind != -1 )
          return scope.consts.GetValue(ind);
        ind = scope.vars.IndexOf(root->data);
        if( ind == -1 )
          throw TFunctionFailedException(__OlxSourceInfo, olxstr("undefined variable: ") << root->data);
        return scope.vars.GetValue(ind);
      }
    }
  }
}
//......................................................................................................
