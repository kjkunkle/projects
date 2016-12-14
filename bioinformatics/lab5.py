# -*- coding: utf-8 -*-
"""
Created on Mon Nov  2 13:54:13 2015

@author: kevinkunkle
"""

#may need to use logarithms
#test answer should be 11.8%

def prob_observed(prev_E, prev_M, startProbs, transProbs, HiddenProbs, observed):    
    next_E = 0;
    next_M = 0;
    
    if(observed != ""):
        if(observed[0] == "H"):
            next_E = (prev_E*transProbs[0][0] + prev_M*transProbs[1][0])*HiddenProbs[0][0];
            next_M = (prev_E*transProbs[0][1] + prev_M*transProbs[1][1])*HiddenProbs[1][0];
        else:
            next_E = (prev_E*transProbs[0][0] + prev_M*transProbs[1][0])*HiddenProbs[0][1];
            next_M = (prev_E*transProbs[0][1] + prev_M*transProbs[1][1])*HiddenProbs[1][1];
        
#        return prev_E + prev_M + prob_observed(next_E, next_M, startProbs, transProbs, HiddenProbs, observed[1:]);
        return prob_observed(next_E, next_M, startProbs, transProbs, HiddenProbs, observed[1:]);

    else:
        
        return prev_E + prev_M;

def viterbi(prev_E, prev_M, startProbs, transProbs, HiddenProbs, observed):
    next_E = 0;
    next_M = 0;
    
    if(observed != ""):
        if(observed[0] == "H"):
            next_E = max(prev_E*transProbs[0][0], prev_M*transProbs[1][0])*HiddenProbs[0][0];
            next_M = max(prev_E*transProbs[0][1], prev_M*transProbs[1][1])*HiddenProbs[1][0];
        else:
            next_E = max(prev_E*transProbs[0][0], prev_M*transProbs[1][0])*HiddenProbs[0][1];
            next_M = max(prev_E*transProbs[0][1], prev_M*transProbs[1][1])*HiddenProbs[1][1];
        
#        return prev_E + prev_M + prob_observed(next_E, next_M, startProbs, transProbs, HiddenProbs, observed[1:]);
        return viterbi(next_E, next_M, startProbs, transProbs, HiddenProbs, observed[1:]);

    else:
        
        return max(prev_E, prev_M);

    

startProbE = .4;
startProbM = .6;

startProbs = [startProbE, startProbM]

transProbEE = .8;
transProbEM = .2;
transProbME = .6;
transProbMM = .4;

transProbs = [[transProbEE, transProbEM],
             [transProbME, transProbMM]];
#print transProbs[0][1]


HgivenE = .5;
TgivenE = .5;
HgivenM = .7;
TgivenM = .3;

HiddenProbs = [[HgivenE, TgivenE],
               [HgivenM, TgivenM]];

observed = "HHHTTTTHHH"

e0 = (startProbE*HgivenE);
m0 = (startProbM*HgivenM);

#observed
print prob_observed(e0, m0, startProbs, transProbs, HiddenProbs, observed[1:]);

#viterbi
print viterbi(e0, m0, startProbs, transProbs, HiddenProbs, observed[1:]);

#prob of Q given O
print viterbi(e0, m0, startProbs, transProbs, HiddenProbs, observed[1:])/prob_observed(e0, m0, startProbs, transProbs, HiddenProbs, observed[1:]);


#e0 = (startProbE*HgivenE);
#m0 = (startProbM*HgivenM);
#
#e1 = (e0*transProbEE + m0*transProbME)*HgivenE;
#m1 = (e0*transProbEM + m0*transProbMM)*HgivenM;
#
#e2 = (e1*transProbEE + m1*transProbME)*HgivenE;
#m2 = (e1*transProbEM + m1*transProbMM)*HgivenM;

#e1 = (e0*transProbEE)*HgivenE;
#m1 = (e0*transProbEM + m0*transProbMM)*HgivenM;
#
#e2 = (e1*transProbEE + m1*transProbME)*HgivenE;
#m2 = (e1*transProbEM)*HgivenM;