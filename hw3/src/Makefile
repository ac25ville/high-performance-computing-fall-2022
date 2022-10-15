# set this to what I want
OTHER_FLAGS=$(ENV_HPC_OPTS) -std=c++17

# Standard all target
all: homework3

# HW 1
homework3: homework3.o
	g++  -pthread -lboost_thread -o homework3 homework3.o 

homework3.o: homework3.cpp 
	g++ ${OTHER_FLAGS} -Werror -Wall -c homework3.cpp

clean:
	rm -f *.o homework3 *.csv