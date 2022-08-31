## See PDF for full details.

Data File on Canvas:
Astronaught.png


For submission, you must have only source code and headers and build sytem support files.
The input data file is acceptable for hw0/

Depending on your build system:
Use make clean or make distclean or remove an out of source build folder.

Test your submission by doing a fresh clone from your master into a temp folder and ensure it builds / runs on the Nautilus system in the HPC container.
e.g., 
git clone <addr_to_student_work> temp_folder_name


## How to build from hpc22_acc9cm directory

1) Make sure you are in the hpc22_acc9cm directory

2) Run the following commands in order

* cmake hw0/CMakeLists.txt

* make -C hw0/ clean

* make -C hw0/

* hw0/homework0 hw0/U.csv hw0/nodeCoordinates.csv hw0/K.csv N (N being a value given by the grader)

To re-run, enter "make", then the line above with the same or different parameters.

## How to build from hpc22_acc9cm/hw0 directory

1) Make sure you are in the hpc22_acc9cm/hw0 directory

2) Run the following commands in order

* cmake CMakeLists.txt

* make clean

* make

* ./homework0 U.csv nodeCoordinates.csv K.csv N (N being a value given by the grader)

To re-run, enter "make", then the line above with the same or different parameters.