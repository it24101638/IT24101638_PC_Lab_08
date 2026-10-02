#include <cstdio>
#include <cstdlib>
#include <mpi.h>
int main(void)
{
    int rank;
    MPI_Status status;
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    char name[30];
    int len;
    MPI_Get_processor_name( name, &len );
    int x[10], y[10];
    if (rank == 1) {
      for (int r =0;r <10; r++)
         x[r] = 10*r;

      // 1. Allocate a user buffer: data size + MPI_BSEND_OVERHEAD
      int bufsize = 10 * sizeof(int) + MPI_BSEND_OVERHEAD;
      char *buffer = (char *) malloc(bufsize);

      // 2. Attach it to MPI
      MPI_Buffer_attach(buffer, bufsize);

      printf("BSending message to computer 3 from computer 1\n");
      // 3. Buffered send: copies x into the buffer and returns immediately
      MPI_Bsend(x, 10, MPI_INT, 3, 0, MPI_COMM_WORLD);

      // 4. Detach: blocks until buffered data has been delivered, then frees it up
      MPI_Buffer_detach(&buffer, &bufsize);
      free(buffer);
    }
    else if (rank == 3) {
      MPI_Recv(y, 10, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
      printf("in computer 3 the value of y is printed\n");
      for (int r=0;r<10;r++)
         printf(" %d ",y[r]);
      printf("\n");
    }
    else
      printf("Just a normal process From rank %d machine %s\n", rank, name);
    MPI_Finalize();
}