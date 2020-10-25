#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int numberOfMeasurements = atoi(argv[1]);
    // This many first measurements will be ignored
    int warmupMeasurements = atoi(argv[2]);
    char *FlushFileName = argv[3];
    char *NoFlushFileName = argv[4];

    FILE *fptr;
    int totalMeasurements = numberOfMeasurements + warmupMeasurements;
    unsigned int measurements[totalMeasurements];
    int i = 0;

    // This is page size (for me 2048 also works)
    // An array large enough is required so it doesn't get cached
    int arrayRows = 256;
    int arrayColumns = 4096;
    char *test = (char*)malloc(arrayRows * arrayColumns);

    // Measure WITH flush "totalMeasurements" times and save to measured cycles array
    for(i = 0; i < totalMeasurements; i++)
    {
        /*
            Function flushReload taken from FLUSH+REALOAD paper:

            Yarom, Y., & Falkner, K. (2014). FLUSH+ RELOAD:
            a high resolution, low noise, L3 cache side-channel attack.
            In 23rd {USENIX} Security Symposium ({USENIX} Security 14) (pp. 719-732).
        */
        asm volatile (
            "mfence             \n"
            "lfence             \n"
            "rdtsc              \n"
            "lfence             \n"
            "movl %%eax, %%esi  \n"
            "movl (%1), %%eax   \n"
            "lfence             \n"
            "rdtsc              \n"
            "subl %%esi, %%eax  \n"
            "clflush 0(%1)      \n"
            : "=a" (measurements[i])
            // Flush and Reload all the time at the same index because
            // we are just measuring speed and not extracting any secret
            : "c" (&test[0])
            :  "%esi", "%edx");
    }
    fptr = fopen(FlushFileName, "w+");

    // Ignore first "warmupMeasurements" measurements and write the rest to file
    for(i = warmupMeasurements; i < totalMeasurements; i++)
    {
        if (i == totalMeasurements - 1)
        {
            fprintf(fptr,"%d", measurements[i]);
        }
        else
        {
            fprintf(fptr,"%d,", measurements[i]);
        }
    }
    fclose(fptr);

    // Measure WITHOUT flush "totalMeasurements" times and save measured cycles to array
    for(i = 0; i < totalMeasurements; i++)
    {
        asm volatile (
            "mfence             \n"
            "lfence             \n"
            "rdtsc              \n"
            "lfence             \n"
            "movl %%eax, %%esi  \n"
            "movl (%1), %%eax   \n"
            "lfence             \n"
            "rdtsc              \n"
            "subl %%esi, %%eax  \n"
            : "=a" (measurements[i])
            // Reload all the time at the same index because
            // we are just measuring speed and not extracting any secret
            : "c" (&test[0])
            :  "%esi", "%edx");
    }

    fptr = fopen(NoFlushFileName, "w+");
    // Ignore first "warmupMeasurements" measurements and write the rest to file
    for(i = warmupMeasurements; i < totalMeasurements; i++)
    {
        if (i == totalMeasurements - 1)
        {
            fprintf(fptr,"%d", measurements[i]);
        }
        else
        {
            fprintf(fptr,"%d,", measurements[i]);
        }
    }

    fclose(fptr);

    return 0;
}