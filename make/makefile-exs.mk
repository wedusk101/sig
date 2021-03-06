SRCDIR = $(ROOT)/examples/$(TARGET)/
BIN = $(ROOT)/make/$(TARGET)$(ARCH).x

LIBS = $(shell echo $(LIBDIR)/*.a)
CPPFILES := $(shell echo $(SRCDIR)*.cpp)
OBJFILES = $(CPPFILES:.cpp=.o)
OBJECTS = $(notdir $(OBJFILES))
DEPENDS = $(OBJECTS:.o=.d)

$(BIN): $(OBJECTS) $(LIBS)
	echo "creating:" $(BIN);
	$(CC) $(OBJECTS) $(LFLAGS) -o $(BIN)

%.o: $(SRCDIR)%.cpp
	echo "compiling:" $<;
	$(CC) -c $(CFLAGS) $< -o $@

%.d: $(SRCDIR)%.cpp
	echo "upddepend:" $<;
	$(CC) -MM $(CFLAGS) $< > $@

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPENDS)
endif
