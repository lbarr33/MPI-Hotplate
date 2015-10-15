#include <stdio.h>
#include <sys/time.h>
#include <mpi.h>
#include<stdlib.h>
#define SIZE 16384

double When()
{
  struct timeval tp;
  gettimeofday(&tp, NULL);
  return ((double) tp.tv_sec + (double) tp.tv_usec * 1e-6);
}

float fabs(float f);

main(int argc, char*argv[])
{
int iproc, nproc, i, iter,done,reallydone,cnt;
char host[255], message[55];
MPI_Status status;

MPI_Init(&argc, &argv);
double starttime = When();
MPI_Comm_size(MPI_COMM_WORLD, &nproc);
MPI_Comm_rank(MPI_COMM_WORLD, &iproc);

int numRow = SIZE/nproc;
int numCol = SIZE;
int row,col = 0;
float *cur[numRow];
float *old[numRow];
float **temp,**currentplate,**oldplate;
currentplate = cur;
oldplate = old;

for(row=0;row<numRow;row++)
{
        cur[row] = (float *)malloc(numCol * sizeof(float));
        old[row] = (float *)malloc(numCol * sizeof(float));
}

float *above;
above = (float *)malloc(numCol * sizeof(float));
float *below;
below =  (float *)malloc(numCol * sizeof(float));

for(row=0; row<numRow;row++)
{
        for(col=0;col<numCol;col++)
        {
                 oldplate[row][col]=50;
                 currentplate[row][col] = 50;
                if(col==0 || col==numCol-1)
                {
                         oldplate[row][col] =0;
                         currentplate[row][col] = 0;
                }
                if(iproc== nproc-1 && row == numRow-1)
                {
                         oldplate[row][col] = 100;
                         currentplate[row][col]= 100;
                }
                if(iproc==0 && row== 0)
                {
                        oldplate[row][col] = 0;
                        currentplate[row][col] = 0;
                }
        }
}

printf("I am proc %d of %d\n",iproc,nproc);

reallydone =0;
float test = 5;
for(cnt=0; !reallydone;cnt++)
{       

        //SENDS AND RECEIVES
        if(iproc!=0)
        {
                //Everyone but 0 send your first row up
                MPI_Send(oldplate[0],numCol,MPI_FLOAT, iproc-1,0,MPI_COMM_WORLD);

        }
        if(iproc!=nproc-1)
        {
                //Everyone but Last receive the row from below
                MPI_Recv(below,numCol,MPI_FLOAT,iproc+1,0,MPI_COMM_WORLD, &status);
                //Everyone but the Last send your bottom row down
                MPI_Send(oldplate[numRow-1],numCol,MPI_FLOAT, iproc+1,0,MPI_COMM_WORLD);

        }
        if(iproc!=0)
        {
                //Everyone but 0 reveive the row from above
                MPI_Recv(above,numCol,MPI_FLOAT,iproc-1,0,MPI_COMM_WORLD, &status);
        }

        //CALCULATIONS
        int start= 0;
        int end = numRow;
        if(iproc==0) start = 1;
        if(iproc==nproc-1) end = numRow-1;
        for(row =start;row<end;row++)
        {
                for(col=1;col<numCol-2;col++)
                {
                        if(row == 0) currentplate[row][col] = (above[col]+oldplate[row+1][col]+oldplate[row][col+1]+oldplate[row][col-1]+(4*oldplate[row][col]))/8;
                        else if(row==numRow-1)
                        {
                         currentplate[row][col] = (oldplate[row-1][col]+below[col]+oldplate[row][col+1]+oldplate[row][col-1]+(4*oldplate[row][col]) )/8;
                        }
                        else currentplate[row][col] = (oldplate[row+1][col]+oldplate[row-1][col]+oldplate[row][col+1]+oldplate[row][col-1]+(4*oldplate[row][col]) )/8;
                }
        }

        //CHECK TO SEE IF WE ARE DONE
        done = 1;
        start = 0;
        end = numRow;
        if(iproc==0) start = 1;
        if(iproc==nproc-1) end = numRow-1;

        for(row=start;row<numRow-2;row++)
        {
                if(done==0) break;
                for(col=1;col<numCol-2;col++)
                {
                        if(row==0)
                        {
                                if(fabs(currentplate[row][col]-((currentplate[row+1][col]+above[col]+currentplate[row][col+1]+currentplate[row][col-1] )/4))> .1)
                                {
                                done=0;
                                break;
                                }

                        }
                        else if(row== numRow-1)
                        {
                                 if(fabs(currentplate[row][col]-((above[col]+currentplate[row-1][col]+currentplate[row][col+1]+currentplate[row][col-1] )/4))> .1)
                                {
                                done=0;
                                break;
                                }

                        }
                        else if(fabs(currentplate[row][col]-((currentplate[row+1][col]+currentplate[row-1][col]+currentplate[row][col+1]+currentplate[row][col-1] )/4))> .1)
                        {       
                                float dif = (currentplate[row][col]-((currentplate[row+1][col]+currentplate[row-1][col]+currentplate[row][col+1]+currentplate[row][col-1] ))/4);
                                done=0;
                                break;
                        }
                }
        }

        MPI_Allreduce(&done, &reallydone, 1, MPI_INT,MPI_MIN,MPI_COMM_WORLD);
        temp = currentplate;
        currentplate = oldplate;
        oldplate = temp;

}

printf("%d: It took %d iterations and %lf seconds to relax the system\n",iproc,cnt,When()-starttime);
MPI_Finalize();

}

float fabs(float f)
{
        return (f>0.0) ? f : -f;
}

