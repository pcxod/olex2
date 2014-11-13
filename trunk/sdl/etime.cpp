/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include <memory.h>
#include <time.h>

#include "etime.h"
#include "library.h"
#include "estrlist.h"

#ifndef __WIN32__
  #include <sys/time.h>
#endif

#ifdef _MSC_VER
  //#define localtime localtime_s  they take different number of arguments!
  //#define ctime ctime_s
  //#define gmtime gmtime_s
#endif
  time_t TETime::TimeStart = TETime::EpochTime();

unsigned char DaysInMonth[2][12] =
    { {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
      {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31} };

static const char* WeekDaysLong[] = {"Sunday", "Monday", "Tuesday", "Wednesday",
                                  "Thursday", "Friday", "Saturday"};

static const char* MonthsLong[] = {"January", "February", "March", "April",
                                "May", "June", "July", "August",
                                "September", "October", "November", "December"};

//..............................................................................
/*
long TETime::EncodeDateSec( short Year, short Month, short Day, int BaseYear )
{
  int ind = 0;
  if( TETime::IsLeapYear(Year) )  ind = 1;
  long d = Day, d1 = Year;
  for(int i=0;  i < Month-1; i++ )
    d += DaysInMonth[ind][i];

  return ( (d1-BaseYear)*365 + d1/4 - d1/100 + d1/400 + d)*SecsADay;
} */
//..............................................................................
uint64_t TETime::msNow()  {
  uint64_t rv = 0;
#ifdef __WIN32__
  SYSTEMTIME st;
  GetSystemTime( &st );
  FILETIME ft;
  SystemTimeToFileTime(&st, &ft);
  rv |= (uint64_t)(ft.dwHighDateTime) << 32;
  rv |= (int64_t)(ft.dwLowDateTime);
  rv /= 10000;
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  rv += tv.tv_sec*1000;
  rv += tv.tv_usec/1000;
#endif
return rv;
}
//..............................................................................
olxstr TETime::FormatDateTime(const olxstr& format, time_t v )  {
  struct tm * tm = localtime(&v);
  olxstr rv;

  for( size_t i=0; i < format.Length(); i++ )  {
    switch( format[i] )  {
      case 's':
        if( (i+1) < format.Length() && format[i+1] == 's' )  {
          if( tm->tm_sec < 10 )  rv << '0';
          i++;
        }
        rv << tm->tm_sec;
        break;
      case 'm':
        if( (i+1) < format.Length() && format[i+1] == 'm')  {
          if( tm->tm_min  < 10 )  rv << '0';
          i++;
        }
        rv << tm->tm_min;
        break;
      case 'h':
        if( (i+1) < format.Length() && format[i+1] == 'h')  {
          if( tm->tm_hour < 10 )  rv << '0';
          i++;
        }
        rv << tm->tm_hour;
        break;
      case 'd':
        if( (i+1) < format.Length() && format[i+1] == 'd' )
          if( (i+2) < format.Length() && format[i+2] == 'd' )
            if( (i+3) < format.Length() && format[i+3] == 'd' )  {
              rv << WeekDaysLong[ tm->tm_wday ];
              i+=3;
            }
            else  {
              rv.Append(WeekDaysLong[ tm->tm_wday ], 3);
              i+=2;
            }
          else  {
            if( tm->tm_mday < 10 )
              rv << '0' << tm->tm_mday;
            else
              rv << tm->tm_mday;
            i++;
          }
        else
          rv << tm->tm_mday;
        break;
      case 'M':
        if( (i+1) < format.Length() && format[i+1] == 'M' )
          if( (i+2) < format.Length() && format[i+2] == 'M' )
            if( (i+3) < format.Length() && format[i+3] == 'M' )  {
              rv << MonthsLong[ tm->tm_mon ];
              i+=3;
            }
            else  {
              rv.Append(MonthsLong[ tm->tm_mon ], 3);
              i+=2;
            }
          else  {
            if( tm->tm_mon < 9 )
              rv << '0' << (tm->tm_mon+1);
            else
              rv << (tm->tm_mon+1);
            i++;
          }
        else
          rv << (tm->tm_mon+1);
        break;
      case 'y':
        if( (i+1) < format.Length() && format[i+1] == 'y' )
          if( (i+2) < format.Length() && format[i+2] == 'y' )
            if( (i+3) < format.Length() && format[i+3] == 'y' )  {
              rv << ( tm->tm_year+1900);
              i+=3;
            }
            else
              ;
          else  {
            if( (tm->tm_year-100) < 10 )
              rv << '0' << (tm->tm_year-100);
            else
              rv << (tm->tm_year-100);
            i++;
          }
        else
          rv << (tm->tm_year-100);
        break;
      default:
        rv << format[i];
    }
  }
  return rv;
}
//..............................................................................
olxstr TETime::FormatDateTime( time_t v )  {
  char * bf = ctime( &v );
  if( bf == NULL )
    return EmptyString();
  bf[24] = '\0';
  return bf;
}
//..............................................................................
void TETime::DecodeDateTimeSec(  time_t datetime,
                                 short& Year, short& Month, short& Day,
                                 short& Hour, short& Min, short& Sec  )
{
  time_t v = datetime;
  struct tm* t = gmtime( &v );
  Year = (short)(t->tm_year + 1900);
  Month = (short)(t->tm_mon + 1);
  Day = (short)t->tm_mday;
  Hour = (short)t->tm_hour;
  Min = (short)t->tm_min;
  Sec = (short)t->tm_sec;
}
//..............................................................................
time_t TETime::EncodeDateTimeSec( short Year, short Month, short Day,
                                short Hour, short Min,  short Sec )
{
  struct tm t;
  memset( &t, 0, sizeof(tm) );
  t.tm_sec = Sec;
  t.tm_min = Min;
  t.tm_hour = Hour;
  t.tm_mday = Day;
  t.tm_mon = Month - 1;
  t.tm_year = Year - 1900;
  t.tm_isdst = 1;
  return mktime( &t );
}
//..............................................................................
time_t TETime::EpochTime()  {
  return (time_t)time(NULL);
}
//..............................................................................
time_t TETime::ParseDate(const olxstr& date)  {
  TStrList toks(date, ' ');
  if( toks.Count() != 3 )
    throw TInvalidArgumentException(__OlxSourceInfo, "three tokens expected for date");
  int month = -1, day = -1, year = -1;
  if( toks[0].Length() == 3 )  {
    for( int i=0; i < 12; i++ )  {
      if( MonthsLong[i][0] == toks[0].CharAt(0) &&
          MonthsLong[i][1] == toks[0].CharAt(1) &&
          MonthsLong[i][2] == toks[0].CharAt(2) )  {
        month = i;
        break;
      }
    }
    if( month == -1 )
      throw TInvalidArgumentException(__OlxSourceInfo, "invalid short month name");
  }
  else
    throw TInvalidArgumentException(__OlxSourceInfo, "not supported form mont names longer than 3 chars");
  if( toks[1].Length() == 2 || toks[1].Length() == 1 )
    day = toks[1].ToInt();
  else
    throw TInvalidArgumentException(__OlxSourceInfo, "day is expected in a 1 or two digit form");
  if( toks[2].Length() == 4 )
    year = toks[2].ToInt();
  else
    throw TInvalidArgumentException(__OlxSourceInfo, "year is expected in a 4 digit form");
  return EncodeDateTimeSec(year, month+1, day, 0, 0, 0);
}
//..............................................................................
time_t TETime::ParseTime(const olxstr& time)  {
  TStrList toks(time, ':');
  if( toks.Count() != 3 )
    throw TInvalidArgumentException(__OlxSourceInfo, "three column separated tokens expected for time");
  return toks[0].ToInt()*3600 + toks[1].ToInt()*60 + toks[2].ToInt();
}
//..............................................................................

void FormatDateTime(const TStrObjList& Params, TMacroData& E)  {
  if( Params.Count() == 1 )
    E.SetRetVal( TETime::FormatDateTime("ddd MMM dd hh:mm:ss yyyy", Params[0].RadInt<time_t>()) );
  else
    E.SetRetVal( TETime::FormatDateTime(Params[1], Params[0].RadInt<time_t>()) );
}

void Now(const TStrObjList& Params, TMacroData& E)  {
  if( Params.Count() == 0 )
    E.SetRetVal( TETime::EpochTime() );
  else
    E.SetRetVal( TETime::FormatDateTime(Params[0], TETime::EpochTime()) );
}

void DF(const TStrObjList& Params, TMacroData& E)  {
  E.SetRetVal( olxstr("yyyy.MM.dd hh:mm:ss") );
}



TLibrary*  TETime::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary( name.IsEmpty() ? olxstr("time") : name);
  lib->Register( new TStaticFunction( ::FormatDateTime, "FormatDateTime", fpOne|fpTwo,
"Formats datatime using default 26 char format or using provided string. Valid formats\
 are y(y(yy) - year like 7, 07 or 2007; M(M(M(M - month like 7, 07, Jul, July; d(d(d(d - day like\
 1, 01, Wed or Wednesday; h(h - for hours, m(m - minutes, s(s - seconds like 1 or 01") );
  lib->Register( new TStaticFunction( ::Now, "Now", fpNone|fpOne,
"Returns current date and time as a long number if no format is provided. If a format string\
 is provided it return a formatted string. The DF() function can be used for default formatting ") );
  lib->Register( new TStaticFunction( ::DF, "DF", fpNone,
"Returns default date format") );
  return lib;
}
