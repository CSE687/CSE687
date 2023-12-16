NAME=project-04

STD=-std=c++11
BOOST=-DBOOST_LOG_DYN_LINK -lboost_log -lboost_log_setup -lboost_thread -lboost_system -lboost_filesystem
DLL=-lmap -lreduce

DEBUG_NAME=$(NAME)-debug

all: build run

build:
	@ g++ -fPIC -o bin/reduce.o -c src/Reduce.cpp
	@ g++ -fPIC -o bin/map.o -c src/Map.cpp
	@ g++ -shared -fPIC -o bin/libmap.so bin/map.o
	@ g++ -shared -fPIC -o bin/libreduce.so bin/reduce.o
	@ g++ -Lbin/ -Wl,-rpath=bin src/main.cpp \
		src/Controller.cpp \
		src/Stub.cpp \
		src/FileManager.cpp \
		src/Workflow.cpp \
		src/threadManager.cpp \
		src/PropertyTreeQueue.cpp \
		-o bin/$(NAME) \
		$(STD) $(BOOST) $(DLL)

stub:
	@ ./bin/$(NAME) stub $(PORT)

controller:
	@ ./bin/$(NAME) controller $(PORT) workdir/input workdir/output workdir/temp 9001 9002


build-debug:
	@ g++ -fPIC -o bin/reduce.o -c src/Reduce.cpp -ggdb
	@ g++ -fPIC -o bin/map.o -c src/Map.cpp -ggdb
	@ g++ -shared -fPIC -o bin/libmap.so bin/map.o -ggdb
	@ g++ -shared -fPIC -o bin/libreduce.so bin/reduce.o -ggdb
	@ g++ -Lbin/ -Wl,-rpath=bin src/main.cpp src/FileManager.cpp src/Workflow.cpp src/threadManager.cpp src/Stub.cpp -ggdb -o bin/$(DEBUG_NAME) \
	$(STD) $(BOOST) $(DLL)

debug: build-debug
	@ gdb --args ./bin/$(DEBUG_NAME) workdir/input workdir/output workdir/temp

build-dflag:
	@ g++ -fPIC -o bin/reduce.o -c src/Reduce.cpp
	@ g++ -fPIC -o bin/map.o -c src/Map.cpp
	@ g++ -shared -fPIC -o bin/libmap.so bin/map.o
	@ g++ -shared -fPIC -o bin/libreduce.so bin/reduce.o
	@ g++ -Lbin/ -Wl,-rpath=bin src/main.cpp src/FileManager.cpp src/Workflow.cpp src/Stub.cpp -DDEBUG -o bin/$(NAME) \
	$(STD) $(BOOST) $(DLL)

run-dflag: build-dflag
	@ ./bin/$(NAME) workdir/input workdir/output workdir/temp

## CLASS REDUCE
build-reduce:
	@ g++ src/Reduce.cpp \
	src/Executor.hpp \
	src/FileManager.cpp \
	tests/testReduce.cpp \
	$(STD) $(BOOST) \
	-o bin/testReduce

test-reduce: build-reduce
	@echo "\n*** TESTING REDUCE CLASS ***"
	@ ./bin/testReduce

## CLASS MAP
build-map:
	@ g++ src/Map.cpp \
	src/Executor.hpp \
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

## CLASS WORKFLOW
# TODO: Broken
# build-workflow:
# 	@ g++ \
# 	src/FileManager.cpp \
# 	src/Map.cpp \
# 	src/Executor.hpp \
# 	src/Reduce.cpp \
# 	src/Workflow.cpp \
# 	tests/testWorkflow.cpp \
# 	$(STD) $(BOOST) \
# 	-o bin/testWorkflow

# test-workflow: build-workflow
# 	@echo "\n*** TESTING WORKFLOW CLASS ***"
# 	@ ./bin/testWorkflow

## CLASS ThreadManager
build-ThreadManager:
	@ g++ \
	src/FileManager.cpp \
	src/Map.cpp \
	src/Executor.hpp \
	src/Reduce.cpp \
	src/Workflow.cpp \
	src/ThreadManager.cpp \
	tests/testThreadManager.cpp \
	$(STD) $(BOOST) \
	-o bin/testThreadManager

test-ThreadManager: build-ThreadManager
	@echo "\n*** TESTING THREADMANAGER CLASS ***"
	@ ./bin/testThreadManager

# run all unit tests
test: test-map test-reduce test-filmgr # test-workflow

# Remove compiled binaries, output files, and temp files
clean: clean-workdir
	@ find ./bin/ ! -name '.gitignore' -type f -exec rm -f {} +

clean-workdir:
	@ rm -rf workdir/temp
	@ rm -rf workdir/output
	@ rm -rf tests/workdir/temp
	@ rm -rf tests/workdir/output
