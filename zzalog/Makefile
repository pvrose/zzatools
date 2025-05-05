# Thanks to Job Vranish (https://spin.atomicobject.com/2016/08/26/makefile-c-projects/)
TARGET_EXEC := zzalog
BUILD_TYPE := Release

BUILD_DIR := ./build
SRC_DIRS := ./src ../widgets/src
EXEC_DIR = $(BUILD_DIR)/$(BUILD_TYPE)
INSTALL_DIR := /usr/local/bin

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
SRCS += ../common/src/xml_writer.cpp
SRCS += ../common/src/xml_reader.cpp
SRCS += ../common/src/xml_element.cpp

# String substitution for every C/C++ file.
# As an example, hello.cpp turns into ./build/hello.cpp.o
OBJS := $(SRCS:%=$(EXEC_DIR)/%.o)

# String substitution (suffix version without %).
# As an example, ./build/hello.cpp.o turns into ./build/hello.cpp.d
DEPS := $(OBJS:.o=.d)

# tar file
TARFILE = $(TARGET_EXEC:.exe=.tgz)

# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := ./include ../widgets/include ../common/include

# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# Set up libraries
FLTK_FLAGS := `fltk-config --use-images --ldflags` 

# Hamlib flags
HAMLIB_FLAGS := -L/usr/local/lib -lhamlib -Wl,-rpath -Wl,/usr/local/lib
# Curl flags
CURL_FLAGS := -lcurl

LDFLAGS := $(FLTK_FLAGS) $(HAMLIB_FLAGS) $(CURL_FLAGS)

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
CPPFLAGS := $(INC_FLAGS) -MMD -MP 

LINTFLAG := -Wunused -Wswitch 
# Set ccompile flags
# include debug data
CFLAGS += -g $(LINTFLAG)
CXXFLAGS += -g $(LINTFLAG)

ifeq ($(BUILD_TYPE), Anal)
CXXFLAGS += -fsanitize=address
LDFLAGS += -fsanitize=address -static-libasan
endif
# DEBUG_FLAG := -Og

# The final build step.
$(EXEC_DIR)/$(TARGET_EXEC): $(OBJS) ${INSTALL_DIR}/fltk-config
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
$(INC_DIRS)/logo.h: ../rose.png
	@echo Creating $@
	@xxd -i $< > $@
	
# Create a target to run the compiled code
.PHONY: run
run: $(EXEC_DIR)/$(TARGET_EXEC)
	@$(EXEC_DIR)/$(TARGET_EXEC) $(ARGS)

# Create a target to run debugger
.PHONY: debug
debug: $(EXEC_DIR)/$(TARGET_EXEC)
	@$(DBG) --args $(EXEC_DIR)/$(TARGET_EXEC) $(ARGS)

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

# install
.PHONY: install
install: $(INSTALL_DIR)/$(TARGET_EXEC)

$(INSTALL_DIR)/$(TARGET_EXEC): $(EXEC_DIR)/$(TARGET_EXEC)
	@echo Installing $(INSTALL_DIR)/$(TARGET_EXEC)
	@sudo cp $(EXEC_DIR)/$(TARGET_EXEC) $(INSTALL_DIR)

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)
