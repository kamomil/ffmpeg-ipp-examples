#include <stdio.h>
#include <stdlib.h>
#include <math.h>
double ncc(const unsigned char* im1 , const unsigned char* im2,int sz){

  double* t1 = (double*)malloc(sizeof(double)*sz);
  double* t2 = (double*)malloc(sizeof(double)*sz);
  double mean1 = 0;
  double mean2 = 0;
  for(int i=0;i<sz;i++){
    //double X=(((double)rand()/(double)RAND_MAX)/100)-0.005;
    t1[i] = im1[i]*1.0;//+X;
    mean1 += t1[i];
    t2[i] = im2[i]*1.0;
    mean2 += t2[i];
    //Output("mean1  = %f, mean2 = %f\n",mean1,mean2);
  }
  mean1 = mean1 / (1.0*sz);
  mean2 = mean2 / (1.0*sz);

  //Output("mean1  = %f, mean2 = %f\n",mean1,mean2);
  //Output("ncc: sz = %d\n",sz);

  double norm1 = 0;
  double norm2 = 0;
  double n = 0;
  for(int i=0;i<sz;i++){    
    t1[i] -= mean1;
    norm1 += t1[i]*t1[i];
    t2[i] -= mean2;
    norm2 += t2[i]*t2[i];
    n += t1[i]*t2[i];
    //Output("n  = %f, norm1 = %f norm2 = %f\n",n,norm1,norm2);
  }
  //Output("n  = %f, norm1 = %f norm2 = %f\n",n,norm1,norm2);

  if(norm1<0.01 || norm2<0.01){
    printf("nnc: WARNING: norms might be too small: norm1=%f norm2=%f\n",norm1,norm2);
  }
  
  free(t1);
  free(t2);
  return n/(sqrt(norm1*norm2));
  
}

//double ncc2(const unsigned char* im1 ,int im1_linesz, const unsigned char* im2,int im2_linesz, int w,int h);

//double ncc(const unsigned char* im1 , const unsigned char* im2,int sz){

int main(int argc, char* argv[]){

  if(argc != 3){
    printf("Usage: %s <file1> <file2>\n",argv[0]);
    return -1;
  }

  //int sz = atoi(argv[3]);
  int buff_sz = 1000000;
  
  unsigned char* m1 = (unsigned char*)malloc(buff_sz);
  unsigned char* m2 = (unsigned char*)malloc(buff_sz);
  //unsigned char* buff = (unsigned char*)malloc(buff_sz);

  if(!m1 || !m2){
    printf("could not allocate buffers\n");
  }
  
  FILE* f1 = fopen(argv[1],"rb");
  if(!f1){
    perror("could not open first file\n");
    return -1;
  }

  FILE* f2 = fopen(argv[2],"rb");
  if(!f2){
    perror("could not open first file\n");
    return -1;
  }
  
  
  
  int i = 0;
  while (!feof(f1) && !feof(f2)) {
    int n1 = fread(m1, 1, buff_sz, f1);
    int n2 = fread(m2, 1, buff_sz, f2);

    if(ferror(f1)){
      perror("failed to read from first file\n");
      return -1;
    }

    if(ferror(f2)){
      perror("failed to read from second file\n");
      return -1;
    }
    if(n1 != n2){
      printf("files are not the same size\n");
      return -1;
    }
    
    double n = ncc(m1,m2,n1);
    printf("%d: the ncc of the files is %f\n",i,n);
    i++;
  }
  

  //printf("%d: the ncc of the files is %f\n",i,n);
  fclose(f1);
  fclose(f2);
  
  return 0;
}
