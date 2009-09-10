#include "expbuilder.h"

using namespace esdl::exparse;

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
    if( left == NULL ||
      (dynamic_cast<ANumberEvaluator*>(left) != NULL && 
      (right == NULL || dynamic_cast<ANumberEvaluator*>(right) != NULL)) )  
    {
      IEvaluable* ce = func->_evaluate();
      delete func;
      return ce;
    }
  }
  return func;
}
//......................................................................................................
IEvaluable* exp_builder::evaluator_from_evator(expression_tree* root)  {
  TPtrList<IEvaluable> args;
  bool all_const = true;
  for( int i=0; i < root->evator->args.Count(); i++ )  {
    args.Add( create_evaluator(root->evator->args[i]) );
    if( args.Last() == NULL )  {
      for( int j=0; j < args.Count()-2; j++ )
        delete args[j];
      throw TInvalidArgumentException(__OlxSourceInfo, "could not find appropriate evluable");
    }
    if( dynamic_cast<ANumberEvaluator*>(args.Last()) == NULL )
      all_const = false;
  }
  IEvaluable *rv = BuiltInsFactory::create(root->evator->name, args);
  // need to test if the function result cannot be changed
  if( all_const )  return process_const_func(rv, NULL);
  return rv;
}
//......................................................................................................
IEvaluable* exp_builder::create_evaluator(expression_tree* root)  {
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
    IEvaluable* rv = BuiltInsFactory::create(root->data, left, right);
    if( rv == NULL )  {
      if( left != NULL )  delete left;
      if( right != NULL )  delete right;
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("could not create specified evaluator: ") << root->data);
    }
    return process_const_func(rv, left, right);
  }
  else  {
    if( root->evator != NULL )
      return evaluator_from_evator(root);
    else  {
      if( root->data.IsNumber() )  {
        if( root->data.IndexOf('.') != -1 )
          return new DoubleValue(root->data.ToDouble());
        else
          return new IntValue(root->data.ToInt());
      }
      else  {
        if( root->data.Equalsi("PI") )
          return new DoubleValue(M_PI);
        else if( root->data.Equalsi("true") )
          return new BoolValue(true);
        else if( root->data.Equalsi("false") )
          return new BoolValue(false);

        int ind = scope.VarNames.IndexOf(root->data);
        if( ind == -1 )  {
          scope.VarNames.Add( root->data );
          scope.VarValues.Add(0);
          ind = scope.VarNames.Count()-1;
        }
        return new Variable(scope, ind);
      }
    }
  }
}
//......................................................................................................
