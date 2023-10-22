N=project-01

all: build run

build:
	@ g++ src/*.cpp -o bin/$N.out \
	-std=c++11 \
	-DBOOST_LOG_DYN_LINK -lboost_log -lboost_log_setup -lboost_thread -lboost_system -lboost_filesystem

run: build
	@ ./bin/$N.out workdir/input workdir/output workdir/temp

build-test:
	@ g++ src/Reduce.cpp tests/*.cpp -o bin/test

run-test: build build-test
	@ ./bin/test
	@ ./bin/$N.out tests/workdir/input tests/workdir/output tests/workdir/temp

build-debug:
	@ g++ src/*.cpp -o bin/$N.out \
	-std=c++11 \
	-DBOOST_LOG_DYN_LINK -lboost_log -lboost_log_setup -lboost_thread -lboost_system -lboost_filesystem \
	-ggdb

debug: build-debug
	# Will need to run this outside of make
	@ gdb --args bin/project-01.out tests/workdir/input tests/workdir/output tests/workdir/temp

# Remove compiled binaries, output files, and temp files
clean:
	@ find ./bin/ ! -name '.gitignore' -type f -exec rm -f {} +
	@ find tests/workdir/output ! -name '.gitignore' -type f -exec rm -f {} +
	@ find tests/workdir/temp ! -name '.gitignore' -type f -exec rm -f {} +
	@ find workdir/output ! -name '.gitignore' -type f -exec rm -f {} +
	@ find workdir/temp ! -name '.gitignore' -type f -exec rm -f {} +
