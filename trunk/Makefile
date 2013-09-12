# $HeadURL$
# $LastChangedDate$
# $LastChangedRevision$
# $LastChangedBy$
# $Id$
# #############################################################################
# Makefile for compiling, linking, installing and updating olex2
# This currently doesn't check the ld paths or libs
#
###############################################################################
# MACROS - some of these can and should be set using configure when this evolves#
# NAME = Olex v1.1 - super banana monkey
#
PROXY = $(shell PROXY)
OWNER = $(shell USER)
GROUP = $(shell GROUP)
#######################################
# Files and path settings
SVN_SERVER = 
USER_SETTINGS = usettings.dat
START_FILE = startup
CWD :=  $(shell pwd)
SRC_DIR = $(CWD)/
OBJ_DIR = $(CWD)/obj/
EXE_DIR = $(CWD)/bin/
OLEX_INS := $(HOME)/olex
OLEX_BIN := $(HOME)/bin
find_files = $(wildcard *.cpp)
VPATH = xlib:alglib:sdl:sdl/smart:sdl/exparse:sdl/math:xlib/macro:xlib/henke:xlib/absorpc:glib:gxlib
OBJ := xlib alglib sdl sdl/smart sdl/exparse sdl/math xlib/macro xlib/henke xlib/absorpc glib gxlib
NPY_CPP_REPO = filesystem.cpp shellutil.cpp url.cpp httpfs.cpp wxzipfs.cpp fsext.cpp pyext.cpp IsoSurface.cpp eprocess.cpp updateapi.cpp patchapi.cpp cdsfs.cpp
OBJ_CPP_REPO := hkl_py.cpp olxvar.cpp py_core.cpp $(NPY_CPP_REPO)
NPY_OBJ_REPO := $(addprefix $(OBJ_DIR), $(NPY_CPP_REPO))
OBJ_REPO := $(addprefix $(OBJ_DIR),$(OBJ_CPP_REPO))

obj_src_files := $(foreach dir,$(OBJ),$(wildcard $(dir)/*.cpp))

obj_xlib_files := $(addprefix $(OBJ_DIR),$(notdir $(wildcard xlib/*.cpp)))
obj_alglib_files := $(addprefix $(OBJ_DIR),$(notdir $(wildcard alglib/*.cpp)))
obj_sdl_files := $(addprefix $(OBJ_DIR),$(notdir $(wildcard sdl/*.cpp)))
obj_sdl_smart_files := $(addprefix $(OBJ_DIR),$(notdir $(wildcard sdl/smart/*.cpp)))
obj_sdl_exparse_files := $(addprefix $(OBJ_DIR),$(notdir $(wildcard sdl/exparse/*.cpp)))
obj_sdl_math_files := $(addprefix $(OBJ_DIR),$(notdir $(wildcard sdl/math/*.cpp)))
obj_xlib_macro_files := $(addprefix $(OBJ_DIR),$(notdir $(wildcard xlib/macro/*.cpp)))
obj_xlib_henke_files := $(addprefix $(OBJ_DIR),$(notdir $(wildcard xlib/henke/*.cpp)))
obj_xlib_absorpc_files := $(addprefix $(OBJ_DIR),$(notdir $(wildcard xlib/absorpc/*.cpp)))
obj_xlib_analysis_files := $(addprefix $(OBJ_DIR),$(notdir $(wildcard xlib/analysis/*.cpp)))
obj_glib_files := $(addprefix $(OBJ_DIR),$(notdir $(wildcard glib/*.cpp)))
obj_gxlib_files := $(addprefix $(OBJ_DIR),$(notdir $(wildcard gxlib/*.cpp)))
OBJ_UNIRUN := $(addprefix $(OBJ_DIR)unirun/,$(notdir $(wildcard unirun/*.cpp)))
OBJ_OLEX := $(addprefix $(OBJ_DIR)olex/,$(notdir $(wildcard olex/*.cpp)))
OBJ_OLEX_HTML := $(addprefix $(OBJ_DIR)olex/,$(notdir $(wildcard olex/html/*.cpp)))
OBJ_OLEX_CTRLS := $(addprefix $(OBJ_DIR)olex/,$(notdir $(wildcard olex/ctrls/*.cpp)))
OBJ_OLEX_NUI := $(addprefix $(OBJ_DIR)olex/,$(notdir $(wildcard olex/nui/*.cpp)))
#######################################
.DEFAULT_GOAL := all
#######################################
# Compiling
CC := gcc
CFLAGS := -O3 -fpermissive -fexcess-precision=fast
#-combine
OPTS =`wx-config --cxxflags --unicode --toolkit=gtk2` `python-config --includes` -I$(SRC_DIR)sdl -I$(SRC_DIR)xlib -I$(SRC_DIR)glib -I$(SRC_DIR)gxlib -I$(SRC_DIR)repository -I$(SRC_DIR)olex -I$(SRC_DIR)alglib -S -D__WXWIDGETS__ -D_UNICODE -DUNICODE
LDFLAGS += `wx-config --libs gl,core,html,net,aui,adv --unicode --toolkit=gtk2` `python-config --libs --ldflags` -L$(OBJ_DIR) -rdynamic -O3 -fpermissive -ldl -lrt -lGLU -lGL -lstdc++
CCFLAGS += $(CFLAGS)
###############################################################################

# All will compile and link all of olex takes about 10 minutes
# Old way to create objs
#.PHONY : obj
#obj: $(obj_src_files:.cpp=.s)
#$(obj_src_files:.cpp=.s): $(OBJ_DIR)
#	cd $(OBJ_DIR); $(CC) $(SRC_DIR)$(@:.s=.cpp) $(OPTS) $(CFLAGS)
	
# obj will create the obj directory and compile the objects
.PHONY : all
all :
	+make objs
	+make bins
	@echo "Type make install to install"

.PHONY : objs
objs: obj obj_xlib obj_alglib obj_sdl obj_sdl_smart obj_sdl_exparse obj_sdl_math obj_xlib_macro obj_xlib_henke\
      obj_xlib_absorpc obj_xlib_analysis obj_glib obj_gxlib obj_repository

obj:
	@if test ! -d $(OBJ_DIR); then mkdir $(OBJ_DIR); else echo "obj directory already present"; fi;

.PHONY : obj_xlib
obj_xlib: obj $(obj_xlib_files:.cpp=.s)

$(obj_xlib_files:.cpp=.s):
	$(CC) $(SRC_DIR)xlib/$(@F:.s=.cpp) -o $(OBJ_DIR)$(@F) $(OPTS) $(CFLAGS)

.PHONY : obj_alglib
obj_alglib: obj $(obj_alglib_files:.cpp=.s)

$(obj_alglib_files:.cpp=.s):
	$(CC) $(SRC_DIR)alglib/$(@F:.s=.cpp) -o $(OBJ_DIR)$(@F) $(OPTS) $(CFLAGS)
	
.PHONY : obj_sdl
obj_sdl: obj $(obj_sdl_files:.cpp=.s)

$(obj_sdl_files:.cpp=.s):
	$(CC) $(SRC_DIR)sdl/$(@F:.s=.cpp) -o $(OBJ_DIR)$(@F) $(OPTS) $(CFLAGS)
	 
.PHONY : obj_sdl_smart
obj_sdl_smart: obj $(obj_sdl_smart_files:.cpp=.s)

$(obj_sdl_smart_files:.cpp=.s):
	$(CC) $(SRC_DIR)sdl/smart/$(@F:.s=.cpp) -o $(OBJ_DIR)$(@F) $(OPTS) $(CFLAGS)
		 
.PHONY : obj_sdl_exparse
obj_sdl_exparse: obj $(obj_sdl_exparse_files:.cpp=.s)

$(obj_sdl_exparse_files:.cpp=.s):
	$(CC) $(SRC_DIR)sdl/exparse/$(@F:.s=.cpp) -o $(OBJ_DIR)$(@F) $(OPTS) $(CFLAGS)

.PHONY : obj_sdl_math
obj_sdl_math: obj $(obj_sdl_math_files:.cpp=.s)

$(obj_sdl_math_files:.cpp=.s):
	$(CC) $(SRC_DIR)sdl/math/$(@F:.s=.cpp) -o $(OBJ_DIR)$(@F) $(OPTS) $(CFLAGS)

.PHONY : obj_xlib_macro
obj_xlib_macro: obj $(obj_xlib_macro_files:.cpp=.s)

$(obj_xlib_macro_files:.cpp=.s):
	$(CC) $(SRC_DIR)xlib/macro/$(@F:.s=.cpp) -o $(OBJ_DIR)$(@F) $(OPTS) $(CFLAGS)

.PHONY : obj_xlib_henke
obj_xlib_henke: obj $(obj_xlib_henke_files:.cpp=.s)

$(obj_xlib_henke_files:.cpp=.s):
	$(CC) $(SRC_DIR)xlib/henke/$(@F:.s=.cpp) -o $(OBJ_DIR)$(@F) $(OPTS) $(CFLAGS)

.PHONY : obj_xlib_absorpc
obj_xlib_absorpc: obj $(obj_xlib_absorpc_files:.cpp=.s)

$(obj_xlib_absorpc_files:.cpp=.s):
	$(CC) $(SRC_DIR)xlib/absorpc/$(@F:.s=.cpp) -o $(OBJ_DIR)$(@F) $(OPTS) $(CFLAGS)

.PHONY : obj_xlib_analysis
obj_xlib_analysis: obj $(obj_xlib_analysis_files:.cpp=.s)

$(obj_xlib_analysis_files:.cpp=.s):
	$(CC) $(SRC_DIR)xlib/analysis/$(@F:.s=.cpp) -o $(OBJ_DIR)$(@F) $(OPTS) $(CFLAGS)

.PHONY : obj_glib
obj_glib: obj $(obj_glib_files:.cpp=.s)

$(obj_glib_files:.cpp=.s):
	$(CC) $(SRC_DIR)glib/$(@F:.s=.cpp) -o $(OBJ_DIR)$(@F) $(OPTS) $(CFLAGS)
	
.PHONY : obj_gxlib
obj_gxlib: obj $(obj_gxlib_files:.cpp=.s)

$(obj_gxlib_files:.cpp=.s):
	$(CC) $(SRC_DIR)gxlib/$(@F:.s=.cpp) -o $(OBJ_DIR)$(@F) $(OPTS) $(CFLAGS)
	
.PHONY : obj_repository
obj_repository: obj $(OBJ_REPO:.cpp=.s)

$(OBJ_REPO:.cpp=.s):
	$(CC) $(SRC_DIR)repository/$(@F:.s=.cpp) -o $(OBJ_DIR)$(@F) $(OPTS) $(CFLAGS)

.PHONY : bins 
bins : objs olex
#unirun

unirun_obj_dir: obj
	@if test ! -d $(OBJ_DIR)unirun; then mkdir $(OBJ_DIR)unirun; else echo "obj/unirun already present"; fi;
olex_obj_dir: obj
	@if test ! -d $(OBJ_DIR)olex; then mkdir $(OBJ_DIR)olex; else echo "obj/olex already present"; fi;
bin:	
	@if test ! -d $(EXE_DIR); then mkdir $(EXE_DIR); else "unirun binary already present"; fi;

.PHONY : unirun
unirun: objs
	+make obj_unirun
	+make $(EXE_DIR)unirun

.PHONY : obj_unirun
obj_unirun: unirun_obj_dir $(OBJ_UNIRUN:.cpp=.s)

$(OBJ_UNIRUN:.cpp=.s):
	$(CC) $(SRC_DIR)unirun/$(@F:.s=.cpp) -o $(OBJ_DIR)unirun/$(@F) -O3 -fpermissive `wx-config --cxxflags --unicode --toolkit=gtk2` -I$(SRC_DIR)sdl -I$(SRC_DIR)xlib -I$(SRC_DIR)glib -I$(SRC_DIR)gxlib -I$(SRC_DIR)repository -I$(SRC_DIR)alglib -S -D__WXWIDGETS__ -D_UNICODE -DUNICODE -D_NO_PYTHON
	$(addprefix $(SRC_DIR)repository/,$(NPY_CPP_REPO)) 

$(EXE_DIR)unirun : bin $(OBJ_UNIRUN:.cpp=.s) $(obj_sdl_files:.cpp=.s) $(obj_sdl_smart_files:.cpp=.s) $(obj_sdl_exparse_files:.cpp=.s) $(OBJ_REPO:.cpp=.s)
	$(CC) $(obj_sdl_files:.cpp=.s) $(obj_sdl_smart_files:.cpp=.s) $(obj_sdl_exparse_files:.cpp=.s) $(addprefix $(OBJ_DIR),$(NPY_CPP_REPO:.cpp=.s)) $(OBJ_UNIRUN:.cpp=.s) -o $(EXE_DIR)unirun $(LDFLAGS) -D_NO_PYTHON

.PHONY : olex
olex : objs
	+make obj_olex obj_olex_html obj_olex_ctrls obj_olex_nui
	+make $(EXE_DIR)olex

.PHONY : obj_olex
obj_olex: olex_obj_dir $(OBJ_OLEX:.cpp=.s)

$(OBJ_OLEX:.cpp=.s):
	$(CC) $(SRC_DIR)olex/$(@F:.s=.cpp) -o $(OBJ_DIR)olex/$(@F) $(OPTS) $(CFLAGS)

####### Newly added
.PHONY : obj_olex_html
obj_olex_html: olex_obj_dir $(OBJ_OLEX_HTML:.cpp=.s)

$(OBJ_OLEX_HTML:.cpp=.s):
	$(CC) $(SRC_DIR)olex/html/$(@F:.s=.cpp) -o $(OBJ_DIR)olex/$(@F) $(OPTS) $(CFLAGS)

.PHONY : obj_olex_ctrls
obj_olex_ctrls: olex_obj_dir $(OBJ_OLEX_CTRLS:.cpp=.s)

$(OBJ_OLEX_CTRLS:.cpp=.s):
	$(CC) $(SRC_DIR)olex/ctrls/$(@F:.s=.cpp) -o $(OBJ_DIR)olex/$(@F) $(OPTS) $(CFLAGS)

.PHONY : obj_olex_nui
obj_olex_nui: olex_obj_dir $(OBJ_OLEX_NUI:.cpp=.s)

$(OBJ_OLEX_NUI:.cpp=.s):
	$(CC) $(SRC_DIR)olex/nui/$(@F:.s=.cpp) -o $(OBJ_DIR)olex/$(@F) $(OPTS) $(CFLAGS)
######

$(EXE_DIR)olex: bin $(OBJ_OLEX:.cpp=.s) $(OBJ_OLEX_CTRLS:.cpp=.s) $(OBJ_OLEX_HTML:.cpp=.s)
	$(CC) $(OBJ_DIR)*.s $(OBJ_OLEX:.cpp=.s) $(OBJ_OLEX_HTML:.cpp=.s)\
        $(OBJ_OLEX_NUI:.cpp=.s) $(OBJ_OLEX_CTRLS:.cpp=.s) -o $(EXE_DIR)olex2 $(LDFLAGS)

############################################################################################
# From here down is to do with installation and cleanup only
#install will allow a user with root/sudo permission to install a central copy of olex2
install-root:
	@echo "You must be root to install"
	install -m 755 -d /usr/share/olex2
	install -m 755 -d /usr/libexec/olex2
	# Copy and install files
	install -m 644 $(SRC_DIR)scripts/usettings.dat /usr/share/olex2/usettings.dat
	install -m 755 scripts/olex2.xpm /usr/share/olex2/icons/olex2.xpm
	install -m 755 bin/olex2 /usr/libexec/olex2/olex2
	install -m 755 bin/unirun /usr/libexec/olex2/unirun
	install -m 644 $(SRC_DIR)scripts/olex2.sh /usr/bin/olex2
	strip /usr/libexec/olex2/unirun
	strip /usr/libexec/olex2/olex2
	chmod +x /usr/bin/olex2
# Create desktop icon
	cat > olex2.desktop << EOF
	[Desktop Entry]
	Type=Application
	Name=olex2
	Comment="Olex2 is visualisation software for small-molecule crystallography"
	TryExec=/usr/bin/olex2
	Exec=/usr/bin/olex2
	Icon=olex2.xpm
	MimeType=image/x-foo;
	Categories=Engineering;
	GenericName=Crystallography GUI
	X-Trminal=true
	EOF
# Install shortcut to menu
	desktop-file-install \
		--dir /usr/share/applications \
		--add-category Application    \
		--add-category Education      \
		 olex2.desktop

# This is my sandbox for testing variables
test:
	@echo "Testing"
	@echo $(HOME)

# install installs into the users home directory ~/olex this should be
# altered to install into the path provided by configure or the user at the top
# of this file
# 
# NOTE: 05/10/08 This needs modifying to bring it up2date with the new source code
# options. It still will work but it does not utilise some of the new olex2 features
install: 
	@echo "Installing to local directory: " $(HOME) 
	@mkdir $(OLEX_BIN);
	@cp -r $(EXE_DIR)* $(OLEX_INS)/;
	@cp $(SRC_DIR)scripts/usettings.dat $(OLEX_INS)/;
	@cp $(SRC_DIR)scripts/startup $(OLEX_INS)/;
# experimental put startup script into bin so olex2 can be called from anywhere
	@if test -d $(OLEX_bin); \
		then cp $(SRC_DIR)scripts/startup $(OLEX_BIN)/olex2; \
	else \
		mkdir $(OLEX_BIN);\
		cp $(SRC_DIR)scripts/startup $(OLEX_BIN)/olex2;\
	fi;
	@cp $(SRC_DIR)scripts/startup $(OLEX_BIN)/olex2;
	@cp $(SRC_DIR)scripts/olex2.xpm $(OLEX_INS)/;
	@cp $(SRC_DIR)scripts/olex2.desktop $(HOME)/Desktop/;
# use sed to alter startup path for different install dir
	@sed -i 's/PROXY/$(PROXY)/' $(OLEX_INS)/usettings.dat;
	@sed -i 's|OLEX2BINPATH|$(OLEX_BIN)/olex2|g' $(HOME)/Desktop/olex2.desktop;
	@sed -i 's|OLEX2PATH|$(OLEX_INS)|g' $(HOME)/Desktop/olex2.desktop;
	@sed -i 's|OLEX_INS|$(OLEX_INS)|g' $(OLEX_BIN)/olex2;
	@sed -i 's|OLEX_BIN|$(OLEX_BIN)|g' $(OLEX_BIN)/olex2;
	@chmod +x $(OLEX_INS)/startup $(OLEX_BIN)/olex2;
	@echo "Installed"
	@echo "Downloading and Updating GUI"
	@$(OLEX_INS)/unirun $(OLEX_INS)
	@echo '*****************************************************'
	@echo 'Done, double click on olex2 icon on your desktop or'
	@echo 'call olex2 from console'

# Update
# This function just updates the binaries of an existing olex2 install.
.PHONY : update
update: 
	@echo "Checking SVN for latest version"
#	svn info | grep Rev
	@svn update
	@echo "Completed SVN update"
	@echo "Cleaning Old Build"
	+make clean
	@echo "Building Binaries"
	+make 
	@echo "Updating binaries to local install directory: " $(OLEX_INS)
	@cp -r $(EXE_DIR)* $(OLEX_INS)/;

# clean - remove build and binary
.PHONY : clean
clean_bin:
	@if test -d $(EXE_DIR); then cd $(EXE_DIR); if test -f olex2; then rm olex2; fi; if test -f unirun; then rm unirun; fi; fi;
	@cd $(SRC_DIR); if test -d $(EXE_DIR); then rmdir $(EXE_DIR); fi;
clean_olex:
	@if test -d $(OBJ_DIR)olex; then cd $(OBJ_DIR)olex; if ls *.s >/dev/null; then rm *.s; fi; fi;
	@if test -d $(OBJ_DIR); then cd $(OBJ_DIR); if test -d olex; then rmdir olex; fi; fi;
clean_unirun:
	@if test -d $(OBJ_DIR)unirun; then cd $(OBJ_DIR)unirun; if ls *.s >/dev/null; then rm *.s; fi; fi;
	@if test -d $(OBJ_DIR); then cd $(OBJ_DIR); if test -d unirun; then rmdir unirun; fi; fi;
clean_obj:
	@if test -d $(OBJ_DIR); then cd $(OBJ_DIR); if ls *.s >/dev/null; then rm *.s; fi; fi;
	@cd $(SRC_DIR); if test -d $(OBJ_DIR); then rmdir $(OBJ_DIR); fi;

clean:	clean_bin clean_olex clean_unirun clean_obj

.PHONY : help
help:
	@echo	'Cleaning targets:'
	@echo	'  clean           - Remove generated files but keep the installed'
	@echo	'                    binary and ~/olex directory intact'
	@echo	'  clean_bin       - Remove binaries only'
	@echo	' '
	@echo	'Other generic targets:'
	@echo	'R  all             - Build all targets marked with [*]'
	@echo	'*  objs            - Build obj files'
	@echo	'*  olex            - Build olex specific files'
	@echo	'*  unirun          - Build unirun specific files'
	@echo	'   install-root    - Install to /usr/local/ **REQUIRES ROOT**'
#	@echo	'   cctbx-local     - Will download, build and install cctbx to olex defaults'
#	@echo	'   cctbx-root      - Will download, build and install central cctbx'
	@echo	'   update          - Update the binaries of an existing install only'
	@echo	'R  install         - Install all to local folder'
	@echo	' '
	@echo	'Execute "make" or "make all" to build all marked with a * '
	@echo	'Recommmended make targets all labelled R'
	@echo	'Execute "make install" to install to $(OLEX_INS)'
#	@echo  'For further info see the ./README file'


# DONE!
# DO NOT DELETE

