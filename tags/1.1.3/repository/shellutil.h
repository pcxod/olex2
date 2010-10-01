#ifndef shellutilH
#define shellutilH

#include "ebase.h"
#include "estrlist.h"
#include "typelist.h"

const short  fiDesktop       = 1,
             fiStartMenu     = 2,
             fiPrograms      = 3,
             fiStartup       = 4,
             fiControls      = 5,
             fiProgramFiles  = 6,
             fiMyDocuments   = 7,
             fiAppData       = 8,    // returns application data folder for specific user
             fiCommonStartMenu = 9,
             fiCommonAppData   = 10,
             fiCommonPrograms  = 11,
             fiCommonDesktop   = 12; 


class TShellUtil  {
public:
  static bool CreateShortcut(const olxstr& ShortcutPath,
                     const olxstr& ObjectPath, const olxstr& description, bool AddRunAs);
  static olxstr GetSpecialFolderLocation( short folderId );
  static olxstr PickFolder( const olxstr& Title, const olxstr& SelectedFolder,
     const olxstr& RootFolder );
  // lists all interface names and related MAC addresses
  typedef TTOStringList<olxstr, TArrayList<unsigned char>, TObjectStrListData<olxstr, TArrayList<unsigned char> > > MACInfo;
protected:
  static bool _MACFromArray(const unsigned char* bf, const char* name, MACInfo& mi, size_t len=6, bool accept_empty=false);
public:
  static void ListMACAddresses(MACInfo& rv);
};
#endif
