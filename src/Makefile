N=project-01

all: build run

build:
	@ g++ *.cpp -o $N.out \
	-std=c++11 \
	-DBOOST_LOG_DYN_LINK -lboost_log -lboost_log_setup -lboost_thread -lboost_system

build-test:
	@ g++ Reduce.cpp tests/*.cpp -o bin/test

run-test: build-test
	@ ./bin/test

run: build
	@ ./$N.out

debug:
	@ g++ *.cpp -o $N.out -ggdb

archive:
	zip $N.zip *.cpp *.h *.png Makefile
	mv $N.zip ~/Downloads/
