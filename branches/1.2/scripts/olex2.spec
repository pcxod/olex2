%define debug_package %{nil}
# This is defined using the automated scripts when working with the live svn version. It is extracted from the snapshot script along with the updates in the snapshot version.

%define _svn_rev 1593

%if 0%{?rhel}%{?fedora} >= 6
%define ospython python
%define osipython python
%else
%define ospython python2.6
%define osipython python26
%endif

Name:		olex2
Version:	1.1.4
#Epoch:		1
Release:	20110107svn.5%{?dist}
Summary:	Is a visualisation software for small-molecule crystallography
Group:		Applications/Engineering
License:	BSD
URL:		http://sourceforge.net/projects/olex2/
#Source1:	https://olex2.svn.sourceforge.net/svnroot/olex2
Source0:	%{name}.tar.gz
Source1:	usettings.dat
Source2:	%{name}.sh
Source3:	%{name}.xpm
Source4:	%{name}.desktop
Source5:	%{name}-config
Source6:	licence.txt
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires: %{?osipython} >= 2.5, %{?osipython}-devel >= 2.5
BuildRequires: %{?osipython}-libs >= 2.5

BuildRequires: gcc, tar, gzip, mesa-libGL, mesa-libGL-devel, mesa-libGLU, mesa-libGLU-devel
BuildRequires: wxGTK >= 2.8 wxGTK-devel >= 2.8 wxPython >= 2.8
BuildRequires: wxPython-devel >= 2.8, wxBase >= 2.8, wxGTK-gl >= 2.8
BuildRequires: desktop-file-utils

Requires: %{?osipython} >= 2.5, %{?osipython}-imaging, wxGTK >= 2.8 wxBase >= 2.8 wxGTK-gl >= 2.8, mesa-libGL, mesa-libGLU
Requires: bash, olex2-gui >= 1.1.3, zenity, xdg-utils
Requires: cctbx >= 20110100000

%description
Olex2 is a program for the determination, visualisation and analysis of
molecular crystal structures. It has a portable, mouse-driven, 
workflow-oriented and fully comprehensive graphical user interface (GUI).

It provides the user with a GUI for structure solution, refinement and report
generation as well as novel tools for structure analysis. All this whilse it
seamlessly links all aspects of the structure solution, refinement and
publication process and present them in a single, workflow-driven package –
with the ultimate goal of producing an application which will be useful to both
chemists and crystallographers.

Reference:
J. Appl. Cryst. (2009). 42, 339–341

%prep
%setup -q -n %{name}
install -m 644 %SOURCE6 ./licence.txt
# Need to add a sed to insert the svn version into the about statement.
# file with statement:
# ./olex/mainform.cpp
#"OLEX 2\n(c) Oleg V. Dolomanov, Horst Puschmann, 2004-2008\nUniversity of Durham"),
sed -i -e "s+OLEX 2+Debroglie Repo\\\nhttp://blog.debroglie.net\\\n%{name}-SVN%{_svn_rev}\\\n+g" %{_builddir}/%{name}/olex/mainform.cpp
# Added this to make a tar for src install but not happy with it.
%build
# Make sure there is no previous bits of make hanging around
make clean
#scons -j 4
#Do make to build everything.

%ifarch x86_64
make -j4 CC="gcc -m64 -O3" OPTS="`wx-config --cxxflags --unicode --toolkit=gtk2` `%{?ospython}-config --includes` -I$(SRC_DIR)sdl -I$(SRC_DIR)xlib -I$(SRC_DIR)glib -I$(SRC_DIR)gxlib -I$(SRC_DIR)repository -I$(SRC_DIR)olex -I$(SRC_DIR)alglib -S -D__WXWIDGETS__ -D_UNICODE -DUNICODE" LDFLAGS="-L$(OBJ_DIR) -rdynamic -O3 -lm -ldl -lrt -lGLU -lGL -lstdc++ `wx-config --libs gl,core,html,net,aui --unicode --toolkit=gtk2` `%{?ospython}-config --libs --ldflags`"
%else
make -j4 CC="gcc -m32 -O3" OPTS="`wx-config --cxxflags --unicode --toolkit=gtk2` `%{?ospython}-config --includes` -I$(SRC_DIR)sdl -I$(SRC_DIR)xlib -I$(SRC_DIR)glib -I$(SRC_DIR)gxlib -I$(SRC_DIR)repository -I$(SRC_DIR)olex -I$(SRC_DIR)alglib -S -D__WXWIDGETS__ -D_UNICODE -DUNICODE" LDFLAGS="-L$(OBJ_DIR) -rdynamic -O3 -lm -ldl -lrt -lGLU -lGL -lstdc++ `wx-config --libs gl,core,html,net,aui --unicode --toolkit=gtk2` `%{?ospython}-config --libs --ldflags`"

%endif
# We can not use the inbuilt makeinstall or do make install as the Makefile
# is setup to put the build into the user HOME directory so will fail during build
# so we do our own build here and use the olex2.sh to install a local setup
%install
rm -rf %{buildroot}
# Create directories
install -m 755 -d %{buildroot}%{_bindir}
install -m 755 -d %{buildroot}%{_datadir}/%{name}
install -m 755 -d %{buildroot}%{_datadir}/applications
install -m 755 -d %{buildroot}%{_datadir}/icons
install -m 755 -d %{buildroot}%{_libexecdir}/%{name}
# Copy and install files
install -m 644 %SOURCE3 %{buildroot}%{_datadir}/icons/%{name}.xpm
install -m 644 %SOURCE1 %{buildroot}%{_datadir}/%{name}/usettings.dat
install -m 644 %SOURCE5 %{buildroot}%{_datadir}/%{name}/%{name}-config
install -m 755 bin/%{name} %{buildroot}%{_libexecdir}/%{name}/%{name}
install -m 755 bin/unirun %{buildroot}%{_libexecdir}/%{name}/unirun
install -m 644 %SOURCE2 %{buildroot}%{_bindir}/%{name}

strip %{buildroot}%{_libexecdir}/%{name}/unirun
strip %{buildroot}%{_libexecdir}/%{name}/%{name}

chmod +x %{buildroot}%{_bindir}/%{name}

# Install shortcut to menu
desktop-file-install \
	--dir %{buildroot}%{_datadir}/applications	\
	--add-category Application			\
	--add-category Education			\
	--vendor debroglie				\
	%SOURCE4
%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%doc licence.txt
%dir %{_libexecdir}/%{name}/
%dir %{_datadir}/%{name}/
%{_bindir}/%{name}
%{_libexecdir}/%{name}/unirun
%{_libexecdir}/%{name}/%{name}
%{_datadir}/%{name}/usettings.dat
%{_datadir}/%{name}/%{name}-config
%{_datadir}/applications/debroglie-%{name}.desktop
%{_datadir}/icons/%{name}.xpm

%changelog
* Fri Jan 07 2011 John <rpm at x hyphen rayman dot co dot uk> 1.1.4-20110107svn.5
- Fixing Fedora 14 build which I fixed but then deleted the fix in the fixing of something else
* Fri Jan 07 2011 John <rpm at x hyphen rayman dot co dot uk> 1.1.4-20110107svn.4
- Fixing python 2.6 naming issues for pythonX-config
* Fri Jan 07 2011 John <rpm at x hyphen rayman dot co dot uk> 1.1.4-20110107svn.3
- Fixing rhel 6 test
* Fri Jan 07 2011 John <rpm at x hyphen rayman dot co dot uk> 1.1.4-20110107svn.2
- Cleaning up ifs and buts
* Fri Jan 07 2011 John <rpm at x hyphen rayman dot co dot uk> 1.1.4-20110107svn.1
- SVN 1593
- First build of Olex2 1.1.4
* Fri Jan 07 2011 John <rpm at x hyphen rayman dot co dot uk> 1.1.3-20101216svn.4
- Test building Epel 6 & Python 2.6 for Epel 5
* Thu Dec 16 2010 John <rpm at x hyphen rayman dot co dot uk> 1.1.3-20101216svn.3
- MOCK issues
* Thu Dec 16 2010 John <rpm at x hyphen rayman dot co dot uk> 1.1.3-20101216svn.2
- SVN 1556
- Updated Makefile
* Thu Dec 16 2010 John <rpm at x hyphen rayman dot co dot uk> 1.1.3-20101216svn.1 
- Hunting for build issues
* Sat Nov 27 2010 John <rpm at x hyphen rayman dot co dot uk> 1.1.3-20101127svn.2
- SVN Version 1527
- Olex2 TAG 1.1.3
- Removed -ffastmath compile option - causing instability
* Fri Sep 10 2010 John <rpm at x hyphen rayman dot co dot uk> 1.1.3-20100910svn.1
- Centos built against Python 2.5
- r1397 updates/fixes to branches
- r1403 fixes to branches; overly complicated, and crashing! sorting is replaced with simpler one and working in CifDP; 
- r1407 changes to 1.1 branch
- r1410 one more try for the alpha
- r1412 one more try to make working alpha
- r1414 fixes to 1.1
* Fri Sep 03 2010 John <rpm at x hyphen rayman dot co dot uk> 1.1.0-2010093svn.1
- r1383 fixes etc to 1.1
- r1386 fixes to 1.1
- Note that Olex2 versioning is somewhat adhoc - 1.1 is now really 1.1.+
- Centos built against Python 2.6
* Sun Aug 22 2010 John <rpm at x hyphen rayman dot co dot uk> 1.1.0-20100712svn.2
- Modified to try and sort out the htmlview issue
* Sat Aug 14 2010 John <rpm at x hyphen rayman dot co dot uk> 1.1.0-20100712svn.1
- Fix libGL libGLU issue for Fedora 13
- Fix broken Centos 5 build in spec
- Lastest svn branch 1.1 source from SF
* Tue Aug 03 2010 John <rpm at x hyphen rayman dot co dot uk> 1.1.0-20100712svn.3
- Still trying to fix libGL issue for Fedora 13
* Mon Jul 12 2010 John <rpm at x hyphen rayman dot co dot uk> 1.1.0-20100712svn.2
- Trying to find libGL requirement
* Mon Jul 12 2010 John <rpm at x hyphen rayman dot co dot uk> 1.1.0-20100712svn.1
- Long overdue update, too many to state
- Move to 1.1 branch
* Thu Dec 10 2009 John <rpm at x hyphen rayman dot co dot uk> 2:1.0.4-20091210svn.2
- Hunting for build errors
* Thu Dec 10 2009 John <rpm at x hyphen rayman dot co dot uk> 2:1.0.4-20091210svn.1
- SVN version TRUNK 1028
- Lots of updates including
- Fix for pdf loading from Olex2
- ORTEP-style postscript image generation added
- Improved handling of labels (including subscripts) for bitmaps/postscript 
  images
- CHN analysis fitting is added to allow calculation of amounts of potentially 
  co-crystallised solvents 
- New reflection graphs added (Fo/Fc vs resolution, Completeness, Normal
  Probability)
- Support for user templates and styles for report generation
- A possibility to calculate angles between best lines and bonds and planes
  is added
- Charge Flipping now uses weak_reflection_improved_iterator
- A distribution for 64-bit Windows is added
- Support added for older CPUs (Pentium III, older AMD Athlons) (#134)
- Structure inversion is no longer done automatically, a warning is printed 
  instead (#168)
- Improved matching of H-atoms
- Q-peaks and H-atoms no longer included in the connectivity when hidden and
  therefore do not affect commands involving symmetry generation and 
  connectivity analysis (such as matching)
- Improved handling of SHELX instructions (MPLA, EQIV, etc)
- Zoom now takes into account whether cell is shown
- Standard atom sorting procedure now works with CIFs
- Problems with UNC paths resolved (#163)
- Fixed OpenGL problems with some ATI Radeon graphics cards (#170)
- "Black window" resizing bug
- Clicking on a bond in mode afix caused a crash (#165)
* Fri Oct 10 2009 John <rpm at x hyphen rayman dot co dot uk> 2:1.0.4-20091010svn.1
- SVN version TRUNK 893
- r856 - File menu item is fixed (besides the case of empty recent files list);
	 improvement to the H-add detecting of the coordination to metal;
- r857 SGInfo is exposed to python; structure inversion now takes the space 
	group inversion centre into account; re-factoring - push, inv and 
	transform are moved to the xlib to be accessible from the headless;
- r858 if symbol H or D is provided for mode name, the H and D atoms labels are
	 switched on
- r859 a test for alphanumeric and alphabetic chars is added to the string class;
	 fixHL now takes fragments into account and does not need multiple runs 
	to converge, however if there are Q-peaks connecting fragments the 
	procedure might give wrong labelling scheme
- r860 typo fixed...
- r861 an attempt to fix OwnerDrawCombobox KILL_FOCUS event...
- r862 non-windows compilation is fixed, though there is a very strange 
	behaviour on GTK, then the onleave/onenter events just recurring
- r863 a bug on linux regarding pending object deletion and causing various
	 problems with reloading html from controls is fixed
- r864 the ASelectionOwner is extended to provide a list of TSAtoms; TTextCtrl
	and TComboBox do not implement of focus change anymore - the THtml
	manages it instead (since problems on Windows, then focus events are 
	broken for wxOwnerDrawnCombobox); Tab item traversal is now managed by
	 the THtml; fixhl now might take a selection to specify the order in 
	this fragments should be processed; 
- r865 some Mac features are fixed, but buttons and other controls which do not
	 accept input cannot be focused and traversed on Mac...
- r866 a fix for #151
- r867 right mouse button click is fixed on Mac...
- r868 a fix for #152
- r869 just a typo fixed...
- r871 fix for the #144, uninitialised variable was used...
- r872 typos fixed; the focus change from mouse is handled now in the html;
	weight and proposed weight is now exported to python and oxm;
- r873 hadd for R3B-H is added; the bug with incorrect virtual function name in
	 TXGrid is fixed (#16, internal); hadd for coordinated linear C-N->M is
	 skipped; 
- r874 a fix for the focus being stolen after clicking or a read-only combobox...
- r875 SAME now can take a set of atoms (provided the number of groups as first
	 argument)
- r876 the SetVisible function of the AGDrawObject is made virtual...;
	some modification are made to the HTML processing, an attempt to get 
	current BGCOLOR to use with controls have failed, though the text color 
	can be easily accessed through the GetActualColor o wxHTMLWinParser 
- r877 some modifications done to the casting, so that underlying object can be
	used as a reference or const reference without the need to create a new
	 instance; the next step would be to pre-cache the casting operators
	(this will require a static method to get an operator though)
- r878 and a crash is fixed when trying to deallocated already deallocated memory...
- r879 no comment--
- r880 typo is fixed
- r881 a bug is fixed in the octahedral distortion calculation using best plane
	approach also automatic evaluation of the total distortion is added
- r882 a Alt+drag is added for MAC for zooming; the use of fastsymm is now can 
	be controlled by defining __OLX_USE_FASTSYMM, it is off for now, since 
	there are no many demanding tasks...; more work on the octahedral 
	distortion calculation
- r883 a newly introduced memory leak is fixed...
- r885 a fix for exyz - only a selected atom and set of new types is sued now; 
	a fix for conn - now both number of bonds and radius are used...; a fix
	 for mode grow -r
- r886 an update for the logic behind fvar macro...
- r887 ExtractFileExt and ChangeFileExt functions are fixed for the '.' only in
	 the path, however these till not work for c:\xxx.yyy ... DataDir is 
	now appended with tag
- r888 some re-factoring to leave all HTTP, ZIP and other protocols in
	UpdateAPI and move all other stuff into the PatchAPI; TEFile dealing
	with extensions now checks if the given name is a folder (so does the 
	ExtractFileName); TIns now replaces all tabs with spaces...; a large 
	change to the DataDir, not DataDir is defined by the BaseDir and the 
	repository tag, however if the DataDir does not exist and old Olex2u 
	folder does, the content of that folder will be copied across (only if
	 Olex2 was not run before); installer will update will also rename 
	DataDir when renaming an installation (will be used for new 
	installations only); 
- r889 and mac/linux picularities
- r890 launch now removes OLEX2_DIR and OLEX2_DATADIR when running olex2, 
	I hope it is for good; a static method Copy for folders is added into
	 TFileTree for convenience;
- r891 never ending bugs...
- r892 debug info stripped..., last for today
- and finally this is what released today
* Wed Sep 15 2009 John <rpm at x hyphen rayman dot co dot uk> 2:1.0.2-20090915svn.1
- SVN Version 855
- Corrected line wraps in changelog
- Updating CCTBX version require
- a bit more progress in generalising expressions
- some refactoring, static and member functions registers added
- Bug #138 is fixed, mind the sign...
- tests are added for the symmparser..., and another bug is fixed there; vector and
	matrix classes are extended by != operator; olxstr from long double is added
- gcc compilation
- Make file is fixed now...
- more experiments with expression parsing; referencing counting is introduced for
	IEvaluable, experimental string implementation is added
	method access (.) operator is added
- and picky gcc compilation...
- a message is printed now after reap if the HKL and source files are different
- asr prints more verbose information
- invalid HTAB's (wih d>5A or with donor atoms having no H atoms) are now removed
	 from ins on file saving
- a fix for Bug #140, when TBasicApp::SharedDir is set different from DataDir
- a typo is fixed and the local FS repository base is also fixed
- a crash is fixed when an invalid number is passed for the date-time formatting
* Wed Sep 09 2009 John <rpm at x hyphen rayman dot co dot uk> 2:1.0.2-20090909svn.2
- Corrected cctbc to cctbx
* Wed Sep 09 2009 John <rpm at x hyphen rayman dot co dot uk> 2:1.0.2-20090909svn.1
- Added CCTBX dependency 
- SVN Version 840
- the offline installation is fixed for unirun, -tag=zip is also added 
	automatic installation from the zip file
- spinctrl is fixed (wxWidgets did not like -1 for the event ids...)
- Bug #119 htmlpanelwidth does not check current screen size now, which fixes
	the appearence on linux
- but those event Ids are not good for other controls...
- glconsole text wrapping is 'fixed' on linux; info with no arguments is also fixed
- missing files are added, installer now looks for the zip file in the right location
- the bitmap top coordinate now depends on the InfoBar visibility...
- some work for the HKLF5 file creation
- a possibility to replace variable names by an external expression is added;
	new approach for the expression evaluation is in progress
- more on the implementation of the parser
- a better idea of working with HKL files is implemented through exporting 
	olex_hkl.Read and olex_hkl.Write functions. A sample script is added to the
	etc/scripts/hkl5.py file on GUI svn.
- Two options are added to @py macro - '-i' - - to open an empty window to
	type python commands in and '-f' show 'File open' dialogue and put the file
	content into that window
- the THklFile::SaveToFile functions are unsafe when using reflections with
	and without batch number, this is solved externally in the hkl_py...
- make and scons files are "fixed"
- a bug is fixed regarding matching overlayed structures. Overlayed structures
	 are now aligned on a 2D grid to avoid overlaps 
- a crash is fixed regarding hyphenating for 0 length in new olex2
	installation; a map attached to zimg as a set of zrect or zcircle is
	implemented, this allows to do z-filtering for the map cells, so that
	for overlapping cells, - the one drawn last is returned for the
	overlapping region; a way to load custom radii (sfil for this moment)
	is added
- a bug with degenerate case is fixed in the plane sort routine
- intel compiler compilation
- Makefile is fixed
- another attempt to compile olex2 with Borland, no luck with the internal 
	compiler errors, some work on the expression evaluation
- an extension for functions with multiple variables added...
- gcc compilation of the ipbase.cpp...
- MERG 0 is now treated correctly; some typos fixed
- a new constructor, taking an array of entries is added for the olxdict; some
	extension, including casting operators is added to the expression
	evaluation
- gcc compilation bug is fixed
* Thu Aug 27 2009 John <rpm at x hyphen rayman dot co dot uk> 2:1.0.1-20090827svn.1
- Interim release
- New Olex2 Update system
- New config settings with user .olex2-config file
- Requires CCTBX as a dependancy
- New zenity update alters
- Removal of zenity offline popup
- svn build trunk 820
* Thu Jul 09 2009 John <rpm at x hyphen rayman dot co dot uk> 1:1.0-20090709svn.1
- Remerged with Pascal spec (of sort)
- Building for all debroglie distros
* Thu Jun 18 2009 John <rpm at x hyphen rayman dot co dot uk> 1.0.020090618svn-2
- Special Centos rpm released for Frederick Hollander
- Fixed Diff Fourier code
- svn update 735
- using old rpm naming
- built from svn/tags/1.0
* Sat May 23 2009 Pascal <pascal22p@parois.net> 1:1.0-20090522svn.1
- Fixed install problem and versioning problem
* Thu May 21 2009 John <rpm at x hyphen rayman dot co dot uk> 1.0.020090521svn-1
- RC 2009.05 1.5 = olex2 version 1.0 SVN 663
- Reduced size of unirun
- Screen flickering removed without recourse to antialiasing settings
* Mon May 18 2009 John <rpm at x hyphen rayman dot co dot uk> 1.20090518svn-1
- Updated Makefile to build binaries
- svn update SVN version 643
* Tue May 14 2009 John <rpm at x hyphen rayman dot co dot uk> 1.20090514svn-1
- svn update SVN version 638
- Corrected issues to scene and generate dialog boxes, thank Pascal
* Tue May 05 2009 John <rpm at x hyphen rayman dot co dot uk> 1.20090505svn-1
- svn update SVN version 584
- Added missing rpm dependencies
- Added reference to Olex2 paper
* Sun Apr 19 2009 John <rpm at x hyphen rayman dot co dot uk> 1.20090419svn-1
- Lots of svn updates. SVN version 584
* Wed Apr 1 2009 John <rpm at x hyphen rayman dot co dot uk> 1.20090401svn-1
- Lots of svn updates. SVN version 556
* Fri Jan 16 2009 J E Warren <rpm at x hyphen rayman dot co dot uk> 1.20090115svn-1
- Lots of svn updates. SVN version 390
