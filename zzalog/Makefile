# Thanks to Job Vranish (https://spin.atomicobject.com/2016/08/26/makefile-c-projects/)
TARGET_EXEC := zzalog.exe
BUILD_TYPE := Release

BUILD_DIR := ./build
SRC_DIRS := ./src
EXEC_DIR = $(BUILD_DIR)/$(BUILD_TYPE)

FLTK_DIR := /c/Users/pvros/dev/fltk
FLTK_BUILD := $(FLTK_DIR)/build/$(BUILD_TYPE)
HAMLIB_DIR := /c/Users/pvros/dev/hamlib-4.5
HAMLIB_BUILD := $(HAMLIB_DIR)/mingw64-inst

# Standard compilers and debuggers
CC = gcc
CXX = g++
DBG = gdb

# Find all the C and C++ files we want to compile
# Note the single quotes around the * expressions. Make will incorrectly expand these otherwise.
SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.s')

# String substitution for every C/C++ file.
# As an example, hello.cpp turns into ./build/hello.cpp.o
OBJS := $(SRCS:%=$(EXEC_DIR)/%.o)

# String substitution (suffix version without %).
# As an example, ./build/hello.cpp.o turns into ./build/hello.cpp.d
DEPS := $(OBJS:.o=.d)

# tar file
TARFILE = $(TARGET_EXEC:.exe=.tgz)

# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := ./include
INC_DIRS += $(FLTK_DIR) $(FLTK_BUILD)
INC_DIRS += $(HAMLIB_BUILD)/include

# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# Set up libraries
LIB_DIRS := $(FLTK_BUILD)/lib 
LIB_DIRS += $(HAMLIB_BUILD)/lib
LIBS := -l:libfltk.a -l:libfltk_gl.a -l:libfltk_images.a \
 -l:libfltk_jpeg.a -l:libfltk_png.a -l:libfltk_z.a -l:libfltk_forms.a\
 -l:libhamlib.dll.a -lgdiplus -lcomctl32 -lws2_32 \
 -lgdiplus -lkernel32 -luser32 -lgdi32 -lwinspool \
 -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32 \
 -lcurl
 LDFLAGS := $(addprefix -L,$(LIB_DIRS)) $(LIBS)

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
CPPFLAGS := $(INC_FLAGS) -MMD -MP 
CFLAGS += -g

# Set ccompile flags
# include debug data
CXXFLAGS += -g
# Avoid muliple definition of byte
CXXFLAGS += -DWIN32_LEAN_AND_MEAN

# DEBUG_FLAG := -Og

# The final build step.
$(EXEC_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Build step for C source
$(EXEC_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
ifeq ($(BUILD_TYPE), Debug)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEBUG_FLAG) -c $< -o $@
else
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
endif
# Build step for C++ source
$(EXEC_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
ifeq ($(BUILD_TYPE), Debug)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEBUG_FLAG) -c $< -o $@
else
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
endif

# Create a target to run the compiled code
.PHONY: run
run: $(EXEC_DIR)/$(TARGET_EXEC)
	$(EXEC_DIR)/$(TARGET_EXEC)

# Create a target to run debugger
.PHONY: debug
debug: $(EXEC_DIR)/$(TARGET_EXEC)
	$(DBG) $(EXEC_DIR)/$(TARGET_EXEC)

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)