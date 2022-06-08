

# -------------- Settings --------------------------

# Main directories
SOURCE_DIR	:= src
BUILD_DIR	:= build

OBJS_DIR	:= $(BUILD_DIR)/objs
DEPS_DIR	:= $(BUILD_DIR)/deps

# Binary target file
TARGET		:= cernyj87

# Compiler and linker settings
CXX			:= g++
LD			:= g++
CXXFLAGS	:= -g -Wall -Wextra -pedantic -std=c++17
LDFLAGS		:= -lstdc++fs -lssl -lcrypto

# Additional variables
SOURCES		:= $(wildcard $(SOURCE_DIR)/*.cpp)
HEADERS		:= $(wildcard $(SOURCE_DIR)/*.h)
OBJS		:= $(patsubst %,$(OBJS_DIR)/%.o,$(notdir $(basename $(SOURCES))))
DEPS		:= $(patsubst %,$(DEPS_DIR)/%.d,$(notdir $(basename $(SOURCES))))


# -------------- Entry points ----------------------

# Main entry
all: compile doc

# Help with targets
.PHONY: help
help:
	@echo Available targets: all, compile, clean, doc, tests, linecount

# Main build
.PHONY: compile
compile: $(TARGET)

# Build and run tests
.PHONY: tests
tests: tests_compile
	./$(TARGET)

.PHONY: tests_compile
tests_compile: CXXFLAGS += -DIS_TESTS
tests_compile: compile;

# Clean
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(TARGET)
	rm -rf doc

# Run target
.PHONY: run
run:
	./$(TARGET)


# -------------- Building and linking --------------

# Link all together to make final target
$(TARGET): $(OBJS)
	$(LD) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Make .o and .d file for every .cpp file
$(OBJS_DIR)/%.o: $(SOURCE_DIR)/%.cpp $(OBJS_DIR) $(DEPS_DIR)
	$(CXX) $(CXXFLAGS) -MMD -MF $(DEPS_DIR)/$(basename $(@F)).d -c $< -o $@

# Add rules for every .cpp file based on .d
-include $(DEPS)

# Create directories if not present
$(OBJS_DIR):
	mkdir -p $@

$(DEPS_DIR):
	mkdir -p $@


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
