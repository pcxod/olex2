#ifndef __olx_catom_list_H
#define __olx_catom_list_H
#include "residue.h"
/* 
Atom list to handle list of explicit (by label), implicit (lastf, first) and expandable
atom labels (C1_*, $C etc) 
*/

BeginXlibNamespace()

class RefinementModel;
class ExplicitCAtomRef;

typedef TTypeListExt<ExplicitCAtomRef, IEObject> TAtomRefList;

class IAtomRef : public IEObject {
public:
  virtual ~IAtomRef()  {}
  // returns either an atom label or a string of C1_tol kind  or $C
  virtual olxstr GetExpression() const = 0;
  virtual int Expand(RefinementModel& rm, TAtomRefList& res, TResidue& resi) const = 0;
};

// C1, C1_$2, C1_2 expressions handling
class ExplicitCAtomRef : public IAtomRef  {
  TCAtom& atom;
  const smatd* matrix;
public:
  ExplicitCAtomRef(const ExplicitCAtomRef& ar) : 
    atom(ar.atom), matrix(ar.matrix) {}
  ExplicitCAtomRef(TCAtom& _atom, const smatd* _matrix=NULL) : 
    atom(_atom), matrix(_matrix) {}
  virtual olxstr GetExpression() const {  return atom.GetLabel();  }
  virtual int Expand(RefinementModel& rm, TAtomRefList& res, TResidue& resi) const {
    res.Add( new ExplicitCAtomRef(*this) );
    return 1;
  }
  TCAtom& GetAtom() {  return atom;  }
  const TCAtom& GetAtom() const {  return atom;  }
  const smatd* GetMatrix() const {  return matrix;  }
  /* full label with residue number, may throw an exception if both residue and matrix are to be appended */
  olxstr GetFullLabel(RefinementModel& rm, const TResidue& ref) const;
  // builds instance from C1 or C1_$1 expression for main residue, may return NULL
  static ExplicitCAtomRef* NewInstance(RefinementModel& rm, const olxstr& exp, TResidue* resi);
};

/* 
Last - last atom of a residue, 
First - first atom of a residue, 
* - all non H atoms of a residue
$Type - all Type atoms
$Type_tol - all Type atoms of a residue
C1_tol - all explicit atoms of a residue
C1_+ - an explicit atom of next residue
C1_- - an explicit atom of previous residue
*/
class ImplicitCAtomRef : public IAtomRef  {
  olxstr Name;
public:
  ImplicitCAtomRef(const ImplicitCAtomRef& ar) : Name(ar.Name) {}
  ImplicitCAtomRef(const olxstr& _Name) : Name(_Name) {}
  // * is special char
  virtual olxstr GetExpression() const {  return Name == '*' ? EmptyString : Name;  }
  virtual int Expand(RefinementModel& rm, TAtomRefList& res, TResidue& resi) const;
  // may return NULL
  static IAtomRef* NewInstance(RefinementModel& rm, const olxstr& exp, const olxstr& resi, TResidue* _resi)  {
    if( resi.IsEmpty() || _resi != NULL )  {  // a chance to create explicit reference
      if( exp.IndexOf('+')  == -1 &&
          exp.IndexOf('-')  == -1 &&
          exp.IndexOf('*')  == -1 )
      {
        int us_ind = exp.IndexOf('_');
        if( us_ind != -1 )  {
          if( us_ind+1 == exp.Length() )  // invalid reference
            return NULL;
          if( exp.CharAt(us_ind) != '$' )  { // is symm reference?
            if( !exp.SubStringFrom(us_ind).IsNumber() )  //is explicit?
              return new ImplicitCAtomRef(exp);
          }
        }
        return ExplicitCAtomRef::NewInstance(rm, exp, _resi);
      }
    }
    return new ImplicitCAtomRef(exp);
  }
};

//manages C1 > C5 and C5 < C1 expressions
class ListIAtomRef : public IAtomRef {
  IAtomRef &start, &end;
  olxstr op;
public:
  ListIAtomRef(IAtomRef& _start, IAtomRef& _end, const olxstr& _op) : 
    start(_start), end(_end), op(_op)  {  }
  virtual ~ListIAtomRef()  {
    delete &start;
    delete &end;
  }
  virtual bool IsExpandable() const {  return true;  }
  // * is special char
  virtual olxstr GetExpression() const {  return olxstr(start.GetExpression() << ' ' << op << ' ' << end.GetExpression());  }
  virtual int Expand(RefinementModel& rm, TAtomRefList& res, TResidue& resi) const;
};

class AtomRefList  {
  TTypeList<IAtomRef> refs;
  olxstr residue;
  RefinementModel& rm;
  olxstr expression;
  bool Valid;
  olxstr BuildExpression() const  {
    olxstr rv;
    for( int i=0; i < refs.Count(); i++ )  {
      rv << refs[i].GetExpression();
      if( (i+1) < refs.Count() )
        rv << ' ';
    }
  }
 public:
   AtomRefList(RefinementModel& rm, const olxstr& exp, const olxstr& resi=EmptyString);
  int Expand(RefinementModel& rm, TAtomRefList& res) const;
  olxstr GetExpression() const {  return Valid ? BuildExpression() : expression;  }
};

EndXlibNamespace()

#endif

