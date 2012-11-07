# Automatically Generated Makefile by EDE.
# For use with: make
#
# DO NOT MODIFY THIS FILE OR YOUR CHANGES MAY BE LOST.
# EDE is the Emacs Development Environment.
# http://cedet.sourceforge.net/ede.shtml
#

top="$(CURDIR)"/
ede_FILES=Project.ede Makefile

main_out_SOURCES=main.cpp
main.out_OBJ= main.o
CXX= g++
CXX_COMPILE=$(CXX) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS)
CXX_DEPENDENCIES=-Wp,-MD,.deps/$(*F).P
CXX_LINK=$(CXX) $(CFLAGS) $(LDFLAGS) -L.
child_out_SOURCES=child.cpp
child.out_OBJ= child.o
pipe_out_SOURCES=pipe.cpp
pipe.out_OBJ= pipe.o
fork_out_SOURCES=fork.cpp
fork.out_OBJ= fork.o
VERSION=1.0
DISTDIR=$(top)shell-$(VERSION)
top_builddir = 

DEP_FILES=.deps/main.P .deps/lexer.P .deps/parser.P .deps/vartable.P .deps/child.P .deps/pipe.P .deps/fork.P

all: main.out child.out pipe.out fork.out

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)
-include $(DEP_FILES)

%.o: %.cpp
	@echo '$(CXX_COMPILE) -c $<'; \
	$(CXX_COMPILE) $(CXX_DEPENDENCIES) -o $@ -c $<

main.out: $(main.out_OBJ)
	$(CXX_LINK) -o $@ $^ $(LDDEPS)

child.out: $(child.out_OBJ)
	$(CXX_LINK) -o $@ $^ $(LDDEPS)

pipe.out: $(pipe.out_OBJ)
	$(CXX_LINK) -o $@ $^ $(LDDEPS)

fork.out: $(fork.out_OBJ)
	$(CXX_LINK) -o $@ $^ $(LDDEPS)

tags: 


clean:
	rm -f *.mod *.o *.obj .deps/*.P .lo

.PHONY: dist

dist:
	rm -rf $(DISTDIR)
	mkdir $(DISTDIR)
	cp $(main_out_SOURCES) $(child_out_SOURCES) $(pipe_out_SOURCES) $(fork_out_SOURCES) $(ede_FILES) $(DISTDIR)
	tar -cvzf $(DISTDIR).tar.gz $(DISTDIR)
	rm -rf $(DISTDIR)

Makefile: Project.ede
	@echo Makefile is out of date!  It needs to be regenerated by EDE.
	@echo If you have not modified Project.ede, you can use 'touch' to update the Makefile time stamp.
	@false



# End of Makefile