%define debug_package %{nil}
# This is defined using the automated scripts when working with the live svn version. It is extracted from the snapshot script along with the updates in the snapshot version.
Name:		olex2-gui
Release:	20110107svn.1%{?dist}
Version:	1.1.4
#Epoch:		1
Summary:	GUI files needed for 1st run of Olex2 after install
Group:		Applications/Engineering
License:	BSD
URL:		http://sourceforge.net/projects/olex2/
#Source1:	https://olex2.svn.sourceforge.net/svnroot/olex2
Source0:	portable-gui.zip
Source1:	licence.txt
BuildArch:	noarch
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires: desktop-file-utils
Requires: olex2 >= 1.1.4
# This is stupid but it is to get around the name change.

%description
This rpm provides the GUI for olex2. It is only run on first install and or a
clean install at user level.

Subsquent GUI updates are then undertaken by the Olex2 update pathway.

# This could be done by repointing the build root, but I like this way better.

%prep
%setup -q -T -c
install -m 644 %SOURCE1 ./licence.txt

%build
# Nothing to build
exit 0

%install
rm -rf %{buildroot}
# Create directories
install -m 755 -d %{buildroot}%{_datadir}/olex2

# We no use unirun to unpack its own zip
install -m 644 %SOURCE0 %{buildroot}%{_datadir}/olex2/

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%doc licence.txt
%dir %{_datadir}/olex2/
%{_datadir}/olex2/portable-gui.zip

%changelog
* Fri Jan 07 2011 John <rpm at x hyphen rayman dot co dot uk> 1.1.4-20110107svn.1
- Tag 1.1.4 taken from portable-gui.zip 07/01/2011
* Sat Nov 27 2010 John <rpm at x hyphen rayman dot co dot uk> 1.1.2-201001127svn.1
- Tag 1.1.3 taken from portable-gui.zip 10/10/2010
* Fri Sep 03 2010 John <rpm at x hyphen rayman dot co dot uk> 1.1.2-201000903svn.1
- Correct version error in spec
- Tag still equivalent to 1.1.2
- Olex2 versioning still an issue
* Sat Aug 14 2010 John <rpm at x hyphen rayman dot co dot uk> 1.1.0-201000814svn.1
- New snapshot equivalent of 1.1.2 tag
- Updated startup shell to fix olex2.tag and update local user space version
- Updated userspace config file .olex2-config to fix opengl stereo issue
* Mon Jul 12 2010 John <rpm at x hyphen rayman dot co dot uk> 1.1.0-201000711svn.1
- New snapshot now on 1.1 branch
- Removed epoch tag
* Thu Dec 10 2009 John <rpm at x hyphen rayman dot co dot uk> 2:1.0.3-200901210svn.1
- New snapshot
* Wed Sep 09 2009 John <rpm at x hyphen rayman dot co dot uk> 2:1.0.1-200900909svn.1
- New interim build
* Thu Aug 27 2009 John <rpm at x hyphen rayman dot co dot uk> 2:1.0.1-20090827svn.1
- Now just moves the olex2 prepared portable-gui.zip
- Using distro TAG 1.0 only
* Thu Jul 09 2009 John <rpm at x hyphen rayman dot co dot uk> 1:1.0-20090709.1
- Change numbering and package versioning
- Remerged with Pascal spec
- Returned to dist-testing branch
- SVN Revision No. 2426
- Snapshot taken 09/07/2009
* Thu Jun 18 2009 John <rpm at x hyphen rayman dot co dot uk> 1.200905051-1
- Change numbering and package versioning
- SVN Revision No. 2363
- Snapshot taken 05/05/2009
* Thu May 28 2009 John <rpm at x hyphen rayman dot co dot uk> 1.0-200905051
- Change numbering and package versioning
- SVN Revision No. 2363
- Snapshot taken 05/05/2009
* Sat May 23 2009 Pascal <pascal22p@parois.net> 1:1.0-20090522svn.1
- Snapshot taken from dist-alpha branch
- fixed versioning
* Tue May 05 2009 John <rpm at x hyphen rayman dot co dot uk> 1.20090505.1
- SVN Revision No. 2363
- Snapshot taken 05/05/2009
* Tue Dec 02 2008 J E Warren <rpm at x hyphen rayman dot co dot uk> 1.20081202.1
- SVN Revision No. 2135
