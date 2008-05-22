#ifndef etimeH
#define etimeH
#include "ebase.h"
//---------------------------------------------------------------------------
#ifdef __WIN32__
  #ifdef __BORLANDC__ // this if for time_t definition
    #include <time.h>
  #endif
#else
  #include <sys/time.h>
#endif
BeginEsdlNamespace()


const long HoursADay   = 24,
           MinsADay    = HoursADay * 60,
           SecsADay    = MinsADay * 60,
           MSecsADay   = SecsADay * 1000;

class TETime {
  static time_t TimeStart;
public:
  // returns a number of seconds elapsed since 1970
  static time_t EncodeDateTimeSec( short Year, short Month, short Day,
                                 short Hour, short Min,  short Sec );
  // decodes given datetime to components
  static void DecodeDateTimeSec( long datetime, short& Year, short& Month, short& Day,
                                            short& Hour, short& Min, short& Sec);

  static inline time_t StartTime()  {  return TimeStart;  }
  static inline time_t Now()        {  return EpochTime();  }

  /* returns number of milliseconds at the time of call
     beware it is to be used only for time differences, as on windows on POSIX
     it will return system dependent values (1601.01.01 win and 1970.01.01 POSIX)
  */
  static uint64_t msNow();

  static time_t EpochTime();

  static olxstr FormatDateTime( time_t v );
  static olxstr FormatDateTime( const olxstr& format, time_t v );

  inline static bool IsLeapYear( int year )  {
    return !(year%4) && ((year%100) || !(year%400));
  }

  static class TLibrary*  ExportLibrary(const olxstr& name=EmptyString);
};

EndEsdlNamespace()
#endif
