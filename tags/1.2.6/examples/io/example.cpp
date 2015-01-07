// example.cpp : Defines the entry point for the console application.
//

#include "xapp.h"
#include "outstream.h"
#include "ins.h"
#include "cif.h"

int main(int argc, char* argv[]) {
  TXApp xapp(argv[0]);
  // enable console output
  xapp.GetLog().AddStream(new TOutStream, true);
  TXFile &xf = xapp.XFile();
  TIns *ins = new TIns;
  xf.RegisterFileFormat(ins, "ins");
  xf.RegisterFileFormat(ins, "res");
  TCif *cif = new TCif;
  xf.RegisterFileFormat(cif, "cif");
  xf.RegisterFileFormat(cif, "fcf");

  xf.LoadFromFile(TEFile::ExpandRelativePath("../../../../../../samples/io/ZP2.cif"));
  TAsymmUnit &au = xf.GetAsymmUnit();
  TTable tab(au.AtomCount(), 4);
  tab.ColName(0) = "Label";
  tab.ColName(1) = "Cell Crd";
  tab.ColName(2) = "Cart Crd";
  tab.ColName(3) = "Ueq";

  for (size_t i=0; i < au.AtomCount(); i++) {
    TCAtom &a = au.GetAtom(i);
    tab[i][0] = a.GetLabel();
    vec3d cc = a.ccrd();
    tab[i][1].stream(olxstr(", ")) << cc[0] << cc[1] << cc[2];
    cc = au.Orthogonalise(a.ccrd());
    tab[i][2].stream(olxstr(", ")) << olxstr::FormatFloat(3, cc[0]) <<
      olxstr::FormatFloat(3, cc[1]) <<
      olxstr::FormatFloat(3, cc[2]);
    tab[i][3] << a.GetUiso();
  }
  TBasicApp::NewLogEntry() << tab.CreateTXTList("Atoms", true, false, ' ');
  xapp.NewLogEntry() << "Finished";
  system("PAUSE");
        return 0;
}
