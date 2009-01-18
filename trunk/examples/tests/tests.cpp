#include "xapp.h"
#include "ins.h"
#include "xmacro.h"

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

    TEFile::AddTrailingBackslashI( bd );

    TXApp XApp(bd);
    XApp.XFile().RegisterFileFormat(new TIns, "ins");
    XApp.GetLog().AddStream( new TOutStream(), true );
    XApp.GetLog().AddStream( new TEFile( bd + "tests.log", "w+b"), true );
    TEFile logf(bd + "test.out", "w+b");
    TOnProgress pg;
    Listener listener;
    TFileTree ft("E:/DATA");
    ft.OnExpand->Add(&listener);
    ft.Root.Expand(pg);
    ft.OnExpand->Remove(&listener);
    XApp.GetLog() << '\n';
    TStrList files;
    ft.Root.ListFiles(files, "*.ins");
    TMacroError me;
    TStrObjList cmds;
    TParamList params;
    TPtrList<TSpaceGroup> sgs;
    int TotalCount = 0, AgreedCount = 0;
    ABasicFunction* macSG = XApp.GetLibrary().FindMacro("SG");
    ABasicFunction* macWilson = XApp.GetLibrary().FindMacro("Wilson");
    if( macSG == NULL || macWilson == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "could not locate library function");
    uint64_t time_start = TETime::msNow();
    for( int i=0; i < files.Count(); i++ )  {
      olxstr hkl( TEFile::ChangeFileExt(files[i], "hkl") );
      if( TEFile::FileExists(hkl) )  {
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
            for( int sgc=0; sgc < sgs.Count(); sgc++ )
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
              for( int sgc=0; sgc < sgs.Count(); sgc++ )  {
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
      logf.Writenl( olxstr(TotalCount) << " files with SG info processed in " << TETime::msNow() - time_start << " s");
      logf.Writenl( olxstr("Agreed calculations: ") << olxstr::FormatFloat(2, (double)AgreedCount*100/TotalCount) << '%');
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