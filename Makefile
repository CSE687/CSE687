NAME=project-01

STD=-std=c++11
BOOST=-DBOOST_LOG_DYN_LINK -lboost_log -lboost_log_setup -lboost_thread -lboost_system -lboost_filesystem

DEBUG_NAME=$(NAME)-debug

all: build run

build:
	@ g++ src/*.cpp -o bin/$(NAME) \
	$(STD) $(BOOST)

build-test:
	@ g++ src/Map.cpp \
	src/FileManager.cpp \
	tests/testMap.cpp \
	$(STD) $(BOOST) \
	-o bin/test

build-debug:
	@ g++ src/*.cpp -o bin/$(DEBUG_NAME) \
	$(STD) $(BOOST) \
	-ggdb

run: build
	@ ./bin/$(NAME) workdir/input workdir/output workdir/temp

test: build build-test
	@ ./bin/test
	@ ./bin/$(NAME) tests/workdir/input tests/workdir/output tests/workdir/temp

debug: build-debug
	@ gdb --args bin/$(DEBUG_NAME) tests/workdir/input tests/workdir/output tests/workdir/temp

# Remove compiled binaries, output files, and temp files
clean:
	@ find ./bin/ ! -name '.gitignore' -type f -exec rm -f {} +
	@ find tests/workdir/output ! -name '.gitignore' -type f -exec rm -f {} +
	@ find tests/workdir/temp ! -name '.gitignore' -type f -exec rm -f {} +
	@ find workdir/output ! -name '.gitignore' -type f -exec rm -f {} +
	@ find workdir/temp ! -name '.gitignore' -type f -exec rm -f {} +
