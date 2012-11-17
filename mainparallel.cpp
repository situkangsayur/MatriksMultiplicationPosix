#include <omp.h>
#include <sys/time.h>
#include <time.h>
#include <cstdio>
#include <stdlib.h>
#include <pthread.h>
#define CHUNKSIZE 10
#define N     1600
int num_thrd;

float *a, *b, *c, *rc;

void initialize(float *a, float *b, int n) {
	int i, j;

	for (i = 0; i < n; ++i) {
		for (j = 0; j < n; ++j) {
			a[i * n + j] = (float) i + j;
			b[i * n + j] = (float) i * j;
			//printf("inisialisasi matrik a  %f \n", a[i * n + j]);
		}
	}

}

void* multiply(void* slice) {
	int i, j, k;
	int s = (int) slice;   // retrive the slice info
	int from = (s * N) / num_thrd; // note that this 'slicing' works fine
	int to = ((s + 1) * N) / num_thrd; // even if SIZE is not divisible by num_thrd

	printf("computing slice %d (from row %d to %d)\n", s, from, to - 1);
	for (i = from; i < to; ++i) {

		for (j = 0; j < N; ++j) {
			float sum = 0;
			for (k = 0; k < N; ++k) {
				sum += a[i * N + k] * b[k * N + j];

			}
			c[i * N + j] = sum;
			//printf(" %f \n", c[i * N + j]);
		}

		//printf(" i  = %d,j= %d,k= %d \n",c[i * n + 1]);
	}
	printf("finished slice %d\n", s);

}

int main() {

	float time_serial;
	clock_t time1, time2;
	time_t start, end;

	pthread_t* thread;
	num_thrd = 4;
	int chunk;
	thread = (pthread_t*) malloc(num_thrd * sizeof(pthread_t));
	a = (float*) malloc(N * N * sizeof(float));
	b = (float*) malloc(N * N * sizeof(float));
	c = (float*) malloc(N * N * sizeof(float));
	rc = (float*) malloc(N * N * sizeof(float));

	initialize(a, b, N);

	//gettimeofday====================================================
	struct timeval tim;
	gettimeofday(&tim, NULL);
	double t1 = tim.tv_sec + (tim.tv_usec / 1000000.0);
	//timespec, clock_gettime();
	timespec startTime, currentTime;
	long elapsed;
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &startTime);
	//time=============================================================
	time1 = clock();
	start = time(NULL);
	int i;
	for (i = 1; i < num_thrd; i++) {
		// creates each thread working on its own slice of i
		//printf("Check1 %d",i);
		if (pthread_create(&thread[i], NULL, multiply, (void*) i) != 0) {
			perror("Can't create thread");
			free(thread);
			exit(-1);
		}
	}
	multiply(0);
	// main thead waiting for other thread to complete
	for (i = 1; i < num_thrd; i++)
		pthread_join(thread[i], NULL);
	//}/* end of parallel section */
	time2 = clock();
	end = time(NULL);
	time_serial = (float) (time2 - time1) / CLOCKS_PER_SEC;
	int rest1, rest2;
	//printf()
	rest1 = end / 3600;
	rest2 = start / 3600;
	rest1 = end - start;
	//gettimeofday
	gettimeofday(&tim, NULL);
	double t2 = tim.tv_sec + (tim.tv_usec / 1000000.0);
	//=========================================================
	clock_gettime(CLOCK_REALTIME, &currentTime);
	elapsed = currentTime.tv_nsec - startTime.tv_nsec;

	//==========================================================
	printf("result time in clock() : %f second \n", time_serial);
	printf("result time in time()  : %f second \n", rest1);
	printf("result time in gettimeofday() %.6lf seconds \n", t2 - t1);
	printf("result time in clock_gettime() : %f second \n", elapsed / 1000000);

}
