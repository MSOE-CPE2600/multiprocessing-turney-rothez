# System Programming Lab 11 Multiprocessing
Zane Rothe

## Overview
The purpose of this lab is to explore multiprocessing in C. Programmers use multiprocessing to decrease the time required to perform repetitive, time-consuming operations. This lab explores multiprocessing the creation of 50 Mandlebrot images. These images will then be stitched together into a movie.

## Multiprocessing
Multiprocessing is acheived by executing simultaneous processes. Therefore, instead of creating 50 images serially, they can be prepared in parallel. Theoretically, every image could be made in parallel and the program could execute very quickly. However, this is not possible due to hardware limitations. In reality, the number of concurrent processes cannot exceed the number of logical processers. Furthermore, there are diminishing returns of speed increase as the number of processes increase. In the figure shown below, the processing time is shown as a function of the requested processes. This was performed on a machine with 20 logical processors.

![processing](https://github.com/user-attachments/assets/046ac23b-714e-4d8b-ae8b-939883f74306)

## The program
This program uses semaphores to control the number of active processes. The semaphore is initially set to the number of desired processes (command line argument). Then, the semaphore is decremented as child processes are created. Each child process calls the mandel program directly. At the end of the mandel program execution, the semaphore is posted and another process can begin. This cycle repeats until 50 frames are created.
