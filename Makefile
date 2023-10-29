NAME=project-01

STD=-std=c++11
BOOST=-DBOOST_LOG_DYN_LINK -lboost_log -lboost_log_setup -lboost_thread -lboost_system -lboost_filesystem

DEBUG_NAME=$(NAME)-debug

all: build run

build:
	@ g++ src/*.cpp -o bin/$(NAME) \
	$(STD) $(BOOST)

build-dflag:
	@ g++ src/*.cpp -DDEBUG -o bin/$(NAME) \
	$(STD) $(BOOST)

run-dflag: build-dflag
	@ ./bin/$(NAME) workdir/input workdir/output workdir/temp

## CLASS REDUCE
build-reduce:
	@ g++ src/Reduce.cpp \
	tests/testReduce.cpp \
	$(STD) $(BOOST) \
	-o bin/testReduce

test-reduce: build-reduce
	@echo "\n*** TESTING REDUCE CLASS ***"
	@ ./bin/testReduce

## CLASS MAP
build-map:
	@ g++ src/Map.cpp \
	src/FileManager.cpp \
	tests/testMap.cpp \
	$(STD) $(BOOST) \
	-o bin/testMap

test-map: build-map
	@echo "\n*** TESTING MAP CLASS ***"
	@ ./bin/testMap

## CLASS FILEMANAGER
build-filmgr:
	@ g++ src/FileManager.cpp \
	tests/testFileManager.cpp \
	$(STD) $(BOOST) \
	-o bin/testFileManager

test-filmgr: build-filmgr
	@echo "\n*** TESTING FILE MANAGER CLASS ***"
	@ ./bin/testFileManager

build-debug:
	@ g++ src/*.cpp -o bin/$(DEBUG_NAME) \
	$(STD) $(BOOST) \
	-ggdb

run: build
	@ ./bin/$(NAME) workdir/input workdir/output workdir/temp

# run all unit tests
test: test-map test-reduce test-filmgr

debug: build-debug
	@ gdb --args bin/$(DEBUG_NAME) tests/workdir/input tests/workdir/output tests/workdir/temp

# Remove compiled binaries, output files, and temp files
clean:
	@ find ./bin/ ! -name '.gitignore' -type f -exec rm -f {} +
	@ find tests/workdir/output ! -name '.gitignore' -type f -exec rm -f {} +
	@ find tests/workdir/temp ! -name '.gitignore' -type f -exec rm -f {} +
	@ find workdir/output ! -name '.gitignore' -type f -exec rm -f {} +
	@ find workdir/temp ! -name '.gitignore' -type f -exec rm -f {} +
