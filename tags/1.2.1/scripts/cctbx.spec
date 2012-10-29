%define debug_package %{nil}

%if 0%{?rhel}%{?fedora} >= 6
%define ospython /usr/bin/python
%define osipython python
%else
%define ospython /usr/bin/python26
%define osipython python26
%endif

# Turn off the brp-python-bytecompile script
%global __os_install_post %(echo '%{__os_install_post}' | sed -e 's!/usr/lib[^[:space:]]*/brp-python-bytecompile[[:space:]].*$!!g')

Name:           cctbx
Version:        201101072107
Release:        1%{?dist}
Summary:        Computational Crystallography Toolbox
Group:          Applications/Engineering
License:        BSD
URL:            http://cctbx.sourceforge.net
Source0:	%{name}_bundle.tar.gz
Source1:	ccp4_mon_lib.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

%{?fedora:BuildRequires: fftw-devel >= 3}
%{?el5:BuildRequires: fftw3-devel >= 3}
%{?el6:BuildRequires: fftw2-devel >= 2}

BuildRequires: gcc, tar, gzip, scons
BuildRequires: %{?osipython} >= 2.5, %{?osipython}-devel >= 2.5
BuildRequires: %{?osipython}-libs >= 2.5
Requires: %{?osipython} >= 2.5

%description
The Computational Crystallography Toolbox (cctbx) is being developed 
as the open source component of the PHENIX system. The goal of the 
PHENIX project is to advance automation of macromolecular structure 
determination. PHENIX depends on the cctbx, but not vice versa. 
This hierarchical approach enforces a clean design as a reusable library. 
The cctbx is therefore also useful for small-molecule crystallography 
and even general scientific applications.

To maximize reusability and, maybe even more importantly, to give 
individual developers a notion of privacy, the cctbx is organized as a 
set of smaller modules. This is very much like a village (the cctbx 
project) with individual houses (modules) for each family (groups of 
developers, of any size including one).

The cctbx code base is available without restrictions and free of charge 
to all interested developers, both academic and commercial. 
The entire community is invited to actively participate in the 
development of the code base. A sophisticated technical infrastructure 
that enables community based software development is provided by 
SourceForge. This service is also free of charge and open to 
the entire world.

%prep
%setup -q -a 0 -c

tar xf %{SOURCE1} -C cctbx_sources

install -d cctbx_build
cd cctbx_build
%{?ospython} ../cctbx_sources/libtbx/configure.py \
	fftw3tbx rstbx smtbx mmtbx clipper

%build
cd cctbx_build
source ./setpaths.sh
libtbx.scons -j 4

# Remove .pyc and strip .so files before packaging
cd ..
find -name "*.pyc" -delete
find -name "*.so"  -exec /usr/bin/strip '{}' \;

%install
rm -rf %{buildroot}

# Create directories
install -m 755 -d %{buildroot}/usr/local/%{name}/

cp -R cctbx_sources cctbx_build %{buildroot}/usr/local/%{name}/

%clean
rm -rf %{buildroot}

%post
# update paths and clean-up/regenerate byte-compiled (.pyc) python modules:
cd /usr/local/%{name}/cctbx_build
%{?ospython} /usr/local/%{name}/cctbx_sources/libtbx/configure.py \
		fftw3tbx rstbx smtbx mmtbx clipper
cd /usr/local/%{name}/
/usr/local/cctbx/cctbx_build/bin/libtbx.py_compile_all

%files
%defattr(-,root,root)
#%doc ChangeLog.txt ChangeLog.html INSTALL* NOTICE README*
%doc cctbx_sources/cctbx/*.txt
%doc cctbx_sources/cctbx/examples/*.txt
/usr/local/%{name}

%changelog
* Sat Jan 08 2011 John < rpm at x hyphen rayman dot co dot uk > - 201101072107-1
- Fix to bugs introduced in previous nightly build - smtbx
- CCTBX nightly 2011_01_07_2107
* Fri Jan 07 2011 John < rpm at x hyphen rayman dot co dot uk > - 201101062053-5
- Fix for rhel 6 test
* Fri Jan 07 2011 John < rpm at x hyphen rayman dot co dot uk > - 201101062053-4
- House keeping trying to fix if and buts
* Fri Jan 07 2011 John < rpm at x hyphen rayman dot co dot uk > - 201101062053-3
- New snapshot from cctbx website 2011_01_06_2053
- Test Epel 6 build
- Looks like not FFTW3 for EPEL6?
- Changed to python 2.6 for Epel 5 build
* Thu Dec 23 2010 John < rpm at x hyphen rayman dot co dot uk > - 201012230400-1
- New snapshot from cctbx website 2010_12_23_0400
* Wed Dec 22 2010 John < rpm at x hyphen rayman dot co dot uk > - 201012220115-1
- New snapshot from cctbx website 2010_12_22_0115
* Mon Dec 20 2010 John < rpm at x hyphen rayman dot co dot uk > - 201012200059-1
- New snapshot from cctbx website 2010_12_20_0059
- Fixes break with Olex2 trunk requirements
* Thu Dec 16 2010 John < rpm at x hyphen rayman dot co dot uk > - 201012152312-1
- New snapshot from cctbx website 2010_12_15_2312
* Thu Dec 09 2010 John < rpm at x hyphen rayman dot co dot uk > - 201012082044-1
- New snapshot from cctbx website 2010_12_08_2044
* Tue Nov 16 2010 John < rpm at x hyphen rayman dot co dot uk > - 201011161005-2
- New snapshot from cctbx website 2010_11_16_0005
- Removed byte compile as causing rpm process to fail
* Thu Oct 28 2010 John < rpm at x hyphen rayman dot co dot uk > - 201010271946-1
- New snapshot from cctbx website 2010_10_27_1946
* Tue Sep 14 2010 John < rpm at x hyphen rayman dot co dot uk > - 201009132033-1
- New snapshot from cctbx website 2010_09_13_2033
- Hopefully sorted strange TAG on previous release
* Fri Sep 10 2010 John < rpm at x hyphen rayman dot co dot uk > - 201009092311-1
- New snapshot from cctbx website 2010_09_09_2311
- Centos built against Python 2.5 as 2.6 failed to MOCK
* Fri Sep 03 2010 John < rpm at x hyphen rayman dot co dot uk > - 201009022256-1
- New snapshot from cctbx website 2010_09_02_2256
- Centos built against Python 2.6
* Sat Aug 14 2010 John < rpm at x hyphen rayman dot co dot uk > - 201008140214-1
- New snapshot from cctbx website 2010_08_14_0214
* Mon Jul 12 2010 John < rpm at x hyphen rayman dot co dot uk > - 201007112359-1
- New snapshot from cctbx website 2010_07_11_2329
* Fri Apr 30 2010 John < rpm at x hyphen rayman dot co dot uk > - 201004300007-1
- New snapshot from cctbx website 2010_04_30_0007
* Mon Apr 12 2010 John < rpm at x hyphen rayman dot co dot uk > - 201004112124-1
- New snapshot from cctbx website 2010_04_11_2124
* Wed Feb 24 2010 John < rpm at x hyphen rayman dot co dot uk > - 201002232151-1
- New snapshot from cctbx website 2010_02_23_2151
* Sun Jan 31 2010 John < rpm at x hyphen rayman dot co dot uk > - 201001302223-1
- New snapshot from cctbx website 2010_01_30_2223
* Fri Jan 15 2010 John < rpm at x hyphen rayman dot co dot uk > - 201001142252-1
- New snapshot from cctbx website 2010_01_14_2252
* Thu Dec 10 2009 John < rpm at x hyphen rayman dot co dot uk > - 200912092310-1
- Last CCTBX release before Christmas
* Wed Dec 02 2009 John < rpm at x hyphen rayman dot co dot uk > - 200912012231-1
- New snapshot from cctbx website 2009_12_01_2231
- Bring in line with current alpha Olex2 requirements
* Thu Nov 26 2009 John < rpm at x hyphen rayman dot co dot uk > - 200911252205-1
- New snapshot from cctbx website
* Wed Nov 04 2009 John < rpm at x hyphen rayman dot co dot uk > - 200911032307-1
- New snapshot from cctbx website
- Changed license from GPL to BSD
* Sat Oct 10 2009 John < rpm at x hyphen rayman dot co dot uk > - 200910092223-1
- New snapshot from cctbx website
* Fri Sep 24 2009 John < rpm at x hyphen rayman dot co dot uk > - 200909240558-1
- New snapshot from cctbx website
- Brings back compatibility with Olex2 GUI >= 2675
* Mon Sep 13 2009 John < rpm at x hyphen rayman dot co dot uk > - 200909132140-1
- New snapshot from cctbx website
* Sat Aug 15 2009 John < rpm at x hyphen rayman dot co dot uk > - 200908132246-1
- Adjusted spec file to remove sed patch for bad env_config on Centos
	thanks to RWGK for modifying the env_config code
- Building from lastest overnight
* Thu Aug 13 2009 John < rpm at x hyphen rayman dot co dot uk > - 200908120204-4
- Adjusted spec file to get Fedora 11 to build - path issues
- Add fftw3 dependancy to build fftw3tbx
* Thu Aug 13 2009 John < rpm at x hyphen rayman dot co dot uk > - 200908120204-3
- Built in fix to allow centos to build - issue with cctbx python definition
	altered the libtbx/env_config.py to allow debroglie python2.5
* Wed Aug 12 2009 John < rpm at x hyphen rayman dot co dot uk > - 200908120204-2
- Alter documentation setup
* Wed Aug 12 2009 John < rpm at x hyphen rayman dot co dot uk > - 200908120204-1
- New CCTBX unreleased build to test Olex2 compatibility
* Wed Aug 12 2009 John < rpm at x hyphen rayman dot co dot uk > - 200902152320-6
- Release for testing repo and mock build test
* Wed Aug 12 2009 John < rpm at x hyphen rayman dot co dot uk > - 200902152320-5
- Added docs
* Wed Aug 12 2009 John < rpm at x hyphen rayman dot co dot uk > - 200902152320-4
- Adjusted find and delete
* Wed Aug 12 2009 John < rpm at x hyphen rayman dot co dot uk > - 200902152320-3
- Merged debroglie.net spec file with CM spec.
- Removed profile.d install - cctbx has been known to break python help function
* Mon Jul 27 2009 Cameron Mura <cmura@virginia.edu) 2009_02_15_2320-2
- Revise the build procedure to make use of cctbx_sources/libtbx/configure.py,
	rather than the ugly (and potentially dangerous) approach of building in the
	system-wide /usr/local/ space.
* Sun Jul 26 2009 Cameron Mura <cmura@virginia.edu) 2009_02_15_2320-1
- Initial RPM build on Fedora 10...
