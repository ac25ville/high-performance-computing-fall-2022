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


## How to build
---

1) Make sure you are in the hpc22_acc9cm/hw0 directory

2) Run the following commands in order

* cmake CMakeLists.txt

* make clean

* make

3) Run ./homework0 U.csv nodeCoordinates.csv K.csv N (N being a value given by the grader. From my understanding the test value was 5)

* To re-run, enter "make", then the line above with the same or different parameters.