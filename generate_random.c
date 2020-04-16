#include <stdio.h> 
#include <stdlib.h> 
#include <time.h> 
#include <linux/random.h>

void printRandoms(int lower, int upper, int count) 
{ 
    int i; 
    for (i = 0; i < count; i++) { 
        int num = (rand() % (upper - lower + 1)) + lower; 
        printf("%d\n", num); 
    } 
} 
  
int main() 
{ 
    printf("RNDGETENTCNT: 0x%lx\n", RNDGETENTCNT);
    int lower = 0, upper = 10, count = 10;   
    printRandoms(lower, upper, count); 
    return 0; 
} 