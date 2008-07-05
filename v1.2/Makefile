# $HeadURL: http://www.dimas.dur.ac.uk/repos/olex_src/v1.1u/Makefile $
# $LastChangedDate: 2008-05-21 00:01:17 +0100 (Wed, 21 May 2008) $
# $LastChangedRevision: 877 $
# $LastChangedBy: xrayman $
# $Id: Makefile 877 2008-05-20 23:01:17Z xrayman $
# #############################################################################
# Makefile for compiling, linking and install olex2
# This currently doesn't check the ld paths
#
###############################################################################
# MACROS - some of these can and should be set using configure when this evolves# 
PROXY = 
OWNER = $(shell USER)
GROUP = $(shell GROUP)
#######################################
# Files and path settings
USER_SETTINGS = usettings.dat
START_FILE = startup
CWD :=  $(shell pwd)
SRC_DIR = $(CWD)/
OBJ_DIR = $(CWD)/obj/
EXE_DIR = $(CWD)/bin/
OLEX_INS := $(HOME)/olex
OLEX_BIN := $(HOME)/bin
#######################################
# Compiling
CC = gcc
CFLAGS = `wx-config --cxxflags --unicode --toolkit=gtk2` `python-config --includes` -I$(SRC_DIR)sdl -I$(SRC_DIR)xlib -I$(SRC_DIR)glib -I$(SRC_DIR)gxlib -I$(SRC_DIR)repository -I$(SRC_DIR)olex -I$(SRC_DIR)alglib -S -D__WXWIDGETS__ -D_UNICODE -D__OD_BUILD__ -fexceptions -O3 -combine
###############################################################################

# All will compile and link all of olex
all: obj unirun olex link

# obj will create the obj directory and compile the objects
obj: $@
	@echo "Building object libraries, this can take a while"
	@mkdir $(OBJ_DIR);
	@cd $(OBJ_DIR); $(CC) $(SRC_DIR)alglib/*.cpp  $(SRC_DIR)sdl/*.cpp  $(SRC_DIR)sdl/smart/*.cpp  $(SRC_DIR)xlib/*.cpp  $(SRC_DIR)xlib/macro/*.cpp  $(SRC_DIR)glib/*.cpp  $(SRC_DIR)gxlib/*.cpp  $(SRC_DIR)repository/filesystem.cpp  $(SRC_DIR)repository/shellutil.cpp  $(SRC_DIR)repository/httpex.cpp  $(SRC_DIR)repository/url.cpp  $(SRC_DIR)repository/wxhttpfs.cpp  $(SRC_DIR)repository/wxzipfs.cpp  $(SRC_DIR)repository/fsext.cpp  $(SRC_DIR)repository/pyext.cpp  $(SRC_DIR)repository/integration.cpp   $(SRC_DIR)repository/IsoSurface.cpp $(CFLAGS)

# unirun will create the obj/unirun directory and compile the source
unirun: $(OBJ_DIR)$@
	@echo "Making unirun this is relatively quick"
	@mkdir $(OBJ_DIR)unirun;
	@cd $(OBJ_DIR)unirun/;	$(CC) $(SRC_DIR)unirun/*.cpp  $(CFLAGS)

# olex will create the obj/olex directory and compile the source
olex: $(OBJ_DIR)$@
	@echo "Making olex this can take a while"
	@mkdir $(OBJ_DIR)olex;
	@cd $(OBJ_DIR)olex/; $(CC) $(SRC_DIR)olex/*.cpp $(SRC_DIR)olex/macro/*.cpp $(CFLAGS)

# link will link the *.s objects created and build the binaries in the bin directory
link: 
	@echo "Linking unirun and olex"
	@mkdir $(EXE_DIR); $(CC) $(OBJ_DIR)unirun/*.s $(OBJ_DIR)*.s -o $(EXE_DIR)unirun `wx-config --libs gl,core,html,net,aui --unicode --toolkit=gtk2` `python-config --libs --ldflags` -L. -fexceptions -g -rdynamic -O3
	$(CC) $(OBJ_DIR)*.s $(OBJ_DIR)olex/*.s -o $(EXE_DIR)olex2 `wx-config --libs gl,core,html,net,aui --unicode --toolkit=gtk2` `python-config --libs --ldflags` -L. -fexceptions -g -rdynamic -O3

# install will allow a user with root/sudo permission to install a central copy of olex2
install:
	@echo "You must be root to install"

# This is my sandbox for testing variables
test:
	@echo "Testing"
	@echo $(HOME)

# install-local installs into the users home directory ~/olex this should be
# altered to install into the path provided by configure or the user at the top
# of this file
#
install-local: 
	@echo "Installing to local directory: " $(HOME) 
	@mkdir $(HOME)/olex;
	@cp -r $(EXE_DIR)* $(HOME)/olex/;
	@cp $(SRC_DIR)scripts/usettings.dat $(HOME)/olex/;
	@cp $(SRC_DIR)scripts/startup $(HOME)/olex/;
# experimental put startup script into bin so olex2 can be called from anywhere
	@cp $(SRC_DIR)scripts/startup $(HOME)/bin/olex2;
	@cp $(SRC_DIR)scripts/olex2.xpm $(HOME)/olex/;
	@cp $(SRC_DIR)scripts/olex2.desktop $(HOME)/Desktop/;
	@sed -i 's/PROXY/$(PROXY)/' $(HOME)/olex/usettings.dat;
	@sed -i 's|OLEX2BINPATH|$(OLEX_BIN)/olex2|g' $(HOME)/Desktop/olex2.desktop;
	@sed -i 's|OLEX2PATH|$(OLEX_INS)|g' $(HOME)/Desktop/olex2.desktop;	
# use sed to alter startup path for different install dir
	@chmod +x $(HOME)/olex/startup $(HOME)/bin/olex2;

# clean - remove build and binary
clean:
	cd $(OBJ_DIR)olex; rm *.s;
	cd $(OBJ_DIR)unirun; rm *.s;
	cd $(OBJ_DIR); rmdir olex unirun;
	cd $(OBJ_DIR); rm *.s;
	cd $(SRC_DIR); rmdir $(OBJ_DIR);
	cd $(EXE_DIR); rm olex2 unirun;

# DONE!
