#include <stdio.h>

int main(int argc, char *argv[]) {
    printf("Hello, world!\n");

    int a = 32;
    printf("a: %d\n", a);

    int* p1;
    p1 = &a;
    printf("p1 (i.e. the address of a): %p\n", p1);

    printf("Data pointed to by p1: %d\n", *p1);

    int* p2 = p1;

    *p2 += 8;
    printf("p1 points to: %d\np2 points to: %d\n", *p1, *p2);

    //free(p1);
    p1 = NULL;
    p2 = NULL;
    //printf("%d", *p2);
    return 0;
}
