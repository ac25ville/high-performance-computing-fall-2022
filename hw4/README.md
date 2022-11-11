# RUN INSTRUCTIONS

* Be sure that you are in the hw4 directory

* make clean

* make

* /path/to/homework4 /path/to/input_file N C P

# RUN ANALYSIS

* Be sure that you are in the hw4 directory

* make clean

* seed csv must be in this path, or edit the path to fit the needs "../coordCSVs/"$$n"_Coord.csv"

* set c in make file (default 10000)

* set max_parallel varaible in make file (default 8)

* outputs to analysis.csv format P,totalTime,multiProcessTime

* default is for it to print for analysis, can be changed in CPP file by changing the ANALYSIS_PRINT varaible to 0

### Note make clean will clear generated *.csv files in the subdirectories as well