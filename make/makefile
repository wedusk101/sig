# Names of the modules to be compiled:
TARGETLIBS = sig sigogl sigos sigkin
TARGETEXS = gstests shapes cameratest customnode fontgen modelviewer polyeditor scenetest skelviewer uidemo vgtest
OBJDIR = obj

# Compiler options: (ARCH can be changed from the cmd line: make ARCH=32)
ARCH = 64
INCLUDEDIR = -I$(ROOT)/include -I/X11
COPT = -m$(ARCH) -O2 -std=c++11
CWARN = -Wall -Wno-switch -Wno-maybe-uninitialized -Wno-unused-function
export CFLAGS = $(COPT) $(CWARN) $(INCLUDEDIR)
export CC = g++

# Linker options:
BASELDIRNAME = linux
LDIRNAME = $(BASELDIRNAME)$(ARCH)
LIBS = -lsigogl -lsigos -lsigkin -lsig -lglfw -lX11 -lGL
export LIBDIR = $(ROOT)/lib/$(LDIRNAME)
export LFLAGS = -L$(LIBDIR) $(LIBS)
export TARGET = target

# SIG location relative to subdirectories of modules:
export ROOT = ../../..

# Be quiet when building:
.SILENT:

# Main build function - $1:module name, $2:label 32 or 64
#define build_module
#echo "=== building $1 $2 bits ==="
#mkdir -p $(OBJDIR)/$(1)$(2)
#cd $(OBJDIR)/$1$2
#CFLAGS = -m$2 $(COPT) $(CWARN) $(INCLUDEDIR)
#LIBDIR = $(ROOT)/lib/$(SYSTEM)$2
#LFLAGS = -L$(LIBDIR) $(LIBS)
#TARGET = $1
#$(MAKE) -f ../../makefile-libs.mk
#cd ../../
#endef
#		$(call build_module,$$dir,'32');\

# Call makefiles of modules:
all: 
	@for dir in $(TARGETLIBS); do\
		echo "=== building $$dir $(ARCH) ===";\
		mkdir -p $(OBJDIR)/m$(ARCH)$$dir;\
		cd $(OBJDIR)/m$(ARCH)$$dir;\
		TARGET=$$dir;\
		$(MAKE) -f ../../makefile-libs.mk;\
		cd ../../; \
	done
	@for dir in $(TARGETEXS); do\
		echo "=== building $$dir $(ARCH) ===";\
		mkdir -p $(OBJDIR)/m$(ARCH)$$dir;\
		cd $(OBJDIR)/m$(ARCH)$$dir;\
		TARGET=$$dir;\
		$(MAKE) -f ../../makefile-exs.mk;\
		cd ../../; \
	done
	echo "=== done ===";

# Clean options:
cleanall:
	$(RM) core *.o *~ *.x
	$(RM) ../lib/$(BASELDIRNAME)32/*.a
	$(RM) ../lib/$(BASELDIRNAME)64/*.a
	$(RM) -f -r $(OBJDIR)

# Clean options:
cleanex:
	$(RM) core *.o *~ *.x
	@for dir in $(TARGETEXS); do\
		$(RM) -f -r $(OBJDIR)/m$(ARCH)$$dir; \
	done

clean:
	$(RM) core *.o *~
	$(RM) -f -r $(OBJDIR)
