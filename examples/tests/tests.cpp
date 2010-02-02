#include "xapp.h"
#include "ins.h"
#include "xmacro.h"
#include "unitcell.h"
#include "maputil.h"

#ifdef __WIN32__
  #include <windows.h>
#endif

#include <iostream>
#include "filetree.h"
#include "outstream.h"

using namespace std;

class Listener : public AActionHandler  {
public:
  virtual bool Execute(const IEObject *Sender, const IEObject *Data) {  
    if( EsdlInstanceOf(*Data, TOnProgress) )  {
      TBasicApp::GetLog() << ((TOnProgress*)Data)->GetAction() << '\n'; 
      return true; 
    }
    return false;
  }

};

int main(int argc, char* argv[])  {
  try  {
    olxstr bd( TEFile::ExtractFilePath( argv[0] ) );
    if( bd.IsEmpty() )
      bd = TEFile::CurrentDir();

    TEFile::AddPathDelimeterI(bd);

    TXApp XApp(bd);
    XApp.XFile().RegisterFileFormat(new TIns, "res");
    XApp.GetLog().AddStream( new TOutStream(), true );
    XApp.GetLog().AddStream( new TEFile( bd + "tests.log", "w+b"), true );
    TEFile logf(bd + "test.out", "w+b");
    TOnProgress pg;
    Listener listener;
    //TFileTree ft("C:/Documents and Settings/oleg/My Documents/DS/Data");
    TFileTree ft("E:/Data");
    ft.OnExpand->Add(&listener);
    ft.Expand();
    ft.OnExpand->Remove(&listener);
    XApp.GetLog() << '\n';
    TStrList files;
    ft.GetRoot().ListFiles(files, "*.res");
    TMacroError me;
    TStrObjList cmds;
    TParamList params;
    TPtrList<TSpaceGroup> sgs;
    int TotalCount = 0, AgreedCount = 0, 
      WilsonTotalCount = 0, WilsonAgreedCount = 0;
    ABasicFunction* macSG = XApp.GetLibrary().FindMacro("SG");
    ABasicFunction* macWilson = XApp.GetLibrary().FindMacro("Wilson");
    if( macSG == NULL || macWilson == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "could not locate library function");
    
    ElementRadii radii;
    olxstr radii_file("e:/radii.txt");
    if( TEFile::Exists(radii_file) )  {
      TBasicApp::GetLog() << "Using user defined radii for: \n";
      TStrList sl, toks;
      sl.LoadFromFile(radii_file);
      for( size_t i=0; i < sl.Count(); i++ )  {
        toks.Clear();
        toks.Strtok(sl[i], ' ');
        if( toks.Count() == 2 )  {
          cm_Element* elm = XElementLib::FindBySymbol(toks[0]);
          if( elm == NULL )  {
            TBasicApp::GetLog() << " invalid atom type: " << toks[0] << '\n';
            continue;
          }
          TBasicApp::GetLog() << ' ' << toks[0] << '\t' << toks[1] << '\n';
          size_t b_i = radii.IndexOf(elm);
          if( b_i == InvalidIndex )
            radii.Add(elm, toks[1].ToDouble());
          else
            radii.GetValue(b_i) = toks[1].ToDouble();
        }
      }
    }

    uint64_t time_start = TETime::msNow();
    for( size_t i=0; i < files.Count(); i++ )  {
      try {
        TBasicApp::GetLog() << "\r               \r" << TEFile::ExtractFileName(files[i]);
        XApp.XFile().LoadFromFile(files[i]);
        TAsymmUnit& au = XApp.XFile().GetAsymmUnit();
        size_t ac = 0;
        for( size_t ai=0; ai < au.AtomCount(); ai++ )
          if( au.GetAtom(ai).GetType() != iQPeakZ )
            ac++;
        if( ac == 0 )  continue;
        const int mapX = (int)(au.Axes()[0].GetV()*5),
          mapY = (int)(au.Axes()[1].GetV()*5),
          mapZ = (int)(au.Axes()[2].GetV()*5);
        const double mapVol = mapX*mapY*mapZ;
        const double vol = XApp.XFile().GetLattice().GetUnitCell().CalcVolume();
        const int minLevel = olx_round(pow(6*mapVol*3/(4*M_PI*vol), 1./3));
        TArray3D<short> map(0, mapX-1, 0, mapY-1, 0, mapZ-1);
        vec3d voidCenter;
        size_t structureGridPoints = 0;
        XApp.XFile().GetUnitCell().BuildStructureMapEx(map, 0, -101, &structureGridPoints, 
          radii.IsEmpty() ? NULL : &radii, NULL);
        const short MaxLevel = MapUtil::AnalyseVoids(map.Data, map.Length1(), map.Length2(), map.Length3(), voidCenter);
        if( MaxLevel > minLevel )  {
          TBasicApp::GetLog() << (olxstr("\r--> ") << TEFile::ExtractFileName(files[i]) << "      \n");
        }
      }
      catch(...)  {}
      continue;

      try  {
        //if( files[i].IndexOf(".olex") != InvalidIndex )  continue;
        XApp.XFile().LoadFromFile(files[i]);
        TCAtomPList atoms;
        TSpaceGroup& file_sg = XApp.XFile().GetLastLoaderSG();
        const TAsymmUnit& au = XApp.XFile().GetAsymmUnit();
        for( size_t j=0; j < au.AtomCount(); j++ )  {
          if( au.GetAtom(j).GetDegeneracy() != 1 )  {
            atoms.Add(au.GetAtom(j));
          }
        }
        if( atoms.IsEmpty() )  continue;
        TBasicApp::GetLog() << files[i] << '\n';
        for( size_t j=0; j < atoms.Count(); j++ )
          TBasicApp::GetLog() << (olxstr('\t') << atoms[j]->GetLabel() << '\t' << atoms[j]->GetDegeneracy() << '\n');
      }
      catch(...)  {  continue;  }
      continue;

      olxstr hkl( TEFile::ChangeFileExt(files[i], "hkl") );
      if( TEFile::Exists(hkl) )  {
        XApp.GetLog() << files[i] << '\n';
        logf.Writenl( files[i] );
        try { 
          me.Reset();
          sgs.Clear();
          XApp.XFile().LoadFromFile(files[i]); 
          TSpaceGroup* file_sg = NULL;
          try { file_sg = &XApp.XFile().GetLastLoaderSG();  }
          catch(...)  {
            logf.Writenl( "File space group is unknown" );
          }
          if( file_sg != NULL )  {
            logf.Writenl(olxstr("File space group is: ") << file_sg->GetName());
            TotalCount++;
          }
          me.SetRetVal(&sgs);
          macSG->Run(cmds, params, me);
          if( !me.IsSuccessful() )  {
            logf.Writenl(olxstr("SG failed: ") << me.GetInfo());
            logf.Writenl(NewLineSequence, NewLineSequenceLength);
            continue;
          }
          bool centro = false, centro_set = false;
          if( sgs.Count() == 0 )
            logf.Writenl("SG failed - no results returned");
          else if( sgs.Count() == 1 )  {
            logf.Writenl( olxstr("SG returned one space group: ") << sgs[0]->GetName());
            if( file_sg != NULL && sgs[0]->GetNumber() == file_sg->GetNumber() )
              AgreedCount++;
          }
          else  {
            logf.Writenl("Possible space groups: ");
            for( size_t sgc=0; sgc < sgs.Count(); sgc++ )
              logf.Write( olxstr(' ') << sgs[sgc]->GetName() );
            logf << '\n';
            // run wilson
            me.Reset();
            macWilson->Run(cmds, params, me);
            if( !me.IsSuccessful() )
              logf.Writenl( olxstr("Wilson failed: ") << me.GetInfo() );
            else  {
              centro = me.GetRetVal().ToBool();
              centro_set = true;
              logf.Writenl( olxstr("Wilson returned: ") << (centro ? "centric" : "non-centric"));
            }
            if( centro_set )  {
              TSpaceGroup* found_sg = NULL;
              if( file_sg != NULL )  {
                WilsonTotalCount++;
                if( file_sg->IsCentrosymmetric() == centro )
                  WilsonAgreedCount++;
              }
              
              for( size_t sgc=0; sgc < sgs.Count(); sgc++ )  {
                if( sgs[sgc]->IsCentrosymmetric() == centro )  {
                  found_sg = sgs[sgc];
                  break;
                }
              }
              if( found_sg != NULL )  {
                logf.Writenl( olxstr("SG+Wilson: ") << found_sg->GetName() );
                if( file_sg != NULL && found_sg->GetNumber() == file_sg->GetNumber() )
                  AgreedCount++;
              }
              else
                logf.Writenl("SG+Wilson failed: nothing returned\n");
            }
          }
          logf.Writenl(NewLineSequence, NewLineSequenceLength);
          logf.Flush();
        
        }
        catch( const TExceptionBase& exc )  {
          XApp.GetLog() << exc.GetException()->GetFullMessage() << '\n';
        }
      }
    }
    if( TotalCount != 0 )  {
      logf.Writenl( olxstr(TotalCount) << " files with SG info processed in " << TETime::msNow() - time_start << " ms");
      logf.Writenl( olxstr("Agreed calculations: ") << 
        olxstr::FormatFloat(2, (double)AgreedCount*100/TotalCount) << '%' <<
        " (" << TotalCount);
      if( WilsonTotalCount != 0 )
        logf.Writenl( olxstr("Wilson agreed calculations: ") << 
          olxstr::FormatFloat(2, (double)WilsonAgreedCount*100/WilsonTotalCount) << '%' <<
          " (" << WilsonTotalCount << ')');
    }
  }
  catch( TExceptionBase& exc )  {
    printf("An exception occured: %s\n", EsdlObjectName(exc).c_str() );
    printf("details: %s\n", exc.GetException()->GetFullMessage().c_str() );
  }
  printf("\n...");
  std::cin.get();
  return 0;
}