# Names of the modules to be compiled:
TARGETLIBS = sig sigogl sigos sigkin
TARGETEXS = gstests shapes cameratest customnode fontgen modelviewer polyeditor scenetest skelviewer uidemo vgtest
OBJDIR = obj

# SIG location relative to subdirectories of modules:
export ROOT = ../../..

# Compilation options:
export INCLUDEDIR = -I$(ROOT)/include -I/X11
export CC = g++
export COPT = -O2 -std=c++11
export CFLAGS = $(COPT) $(WARN) $(INCLUDEDIR)
export WARN = -Wall -Wno-switch -Wno-maybe-uninitialized -Wno-unused-function

# Linker options:
export SYSTEM = linux
export LIBDIR = $(ROOT)/lib/$(SYSTEM)
export LIBS = -lsigogl -lsigos -lsigkin -lsig -lglfw -lX11 -lGL
export LFLAGS = -L$(LIBDIR) $(LIBS)
export TARGET = target

# Be quiet when building:
.SILENT:

# Call makefiles of modules:
all: 
	@for dir in $(TARGETLIBS); do\
		echo "=== building $$dir ===";\
		mkdir -p $(OBJDIR)/$$dir;\
		cd $(OBJDIR)/$$dir;\
		TARGET=$$dir;\
		$(MAKE) -f ../../makefilelib.mk;\
		cd ../../; \
	done
	@for dir in $(TARGETEXS); do\
		echo "=== building $$dir ===";\
		mkdir -p $(OBJDIR)/$$dir;\
		cd $(OBJDIR)/$$dir;\
		TARGET=$$dir;\
		$(MAKE) -f ../../makefileex.mk;\
		cd ../../; \
	done
	echo "=== done ===";

# Clean options:
cleanall:
	$(RM) core *.o *~ *.x
	$(RM) ../lib/$(SYSTEM)/*.a
	$(RM) -f -r $(OBJDIR)

# Clean options:
cleanex:
	$(RM) core *.o *~ *.x
	@for dir in $(TARGETEXS); do\
		$(RM) -f -r $(OBJDIR)/$$dir; \
	done

clean:
	$(RM) core *.o *~
	$(RM) -f -r $(OBJDIR)
