/// 
//  mandel.c
//  Based on example code found here:
//  https://users.cs.fiu.edu/~cpoellab/teaching/cop4610_fall22/project3.html
//
//  Converted to use jpg instead of BMP and other minor changes
//  Modified by Zane Rothe
//     Added semaphores (lab 11)
//     Added multithreading (lab 12)
//     CPE 2600-111
//     Lab 12
//     11/27/24
///
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "jpegrw.h"
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <math.h>
#include <pthread.h>

// local routines
static int iteration_to_color( int i, int max );
static int iterations_at_point( double x, double y, int max );
void* compute_image(void* args); // now pointed to by thread call
static void show_help();

struct compute_args // argument struct sent to thread call
{
	imgRawImage *img;
	double xmin;
	double xmax;
	double ymin;
	double ymax;
	int max;
	int n_threads;
	int nth_thread;
};

int main( int argc, char *argv[] )
{
///////////////////////////////////////////////////////////////////////////////
// Added semaphore usage
 	sem_t *sem;
    sem = sem_open("/mysem", O_CREAT, 0666, 0); //link to named semaphore
    if (sem == SEM_FAILED)
    {
        perror("sem_open");
        return 1;
    }
///////////////////////////////////////////////////////////////////////////////
	char c;

	// These are the default configuration values used
	// if no command line arguments are given.
	const char *outfile = "mandel.jpg";
	double xcenter = 0;
	double ycenter = 0;
	double xscale = 4;
	double yscale = 0; // calc later
	int    image_width = 1000;
	int    image_height = 1000;
	int    max = 1000;
	int	   n_threads = 1;

	// For each command line argument given,
	// override the appropriate configuration value.

	while((c = getopt(argc,argv,"x:y:s:W:H:m:o:n:h"))!=-1) {
		switch(c) 
		{
			case 'x':
				xcenter = atof(optarg);
				break;
			case 'y':
				ycenter = atof(optarg);
				break;
			case 's':
				xscale = atof(optarg);
				break;
			case 'W':
				image_width = atoi(optarg);
				break;
			case 'H':
				image_height = atoi(optarg);
				break;
			case 'm':
				max = atoi(optarg);
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'n':
				n_threads = atoi(optarg); // added option for number of threads
				break;
			case 'h':
				show_help();
				exit(1);
				break;
		}
	}

	// Calculate y scale based on x scale (settable) and image sizes in X and Y (settable)
	yscale = xscale / image_width * image_height;

	// Display the configuration of the image.
	//printf("mandel: x=%lf y=%lf xscale=%lf yscale=%1f max=%d outfile=%s\n",xcenter,ycenter,xscale,yscale,max,outfile);

	// Create a raw image of the appropriate size.
	imgRawImage* img = initRawImage(image_width,image_height);

	// Fill it with a black
	setImageCOLOR(img,0);

	///////////////////////////////////////////////////////////////////////////
	// Compute the Mandelbrot image with threads
	pthread_t threads[n_threads]; //array of threads
    struct compute_args myargs[n_threads]; //array of structs to pass in
    for (int i=0; i<n_threads; i++)
    {
		myargs[i].img=img;
        myargs[i].xmin=(xcenter-xscale/2)+(xscale*i/n_threads); //split image in x
        myargs[i].xmax=(xcenter-xscale/2)+(xscale*(i+1)/n_threads);
		myargs[i].ymin=ycenter-yscale/2;
        myargs[i].ymax=ycenter+yscale/2;
		myargs[i].max=max;
		myargs[i].n_threads=n_threads;
		myargs[i].nth_thread=i;
    }

	for (int i=0; i<n_threads; i++)
    {
		// make threads to split up processing of image
        if(pthread_create(&threads[i],NULL,&compute_image,(void*)&myargs[i]))
		{
			printf("pthread_create failed\n");
		}
    }
	//compute_image(img,xcenter-xscale/2,xcenter+xscale/2,ycenter-yscale/2,ycenter+yscale/2,max);
    for (int i=0; i<n_threads; i++)
    {
        pthread_join(threads[i],NULL);
    }
	///////////////////////////////////////////////////////////////////////////

	// Save the image in the stated file.
	storeJpegImageFile(img,outfile);

	// free the mallocs
	freeRawImage(img);


   
///////////////////////////////////////////////////////////////////////////////
// Added semaphore usage
    printf("writing  %s\n",outfile);
    sem_post(sem);
    sem_close(sem);
///////////////////////////////////////////////////////////////////////////////
	return 0;
}

/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max )
{
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) {

		double xt = x*x - y*y + x0;
		double yt = 2*x*y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iter;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/
void* compute_image(void* args)
{
	// get argument struct
	struct compute_args *local_args=(struct compute_args *) args; 
	imgRawImage* img =local_args->img;
    double xmin = local_args->xmin;
	double xmax = local_args->xmax;
	double ymin = local_args->ymin;
	double ymax = local_args->ymax;
	int max = local_args->max;
	int n_threads = local_args->n_threads;
	int nth_thread = local_args->nth_thread;

	int i,j;
	int width = (img->width)/n_threads;
	int height = img->height;

	// For every pixel in the image...
	for(j=0;j<height;j++) {

		for(i=(width*nth_thread);i<(width*(nth_thread+1));i++) {

			// Determine the point in x,y space for that pixel.
			double x = xmin + (i%width)*(xmax-xmin)/width;
			double y = ymin + j*(ymax-ymin)/height;

			// Compute the iterations at that point.
			int iters = iterations_at_point(x,y,max);

			// Set the pixel in the bitmap.
			setPixelCOLOR(img,i,j,iteration_to_color(iters,max));
		}
	}
	return NULL;
}


/*
Convert a iteration number to a color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/
int iteration_to_color( int iters, int max )
{
	int color = 0xFFFFFF*(1-pow((iters/(double)max),0.5)); //changed color mapping
	return color;
}


// Show help message
void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates (X-axis). (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=1000)\n");
	printf("-H <pixels> Height of the image in pixels. (default=1000)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}
