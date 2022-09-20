# set this to what I want
OTHER_FLAGS=$(ENV_HPC_OPTS) -std=c++17

# Standard all target
all: homework2

# HW 1
homework2: homework2.o
	g++ -o homework2 homework2.o 

homework2.o: homework2.cpp 
	g++ ${OTHER_FLAGS} -Werror -Wall -c homework2.cpp

clean:
	rm -f *.o homework2