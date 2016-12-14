# -*- coding: utf-8 -*-
"""
Created on Mon Nov 16 13:09:19 2015

@author: kevinkunkle

google numpy curve fitting

pick a scoring scheme Si (A-rich regions)
determine lambda
re-scale scores ti = 2*lambda/ln(2)
determine by simulation S0.01 for r seq of length N
verify N -> 2N ~> S0.01 -> S0.01 + 2
verify qi ~= pi e^(lambda*Si)

scipy optimize, bisect
- use for finding single positive root

"""
import random
import matplotlib.pyplot as plt

def genMonSeq(length, probFirst):
    return_string = ""
    my_list = ['A'] * (int)(probFirst[0]*100) + ['C'] * (int)(probFirst[1]*100) + ['G'] * (int)(probFirst[2]*100) + ['T'] * (int)(probFirst[3]*100)
    
    return_string += random.choice(my_list)
    for i in xrange(0,length-1):
        return_string += random.choice(my_list)
    
    return return_string

def score(seq, scoreMatrix):
    score = []    
    for z in xrange(0,len(seq)):
        score.append(0) 
    
    i = 0;
    
    if(seq[i] == 'G'):
        score[i] = scoreMatrix[0]
        if(score[i] < 0):
            score[i] = 0
    elif(seq[i] == 'C'):
        score[i] = scoreMatrix[1]
        if(score[i] < 0):
            score[i] = 0
    elif(seq[i] == 'A'):
        score[i] = scoreMatrix[2]
        if(score[i] < 0):
            score[i] = 0
    elif(seq[i] == 'T'):
        score[i] = scoreMatrix[3]
        if(score[i] < 0):
            score[i] = 0
    
    
    for i in xrange(1, len(seq)):
        if(seq[i] == 'G'):
            score[i] = score[i-1] + scoreMatrix[0]
            if(score[i] < 0):
                score[i] = 0
        elif(seq[i] == 'C'):
            score[i] = score[i-1] + scoreMatrix[1]
            if(score[i] < 0):
                score[i] = 0
        elif(seq[i] == 'A'):
            score[i] = score[i-1] + scoreMatrix[2]
            if(score[i] < 0):
                score[i] = 0
        elif(seq[i] == 'T'):
            score[i] = score[i-1] + scoreMatrix[3]
            if(score[i] < 0):
                score[i] = 0
                
    max_score = max(score);
    l = score.index(max(score))
    k = 0;
    found = False;
    i = 0;
    while(found != True):
        if(score[l-i] == 0):
            k = l - i + 1
            found = True
        i = i + 1;

    return max_score, k, l, seq[k:l+1];
    

# [G, C, A, T]
    
score1Matrix = [-10, -10, 1, -10]

score2Matrix = [-1, -1, 3, -1]

probMatrix = [.25, .25, .25, .25]

hist_scorelist = []

random_length = 10000

num_seq = 10000

seq = genMonSeq(random_length, probMatrix)

print "Random Sequence:"
print seq

m_score, k, l, subseq = score(seq, score1Matrix)

print "Max score:", m_score
print "k: ", k
print "l: ", l
print "Subsequence:", subseq

m_score, k, l, subseq = score(seq, score2Matrix)

print "Max score:", m_score
print "k: ", k
print "l: ", l
print "Subsequence:", subseq


A_count = 0;

for i in xrange(k, l):
    if (seq[i] == 'A'):
        A_count = A_count + 1;

print "# of A's: ", A_count 
print "Length: ", l - k

#for i in xrange(0, num_seq):
#    seq = genMonSeq(random_length, probMatrix)
#    m_score, k, l, subseq = score(seq, scoreMatrix)
#    
#    hist_scorelist.append(m_score)
#
#plt.hist(hist_scorelist, bins = range(0, max(hist_scorelist)+1, 1))
#
#print max(hist_scorelist)


