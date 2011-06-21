#include "integration.h"

using namespace olex;

// we need oly one copy of the instance inside the dll; might not work on other compilers
#if defined(__DLL__) || defined(__GNUC__)
    IOlexRunnable* OlexPort::OlexRunnable;
    IOlexProcessor* IOlexProcessor::Instance;
#else
    IOlexRunnable* OlexPort::OlexRunnable;
    IOlexProcessor* IOlexProcessor::Instance;
#endif

  const olxstr IOlexProcessor::SGListVarName("olx_int_sglist");

IOlexRunnable* OlexPort::GetOlexRunnable()  {  return OlexRunnable;  }
