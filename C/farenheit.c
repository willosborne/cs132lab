#include <stdio.h>

int main(int argc, char* argv[]) {
    float celcius;
    printf("Enter the temperature in Celcius: ");
    scanf("%f", &celcius);
    printf("\nEntered value: %f", celcius);
    float farenheit = celcius * (9/5) + 32;
    printf("\nValue in farenheit: %f\n", farenheit);

    return 0;
}
