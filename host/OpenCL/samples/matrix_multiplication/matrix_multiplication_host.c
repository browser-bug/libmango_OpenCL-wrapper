nimum_/* 
 *
 * Test application. This application
 * will launch a kernel in PEAK. The kernel will just
 * compute a matrix multiplication. The size of the matrix 
 * and the matrices pointers will be passed as parameters
 *
 * After executing the kernel, the application shows
 * the resulting output matrix
 *
 * This application demonstrates read & write communication between
 * kernel and host application.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mango.h"

#define KID 1
#define B1 1
#define B2 2
#define B3 3

/* function prototypes */
void init_matrix(int *matrix, int cols, int rows);

/* kernel function, reported here to allow checking the results 
 * obtained in the offloaded version 
 */
void kernel_function(int *A, int *B, int *C, int rows, int cols) {
  for (int r=0;r<rows;r++) {
    for (int c=0;c<cols;c++) {
      int v = 0;
      for (int p=0;p<rows;p++) {
        v = v + A[r * cols + p] * B[p * cols + c];
      }
      C[r * cols + c] = v;
    }
  }
	return;
}


void main(int argc, char**argv) {
  int *A;
  int *B;
  int *C;
  int *D;
  int rows;
  int columns;
	int out=0;

  if (argc < 3) {
    printf("error, bad arguments\n");
    printf("Arguments: <rows> <columns>\n");
    exit(1);
  }

  rows = atoi(argv[1]);
  columns = atoi(argv[2]);

  if ((rows == 0) || (columns == 0)) {
    printf("error, rows and columns should be not zero\n");
		printf("rows = %d; columns = %d\n", rows, columns);
    exit(1);
  }
 
  /* matrix allocation */
  A = malloc(rows*columns*sizeof(int));
  B = malloc(rows*columns*sizeof(int));
  C = malloc(rows*columns*sizeof(int));
  D = malloc(rows*columns*sizeof(int));

  /* input matrices initialization */
  init_matrix(A, rows, columns);
  init_matrix(B, rows, columns);

  /* initialization of the mango context */
  mango_init("matrix_multiplication", "test_manga");

#ifdef GNEMU
  char kernel_file[] = "/opt/mango/usr/local/share/matrix_multiplication/matrix_multiplication_dev";
#else
  char kernel_file[] = "/opt/mango/usr/local/share/matrix_multiplication/memory.data.fpga.datafile";
#endif


  kernelfunction *k = mango_kernelfunction_init();
#ifdef GNEMU
  mango_load_kernel(kernel_file, k, GN, BINARY);
#else
  mango_load_kernel(kernel_file, k, PEAK, BINARY);
#endif
  mango_kernel_t k1 = mango_register_kernel(KID, k, 2, 1, B1, B2, B3);  

  /* Registration of buffers */
  mango_buffer_t b1 = mango_register_memory(B1, rows*columns*sizeof(int), BUFFER, 0, 1, k1);
  mango_buffer_t b2 = mango_register_memory(B2, rows*columns*sizeof(int), BUFFER, 0, 1, k1);
  mango_buffer_t b3 = mango_register_memory(B3, rows*columns*sizeof(int), BUFFER, 1, 0, k1);
  
  /* Registration of task graph */
  mango_task_graph_t *tg = mango_task_graph_create(1, 3, 0, k1, b1, b2, b3);

  /* resource allocation */
  mango_resource_allocation(tg);

  /* Execution preparation */
  mango_arg_t *arg1 = mango_arg( k1, &b1, sizeof(uint64_t), BUFFER );
  mango_arg_t *arg2 = mango_arg( k1, &b2, sizeof(uint64_t), BUFFER );
  mango_arg_t *arg3 = mango_arg( k1, &b3, sizeof(uint64_t), BUFFER );
  mango_arg_t *arg4 = mango_arg( k1, &rows, sizeof(uint32_t), SCALAR );
  mango_arg_t *arg5 = mango_arg( k1, &columns, sizeof(uint32_t), SCALAR );
  mango_event_t e = mango_get_buffer_event(b3);
  mango_arg_t *arg6 = mango_arg( k1, &e, sizeof(uint64_t), EVENT );

  mango_args_t *args=mango_set_args(k1, 6, arg1, arg2, arg3, arg4, arg5, arg6);


  /* Data transfer host->device */
  mango_write(A, b1, DIRECT, 0);
  mango_write(B, b2, DIRECT, 0);

  /* spawn kernel */
  mango_event_t ev = mango_start_kernel(k1, args, NULL);

  /* reading results */
  mango_wait(e);
  mango_read(C, b3, DIRECT, 0);

  /* wait for kernel completion */
  mango_wait(ev);


  /* shut down the mango infrastructure */
	mango_resource_deallocation(tg);
	mango_task_graph_destroy_all(tg);
	mango_release();
	
	/* check results */
	kernel_function(A, B, D, rows, columns);
	
	for(int i=0; i<rows; i++)
		for(int j=0; j<columns; j++)
			if(D[i*columns+j]!=C[i*columns+j]) {
				printf("Incorrect value at %d, %d: %d vs %d\n", i, j, D[i*columns+j], C[i*columns+j]);
				out++;
			}
	
	if (out) {
		printf("Detected %d errors in the computation\n", out);
		exit(out);
	} else {
		printf("Matrix multiplication correctly performed\n");
		exit(0);
	}
}	

void init_matrix(int *matrix, int rows, int cols)
{
  for (int r=0;r<rows;r++) {
    for (int c=0;c<cols;c++) {
      matrix[r*cols+c] = random() % 100;
    }
  }
}
