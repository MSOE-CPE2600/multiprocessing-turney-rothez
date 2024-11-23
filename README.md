# Systems Programming Labs 11 & 12 Multiprocessing & Multithreading
Zane Rothe

## Overview (Lab 11)
The purpose of this lab is to explore multiprocessing in C. Programmers use multiprocessing to decrease the time required to perform repetitive, time-consuming operations. This lab explores multiprocessing the creation of 50 Mandlebrot images. These images will then be stitched together into a movie.

## Overview (Lab 12)
The purpose of this lab is to expolore multithreading in C. Multithreading is another method used to decrease processing time. This lab will add multithreading to create a single Mandlebrot image in a shorter amount of time by dividing the processing across multiple cores. This will be implemented with the already-multiprocessed lab 11 code.

## Multiprocessing (lab 11)
Multiprocessing is acheived by executing simultaneous processes. Therefore, instead of creating 50 images serially, they can be prepared in parallel. Theoretically, every image could be made in parallel and the program could execute very quickly. However, this is not possible due to hardware limitations. In reality, the number of concurrent processes cannot exceed the number of logical processers. Furthermore, there are diminishing returns of speed increase as the number of processes increase. In the figure shown below, the processing time is shown as a function of the requested processes. This was performed on a machine with 20 logical processors.

![processing](https://github.com/user-attachments/assets/046ac23b-714e-4d8b-ae8b-939883f74306)

## Multithreading (lab 12)
Multithreading is similar to multiprocessing as they both can perform tasks in parallel. However, threads all have access to the same memory and do not receive their own variables, like processes do. In this program, multiple threeads will be used to create a single image. Each part of the image is independent, which means that each thread can work on its own to process a slice of the image. This now allows the program to multiprocess the making of the images while also multithreading the creation of each individual image. Theoretically, this should even further speed up the processing. However, there are still hardware limitations. Much like multiprocessing, multithreading utilizes multiple cores to complete tasks. If the number of requested cores exceeds the number of available cores, the process will likely not become any faster. The following table and figure show the processing time required as a function of the number of processes and number of threads.
 
|            | 1 process | 2 processes | 5 processes | 10 processes | 20 processes |
|:-----------|:---------:|:-----------:|:-----------:|:------------:|:------------:|
| 1 thread   | 33.4 s    | 17.1 s      | 8.8 s       | 7.2 s        | 6.2 s        |
| 2 threads  | 26.9 s    | 14.0 s      | 8.3 s       | 6.9 s        | 6.0 s        |
| 5 threads  | 17.1 s    | 9.6 s       | 6.9 s       | 6.2 s        | 6.0 s        |
| 10 threads | 13.4 s    | 8.2 s       | 6.3 s       | 6.0 s        | 6.0 s        |
| 20 threads | 11.4 s    | 6.9 s       | 6.2 s       | 6.1 s        | 6.0 s        |
 
 ![threads_processes_time](https://github.com/user-attachments/assets/b30367b4-828f-4da7-968a-f77633715a2e)
 
From the data, it can be observed that increasing the number of processes decreases the processing time more effectively than increasing the number of threads. Using 20 processes and 1 thread (20 cores used), the processing time is 6.2 seconds. On the other hand, using 1 process and 20 threads (also 20 cores used) results in a processing time of 11.4 seconds. The fastest processing times occurred when using 20 processes regardless of the number of threads. Multithreading when all cores are in use by the processes will result in no speed advantage; a floor of 6 seconds is reached.

## The program
This program uses semaphores to control the number of active processes. The semaphore is initially set to the number of desired processes (command line argument). Then, the semaphore is decremented as child processes are created. Each child process calls the mandel program directly. At the end of the mandel program execution, the semaphore is posted and another process can begin. This cycle repeats until 50 frames are created. The multithreading occurs when the image is about to be generated. The image is cut into vertical slices that are of a width inversely proportional to the number of threads (command line argument). Each thread computes their portion of the image and are then joined back together.
