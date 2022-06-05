

# -------------- Settings --------------------------

# Main directories
SOURCE_DIR	:= src
BUILD_DIR	:= build

OBJS_DIR	:= $(BUILD_DIR)/objs
DEPS_DIR	:= $(BUILD_DIR)/deps

# Binary target file
TARGET		:= $(BUILD_DIR)/wget.out

# Compiler and linker settings
CXX			:= g++
LD			:= g++
CXXFLAGS	:= -g -Wall -Wextra -pedantic -std=c++17
LDFLAGS		:= -lssl -lcrypto

# Additional variables
SOURCES		:= $(wildcard $(SOURCE_DIR)/*.cpp)
HEADERS		:= $(wildcard $(SOURCE_DIR)/*.h)
OBJS		:= $(patsubst %,$(OBJS_DIR)/%.o,$(notdir $(basename $(SOURCES))))
DEPS		:= $(patsubst %,$(DEPS_DIR)/%.d,$(notdir $(basename $(SOURCES))))


# -------------- Entry points ----------------------

# Main entry
all: build doc

# Help with targets
.PHONY: help
help:
	@echo Available targets: all, build, clean, doc, tests, linecount

# Main build
.PHONY: build
build: $(TARGET)

# Build and run tests
.PHONY: tests
tests: tests_build
	./$(TARGET)

.PHONY: tests_build
tests_build: CXXFLAGS += -DIS_TESTS
tests_build: build;

# Clean
.PHONY: clean
clean:
	rm -rf $(OBJS_DIR) $(DEPS_DIR)
	rm -rf doc
	rm -rf output


# -------------- Building and linking --------------

# Link all together to make final target
$(TARGET): $(OBJS)
	mkdir -p $(@D)
	$(LD) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Make .o and .d file for every .cpp file
$(OBJS_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	mkdir -p $(@D)
	mkdir -p $(DEPS_DIR)
	$(CXX) $(CXXFLAGS) -MMD -MF $(DEPS_DIR)/$(basename $(@F)).d -c $< -o $@

# Add rules for every .cpp file based on .d
-include $(DEPS)


# -------------- Documentation ---------------------

# Create doxygen documentation and index.html redirection
.PHONY: doc
doc: doc/index.html

doc/index.html: Doxyfile assets/docs_files/* $(SOURCES) $(HEADERS)
	@doxygen ./Doxyfile
	@echo '<meta http-equiv="REFRESH" content="0;URL=html/index.html">' > doc/index.html


# -------------- Utils -----------------------------

.PHONY: linecount
linecount:
	wc -l $(SOURCES) $(HEADERS) Makefile

# run program with valgrind for errors
.PHONY: valgrind
valgrind: $(TARGET)
	valgrind ./$(TARGET)

# run program with valgrind for leak checks
.PHONY: valgrind_leakcheck
valgrind_leakcheck: $(TARGET)
	valgrind --leak-check=full ./$(TARGET)

# run program with valgrind for leak checks (extreme)
.PHONY: valgrind_extreme
valgrind_extreme: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes ./$(TARGET)