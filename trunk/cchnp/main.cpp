#include "bapp.h"
#include "log.h"
#include "edict.h"
#include "outstream.h"
#include "ematrix.h"

#include "xapp.h"


int main(int argc, char** argv)  {
  TXApp app(argv[0]);
  app.GetLog().AddStream(new TOutStream, true);
  TStrObjList args("C12H22O11 C:40.1 H:6 N:0 H2O CCl3H", ' ');
  TParamList params;
  TMacroError me;
  ABasicFunction* macFitCHN = app.GetLibrary().FindMacro("FitCHN");
  macFitCHN->Run(args, params, me);
  //app.GetLog() << me.
  app.GetLog() << "Hello\n";
}