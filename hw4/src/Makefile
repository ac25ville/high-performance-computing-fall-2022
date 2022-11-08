# set this to what I want
OTHER_FLAGS=$(ENV_HPC_OPTS) -std=c++17 -O2

# Standard all target
all: homework4

# HW 1
homework4: homework4.o
	mpic++ -o homework4 homework4.o -pthread -lboost_thread

homework4.o: homework4.cpp 
	mpic++ ${OTHER_FLAGS} -Werror -Wall -c homework4.cpp

clean:
	rm -f *.o homework4 *.csv