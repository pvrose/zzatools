# Thanks to Job Vranish (https://spin.atomicobject.com/2016/08/26/makefile-c-projects/)
TARGET_EXEC := zzalog
BUILD_TYPE := Release

BUILD_DIR := ./build
SRC_DIRS := ./src ../widgets/src
REF_DIR := ../reference
UG_DIR := ./userguide
CDX_DIR := ./docs
EXEC_DIR = $(BUILD_DIR)/$(BUILD_TYPE)
INSTALL_DIR := /usr/local/bin
LIBRARY_DIR := /usr/local/lib
APPDATA_DIR := ${HOME}/.config/GM3ZZA/ZZALOG
DOCUMENTS_DIR :=$(APPDATA_DIR)

# Standard compilers and debuggers
CC = gcc
CXX = g++
DBG = gdb

#Suppress display of executed commands.
#$(VERBOSE).TARGET:

# Find all the C and C++ files we want to compile
# Note the single quotes around the * expressions. Make will incorrectly expand these otherwise.
SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.s')
SRCS += ../common/src/utils.cpp
SRCS += ../common/src/pugixml.cpp
# String substitution for every C/C++ file.
# As an example, hello.cpp turns into ./build/hello.cpp.o
OBJS := $(SRCS:%=$(EXEC_DIR)/%.o)

# String substitution (suffix version without %).
# As an example, ./build/hello.cpp.o turns into ./build/hello.cpp.d
DEPS := $(OBJS:.o=.d)

# Doxygen source
DOXS := $(shell find $(UG_DIR) -name '*.dox')

# tar file
TARFILE = $(TARGET_EXEC:.exe=.tgz)

# Refenerce files
REFS := $(shell ls $(REF_DIR))
# Target references
TREFS := $(REFS:%=$(APPDATA_DIR)/%)

# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := ./include ../widgets/include ../common/include
# Include files for code documentation
INCS := $(shell find $(INC_DIRS) -name '*.h' -or -name '*.hpp' -or -name '*.H')

# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# Set up libraries
FLTK_FLAGS := `fltk-config --use-images --ldflags` 

# Set up working directory
PWD := `pwd`

# Hamlib flags
HAMLIB_FLAGS := -L$(LIBRARY_DIR) -lhamlib -Wl,-rpath -Wl,$(LIBRARY_DIR)
# Curl flags
CURL_FLAGS := -lcurl

LDFLAGS := $(FLTK_FLAGS) $(HAMLIB_FLAGS) $(CURL_FLAGS)

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
CPPFLAGS := $(INC_FLAGS) -MMD -MP 

LINTFLAG := -Wunused -Wswitch 
# Set ccompile flags
# include debug data
CFLAGS += -g $(LINTFLAG) -std=c++20
CXXFLAGS += -g $(LINTFLAG) 

ifeq ($(BUILD_TYPE), Anal)
CXXFLAGS += -fsanitize=address
LDFLAGS += -fsanitize=address -static-libasan
endif
# DEBUG_FLAG := -Og

# The final build step.
$(EXEC_DIR)/$(TARGET_EXEC): $(OBJS) ${INSTALL_DIR}/fltk-config $(LIBRARY_DIR)/libhamlib.so
	@echo Linking $@
	@$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Build step for C source
$(EXEC_DIR)/%.c.o: %.c
	@echo Compiling $@
	@mkdir -p $(dir $@)
ifeq ($(BUILD_TYPE), Debug)
	@$(CC) $(CPPFLAGS) $(CFLAGS) $(DEBUG_FLAG) -c $< -o $@
else
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
endif
# Build step for C++ source
$(EXEC_DIR)/%.cpp.o: %.cpp
	@echo Compiling $@
	@mkdir -p $(dir $@)
ifeq ($(BUILD_TYPE), Debug)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEBUG_FLAG) -c $< -o $@
else
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
endif
# Create the logo includefile
./include/logo.h: ../rose.png
	@echo Creating $@
	@xxd -i $< > $@
	
# Touch main if any other source has changed
./include/timestamp.h: $(SRCS)
	touch ./include/timestamp.h

# Build all without running
.PHONY: all
all: $(EXEC_DIR)/$(TARGET_EXEC) $(TREFS) documents

# Create a target to run the compiled code
.PHONY: run
run: all
	@$(EXEC_DIR)/$(TARGET_EXEC) $(ARGS) -g ./

# Create a target to run debugger
.PHONY: debug
debug: $(EXEC_DIR)/$(TARGET_EXEC)
	@$(DBG) --args $(EXEC_DIR)/$(TARGET_EXEC) $(ARGS) -g ./

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

.PHONY: clean_ug
clean_ug:
	rm -r $(UG_DIR)/html $(UG_DIR)/latex
	sudo rm -r $(DOCUMENTS_DIR)

# Copy references
$(APPDATA_DIR)/%: $(REF_DIR)/%
	@echo $@
	@mkdir -p $(APPDATA_DIR)
	@cp $< $@

# Copy User guide
$(DOCUMENTS_DIR)/userguide/html/index.html : $(UG_DIR)/html/index.html ./Doxyfile
	@echo Copying Userguide HTML files
	@sudo mkdir -p $(DOCUMENTS_DIR)/userguide/html
	@sudo cp -r $(UG_DIR)/html/* $(DOCUMENTS_DIR)/userguide/html/
	@sudo mkdir -p $(DOCUMENTS_DIR)/userguide/images
	@sudo cp $(UG_DIR)/images/* $(DOCUMENTS_DIR)/userguide/images/

# Copy Code documenation
$(DOCUMENTS_DIR)/docs/html/index.html : $(UG_DIR)/html/index.html ./Doxyfile
	@echo Copying Code Documentation HTML files
	@sudo mkdir -p $(DOCUMENTS_DIR)/docs/html
	@sudo cp -r $(CDX_DIR)/html/* $(DOCUMENTS_DIR)/docs/html/
# 	@sudo mkdir -p $(DOCUMENTS_DIR)/docs/images
# 	@sudo cp $(CDX_DIR)/images/* $(DOCUMENTS_DIR)/docs/images/

# Copy PDF
$(DOCUMENTS_DIR)/userguide/ZZALOG.pdf: $(UG_DIR)/ZZALOG.pdf
	@echo Copying PDF
	@sudo cp $(UG_DIR)/ZZALOG.pdf $(DOCUMENTS_DIR)/userguide/ZZALOG.pdf

$(UG_DIR)/html/index.html: ./Doxyfile $(DOXS)
	@echo Creating User Guide
	@doxygen

$(UG_DIR)/latex/refman.tex: $(UG_DIR)/html/index.html
	@echo Making latex documents

$(CDX_DIR)/html/index.html: ./Doxyfile_code $(INCS)
	@echo Creating Code documentation
	@doxygen ./Doxyfile_code

.PHONY: documents
documents: $(UG_DIR)/html/index.html $(CDX_DIR)/html/index.html $(UG_DIR)/ZZALOG.pdf

$(UG_DIR)/ZZALOG.pdf: $(UG_DIR)/latex/refman.pdf
	cp $(UG_DIR)/latex/refman.pdf $(UG_DIR)/ZZALOG.pdf

$(UG_DIR)/latex/refman.pdf: $(UG_DIR)/latex/refman.tex
	@echo Creating PDF
	@mkdir -p $(UG_DIR)/latex
	@make -C $(UG_DIR)/latex

.PHONY: install_docs
install_docs: $(DOCUMENTS_DIR)/userguide/html/index.html $(DOCUMENTS_DIR)/docs/html/index.html $(DOCUMENTS_DIR)/userguide/ZZALOG.pdf

# # force recompile
# .PHONY: touch_main
# touch_main:
# 	@touch ./src/main.cpp

# install
.PHONY: install
install: install_docs $(TREFS)  $(INSTALL_DIR)/$(TARGET_EXEC)

$(INSTALL_DIR)/$(TARGET_EXEC): $(EXEC_DIR)/$(TARGET_EXEC)
	@echo Installing $(INSTALL_DIR)/$(TARGET_EXEC)
	@sudo cp $(EXEC_DIR)/$(TARGET_EXEC) $(INSTALL_DIR)

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)
