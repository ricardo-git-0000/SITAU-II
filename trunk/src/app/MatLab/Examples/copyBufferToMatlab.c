// Minimal code to copy the data from the buffer pointer int16 to a matlab matrix
// Compile with this matlab line:
// >> mex copyBufferToMatlab.c

#include "mex.h"

void mexFunction( int nlhs, mxArray *plhs[],int nrhs, const mxArray *prhs[] )
{   
  unsigned long long *tmp; // pointers are 64bits use long long
  short int *buffer, *BF;
  int i,ndat;
  double *tmp2; 
 
  
  // get the adress in the first argument
  
  tmp=(unsigned long long*)mxGetPr(prhs[0]);  //tmp points to the matalb input
  buffer=(int *)tmp[0];         //the firs element of tmp is the buffer pointer
  
  tmp2=(double*)mxGetPr(prhs[1]);
  ndat=(int)(tmp2[0]);
  
  // printf("*recuperate adress %p ndat %d\n", buffer,ndat);           
  
   plhs[0] = mxCreateNumericMatrix(ndat,1, mxINT16_CLASS, mxREAL);
   BF = (short int *)mxGetPr(plhs[0]);
             
   for(i=0; i<ndat; i++)
       BF[i]=buffer[i];
}


