# -*- coding: utf-8 -*-
"""
Created on Wed Sep 30 13:08:21 2015

@author: kevinkunkle
"""

import random
import numpy as np

def calcWaitTime(target_seq, probH, probT):
    return_string = "" 
    my_list = ['H'] * probH + ['T'] * probT
    
    for i in xrange(0,len(target_seq)-1):    
        return_string += random.choice(my_list)
    
    found = False
    i = 0
    while(found == False):
        return_string += random.choice(my_list)
        if (return_string[i:i+4] == target_seq):
            found = True
        i += 1
    
    return return_string

def calcBeforeOccur(target_seq, probH, probT):
    return_string = "" 
    my_list = ['H'] * probH + ['T'] * probT
    
    for i in xrange(0,len(target_seq)-1):    
        return_string += random.choice(my_list)
    
    found = False
    i = 0
    while(found == False):
        return_string += random.choice(my_list)
        if (return_string[i:i+4] == target_seq):
            found = True
        i += 1
    
    output = 1
    for i in xrange(0,len(return_string)-3):
        if (return_string[i:i+4] == 'HTHH'):
            output = 0 
         
    return output
        

seq_length = 3
target_seq = 'HTH'
probH = 1
probT = 1

wait_count_list = []

for i in xrange(0,1000):
    wait_count_list.append(len(calcWaitTime(target_seq, probH, probT)))

print np.mean(wait_count_list)

#for i in xrange(0,10000):
#    wait_count_list.append(calcBeforeOccur(target_seq, probH, probT))

print np.mean(wait_count_list)




