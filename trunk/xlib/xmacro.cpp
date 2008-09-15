#ifdef __BORLANC__
  #pragma hdrstop
#endif

#include "xmacro.h"
#include "xapp.h"

#include "p4p.h"
#include "mol.h"
#include "crs.h"
#include "ins.h"
#include "cif.h"
#include "hkl.h"

#include "unitcell.h"
#include "symmlib.h"
#include "symmtest.h"
#include "integration.h"
#include "utf8file.h"
#include "datafile.h"
#include "dataitem.h"
#include "fsext.h"


#include <stdarg.h>

#define xlib_InitMacro(macroName, validOptions, argc, desc)\
  lib.RegisterStaticMacro( new TStaticMacro(&XLibMacros::mac##macroName, #macroName, (validOptions), argc, desc))
#define xlib_InitFunc(funcName, argc, desc) \
  lib.RegisterStaticFunction( new TStaticFunction(&XLibMacros::fun##funcName, #funcName, argc, desc))

const olxstr XLibMacros::NoneString("none");


void XLibMacros::Export(TLibrary& lib)  {
  xlib_InitMacro(BrushHkl, "f-consider Friedel law", fpAny, "for high redundancy\
 data sets, removes equivalents with high sigma");
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(SG, "a", fpNone|fpOne, "suggest space group");
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(GraphSR, "b-number of bins", fpNone|fpOne|psFileLoaded,
"Prints a scale vs resolution graph for current file (fcf file must exist in current folder)");
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(Wilson, "b-number of bins&;p-uses linear vins for picture, otherwise uses spherical bins", 
    fpNone|fpOne|psFileLoaded, "Prints Wilson plot data");
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(TestSymm, "e-tolerance limit", fpNone|psFileLoaded, "Tests current \
  structure for missing symmetry");
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(VATA, EmptyString, fpAny|psFileLoaded,
"Compares current model with the cif file and write the report to provided file (appending)" );
  xlib_InitMacro(Clean, "npd-does not change atom types of NPD atoms\
&;f-does not run 'fuse' after the completion\
&;aq-disables analysis of the Q-peaks based on thresholds\
&;at-disables lonely atom types assignment to O and Cl", fpNone,
"Tidies up current model" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(AtomInfo, "", fpAny|psFileLoaded,
"Searches information for given atoms in the database" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(Compaq, "a-analyse connectivity on atom level, bu default fragment level is used", fpNone|psFileLoaded,
"Moves all atoms or fragments of the asymmetric unit as close to each other as possible." );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(Envi, "q-adds Q-peaks to the list&;h-adds hydrogen atoms to the list&;cs-leaves selection unchanged",
    fpNone|fpOne|fpTwo,
"This macro prints environment of any particular atom. Default search radius is 2.7A."  );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(AddSE, "", (fpAny^fpNone)|psFileLoaded,
"Tries to add a new symmetry element to current space group to form a new one. [-1] is for center of symmetry" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(Fuse, "f-removes symmetrical equivalents", fpNone|psFileLoaded,
"Re-initialises the connectivity list" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(Cif2Doc, "", fpNone|fpOne|psFileLoaded, "converts cif to a document" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(Cif2Tab, "", fpAny|psFileLoaded, "creates a table from a cif" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(CifMerge, "", (fpAny^fpNone)|psFileLoaded,
  "Merges loaded or provided as first argument cif with other cif(s)" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(CifExtract, "", fpTwo|psFileLoaded, "extract a list of items from one cif to another" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(VoidE, "", fpNone|psFileLoaded, "calculates number of electrons in the voids area" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(ChangeSG, "", fpOne|fpFour|psFileLoaded, "[shift] SG Changes space group of current structure" );
//_________________________________________________________________________________________________________________________
//_________________________________________________________________________________________________________________________
//_________________________________________________________________________________________________________________________

  xlib_InitFunc(FileName, fpNone|fpOne,
"If no arguments provided, returns file name of currently loaded file, for one\
 argument returns extracted file name");
//_________________________________________________________________________________________________________________________
  xlib_InitFunc(FileExt, fpNone|fpOne, "Retursn file extension. If no arguments provided - of currently loaded file");
  xlib_InitFunc(FilePath, fpNone|fpOne, "Returns file path. If no arguments provided - of currently loaded file");
  xlib_InitFunc(FileFull, fpNone, "Returns full path of currently loaded file");
  xlib_InitFunc(FileDrive, fpNone|fpOne, "Returns file drive. If no arguments provided - of currently loaded file");
  xlib_InitFunc(IsFileLoaded, fpNone, "Returns true/false");
  xlib_InitFunc(IsFileType, fpOne, "Checks type of currently loaded file [ins,res,ires,cif,mol,xyz]");
//_________________________________________________________________________________________________________________________
  xlib_InitFunc(BaseDir, fpNone|fpOne, "Returns the startup folder");
  xlib_InitFunc(HKLSrc, fpNone|fpOne|psFileLoaded, "Returns/sets hkl source for currently loaded file");
//_________________________________________________________________________________________________________________________
  xlib_InitFunc(ATA, fpAny|psFileLoaded, "Test current structure agains database.\
  (Atom Tye Assignment). Returns true if any atom type changed" );
//_________________________________________________________________________________________________________________________
  xlib_InitFunc(VSS, fpOne|psFileLoaded, "Validate Structure or Solution.\
  Takes a boolean value. if value is true, the number of tested atoms is limited\
 by the 18A rule. Returns proportion of know atom types to the all atoms number." );
//_________________________________________________________________________________________________________________________
  xlib_InitFunc(RemoveSE, fpOne|psFileLoaded, "Returns a new space group name without provided element");
//_________________________________________________________________________________________________________________________
}
//..............................................................................
void XLibMacros::ParseDoubles(TStrObjList& Cmds, int cnt, ...)  {
  va_list argptr;
  va_start(argptr, cnt);
  if( cnt == 0 )  {
    va_end(argptr);
    return;
  }
  TPtrList<double> vars(cnt);
  for( int i=0; i < cnt; i++ )
    vars[i] = va_arg(argptr, double*);
  int fc=0;
  for( int i=0; i < Cmds.Count(); i++ )  {
    if( Cmds[i].IsNumber() )  {
      *vars[fc] = Cmds[i].ToDouble();
      Cmds.Delete(i);
      if( fc >= cnt )  break;
      i--;
    }
  }
  va_end(argptr);
}
//..............................................................................
//..............................................................................
void XLibMacros::macFuse(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  TXApp::GetInstance().XFile().GetLattice().Uniq( Options.Contains("f") );
}
//..............................................................................
void XLibMacros::macAddSE(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  TXApp& xapp = TXApp::GetInstance();
  
  TLattice& latt = xapp.XFile().GetLattice();
  TUnitCell& uc = latt.GetUnitCell();
  TAsymmUnit& au = latt.GetAsymmUnit();
  if( au.AtomCount() == 0 )  {
    E.ProcessingError(__OlxSrcInfo, "Empty asymmetric unit");
    return;
  }
  TSpaceGroup* sg = NULL;
  try  { sg = &xapp.XFile().GetLastLoaderSG();  }
  catch(...)  {
    E.ProcessingError(__OlxSrcInfo, "Could not identify current space group");
    return;
  }
  if( sg->IsCentrosymmetric() && Cmds.Count() == 1 )  {
    E.ProcessingError(__OlxSrcInfo, "Centrosymmetric space group");
    return;
  }
  if( Cmds.Count() == 1 )  {
    smatd_list ml;
    sg->GetMatrices(ml, mattAll);
    ml.SetCapacity( ml.Count()*2 );
    int mc = ml.Count();
    for( int i=0; i < mc; i++ )  {
      ml.AddCCopy(ml[i]);
      ml[i+mc] *= -1;
    }
    for( int i=0; i < TSymmLib::GetInstance()->SGCount(); i++ )  {
      if( TSymmLib::GetInstance()->GetGroup(i) == ml )  {
        xapp.GetLog() << "found " << TSymmLib::GetInstance()->GetGroup(i).GetName() << '\n';
        sg = &TSymmLib::GetInstance()->GetGroup(i);
      }
    }
  }
  else if( Cmds.Count() == 2 )  {
    TSAtomPList atoms;
    xapp.FindSAtoms(EmptyString, atoms);
    for( int i=0; i < au.AtomCount(); i++ )  {
      for( int j=i+1; j < au.AtomCount(); j++ )  {
        if( au.GetAtom(j).IsDeleted() )  continue;
        double d = uc.FindClosestDistance(au.GetAtom(i), au.GetAtom(j));
        if( d < 0.5 )  {
          au.GetAtom(i).SetDeleted(true);
          break;
        }
      }
    }
    latt.Init();
    latt.Compaq();
    latt.CompaqAll();
    return;
  }
  TSymmTest st(uc);
  smatd m;
  m.r.I();
  double tol = 0.1;
  st.TestMatrix(m, tol);
  if( !st.GetResults().IsEmpty() )  {
    int ind = st.GetResults().Count()-1;
    double match = st.GetResults()[ind].Count()*200/st.AtomCount();
    while( match > 125 && tol > 1e-4 )  {
      tol /= 4;
      st.TestMatrix(m, tol);
      ind = st.GetResults().Count()-1;
      match = st.GetResults().IsEmpty() ? 0.0 :st.GetResults()[ind].Count()*200/st.AtomCount();
      continue;
    }
    if( st.GetResults().IsEmpty() )  {
      E.ProcessingError(__OlxSrcInfo, "ooops...");
      return;
    }
    TEStrBuffer out;
    vec3d trans( st.GetResults()[ind].Center );
    //TVectorD trans = st.GetGravityCenter();
    trans /= 2;
    trans *= -1;
    m.t = trans;
    TSAtomPList atoms;
    xapp.FindSAtoms(EmptyString, atoms);
    xapp.XFile().GetLattice().TransformFragments(atoms, m);
    au.ChangeSpaceGroup(*sg);
    xapp.XFile().GetLastLoader()->GetAsymmUnit().ChangeSpaceGroup(*sg);
    latt.Init();
    for( int i=0; i < au.AtomCount(); i++ )  {
      for( int j=i+1; j < au.AtomCount(); j++ )  {
        if( au.GetAtom(j).IsDeleted() )  continue;
        double d = uc.FindClosestDistance(au.GetAtom(i), au.GetAtom(j));
        if( d < 0.5 )  {
          au.GetAtom(i).SetDeleted(true);
          break;
        }
      }
    }
//    latt.OnStructureGrow->SetEnabled(false);
//    latt.OnStructureUniq->SetEnabled(false);
    latt.Init();
    latt.Compaq();
//    latt.OnStructureGrow->SetEnabled(true);
//    latt.OnStructureUniq->SetEnabled(true);
    latt.CompaqAll();
  }
  else  {
    E.ProcessingError(__OlxSrcInfo, "could not find interatomic relations");
  }
}
//..............................................................................
void XLibMacros::macCompaq(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  if( Options.Contains("a") )  
    TXApp::GetInstance().XFile().GetLattice().CompaqAll();
  else
    TXApp::GetInstance().XFile().GetLattice().Compaq();
}
//..............................................................................
void XLibMacros::macEnvi(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  double r = 2.7;
  ParseDoubles(Cmds, 1, &r);
  if( r < 1 || r > 10 )  {
    E.ProcessingError(__OlxSrcInfo, "radius must be within [1;10] range" );
    return;
  }
  TSAtomPList atoms;
  TXApp& xapp = TXApp::GetInstance();
  if( !xapp.FindSAtoms(Cmds.Text(' '), atoms) )  {
    E.ProcessingError(__OlxSrcInfo, "no atoms provided" );
    return;
  }

  TStrList output;
  TPtrList<TBasicAtomInfo> Exceptions;
  Exceptions.Add(xapp.AtomsInfo()->FindAtomInfoBySymbol("Q"));
  Exceptions.Add(xapp.AtomsInfo()->FindAtomInfoBySymbol("H"));
  if( Options.Contains('q') )
    Exceptions.Remove(xapp.AtomsInfo()->FindAtomInfoBySymbol("Q"));
  if( Options.Contains('h') )
    Exceptions.Remove(xapp.AtomsInfo()->FindAtomInfoBySymbol("H"));

  TSAtom& SA = *atoms[0];
  TLattice& latt = TXApp::GetInstance().XFile().GetLattice();
  TAsymmUnit& au = latt.GetAsymmUnit();
  vec3d V;
  smatd_list* L;
  TArrayList< AnAssociation3<TCAtom*, vec3d, smatd> > rowData;
  TCAtomPList allAtoms;

  for( int i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).IsDeleted() )  continue;
    bool skip = false;
    for( int j=0; j < Exceptions.Count(); j++ )  {
      if( au.GetAtom(i).GetAtomInfo() == *Exceptions[j] )
      {  skip = true;  break;  }
    }
    if( !skip )  allAtoms.Add( &au.GetAtom(i) );
  }
  for( int i=0; i < au.CentroidCount(); i++ )  {
    if( au.GetCentroid(i).IsDeleted() )  continue;
    allAtoms.Add( &au.GetCentroid(i) );
  }
  for( int i=0; i < allAtoms.Count(); i++ )  {
    if( SA.CAtom().GetId() == i )
      L = latt.GetUnitCell().GetInRange(SA.ccrd(), allAtoms[i]->ccrd(), r, false);
    else
      L = latt.GetUnitCell().GetInRange(SA.ccrd(), allAtoms[i]->ccrd(), r, true);
    if( L->Count() != 0 )  {
      for( int j=0; j < L->Count(); j++ )  {
        const smatd& m = L->Item(j);
        V = m * allAtoms[i]->ccrd() - SA.ccrd();
        au.CellToCartesian(V);
        if( V.Length() == 0 )  // symmetrical equivalent?
          continue;
        rowData.Add( AnAssociation3<TCAtom*, vec3d, smatd>(allAtoms[i], V, m) );
      }
    }
    delete L;
  }
  TTTable<TStrList> table(rowData.Count(), rowData.Count()+2); // +SYM + LEN
  table.ColName(0) = SA.GetLabel();
  table.ColName(1) = "SYMM";
  rowData.BubleSorter.Sort<XLibMacros::TEnviComparator>(rowData);
  for( int i=0; i < rowData.Count(); i++ )  {
    const AnAssociation3<TCAtom*, vec3d, smatd>& rd = rowData[i];
    table.RowName(i) = rd.GetA()->GetLabel();
    table.ColName(i+2) = table.RowName(i);
    if( rd.GetC().r.IsI() && rd.GetC().t.IsNull() )
     table[i][1] = 'I';  // identity
    else
      table[i][1] = TSymmParser::MatrixToSymm( rd.GetC() );
    table[i][0] = olxstr::FormatFloat(2, rd.GetB().Length());
    for( int j=0; j < rowData.Count(); j++ )  {
      if( i == j )  { table[i][j+2] = '-'; continue; }
      if( i < j )   { table[i][j+2] = '-'; continue; }
      const AnAssociation3<TCAtom*, vec3d, smatd>& rd1 = rowData[j];
      if( rd.GetB().Length() != 0 && rd1.GetB().Length() != 0 )  {
        double angle = rd.GetB().CAngle(rd1.GetB());
        angle = acos(angle)*180/M_PI;
        table[i][j+2] = olxstr::FormatFloat(1, angle);
      }
      else
        table[i][j+2] = '-';
    }
  }
  table.CreateTXTList(output, EmptyString, true, true, ' ');
  TBasicApp::GetLog() << output << '\n';
  //TBasicApp::GetLog() << ("----\n");
  //if( !latt.IsGenerated() )
  //  TBasicApp::GetLog() << ("Use move \"atom_to atom_to_move\" command to move the atom/fragment\n");
  //else
  //  TBasicApp::GetLog() << ("Use move \"atom_to atom_to_move -c\" command to move the atom/fragment\n");
}
//..............................................................................
void XLibMacros::funRemoveSE(const TStrObjList &Params, TMacroError &E)  {
  TXApp& xapp = TXApp::GetInstance();
  TSpaceGroup* sg = NULL;
  try  { sg = &xapp.XFile().GetLastLoaderSG();  }
  catch(...)  {
    E.ProcessingError(__OlxSrcInfo, "could not identify current space group");
    return;
  }
  if( Params[0] == "-1" )  {
    if( !sg->IsCentrosymmetric() )  {
      E.SetRetVal( sg->GetName() );
      return;
    }
    smatd_list ml;
    sg->GetMatrices(ml, mattAll^mattInversion);
    TPSTypeList<double, TSpaceGroup*> sglist;
    for( int i=0; i < TSymmLib::GetInstance()->SGCount(); i++ )  {
      double st=0;
      if( TSymmLib::GetInstance()->GetGroup(i).Compare(ml, st) )
        sglist.Add(st, &TSymmLib::GetInstance()->GetGroup(i) );
    }
    E.SetRetVal( sglist.IsEmpty() ? sg->GetName() : sglist.Object(0)->GetName() );
  }
}
//..............................................................................
void XLibMacros::funFileName(const TStrObjList &Params, TMacroError &E)  {
  olxstr Tmp;
  if( !Params.IsEmpty() )
    Tmp = TEFile::ExtractFileName(Params[0]);
  else  {
    if( TXApp::GetInstance().XFile().GetLastLoader() != NULL )
      Tmp = TEFile::ExtractFileName( TXApp::GetInstance().XFile().GetFileName() );
    else
      Tmp = NoneString;
  }
  E.SetRetVal( TEFile::ChangeFileExt(Tmp, EmptyString) );
}
//..............................................................................
void XLibMacros::funFileExt(const TStrObjList &Params, TMacroError &E)  {
  olxstr Tmp;
  if( !Params.IsEmpty() )
    E.SetRetVal( TEFile::ExtractFileExt(Params[0]) );
  else  {
    if( TXApp::GetInstance().XFile().GetLastLoader() != NULL )
      E.SetRetVal( TEFile::ExtractFileExt(TXApp::GetInstance().XFile().GetFileName()) );
    else
      E.SetRetVal( NoneString );
  }
}
//..............................................................................
void XLibMacros::funFilePath(const TStrObjList &Params, TMacroError &E)  {
  olxstr Tmp;
  if( !Params.IsEmpty() )
    Tmp = TEFile::ExtractFilePath( Params[0] );
  else  {
    if( TXApp::GetInstance().XFile().GetLastLoader() )
      Tmp = TEFile::ExtractFilePath( TXApp::GetInstance().XFile().GetFileName() );
    else
      Tmp = NoneString;
  }
  // see notes in funBaseDir
  TEFile::RemoveTrailingBackslashI(Tmp);
  E.SetRetVal( Tmp );
}
//..............................................................................
void XLibMacros::funFileDrive(const TStrObjList &Params, TMacroError &E)  {
  olxstr Tmp;
  if( !Params.IsEmpty() )
    E.SetRetVal( TEFile::ExtractFileDrive(Params[0]) );
  else  {
    if( TXApp::GetInstance().XFile().GetLastLoader() != NULL )
      E.SetRetVal( TEFile::ExtractFileDrive(TXApp::GetInstance().XFile().GetFileName()) );
    else
      E.SetRetVal( NoneString );
  }
}
//..............................................................................
void XLibMacros::funFileFull(const TStrObjList &Params, TMacroError &E)  {
  if( TXApp::GetInstance().XFile().GetLastLoader() == NULL )
    E.SetRetVal( NoneString );
  else
    E.SetRetVal( TXApp::GetInstance().XFile().GetFileName() );
}
//..............................................................................
void XLibMacros::funIsFileLoaded(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal( TXApp::GetInstance().XFile().GetLastLoader() != NULL );
}
//..............................................................................
void XLibMacros::funIsFileType(const TStrObjList& Params, TMacroError &E) {
  if( Params[0].Comparei("ins") == 0 )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TIns>() && 
      (TEFile::ExtractFileExt(TXApp::GetInstance().XFile().GetFileName()).Comparei("ins") == 0) );
  }
  else if( Params[0].Comparei("res") == 0 )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TIns>() && 
      (TEFile::ExtractFileExt(TXApp::GetInstance().XFile().GetFileName()).Comparei("res") == 0) );
  }
  else if( Params[0].Comparei("ires") == 0 )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TIns>() );
  }
  else if( Params[0].Comparei("cif") == 0 )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TCif>() );
  }
  else if( Params[0].Comparei("p4p") == 0 )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TP4PFile>() );
  }
  else if( Params[0].Comparei("mol") == 0 )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TMol>() );
  }
  else if( Params[0].Comparei("xyz") == 0 )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TMol>() );
  }
  else if( Params[0].Comparei("crs") == 0 )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TCRSFile>() );
  }
  else
    E.SetRetVal( false );
}
//..............................................................................
void XLibMacros::funBaseDir(const TStrObjList& Params, TMacroError &E)  {
  olxstr tmp( TBasicApp::GetInstance()->BaseDir() );
  // remove the trailing backslash, as it causes a lot of problems with
  // passing parameters to other programs:
  // windows parser assumes that \" is " and does wrong parsing...
  if( !tmp.IsEmpty() )  tmp.SetLength( tmp.Length()-1 );
  E.SetRetVal( tmp );
}
//..............................................................................
void XLibMacros::funHKLSrc(const TStrObjList& Params, TMacroError &E)  {
  if( Params.Count() == 1 )
    TXApp::GetInstance().XFile().GetLastLoader()->SetHKLSource( Params[0] );
  else
    E.SetRetVal( TXApp::GetInstance().XFile().GetLastLoader()->GetHKLSource() );
}
//..............................................................................
void XLibMacros::macCif2Doc(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  olxstr CifDictionaryFile( xapp.GetCifTemplatesDir() + "cifindex.dat");
  if( Cmds.IsEmpty() )  {
    TStrList Output;
    olxstr CDir = TEFile::CurrentDir();
    TEFile::ChangeDir( xapp.GetCifTemplatesDir() );
    TEFile::ListCurrentDir(Output, "*.rtf;*html;*.htm", sefFile);
    TEFile::ChangeDir(CDir);
    xapp.GetLog() << "Templates found: \n";
    xapp.GetLog() << Output << '\n';
    return;
  }

  olxstr TN = Cmds[0];
  if( !TEFile::FileExists(TN) )
    TN = xapp.GetCifTemplatesDir() + TN;
  if( !TEFile::FileExists(TN) )  {
    Error.ProcessingError(__OlxSrcInfo, "template for CIF does not exist: ") << Cmds[0];
    return;
  }
  // resolvind the index file
  if( !TEFile::FileExists(CifDictionaryFile) )  {
    Error.ProcessingError(__OlxSrcInfo, "CIF dictionary does not exist" );
    return;
  }

  TCif *Cif, Cif1(xapp.AtomsInfo());
  if( xapp.CheckFileType<TCif>() )
    Cif = (TCif*)xapp.XFile().GetLastLoader();
  else  {
    olxstr cifFN = TEFile::ChangeFileExt( xapp.XFile().GetFileName(), "cif");
    if( TEFile::FileExists( cifFN ) ) 
      Cif1.LoadFromFile( cifFN );
    else  {
      Error.ProcessingError(__OlxSrcInfo, "existing cif is expected");
      return;
    }
    Cif = &Cif1;
  }

  TStrList SL, Dic;
  olxstr RF = TEFile::ChangeFileExt(Cif->GetFileName(), TEFile::ExtractFileExt(TN));
  SL.LoadFromFile( TN );
  Dic.LoadFromFile( CifDictionaryFile );
  for( int i=0; i < SL.Count(); i++ )
    Cif->ResolveParamsFromDictionary(Dic, SL[i], '%', '%', &XLibMacros::CifResolve);
  TUtf8File::WriteLines( RF, SL, false );
  TBasicApp::GetLog().Info(olxstr("Document name: ") << RF);
}
//..............................................................................
void XLibMacros::macCif2Tab(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  olxstr CifTablesFile( xapp.GetCifTemplatesDir() + "tables.xlt");
  olxstr CifDictionaryFile( xapp.GetCifTemplatesDir() + "cifindex.dat");
  if( Cmds.IsEmpty() )  {
    if( !TEFile::FileExists(CifTablesFile) )  {
      Error.ProcessingError(__OlxSrcInfo, "tables definition file is not found" );
      return;
    }

    TDataFile DF;
    TStrList SL;
    TDataItem *Root;
    olxstr Tmp;
    DF.LoadFromXLFile(CifTablesFile, &SL);

    Root = DF.Root().FindItemCI("Cif_Tables");
    if( Root != NULL )  {
      xapp.GetLog().Info("Found table definitions:");
      for( int i=0; i < Root->ItemCount(); i++ )  {
        Tmp = "Table "; 
        Tmp << Root->Item(i).GetName()  << "(" << " #" << (int)i+1 <<  "): caption <---";
        xapp.GetLog().Info(Tmp);
        xapp.GetLog().Info(Root->Item(i).GetFieldValueCI("caption"));
        xapp.GetLog().Info("--->");
      }
    }
    else  {
      Error.ProcessingError(__OlxSrcInfo, "tables definition node is not found" );
      return;
    }
    return;
  }
  TCif *Cif, Cif1(xapp.AtomsInfo());

  if( xapp.CheckFileType<TCif>() )
    Cif = (TCif*)xapp.XFile().GetLastLoader();
  else  {
    olxstr cifFN = TEFile::ChangeFileExt( xapp.XFile().GetFileName(), "cif");
    if( TEFile::FileExists( cifFN ) )  {
      Cif1.LoadFromFile( cifFN );
    }
    else
        throw TFunctionFailedException(__OlxSourceInfo, "existing cif is expected");
    Cif = &Cif1;
  }

  TStrList SL, Dic, SL1;
  TDataFile DF;
  TDataItem *TD, *Root;
  TTTable<TStrList> DT;
  DF.LoadFromXLFile(CifTablesFile, NULL);
  Dic.LoadFromFile( CifDictionaryFile );

  olxstr RF = TEFile::ExtractFilePath(Cif->GetFileName()) + "tables.html", Tmp;
  Root = DF.Root().FindItemCI("Cif_Tables");
  smatd_list SymmList;
  for( int i=0; i < Cmds.Count(); i++ )  {
    TD = NULL;
    if( Cmds[i].IsNumber() )  {
      int index = Cmds[i].ToInt();
      if( index >=0 && index < Root->ItemCount() )
        TD = &Root->Item(index);
    }
    if( TD == NULL  )
      TD = Root->FindItem(Cmds[i]);
    if( TD == NULL )  {
      xapp.GetLog().Warning( olxstr("Could not find table definition: ") << Cmds[i] );
      continue;
    }
    if( !TD->GetName().Comparei("footer") || !TD->GetName().Comparei("header") )  {
      olxstr fn = TD->GetFieldValue("source");
      if( !TEFile::IsAbsolutePath(fn) )
        fn = xapp.GetCifTemplatesDir() + fn;
      SL1.LoadFromFile( fn );
      for( int j=0; j < SL1.Count(); j++ )  {
        Cif->ResolveParamsFromDictionary(Dic, SL1[j], '%', '%', &XLibMacros::CifResolve);
        SL.Add( SL1[j] );
      }
      continue;
    }
    if( Cif->CreateTable(TD, DT, SymmList) )  {
      Tmp = "Table "; Tmp << (i+1) << ' ' << TD->GetFieldValueCI("caption");
      Tmp.Replace("%DATA_NAME%", Cif->GetDataName());
      if( Tmp.IndexOf("$") >= 0 )
        ProcessExternalFunction( Tmp );
      SL1.Clear();
      // attributes of the row names ...
      SL1.Add(EmptyString);
      for( int j=0; j < TD->ItemCount(); j++ )
        SL1.Add( TD->Item(j).GetFieldValue("cola", EmptyString) );

      olxstr footer;
      for(int i=0; i < SymmList.Count(); i++ )  {
        footer << "<sup>" << (i+1) << "</sup>" <<
           TSymmParser::MatrixToSymm(SymmList[i]);
        if( (i+1) < SymmList.Count() )
          footer << "; ";
      }


      DT.CreateHTMLList(SL, Tmp,
                      footer,
                      true, false,
                      TD->GetFieldValue("tita", EmptyString),  // title paragraph attributes
                      TD->GetFieldValue("foota", EmptyString),  // footer paragraph attributes
                      TD->GetFieldValue("tha", EmptyString), // const olxstr& colTitleRowAttr,
                      TD->GetFieldValue("taba", EmptyString),  //const olxstr& tabAttr,
                      TD->GetFieldValue("rowa", EmptyString),  //const olxstr& rowAttr,
                      SL1, //const TStrList& colAttr,
                      true,
                      TD->GetFieldValue("coln", "1").ToInt()
                      ); //bool Format) const  {

      //DT.CreateHTMLList(SL, Tmp, true, false, true);
    }
  }
  TUtf8File::WriteLines(RF, SL, false);
  TBasicApp::GetLog().Info(olxstr("Tables file: ") << RF);
}
//..............................................................................
olxstr XLibMacros::CifResolve(const olxstr& func)  {
  using namespace olex;
  IOlexProcessor* op = IOlexProcessor::GetInstance();
  if( op == NULL )  return func;
  olxstr rv;
  if( op->executeFunction(func, rv) )
    return rv;
  return func;
}
//..............................................................................
bool XLibMacros::ProcessExternalFunction(olxstr& func)  {
  using namespace olex;
  IOlexProcessor* op = IOlexProcessor::GetInstance();
  if( op == NULL )  return false;
  olxstr rv;
  if( op->executeFunction(func, rv) )  {
    func = rv;
    return true;
  }
  return false;
}
//..............................................................................
void XLibMacros::MergePublTableData(TCifLoopTable& to, TCifLoopTable& from)  {
  if( from.RowCount() == 0 )  return;
  static const olxstr authorNameCN("_publ_author_name");
  // create a list of unique colums, and prepeare them for indexed access
  TSStrPObjList<olxstr, AnAssociation2<int,int>, false> uniqCols;
  for( int i=0; i < from.ColCount(); i++ )  {
    if( uniqCols.IndexOfComparable( from.ColName(i) ) == -1 )  {
      uniqCols.Add( from.ColName(i), AnAssociation2<int,int>(i, -1) );
    }
  }
  for( int i=0; i < to.ColCount(); i++ )  {
    int ind = uniqCols.IndexOfComparable( to.ColName(i) );
    if( ind == -1 )
      uniqCols.Add( to.ColName(i), AnAssociation2<int,int>(-1, i) );
    else
      uniqCols.Object(ind).B() = i;
  }
  // add new columns, if any
  for( int i=0; i < uniqCols.Count(); i++ ) {
    if( uniqCols.Object(i).GetB() == -1 )  {
      to.AddCol( uniqCols.GetComparable(i) );
      uniqCols.Object(i).B() = to.ColCount() - 1;
    }
  }
  /* by this point the uniqCols contains all the column names and the association
  holds corresponding column indexes in from and to tables */
  // the actual merge, by author name
  int authNCI = uniqCols.IndexOfComparable( authorNameCN );
  if( authNCI == -1 )  return;  // cannot do much, can we?
  AnAssociation2<int,int> authCA( uniqCols.Object(authNCI) );
  if( authCA.GetA() == -1 )  return;  // no author?, bad ...
  for( int i=0; i < from.RowCount(); i++ )  {
    int ri = -1;
    for( int j=0; j < to.RowCount(); j++ )  {
      if( to[j][ authCA.GetB() ].Comparei( from[i][ authCA.GetA() ]) == 0 )  {
        ri = j;
        break;
      }
    }
    if( ri == -1 )  {  // add a new row
      to.AddRow( EmptyString );
      ri = to.RowCount()-1;
    }
    for( int j=0; j < uniqCols.Count(); j++ )  {
      AnAssociation2<int,int>& as = uniqCols.Object(j);
      if( as.GetA() == -1 )  continue;
      to[ ri ][as.GetB()] = from[i][ as.GetA() ];
    }
  }
  // null the objects - they must not be here anyway ..
  for( int i=0; i < to.RowCount(); i++ )  {
    for( int j=0; j < to.ColCount(); j++ )  {
      if( to[i].Object(j) == NULL )
        to[i].Object(j) = new TCifLoopData(true);
      to[i].Object(j)->String = true;
    }
  }
}
//..............................................................................
void XLibMacros::macCifMerge(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  TCif *Cif, Cif1(xapp.AtomsInfo()), Cif2(xapp.AtomsInfo());

  if( xapp.CheckFileType<TCif>() )
    Cif = (TCif*)xapp.XFile().GetLastLoader();
  else  {
    olxstr cifFN = TEFile::ChangeFileExt( xapp.XFile().GetFileName(), "cif");
    if( TEFile::FileExists( cifFN ) )  {
      Cif2.LoadFromFile( cifFN );
    }
    else
      throw TFunctionFailedException(__OlxSourceInfo, "existing cif is expected");
    Cif = &Cif2;
  }

  TCifLoop& publ_info = Cif->PublicationInfoLoop();

  for( int i=0; i < Cmds.Count(); i++ )  {
    try {
      IInputStream *is = TFileHandlerManager::GetInputStream(Cmds[i]);
      if( is == NULL )  {
        TBasicApp::GetLog().Error( olxstr("Could not find file: ") << Cmds[i] );
        continue;
      }
      TStrList sl;
      sl.LoadFromTextStream(*is);
      delete is;
      Cif1.LoadFromStrings(sl);
    }
    catch( ... )  {    }  // most like the cif does not have cell, so pass it
    TCifLoop& pil = Cif1.PublicationInfoLoop();
    for( int j=0; j < Cif1.ParamCount(); j++ )  {
      if( !Cif->ParamExists(Cif1.Param(j)) )
        Cif->AddParam(Cif1.Param(j), Cif1.ParamValue(j));
      else
        Cif->SetParam(Cif1.Param(j), Cif1.ParamValue(j));
    }
    // update publication info loop
    MergePublTableData( publ_info.Table(), pil.Table() );
  }
  TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG( Cif->GetAsymmUnit() );
  if( sg != NULL )  {
    if( !Cif->ParamExists("_symmetry_cell_setting") )
      Cif->AddParam("_symmetry_cell_setting", sg->GetBravaisLattice().GetName(), true);
    else  {
      TCifData* cd = Cif->FindParam("_symmetry_cell_setting");
      if( cd->Data->IsEmpty() )
        cd->Data->Add( sg->GetBravaisLattice().GetName() );
      else
        cd->Data->String(0) = sg->GetBravaisLattice().GetName();
      cd->String = true;
    }
    if( !Cif->ParamExists("_symmetry_space_group_name_H-M") )
      Cif->AddParam("_symmetry_space_group_name_H-M", sg->GetFullName(), true);
    else  {
      TCifData* cd = Cif->FindParam("_symmetry_space_group_name_H-M");
      if( cd->Data->IsEmpty() )
        cd->Data->Add( sg->GetFullName() );
      else
        cd->Data->String(0) = sg->GetFullName();
      cd->String = true;
    }
    if( !Cif->ParamExists("_symmetry_space_group_name_Hall") )
      Cif->AddParam("_symmetry_space_group_name_Hall", sg->GetHallSymbol(), true);
    else  {
      TCifData* cd = Cif->FindParam("_symmetry_space_group_name_Hall");
      if( cd->Data->IsEmpty() )
        cd->Data->Add( sg->GetHallSymbol() );
      else
        cd->Data->String(0) = sg->GetHallSymbol();
      cd->String = true;
    }
  }
  else
    TBasicApp::GetLog().Error("Could not locate space group ...");
  Cif->Group();
  Cif->SaveToFile( Cif->GetFileName() );
}
//..............................................................................
void XLibMacros::macCifExtract(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  olxstr Dictionary = Cmds[0];
  if( !TEFile::FileExists(Dictionary) )  {  // check if the dictionary exists
    Dictionary = xapp.GetCifTemplatesDir();  Dictionary << Cmds[0];
    if( !TEFile::FileExists(Dictionary) )  {
      Error.ProcessingError(__OlxSrcInfo, "dictionary file does not exists" );
      return;
    }
  }

  TCif In(xapp.AtomsInfo()),  Out(xapp.AtomsInfo()), *Cif, Cif1(xapp.AtomsInfo());

  if( xapp.CheckFileType<TCif>() )
    Cif = (TCif*)xapp.XFile().GetLastLoader();
  else  {
    olxstr cifFN = TEFile::ChangeFileExt( xapp.XFile().GetFileName(), "cif");
    if( TEFile::FileExists( cifFN ) )  {
      Cif1.LoadFromFile( cifFN );
    }
    else
      throw TFunctionFailedException(__OlxSourceInfo, "existing cif is expected");
    Cif = &Cif1;
  }

  try  {  In.LoadFromFile(Dictionary);  }
  catch( TExceptionBase& )  {
    Error.ProcessingError(__OlxSrcInfo, "could not load dictionary file" );
    return;
  }

  TCifData *CifData;
  for( int i=0; i < In.ParamCount(); i++ )  {
    CifData = Cif->FindParam(In.Param(i));
    if( CifData )
      Out.AddParam(In.Param(i), CifData);
  }
  try  {  Out.SaveToFile(Cmds[1]);  }
  catch( TExceptionBase& )  {
    Error.ProcessingError(__OlxSrcInfo, "could not save file: ") << Cmds[1];
    return;
  }
}
//..............................................................................
struct XLibMacros_StrF  {
  int h, k, l;
  double ps;
  TEComplex<double> v;
};
void XLibMacros::macVoidE(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp& XApp = TXApp::GetInstance();
  double F000 = 0;
  double factor = 2;
  TRefList refs;
  TArrayList<TEComplex<double> > F;
  TAsymmUnit& au = XApp.XFile().GetAsymmUnit();
  const TUnitCell& uc = XApp.XFile().GetUnitCell();
  // space group matrix list
  TSpaceGroup* sg = NULL;
  try  { sg = &XApp.XFile().GetLastLoaderSG();  }
  catch(...)  {
    E.ProcessingError(__OlxSrcInfo, "could not locate space group");
    return;
  }
  smatd_list ml;
  sg->GetMatrices(ml, mattAll^mattInversion);
  for( int i=0; i < au.AtomCount(); i++ )  {
    TCAtom& ca = au.GetAtom(i);
    if( ca.IsDeleted() || ca.GetAtomInfo() == iQPeakIndex )  
      continue;
    int ec = (ca.GetAtomInfo() == iDeuteriumIndex) ? 1 : ca.GetAtomInfo().GetIndex()+1;
    F000 += ec*uc.MatrixCount()*ca.GetOccp();
  }
  olxstr fcffn = TEFile::ChangeFileExt(XApp.XFile().GetFileName(), "fcf");
  if( !TEFile::FileExists(fcffn) )  {
    fcffn = TEFile::ChangeFileExt(XApp.XFile().GetFileName(), "fco");
    if( !TEFile::FileExists(fcffn) )  {
      E.ProcessingError(__OlxSrcInfo, "please load fcf file or make sure the one exists in current folder");
      return;
    }
  }
  TCif cif( XApp.AtomsInfo() );
  cif.LoadFromFile( fcffn );
//  F000 = cif.GetSParam("_exptl_crystal_F_000").ToDouble();
  TCifLoop* hklLoop = cif.FindLoop("_refln");
  if( hklLoop == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "no hkl loop found");
    return;
  }
  int hInd = hklLoop->Table().ColIndex("_refln_index_h");
  int kInd = hklLoop->Table().ColIndex("_refln_index_k");
  int lInd = hklLoop->Table().ColIndex("_refln_index_l");
  // list 3, F
  int mfInd = hklLoop->Table().ColIndex("_refln_F_meas");
  int sfInd = hklLoop->Table().ColIndex("_refln_F_sigma");
  int aInd = hklLoop->Table().ColIndex("_refln_A_calc");
  int bInd = hklLoop->Table().ColIndex("_refln_B_calc");

  if( hInd == -1 || kInd == -1 || lInd == -1 || 
    mfInd == -1 || sfInd == -1 || aInd == -1 || bInd == -1  ) {
      E.ProcessingError(__OlxSrcInfo, "list 3 fcf file is expected");
      return;
  }
  refs.SetCapacity( hklLoop->Table().RowCount() );
  F.SetCount( hklLoop->Table().RowCount() );
  for( int i=0; i < hklLoop->Table().RowCount(); i++ )  {
    TStrPObjList<olxstr,TCifLoopData*>& row = hklLoop->Table()[i];
    TReflection& ref = refs.AddNew(row[hInd].ToInt(), row[kInd].ToInt(), 
      row[lInd].ToInt(), row[mfInd].ToDouble(), row[sfInd].ToDouble());
    if( ref.GetH() < 0 )
      factor = 4;
//    const TEComplex<double> rv(row[aInd].ToDouble(), row[bInd].ToDouble());
//    F[i] = TEComplex<double>::polar(ref.GetI(), rv.arg());
//    F[i].A() = row[aInd].ToDouble();
//    F[i].B() = row[bInd].ToDouble();
      const TEComplex<double> rv(row[aInd].ToDouble(), row[bInd].ToDouble());
      double dI = (ref.GetI() - rv.mod());
      F[i] = TEComplex<double>::polar(dI, rv.arg());
  }
  olxstr hklFileName = XApp.LocateHklFile();
  if( !TEFile::FileExists(hklFileName) )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate hkl file");
    return;
  }
  double vol = XApp.XFile().GetLattice().GetUnitCell().CalcVolume();
  int minH = 100,  minK = 100,  minL = 100;
  int maxH = -100, maxK = -100, maxL = -100;

  vec3d hkl;
  TArrayList<XLibMacros_StrF> AllF(refs.Count()*ml.Count());
  int index = 0;
  double f000 = 0;
  for( int i=0; i < refs.Count(); i++ )  {
    const TReflection& ref = refs[i];
    for( int j=0; j < ml.Count(); j++, index++ )  {
      ref.MulHklT(hkl, ml[j]);
      if( hkl[0] < minH )  minH = (int)hkl[0];
      if( hkl[1] < minK )  minK = (int)hkl[1];
      if( hkl[2] < minL )  minL = (int)hkl[2];
      if( hkl[0] > maxH )  maxH = (int)hkl[0];
      if( hkl[1] > maxK )  maxK = (int)hkl[1];
      if( hkl[2] > maxL )  maxL = (int)hkl[2];
      AllF[index].h = (int)hkl[0];
      AllF[index].k = (int)hkl[1];
      AllF[index].l = (int)hkl[2];
      AllF[index].ps = hkl[0]*ml[j].t[0] + hkl[1]*ml[j].t[1] + hkl[2]*ml[j].t[2];
      AllF[index].v = F[i];
      AllF[index].v *= TEComplex<double>::polar(1, 2*M_PI*AllF[index].ps);
    }
  }
// init map, 0.1A for now
  const int mapX = (int)au.Axes()[0].GetV()*3,
			mapY = (int)au.Axes()[1].GetV()*3,
			mapZ = (int)au.Axes()[2].GetV()*3;
  double mapVol = mapX*mapY*mapZ;
  TArray3D<double> fMap(0, mapX-1, 0, mapY-1, 0, mapZ-1);
//////////////////////////////////////////////////////////////////////////////////////////
  TEComplex<double> ** S, *T;
  int kLen = maxK-minK+1, hLen = maxH-minH+1, lLen = maxL-minL+1;
  S = new TEComplex<double>*[kLen];
  for( int i=0; i < kLen; i++ )
    S[i] = new TEComplex<double>[lLen];
  T = new TEComplex<double>[lLen];
  const double T_PI = 2*M_PI;
// precalculations
  int minInd = olx_min(minH, minK);
  if( minL < minInd )  minInd = minL;
  int maxInd = olx_max(maxH, maxK);
  if( maxL > maxInd )  maxInd = maxL;
  int iLen = maxInd - minInd + 1;
  int mapMax = olx_max(mapX, mapY);
  if( mapZ > mapMax )  mapMax = mapZ;
  TEComplex<double>** sin_cosX = new TEComplex<double>*[mapX],
                      **sin_cosY, **sin_cosZ;
  for( int i=0; i < mapX; i++ )  {
    sin_cosX[i] = new TEComplex<double>[iLen];
    for( int j=minInd; j <= maxInd; j++ )  {
      double rv = (double)(i*j)/mapX, ca, sa;
      rv *= T_PI;
      SinCos(-rv, &sa, &ca);
      sin_cosX[i][j-minInd].SetRe(ca);
      sin_cosX[i][j-minInd].SetIm(sa);
    }
  }
  if( mapX == mapY )  {
    sin_cosY = sin_cosX;
  }
  else  {
    sin_cosY = new TEComplex<double>*[mapY];
    for( int i=0; i < mapY; i++ )  {
      sin_cosY[i] = new TEComplex<double>[iLen];
      for( int j=minInd; j <= maxInd; j++ )  {
        double rv = (double)(i*j)/mapY, ca, sa;
        rv *= T_PI;
        SinCos(-rv, &sa, &ca);
        sin_cosY[i][j-minInd].SetRe(ca);
        sin_cosY[i][j-minInd].SetIm(sa);
      }
    }
  }
  if( mapX == mapZ )  {
    sin_cosZ = sin_cosX;
  }
  else if( mapY == mapZ )  {
    sin_cosZ = sin_cosY;
  }
  else  {
    sin_cosZ = new TEComplex<double>*[mapZ];
    for( int i=0; i < mapZ; i++ )  {
      sin_cosZ[i] = new TEComplex<double>[iLen];
      for( int j=minInd; j <= maxInd; j++ )  {
        double rv = (double)(i*j)/mapZ, ca, sa;
        rv *= T_PI;
        SinCos(-rv, &sa, &ca);
        sin_cosZ[i][j-minInd].SetRe(ca);
        sin_cosZ[i][j-minInd].SetIm(sa);
      }
    }
  }
  TEComplex<double> R;
  double maxMapV = -1000, minMapV = 1000;
  for( int ix=0; ix < mapX; ix++ )  {
    for( int i=0; i < AllF.Count(); i++ )  {
      const XLibMacros_StrF& sf = AllF[i];
      S[sf.k-minK][sf.l-minL] += sf.v*sin_cosX[ix][sf.h-minInd];
    }
    for( int iy=0; iy < mapY; iy++ )  {
      for( int i=minK; i <= maxK; i++ )  {
        for( int j=minL; j <= maxL; j++ )  {
          T[j-minL] += S[i-minK][j-minL]*sin_cosY[iy][i-minInd];
        }
      }
      for( int iz=0; iz < mapZ; iz++ )  {
        R.Null();
        for( int i=minL; i <= maxL; i++ )  {
          R += T[i-minL]*sin_cosZ[iz][i-minInd];
        }
        double val = factor*R.Re()/vol;
        if( val > maxMapV )  maxMapV = val;
        if( val < minMapV )  minMapV = val;
        fMap.Data[ix][iy][iz] = val;
      }
      for( int i=0; i < lLen; i++ )  
        T[i].Null();
    }
    for( int i=0; i < kLen; i++ )  
      for( int j=0; j < lLen; j++ )  
        S[i][j].Null();
  }
  TBasicApp::GetLog() << (olxstr("Map max val ") << olxstr::FormatFloat(3, maxMapV) << " min val " << olxstr::FormatFloat(3, minMapV) << '\n');
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  // calculate the map
  double surfdis = Options.FindValue("d", "1.0").ToDouble();
  long structurePoints = 0;
  vec3d voidCenter;
  TArray3D<short> maskMap(0, mapX-1, 0, mapY-1, 0, mapZ-1);
  short MaxLevel = XApp.CalcVoid(maskMap, surfdis, -101, &structurePoints, voidCenter, NULL);
  XApp.GetLog() << ( olxstr("Cell volume (A^3) ") << olxstr::FormatFloat(3, vol) << '\n');
  XApp.GetLog() << ( olxstr("Max level reached ") << MaxLevel << '\n');
  XApp.GetLog() << ( olxstr("Largest spherical void is (A^3) ") << olxstr::FormatFloat(3, MaxLevel*MaxLevel*MaxLevel*4*M_PI/(3*mapVol)*vol) << '\n');
  XApp.GetLog() << ( olxstr("Structure occupies (A^3) ") << olxstr::FormatFloat(3, structurePoints*vol/mapVol) << '\n');
  int minLevel = Round( pow( 6*mapVol*3/(4*M_PI*vol), 1./3) );
  XApp.GetLog() << ( olxstr("6A^3 level is ") << minLevel << '\n');
  // calculate new structure factors
  double Re = 0, Te=0, F0 = 0;
  int RePointCount = 0, TePointCount = 0;
//  for( int i=0; i < refs.Count(); i++ )  {
//    TReflection& ref = refs[i];
//    double A = 0, B = 0;
    for( int ix=0; ix < mapX; ix++ )  {
      for( int iy=0; iy < mapY; iy++ )  {
        for( int iz=0; iz < mapZ; iz++ )  {
          if( maskMap.Data[ix][iy][iz] <= 0  )  {
//            double tv =  (double)ref.GetH()*ix/mapX;  
//            tv += (double)ref.GetK()*iy/mapY;  
//            tv += (double)ref.GetL()*iz/mapZ;
//            tv *= T_PI;
//            double ca, sa;
//            SinCos(tv, &sa, &ca);
//            A += fMap.Data[ix][iy][iz]*ca;
//            B += fMap.Data[ix][iy][iz]*sa;
//            if( i == 0 )  {
              Te += fMap.Data[ix][iy][iz];
              TePointCount++;
//            }
          }
          else   {
//            if( i == 0 )  {
              Re += fMap.Data[ix][iy][iz];
              RePointCount++;
//            }
          }
//          if( i == 0 )  {
            F0 += fMap.Data[ix][iy][iz];
//          }
        }
      }
    }
//    ref.SetI( sqrt(A*A+B*B)/100 );
//  }
//  TCStrList sl;
//  for( int i=0;  i < refs.Count(); i++ )
//    sl.Add( refs[i].ToString() );
//  sl.SaveToFile( "test.hkl" );
  XApp.GetLog() << "Voids         " << Re*vol/(mapVol) << "e-\n";
//  XApp.GetLog() << "F000 calc     " << Te*vol/(mapVol) << "e-\n";
  XApp.GetLog() << "F000 (formula)" << F000 << "e-\n";
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  for( int i=0; i < kLen; i++ )
    delete [] S[i];
  delete [] S;
  delete [] T;
  if( sin_cosY == sin_cosX )  sin_cosY = NULL;
  if( sin_cosZ == sin_cosX || sin_cosZ == sin_cosY )  sin_cosZ = NULL;
  for( int i=0; i < mapX; i++ )
    delete [] sin_cosX[i];
  delete [] sin_cosX;
  if( sin_cosY != NULL )  {
    for( int i=0; i < mapY; i++ )
      delete [] sin_cosY[i];
    delete [] sin_cosY;
  }
  if( sin_cosZ != NULL )  {
    for( int i=0; i < mapZ; i++ )
      delete [] sin_cosZ[i];
    delete [] sin_cosZ;
  }
}

//..............................................................................
void XLibMacros::macChangeSG(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp& xapp = TXApp::GetInstance();
  
  TLattice& latt = xapp.XFile().GetLattice();
  TUnitCell& uc = latt.GetUnitCell();
  TAsymmUnit& au = latt.GetAsymmUnit();
  if( au.AtomCount() == 0 )  {
    E.ProcessingError(__OlxSrcInfo, "Empty asymmetric unit");
    return;
  }
  TSpaceGroup* sg = TSymmLib::GetInstance()->FindGroup(Cmds.Last().String());
  if( sg == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "Could not identify given space group");
    return;
  }
  smatd_list ml;
  sg->GetMatrices(ml, mattAll );
  TTypeList< AnAssociation3<vec3d,TCAtom*, int> > list;
  uc.GenereteAtomCoordinates(list, true);
  if( Cmds.Count() == 4 )  {
    vec3d trans( Cmds[0].ToDouble(), Cmds[1].ToDouble(), Cmds[2].ToDouble());
    for( int i=0; i < list.Count(); i++ )  {
      list[i].A() += trans;
      list[i].SetC(1);
    }
  }
  else   {
    for( int i=0; i < list.Count(); i++ )  { 
      list[i].SetC(1);
    }
  }
  vec3d v;
  for( int i=0; i < list.Count(); i++ )  {
    if( list[i].GetC() == 0 )  continue;
    for( int j=i+1; j < list.Count(); j++ )  {
      if( list[j].GetC() == 0 )  continue;
      for( int k=1; k < ml.Count(); k++ )  {
        v = ml[k] * list[i].GetA();
        v -= list[j].GetA();
        v[0] -= Round(v[0]);  v[1] -= Round(v[1]);  v[2] -= Round(v[2]);
        au.CellToCartesian(v);
        if( v.QLength() < 0.01 )  {
          list[i].C() ++;
          list[j].SetC(0);
        }
      }
    }
  }
  for( int i=0; i < au.AtomCount(); i++ )
    au.GetAtom(i).SetTag(0);
  TCAtomPList newAtoms;
  for( int i=0; i < list.Count(); i++ )  {
    if( list[i].GetC() == 0 )  continue;
    TCAtom* ca;
    if( list[i].GetB()->GetTag() > 0 )  {
      ca = &au.NewAtom();
      ca->Assign( *list[i].GetB() );
      ca->SetLoaderId(liNewAtom);
    }
    else  {
      ca = list[i].GetB();
      ca->SetTag( ca->GetTag() + 1 );
    }
    ca->ccrd() = list[i].GetA();
    ca->AssignEllps(NULL);
  }
  for( int i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).GetTag() == 0 )
      au.GetAtom(i).SetDeleted(true);
  }
  au.InitAtomIds();
  au.ChangeSpaceGroup(*sg);
  xapp.XFile().GetLastLoader()->GetAsymmUnit().ChangeSpaceGroup(*sg);
  latt.Init();
  latt.CompaqAll();
}
//..............................................................................




#ifdef __BORLANC__
  #pragma package(smart_init)
#endif
