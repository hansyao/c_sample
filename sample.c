#include <stdio.h>
int a=-1;
float i=1.0,b,sum=0;
int main(){
while (i<=100){
  a=-a;
  b=(1/i)*a;
  sum=sum+b;
  i=i+1;
printf("count %f, sum is %f\n",i,sum);
}
//printf("total is %f\n", sum);
return 0;
}
