# -*- coding: utf-8 -*-
"""
Created on Tue Sep 22 14:57:46 2015

@author: kevinkunkle
"""

import random
import matplotlib.pyplot as plt
from collections import Counter

def randomReads(nucleo_string, n, R):
    return_list = []
    for i in xrange(0,n):
        x = random.randint(0, len(nucleo_string)-R)
        return_list.append(nucleo_string[x:x+R])
    return return_list

def countKmers(read_list, k):
    all_reads = []
    for x in read_list:
        for i in xrange(0, len(x)-k+1):
            all_reads.append(x[i:k+i])
    
    return Counter(all_reads)

R = 100     #read_length
k = 7        #kmer size

input_list = []
fo = open("mySequence", "rwb")

for x in fo:
    input_list.append(x)

n = len(input_list)

kmer_counts = countKmers(input_list, k).values()

plt.hist(kmer_counts, bins = range(0, max(kmer_counts)+1, 1))

data = Counter(kmer_counts)
mean_kmer_coverage = data.most_common(1)[0][0]

coverage_depth = mean_kmer_coverage/((R-k+1)/float(R))

G_estimate = n*R/coverage_depth

print G_estimate