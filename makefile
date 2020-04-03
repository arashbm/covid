# object files, auto generated from source files
OBJDIR := .o
TSTOBJDIR := $(OBJDIR)/test/covid

# dependency files, auto generated from source files
DEPDIR := .d
TSTDEPDIR := $(DEPDIR)/test/covid

SRCDIR := src
TSTDIR := $(SRCDIR)/test/covid

CXX = g++
CC = $(CXX)
CXXFLAGS = -Werror -Wall -Wextra -Wconversion \
					 -O3 \
					 -std=c++17 \
					 -g\
					 -Idep/catch2/single_include/catch2

LD = $(CXX)
# LDLIBS = -static-libstdc++


DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td


COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
COMPILE.cc = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
POSTCOMPILE = @mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

LINK.o = $(LD) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

.PHONY: check
check: tests
	./tests

.PHONY: lint
lint:
	cpplint --extensions=tpp,cpp,hpp --quiet --recursive .

.PHONY: clean
clean:
	$(RM) -r $(OBJDIR) $(DEPDIR)


tests: $(TSTOBJDIR)/tests.o \
			 $(TSTOBJDIR)/patch.o \
			 $(OBJDIR)/patch.o
	$(LINK.o)

BUILD_DIRS := $(DEPDIR) $(TSTDEPDIR) $(OBJDIR) $(TSTOBJDIR)
$(OBJDIR):
	$(shell mkdir -p $(OBJDIR))
$(TSTOBJDIR):
	$(shell mkdir -p $(TSTOBJDIR))
$(DEPDIR):
	$(shell mkdir -p $(DEPDIR))
$(TSTDEPDIR):
	$(shell mkdir -p $(TSTDEPDIR))


$(OBJDIR)/%.o : $(SRCDIR)/%.c
$(OBJDIR)/%.o : $(SRCDIR)/%.c $(DEPDIR)/%.d | $(BUILD_DIRS)
	$(COMPILE.c) $(OUTPUT_OPTION) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o : $(SRCDIR)/%.cc
$(OBJDIR)/%.o : $(SRCDIR)/%.cc $(DEPDIR)/%.d | $(BUILD_DIRS)
	$(COMPILE.cc) $(OUTPUT_OPTION) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
$(OBJDIR)/%.o : $(SRCDIR)/%.cpp $(DEPDIR)/%.d | $(BUILD_DIRS)
	$(COMPILE.cc) $(OUTPUT_OPTION) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o : $(SRCDIR)/%.cxx
$(OBJDIR)/%.o : $(SRCDIR)/%.cxx $(DEPDIR)/%.d | $(BUILD_DIRS)
	$(COMPILE.cc) $(OUTPUT_OPTION) $<
	$(POSTCOMPILE)

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(DEPDIR)/*.d TSTOBJDIR/*.d)
