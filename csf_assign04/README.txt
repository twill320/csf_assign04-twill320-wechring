Command to update Assignment_4.zip contents:
zip -9r Assignment_4.zip parsort.c Makefile README.txt

CONTRIBUTIONS
Toluwani Williams
Wech Ring

REPORT:
Test run with threshold 2097152
real    0m0.748s
user    0m0.486s
sys     0m0.011s

Test run with threshold 1048576
real    0m0.253s
user    0m0.472s
sys     0m0.015s

Test run with threshold 524288
real    0m0.149s
user    0m0.489s
sys     0m0.017s

Test run with threshold 262144
real    0m0.115s
user    0m0.477s
sys     0m0.023s

Test run with threshold 131072
real    0m0.118s
user    0m0.513s
sys     0m0.040s

Test run with threshold 65536
real    0m0.118s
user    0m0.513s
sys     0m0.064s

Test run with threshold 32768
real    0m0.127s
user    0m0.526s
sys     0m0.122s

Test run with threshold 16384
real    0m0.192s
user    0m0.616s
sys     0m0.275s

The parsort program was tested with various threshold values, and the real-time performance showed a clear trend. 
As the threshold decreased, the real time decreased as well, with the real time dropping from 0.748 seconds at a 
threshold of 2M (2097152) to 0.127 seconds at a threshold of 32K (32768). This suggests that as the threshold 
decreases, the program starts using sequential sorting more frequently, which reduces the overhead associated 
with managing parallel processes. The reduction in real time indicates that, for smaller threshold values, the 
program benefits from faster, sequential sorting, while larger thresholds tend to introduce more parallel processing, 
which may have higher overhead due to process management.
