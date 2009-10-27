#ifndef __olx_catom_list_H
#define __olx_catom_list_H
#include "residue.h"
/* 
Atom list to handle list of explicit (by label), implicit (lastf, first) and expandable
atom labels (C1_*, $C etc) 
*/

BeginXlibNamespace()

// atom list types
const short 
  atom_list_type_none = 0, // any list type
  atom_list_type_pairs = 1,  // list is validated to have pairs
  atom_list_type_triplets = 2;  // list is validated to have triplets

class RefinementModel;
class ExplicitCAtomRef;

typedef TTypeListExt<ExplicitCAtomRef, IEObject> TAtomRefList;

class IAtomRef : public IEObject {
public:
  virtual ~IAtomRef()  {}
  // returns either an atom label or a string of C1_tol kind  or $C
  virtual olxstr GetExpression() const = 0;
  virtual bool IsExplicit() const = 0;
  virtual size_t Expand(RefinementModel& rm, TAtomRefList& res, TResidue& resi) const = 0;
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
  virtual bool IsExplicit() const {  return true;  }
  virtual size_t Expand(RefinementModel& rm, TAtomRefList& res, TResidue& resi) const {
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
  virtual bool IsExplicit() const {  return false;  }
  virtual size_t Expand(RefinementModel& rm, TAtomRefList& res, TResidue& resi) const;
  // may return NULL
  static IAtomRef* NewInstance(RefinementModel& rm, const olxstr& exp, const olxstr& resi, TResidue* _resi)  {
    if( resi.IsEmpty() || _resi != NULL )  {  // a chance to create explicit reference
      if( exp.IndexOf('+')  == InvalidIndex &&
          exp.IndexOf('-')  == InvalidIndex &&
          exp.IndexOf('*')  == InvalidIndex &&
          !exp.StartsFrom('$') )
      {
        size_t us_ind = exp.IndexOf('_');
        if( us_ind != InvalidIndex )  {
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
  virtual bool IsExplicit() const {  return false;  }
  // * is special char
  virtual olxstr GetExpression() const {  return olxstr(start.GetExpression() << ' ' << op << ' ' << end.GetExpression());  }
  virtual size_t Expand(RefinementModel& rm, TAtomRefList& res, TResidue& resi) const;
};

class AtomRefList  {
  TTypeList<IAtomRef> refs;
  olxstr residue;
  RefinementModel& rm;
  olxstr expression;
  bool Valid, ContainsImplicitAtoms;
  olxstr BuildExpression() const  {
    olxstr rv;
    for( size_t i=0; i < refs.Count(); i++ )  {
      rv << refs[i].GetExpression();
      if( (i+1) < refs.Count() )
        rv << ' ';
    }
    return rv;
  }
  void EnsureAtomPairs(RefinementModel& rm, TAtomRefList& al) const;
  void EnsureAtomTriplets(RefinementModel& rm, TAtomRefList& al) const;
public:
  /* creates an instance of the object from given expression for given residue class, number or
  alias. Empty residue specifies the main residue. */
  AtomRefList(RefinementModel& rm, const olxstr& exp, const olxstr& resi=EmptyString);
  /* expands the underlying expressions into a list. If the residue name is a class name (and 
  there are several residues of the kind), there will be more than one entry in the res with each
  entry corresponding to any particular residue. One of the list type constants can be provided to 
  validate the lists content to have pairs or triplets of atoms */
  void Expand(RefinementModel& rm, TTypeList<TAtomRefList>& res, const short atom_list_type=atom_list_type_none) const;
  /* recreates the expression for the object. If there are any explicit atom names - the new
  names will come from the updated model. Implicit atoms will stay as provided in the constructor*/
  olxstr GetExpression() const {  return Valid ? BuildExpression() : expression;  }
  /* expands the list and returns if resulting explicit list is not empty */
  bool IsExpandable(RefinementModel& rm, const short atom_list_type) const;
  /* this can be used to decide if the atom list is valid*/
  virtual bool IsExplicit() const {  return (!ContainsImplicitAtoms && residue.IsEmpty());  }
};

EndXlibNamespace()

#endif

