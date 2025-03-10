CC=gcc
CXX=g++
CFLAGS=-Wall -Wextra -Wno-missing-field-initializers -Wno-sign-compare -Wno-unused-parameter -g3 -ldl -I include
CPPFLAGS=-DAIMAKE_ABI_VERSION\(x\)=' ' -DAIMAKE_NORETURN=_Noreturn -DAIMAKE_BUILDOS_linux -D_XOPEN_SOURCE=700 -D_REENTRANT
CXXFLAGS=-I include

# Note that tty.o and wrap_tty.o _must_ be linked in, and a static library does not guarantee this
# As far as I can tell, the .o files must be explicitly listed on the command line
ALL_O = $(addprefix src/,libuncursed.o plugins.o plugins/tty.o plugins/wrap_tty.o)
LIB_O = $(addprefix src/,libuncursed.o plugins.o)

default: $(ALL_O) libuncursed.a

libuncursed.a: $(LIB_O)
	ar rcs $@ $^

src/plugins/wrap_%.o: src/plugins/%.cxx
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MMD -MP -c -o $@ $<

src/plugins/wrap_%.d:
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MM -MP -MG -MT src/plugins/wrap_$*.o -MF src/plugins/wrap_$*.d src/plugins/$*.cxx

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c -o $@ $<

%.d:
	@$(CC) $(CFLAGS) $(CPPFLAGS) -MM -MP -MG -MT $*.o -MF $@ $*.c

ALL_D = $(ALL_O:%.o=%.d)
-include $(ALL_D)

.PHONY: clean
clean:
	rm -f libuncursed.a $(ALL_O) $(ALL_D)
