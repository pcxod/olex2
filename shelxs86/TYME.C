#include <stdio.h>
#include <sys/time.h>
#include <sys/times.h>

int date(string)
char *string;
{
  static char tmp[50];
  time_t tloc;
  time(&tloc);
  strcpy(tmp,ctime(&tloc));
  strncpy(string,tmp+8,2);
  strncpy(string+2,tmp+3,5);
  strncpy(string+7,tmp+22,2);
  string[9] = '\0';
  return(1);
}

int tyme(string)
char *string;
{
  static char tmp[50];
  time_t tloc;
  time(&tloc);
  strcpy(tmp,ctime(&tloc));
  strncpy(string,tmp+11,8);
  string[8] = '\0';
  return(1);
}
