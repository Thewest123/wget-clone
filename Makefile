SOURCE_DIR = src
BUILD_DIR = build

OBJS	= $(BUILD_DIR)/main.o $(BUILD_DIR)/CLogger.o $(BUILD_DIR)/CHttpDownloader.o $(BUILD_DIR)/CConfig.o $(BUILD_DIR)/CFile.o $(BUILD_DIR)/CFileHtml.o
SOURCE	= $(SOURCE_DIR)/main.cpp $(SOURCE_DIR)/CLogger.cpp $(SOURCE_DIR)/CHttpDownloader.cpp $(SOURCE_DIR)/CConfig.cpp $(SOURCE_DIR)/CFile.cpp $(SOURCE_DIR)/CFileHtml.cpp
HEADER	= $(SOURCE_DIR)/CConfig.h $(SOURCE_DIR)/CLogger.h $(SOURCE_DIR)/CHttpDownloader.h $(SOURCE_DIR)/CConfig.h $(SOURCE_DIR)/CFile.h $(SOURCE_DIR)/CFileHtml.h
OUT		= $(BUILD_DIR)/wget.out
CC		= g++
FLAGS	= -g -c -Wall -pedantic -std=c++17


all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

# create/compile the individual files >>separately<<
$(BUILD_DIR)/main.o: $(SOURCE_DIR)/main.cpp
	$(CC) $(FLAGS) $(SOURCE_DIR)/main.cpp -o $@

$(BUILD_DIR)/CLogger.o: $(SOURCE_DIR)/CLogger.cpp
	$(CC) $(FLAGS) $(SOURCE_DIR)/CLogger.cpp -o $@

$(BUILD_DIR)/CHttpDownloader.o: $(SOURCE_DIR)/CHttpDownloader.cpp
	$(CC) $(FLAGS) $(SOURCE_DIR)/CHttpDownloader.cpp -o $@

$(BUILD_DIR)/CConfig.o: $(SOURCE_DIR)/CConfig.cpp
	$(CC) $(FLAGS) $(SOURCE_DIR)/CConfig.cpp -o $@

$(BUILD_DIR)/CFile.o: $(SOURCE_DIR)/CFile.cpp
	$(CC) $(FLAGS) $(SOURCE_DIR)/CFile.cpp -o $@

$(BUILD_DIR)/CFileHtml.o: $(SOURCE_DIR)/CFileHtml.cpp
	$(CC) $(FLAGS) $(SOURCE_DIR)/CFileHtml.cpp -o $@

run: $(OUT)
	./$(OUT)

# clean house
clean:
	rm -f $(OBJS) $(OUT)
	rm -rf docs/html docs/latex

# compile program with debugging information
debug: $(OUT)
	valgrind ./$(OUT)

# create doxygen documentation
docs: Doxyfile docs/header.html docs/doxygen-awesome-css/* $(OBJS)
	doxygen Doxyfile

# run program with valgrind for errors
valgrind: $(OUT)
	valgrind ./$(OUT)

# run program with valgrind for leak checks
valgrind_leakcheck: $(OUT)
	valgrind --leak-check=full ./$(OUT)

# run program with valgrind for leak checks (extreme)
valgrind_extreme: $(OUT)
	valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes ./$(OUT)