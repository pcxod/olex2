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
#######################################
.DEFAULT_GOAL := all
#######################################
# Compiling
CC := gcc
CFLAGS := -fexceptions -O3 -combine
OPTS = `wx-config --cxxflags --unicode --toolkit=gtk2 --libs gl,core,html,net,aui` `python-config --includes` -I$(SRC_DIR)sdl -I$(SRC_DIR)xlib -I$(SRC_DIR)glib -I$(SRC_DIR)gxlib -I$(SRC_DIR)repository -I$(SRC_DIR)olex -I$(SRC_DIR)alglib -S -D__WXWIDGETS__ -D_UNICODE
LDFLAGS += `wx-config --libs gl,core,html,net,aui --unicode --toolkit=gtk2` `python-config --libs --ldflags` -L. -fexceptions -g -rdynamic -O3
CCFLAGS += $(CFLAGS)
###############################################################################

# All will compile and link all of olex takes about 10 minutes
all: obj bins link
	@echo "Type make install to install"

# obj will create the obj directory and compile the objects
$(OBJ_DIR): 
	@echo "Building object libraries, this can take a while"
	@if test ! -d $(OBJ_DIR); then mkdir $(OBJ_DIR); else touch $(OBJ_DIR); fi;
obj_xlib : $(OBJ_DIR) $@
	@echo "[1] Building xlib object libraries"
	@cd $(OBJ_DIR); $(CC) $(SRC_DIR)xlib/*.cpp $(OPTS) $(CFLAGS)
	@echo "[1] Done! Building xlib object libraries" #This takes a while perhaps split?
obj_alglib : $(OBJ_DIR) $@
	@echo "[2] Building alglib object libraries"
	@cd $(OBJ_DIR); $(CC) $(SRC_DIR)alglib/*.cpp $(OPTS) $(CFLAGS)
	@echo "[2] Done! Building alglib object libraries"
obj_sdl : $(OBJ_DIR)  $@
	@echo "[3] Building sdl object libraries"
	@cd $(OBJ_DIR);	$(CC) $(SRC_DIR)sdl/*.cpp $(OPTS) $(CFLAGS)
	@echo "[3] Done! Building sdl object libraries"
obj_sdl_smart : $(OBJ_DIR)  $@
	@echo "[4] Building sdl smart object libraries"
	@cd $(OBJ_DIR);	$(CC) $(SRC_DIR)sdl/smart/*.cpp $(OPTS) $(CFLAGS)
	@echo "[4] Done! Building sdl smart object libraries"
obj_xlib_macro : $(OBJ_DIR) $@
	@echo "[5] Building xlib macro object libraries"
	@cd $(OBJ_DIR);	$(CC) $(SRC_DIR)xlib/macro/*.cpp $(OPTS) $(CFLAGS)
	@echo "[5] Done! Building xlib macro object libraries"
obj_glib : $(OBJ_DIR) $@
	@echo "[6] Building glib object libraries"
	@cd $(OBJ_DIR);	$(CC) $(SRC_DIR)glib/*.cpp $(OPTS) $(CFLAGS)
	@echo "[6] Done! Building glib object libraries"
obj_gxlib : $(OBJ_DIR) $@
	@echo "[7] Building gxlib object libraries"
	@cd $(OBJ_DIR);	$(CC) $(SRC_DIR)gxlib/*.cpp $(OPTS) $(CFLAGS)
	@echo "[7] Done! Building gxlib object libraries"
obj_repository : $(OBJ_DIR) $@
	@echo "[8] Building repository object libraries"
	@cd $(OBJ_DIR); $(CC) $(SRC_DIR)repository/filesystem.cpp $(OPTS) $(CFLAGS)
	@cd $(OBJ_DIR); $(CC) $(SRC_DIR)repository/shellutil.cpp $(OPTS) $(CFLAGS)
	@cd $(OBJ_DIR); $(CC) $(SRC_DIR)repository/httpex.cpp $(OPTS) $(CFLAGS)
	@cd $(OBJ_DIR); $(CC) $(SRC_DIR)repository/url.cpp $(OPTS) $(CFLAGS)
	@cd $(OBJ_DIR); $(CC) $(SRC_DIR)repository/wxhttpfs.cpp $(OPTS) $(CFLAGS)
	@cd $(OBJ_DIR); $(CC) $(SRC_DIR)repository/wxzipfs.cpp $(OPTS) $(CFLAGS)
	@cd $(OBJ_DIR); $(CC) $(SRC_DIR)repository/fsext.cpp $(OPTS) $(CFLAGS)
	@cd $(OBJ_DIR); $(CC) $(SRC_DIR)repository/pyext.cpp $(OPTS) $(CFLAGS)
	@cd $(OBJ_DIR); $(CC) $(SRC_DIR)repository/integration.cpp $(OPTS) $(CFLAGS)
	@cd $(OBJ_DIR); $(CC) $(SRC_DIR)repository/IsoSurface.cpp $(OPTS) $(CFLAGS)
	@cd $(OBJ_DIR); $(CC) $(SRC_DIR)repository/eprocess.cpp $(OPTS) $(CFLAGS)
	@cd $(OBJ_DIR); $(CC) $(SRC_DIR)repository/olxvar.cpp $(OPTS) $(CFLAGS)
	@cd $(OBJ_DIR); $(CC) $(SRC_DIR)repository/py_core.cpp $(OPTS) $(CFLAGS)
	@echo "[8] Done! Building repository object libraries"

obj : $(OBJ_DIR) obj_alglib obj_sdl obj_sdl_smart obj_xlib obj_xlib_macro obj_glib obj_gxlib obj_repository

# unirun will create the obj/unirun directory and compile the source
unirun : obj
	@echo "[A] Making unirun this is relatively quick"
	@mkdir $(OBJ_DIR)unirun;
	@cd $(OBJ_DIR)unirun/;	$(CC) $(SRC_DIR)unirun/*.cpp  $(OPTS) $(CFLAGS)
	@echo "[A] Done! Making unirun I told you this was relatively quick"

# olex will create the obj/olex directory and compile the source
olex : obj
	@echo "[B] Making olex this can take a while"
	@mkdir $(OBJ_DIR)olex;
	@cd $(OBJ_DIR)olex/; $(CC) $(SRC_DIR)olex/*.cpp $(OPTS) $(CFLAGS)
	@echo "[B] Done! Making olex I told you that this can take a while"
# There now appears to be no files in the olex/macro directory?
#	@cd $(OBJ_DIR)olex/; $(CC) $(SRC_DIR)olex/*.cpp $(SRC_DIR)olex/macro/*.cpp $(CFLAGS)

bins : unirun olex

# link will link the *.s objects created and build the binaries in the bin directory
link : unirun olex
	@echo "Linking unirun and olex"
	@mkdir $(EXE_DIR); $(CC) $(OBJ_DIR)unirun/*.s $(OBJ_DIR)*.s -o $(EXE_DIR)unirun $(LDFLAGS)
	@$(CC) $(OBJ_DIR)*.s $(OBJ_DIR)olex/*.s -o $(EXE_DIR)olex2 $(LDFLAGS)

# install will allow a user with root/sudo permission to install a central copy of olex2
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
	@echo	'*  obj             - Build obj files'
	@echo	'*  olex            - Build olex specific files'
	@echo	'*  unirun          - Build unirun specific files'
	@echo	'*  link            - Links unirun and olex2 creating binaries'
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
