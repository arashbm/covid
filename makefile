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
					 -Wno-unknown-pragmas \
					 -O3 \
					 -std=c++17 \
					 -g\
					 -Idep/catch2/single_include/catch2 \
					 -Idep/csv-parser/single_include/ \
					 -Idep/args/

LD = $(CXX)
LDLIBS = -static-libstdc++ -lpthread


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

.PHONY: figures
figures: kisdi_scenario
	$(eval RESDIR := $(shell mktemp -d))
	@mkdir $(RESDIR)
	./kisdi_scenario \
		-p data/age-distribution-municipalities-arenas.csv \
		-i data/initial-condition-municipalities-arenas.csv \
		-c data/model-config-kisdi.csv \
		-t 50 \
		> $(RESDIR)/kisdi-municipalities.csv
	./kisdi_scenario \
		-p data/age-distribution-whole-country-arenas.csv \
		-i data/initial-condition-whole-country-arenas.csv \
		-c data/model-config-kisdi.csv \
		-t 50 \
		> $(RESDIR)/kisdi-country.csv
	@mkdir -p figures
	python visualisations/spreading.py $(RESDIR)/kisdi-municipalities.csv \
		> figures/kisdi-municipalities.pdf
	python visualisations/spreading.py $(RESDIR)/kisdi-country.csv \
		> figures/kisdi-country.pdf
	$(shell rm -rf $(RESDIR))

kisdi_scenario: $(OBJDIR)/kisdi.o \
								$(OBJDIR)/kisdi-scenario.o \
								$(OBJDIR)/gravity.o
	$(LINK.o)

arenas_scenario: $(OBJDIR)/arenas.o \
								$(OBJDIR)/arenas-scenario.o \
								$(OBJDIR)/gravity.o
	$(LINK.o)

tests: $(TSTOBJDIR)/tests.o \
			 $(TSTOBJDIR)/kisdi.o \
			 $(TSTOBJDIR)/categorical_array.o \
			 $(OBJDIR)/kisdi.o
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
