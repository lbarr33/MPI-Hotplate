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
//printf("start of program\n");
int iproc, nproc, i, iter,done,reallydone,cnt;
char host[255], message[55];
MPI_Status status;

MPI_Init(&argc, &argv);
double starttime = When();
MPI_Comm_size(MPI_COMM_WORLD, &nproc);
MPI_Comm_rank(MPI_COMM_WORLD, &iproc);

//printf("First I am proc %d of %d\n",iproc,nproc);


int numRow = SIZE/nproc;
int numCol = SIZE;
//printf("Rows is : %d Col is: %d\n",numRow,numCol);
int row,col = 0;
float *cur[numRow];
float *old[numRow];
float **temp,**currentplate,**oldplate;
currentplate = cur;
oldplate = old;

//printf("here1\n");
for(row=0;row<numRow;row++)
{
        cur[row] = (float *)malloc(numCol * sizeof(float));
        old[row] = (float *)malloc(numCol * sizeof(float));
}

//printf("here2\n");

float *above;
above = (float *)malloc(numCol * sizeof(float));
float *below;
below =  (float *)malloc(numCol * sizeof(float));

//printf("Here3\n");

for(row=0; row<numRow;row++)
{
        //printf("on row %d",row);

        for(col=0;col<numCol;col++)
        {
                 oldplate[row][col]=50;
                //printf("THE VALUE OF THE PLATE IS: %f\n",oldplate[row][col]);
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


//printf("here4\n");


printf("I am proc %d of %d\n",iproc,nproc);

reallydone =0;
float test = 5;
for(cnt=0; !reallydone;cnt++)
{       //printf("THE VALUE OF THE PLATE IS: %f\n",oldplate[3][15]);

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
                //Everyone but ) reveive the row from above
                MPI_Recv(above,numCol,MPI_FLOAT,iproc-1,0,MPI_COMM_WORLD, &status);
        }

        //printf("done with the sends for proc :%d\n",iproc);
        /*if(iproc!=0 && iproc!=nproc-1){
        for(row=0;row<numCol;row++)
        {
                printf("%d: Below at %d is: %fl\n",iproc,row,below[row]);
                printf("%d: Above at %d is: %fl\n",iproc,row,above[row]);

        }
        }*/
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
  //printf("BELOW IS: %fl\n",below[col]);
                         currentplate[row][col] = (oldplate[row-1][col]+below[col]+oldplate[row][col+1]+oldplate[row][col-1]+(4*oldplate[row][col]) )/8;
                        }
                        else currentplate[row][col] = (oldplate[row+1][col]+oldplate[row-1][col]+oldplate[row][col+1]+oldplate[row][col-1]+(4*oldplate[row][col]) )/8;


                }
        }

        //printf("done with the calculations\n");
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
                                //printf("1check failed on proc: %d on cnt: %d\n",iproc,cnt);
                                done=0;
                                break;
                                }

                        }
                        else if(row== numRow-1)
                        {
                                 if(fabs(currentplate[row][col]-((above[col]+currentplate[row-1][col]+currentplate[row][col+1]+currentplate[row][col-1] )/4))> .1)
                                {
                                //printf("2check failed on proc: %d on cnt: %d\n",iproc,cnt);

                                done=0;
                                break;
                                }

                        }
                        else if(fabs(currentplate[row][col]-((currentplate[row+1][col]+currentplate[row-1][col]+currentplate[row][col+1]+currentplate[row][col-1] )/4))> .1)
                        {       //printf("3check failed on proc: %d on cnt: %d\n",iproc,cnt);
                                float dif = (currentplate[row][col]-((currentplate[row+1][col]+currentplate[row-1][col]+currentplate[row][col+1]+currentplate[row][col-1] ))/4);
                                //printf("Difference was %fl\n",dif);
                                done=0;
                                break;
                        }
                }
        }
        //printf("done with the checking\n");

        MPI_Allreduce(&done, &reallydone, 1, MPI_INT,MPI_MIN,MPI_COMM_WORLD);
        //if(iproc==0) printf("THE NUMBER IS %lf\n",oldplate[1][1]);
        //if(iproc==0) printf("3THE NUMBER IS %lf\n",currentplate[1][1]);

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

