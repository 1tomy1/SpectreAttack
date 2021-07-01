# Cache Access Speed
`01_testcache.py`: In this experiment we demonstrate the access speed difference between CPU's L1 cache and main memory. Retrieving a value from CPU's cache takes on average around 26 CPU clock cycles, whereas reading from main memory around 170 CPU clock cycles. These values vary between different CPUs.

# CPU Cache Side Channel Attack
`01_cachedemo.c`: This program demonstrates how we can leverage different data retrieval times - retrieving data from cache or RAM - and use the information to leak secrets from RAM.

# Spectre Attack
`spectre.c`: Implementation of [Spectre](https://meltdownattack.com/) attack. This experiment shows, that the CPU's speculative execution of code can be exploited to leak secrets from RAM.
