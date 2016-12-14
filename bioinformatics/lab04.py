# -*- coding: utf-8 -*-
"""
Created on Mon Oct 12 13:49:40 2015

@author: kevinkunkle Lab 4
"""

#scoreAlignments(seqX, seqY, match, mismatch_penalty, gap_penalty):


def alignmentNumber(seqX, seqY):
    scoringMatrix = int [][]
    for i in xrange(0,len(seqX)):
        scoringMatrix[0][i] = 1;
        scoringMatrix[i][0] = 1;
    
    return scoringMatrix


seqX = 'CCGTGTAGCT'
seqY = 'CGATAAGTA'

score = alignmentNumber(seqX, seqY)

print score