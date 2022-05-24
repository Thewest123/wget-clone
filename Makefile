SOURCE_DIR = src
BUILD_DIR = build

OBJS	= $(BUILD_DIR)/Utils.o $(BUILD_DIR)/CLogger.o $(BUILD_DIR)/CHttpsDownloader.o $(BUILD_DIR)/CConfig.o $(BUILD_DIR)/CURLHandler.o $(BUILD_DIR)/CFile.o $(BUILD_DIR)/CFileHtml.o
SOURCE	= $(SOURCE_DIR)/main.cpp $(SOURCE_DIR)/Utils.cpp $(SOURCE_DIR)/CLogger.cpp $(SOURCE_DIR)/CHttpsDownloader.cpp $(SOURCE_DIR)/CConfig.cpp $(SOURCE_DIR)/CURLHandler.cpp $(SOURCE_DIR)/CFile.cpp $(SOURCE_DIR)/CFileHtml.cpp
HEADER	= $(SOURCE_DIR)/CConfig.h $(SOURCE_DIR)/Utils.h $(SOURCE_DIR)/CLogger.h $(SOURCE_DIR)/CHttpsDownloader.h $(SOURCE_DIR)/CConfig.h $(SOURCE_DIR)/CURLHandler.h $(SOURCE_DIR)/CFile.h $(SOURCE_DIR)/CFileHtml.h
OUT		= $(BUILD_DIR)/wget.out
CC		= g++
FLAGS	= -g3 -c -Wall -pedantic -std=c++17
LFLAGS	= -lssl -lcrypto


all: $(OBJS) $(BUILD_DIR)/main.o 
	$(CC) -g3 $(BUILD_DIR)/main.o $(OBJS) -o $(OUT) $(LFLAGS)

all_fsanitize: $(OBJS)
	$(CC) -g3 $(BUILD_DIR)/main.o $(OBJS) -o $(OUT) -fsanitize=address $(LFLAGS)

tests: $(OBJS) $(BUILD_DIR)/tests.o
	$(CC) -g3 $(BUILD_DIR)/tests.o  $(OBJS) -o $(BUILD_DIR)/wget_tests.out $(LFLAGS)
	./$(BUILD_DIR)/wget_tests.out

# create/compile the individual files >>separately<<
$(BUILD_DIR)/main.o: $(SOURCE_DIR)/main.cpp
	$(CC) $(FLAGS) $(SOURCE_DIR)/main.cpp -o $@

$(BUILD_DIR)/tests.o: $(SOURCE_DIR)/tests.cpp
	$(CC) $(FLAGS) $(SOURCE_DIR)/tests.cpp -o $@

$(BUILD_DIR)/Utils.o: $(SOURCE_DIR)/Utils.cpp
	$(CC) $(FLAGS) $(SOURCE_DIR)/Utils.cpp -o $@

$(BUILD_DIR)/CLogger.o: $(SOURCE_DIR)/CLogger.cpp
	$(CC) $(FLAGS) $(SOURCE_DIR)/CLogger.cpp -o $@

$(BUILD_DIR)/CHttpsDownloader.o: $(SOURCE_DIR)/CHttpsDownloader.cpp
	$(CC) $(FLAGS) $(SOURCE_DIR)/CHttpsDownloader.cpp -o $@

$(BUILD_DIR)/CConfig.o: $(SOURCE_DIR)/CConfig.cpp
	$(CC) $(FLAGS) $(SOURCE_DIR)/CConfig.cpp -o $@

$(BUILD_DIR)/CURLHandler.o: $(SOURCE_DIR)/CURLHandler.cpp
	$(CC) $(FLAGS) $(SOURCE_DIR)/CURLHandler.cpp -o $@

$(BUILD_DIR)/CFile.o: $(SOURCE_DIR)/CFile.cpp
	$(CC) $(FLAGS) $(SOURCE_DIR)/CFile.cpp -o $@

$(BUILD_DIR)/CFileHtml.o: $(SOURCE_DIR)/CFileHtml.cpp
	$(CC) $(FLAGS) $(SOURCE_DIR)/CFileHtml.cpp -o $@

run: $(OUT)
	./$(OUT)

linecount:
	wc -l $(SOURCE) $(HEADER) Makefile

# clean house
clean:
	rm -f $(OBJS) $(OUT) $(BUILD_DIR)/wget_tests.out
	rm -rf docs/html docs/latex
	rm -rf output

# compile program with debugging information
debug: $(OUT)
	valgrind ./$(OUT)

# create doxygen documentation
docs: Doxyfile docs/header.html docs/doxygen-awesome-css/* $(SOURCE) $(HEADER)
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