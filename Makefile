N=project-01

all: build run

build:
	@ g++ -I /usr/local/Cellar/boost/1.82.0_1 *.cpp -o $N.out \
	-std=c++11 \
	-DBOOST_LOG_DYN_LINK -lboost_log-mt

run: build
	@ ./$N.out

debug:
	@ g++ *.cpp -o $N.out -ggdb

archive:
	zip $N.zip *.cpp *.h *.png Makefile
	mv $N.zip ~/Downloads/
