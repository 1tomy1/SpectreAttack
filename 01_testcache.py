# This is just a script I copied from my master's thesis.
# It reads bunch of numbers seperated by commas from a file,
# cuts off some of the largest ones (because of noise),
# and spits out histograms with distribution curve and some extra
# cool looking numbers (average - mu and standard deviation - sigma).
import numpy
import matplotlib.pyplot as plt

from scipy.stats import norm
from subprocess import Popen

# Compile and run our program
process = Popen(["gcc", "-o", "01_cachedemo", "01_cachedemo.c"])
process.wait()
process = Popen(["./01_cachedemo", "10000", "10000", "01_flush.txt", "01_noflush.txt"])
process.wait()


# Generate histograms for measurements of NOT cached values
s = ""
with open('01_flush.txt') as f:
    s = f.read()
s = numpy.fromstring(s, dtype=int, sep=',')
srt_row = numpy.array(sorted(s, key=int, reverse = False)).astype(numpy.int)
# Sort and cut away 5% of the biggest measurements
cutSortedRow = srt_row[:int(len(srt_row)*0.95)]
(mu, sigma) = norm.fit(cutSortedRow)
n, bins, patches = plt.hist(cutSortedRow, weights=numpy.zeros_like(cutSortedRow) + 1. / cutSortedRow.size)
y = norm.pdf( bins, mu, sigma)
plt.plot(bins, y, 'r--', linewidth=2)
plt.xlabel('Cycles')
plt.ylabel('Frequency')
plt.title(f"""Main memory access speed - \u03BC: {round(mu, 2)}, \u03C3: {round(sigma, 2)}""")
plt.grid(True)
plt.locator_params(axis='x', nbins=5)
plt.tight_layout()
plt.savefig('01_flush.png')
plt.close()


# Generate histograms for measurements of cached values
s = ""
with open('01_noflush.txt') as f:
    s = f.read()
s = numpy.fromstring(s, dtype=int, sep=',')
srt_row = numpy.array(sorted(s, key=int, reverse = False)).astype(numpy.int)
# Here we don't need to cut away measurements because usually
# there is no noise for measuring the speed of cached values
cutSortedRow = srt_row[:int(len(srt_row)*1)]
(mu, sigma) = norm.fit(cutSortedRow)
n, bins, patches = plt.hist(cutSortedRow, weights=numpy.zeros_like(cutSortedRow) + 1. / cutSortedRow.size)
y = norm.pdf( bins, mu, sigma)
plt.plot(bins, y, 'r--', linewidth=2)
plt.xlabel('Cycles')
plt.ylabel('Frequency')
plt.title(f"""CPU cache access speed - \u03BC: {round(mu, 2)}, \u03C3: {round(sigma, 2)}""")
plt.grid(True)
plt.locator_params(axis='x', nbins=5)
plt.tight_layout()
plt.savefig('01_noflush.png')
plt.close()