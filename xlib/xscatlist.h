#ifndef _olx_xscat_list
#define _olx_xscat_list
#include "xmodel.h"
#include "xexp_parser.h"

BeginXlibNamespace()

/* explicit atoms list represent a set of atoms, referenced explicetely by name, residue name
or symmetry equivalent, the list is can be evaluated at the creation time */
class XExplicitScattererList : public AScattererParamList {
  XScattererRefList atoms;
protected:
  virtual void DoExpand(IRefinementModel& parent, XResidue* resi, XScattererRefList& ag)  { 
    ag = atoms;  
  }
  virtual bool GetExplicit() const {  return true;  }
public:
  XExplicitScattererList(IRefinementModel& parent, const olxstr& exp)  { 
    TShelxAtomListParser(exp).Expand(parent, atoms, NULL);
  }
};

/* expandable atoms list maight contain indirection operators < or > and residue scrolling
 +,- operators. The list can be evalueated at any time, but might change at runtime */
class XExpandableScattererList : public AScattererParamList {
  TStrPObjList<olxstr, XScattererRef*> atoms;
protected:
  virtual void DoExpand(IRefinementModel& parent, XResidue* resi, XScattererRefList& ag)  {
    for( int i=0; i < atoms.Count(); i++ )  {
      if( atoms.Object(i) != NULL ) 
        atoms[i] = atoms.Object(i)->GetLabel();
    }
    TShelxAtomListParser ar( atoms.Text(' ') );
    ar.Expand(parent, ag, resi);
  }
  virtual bool GetExplicit() const {  return false;  }
public:
  XExpandableScattererList(IRefinementModel& parent, const olxstr& expression) : atoms(expression, ' ')  {
    for( int i=0; i < atoms.Count(); i++ )  {
      if( atoms[i].CharAt(0) == '$' )  //SFAC
        continue;
      if( atoms[i].IndexOf('+') != -1 || atoms[i].IndexOf('-') != -1 || // residue scrolling
          atoms[i].IndexOf('*') != -1 || //reference to all residues
          atoms[i].CharAt(0) == '>' || atoms[i].CharAt(0) == '<' ) //atom indirection
        continue;
      // here should be only proper atoms
      int eq_ind = atoms[i].IndexOf('$');
      int resi_ind = expression.IndexOf('_');
      olxstr resi_name = (resi_ind == -1 ? EmptyString : expression.SubStringFrom(resi_ind+1));
      // check if it is just an equivalent position
      const smatd* eqiv = NULL;
      int eqiv_ind = resi_name.IndexOf('$');
      if( eqiv_ind != -1 )  {  // 0 is for SFAC type, skipped above
        olxstr str_eqiv( resi_name.SubStringFrom(eqiv_ind+1) );
        if( !str_eqiv.IsNumber() )  
          throw TInvalidArgumentException(__OlxSourceInfo, olxstr("invalid equivalent id: ") << str_eqiv);
        int eqi = str_eqiv.ToInt()-1;
        if( eqi < 0 || eqi >= parent.UsedSymmCount() )  
          throw TInvalidArgumentException(__OlxSourceInfo, olxstr("invalid equivalent index: ") << str_eqiv);
        eqiv = parent.GetUsedSymm(eqi);
        resi_name = resi_name.SubStringTo(eqiv_ind);
      }
      if( !resi_name.IsEmpty() && !resi_name.IsNumber() )  // must be a number
        throw TInvalidArgumentException(__OlxSourceInfo, "invalid residue number");
      olxstr aname = ( (resi_ind == -1) ? expression : expression.SubStringTo(resi_ind) );
      XResidue* resi = parent.FindResidueByNumber(resi_name.ToInt());
      if( resi == NULL )  {
        TBasicApp::GetLog() << (olxstr("invalid residue '") << resi_name << "' in [" << expression << ']' );
        continue;
      }
      XScatterer* xs = resi->FindScattererByName(aname);
      if( xs == NULL )  {
        TBasicApp::GetLog() << (olxstr("invalid atom '") << aname << "' in [" << expression << ']' );
        continue;
      }
      atoms.Object(i) = new XScattererRef(xs, eqiv);
    }
  }
  virtual ~XExpandableScattererList()  {
    for( int i=0; i < atoms.Count(); i++ )
      if( atoms.Object(i) != NULL )  delete atoms.Object(i);
  }
};

class XParamAtomListFactory {
public:
  static AScattererParamList* New(IRefinementModel& parent, const olxstr& exp)  {
    if( exp.IndexOf('$') == 0 ||  // sfac 
        exp.IndexOf('*') == 0 ||  // all residues
        exp.IndexOf('>') != -1 || exp.IndexOf('<') != -1 || // residue indirections
        exp.IndexOf('+') != -1 || exp.IndexOf('-') != -1 )  // resifue scrolling
      return new XExpandableScattererList(parent, exp);
    else
      return new XExplicitScattererList(parent, exp);
  }
};

EndXlibNamespace()
#endif
