#include <stdio.h>

int main(int argc, char *argv[]) {
   int sum=0;
   int i;
   for (i = 0; i < 10; i++) {
      sum = sum + i;
   }
   printf("The sum is: %d\n", sum);
}
