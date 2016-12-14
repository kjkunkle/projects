# -*- coding: utf-8 -*-
"""
Kevin Kunkle
CSCI B490 Lab 01
"""
import numpy
import matplotlib.pyplot as plt

def parse_fasta(data):
    """
    Read DNA, RNA, or protein sequences in Fasta format.

    This generator function yields a tuple containing a defline and a sequence
    for each record in the Fasta data. Stolen shamelessly from
    http://stackoverflow.com/a/7655072/459780.
    """
    name, seq = None, []
    for line in data:
        line = line.rstrip()
        if line.startswith('>'):
            if name:
                yield (name, ''.join(seq))
            name, seq = line, []
        else:
            seq.append(line)
    if name:
        yield (name, ''.join(seq))


fh = open('Escherichia_coli_str_k_12_substr_mg1655_gca_000005845_2.GCA_000005845.2.27.chromosome.Chromosome.gff3', 'r')


lengthsX = [];
GCcountsY = [];


lengthsChr = [];


startsChr = [];


endsChr = [];

for line in fh:
    if "\tgene\t" in line:
        values = line.split('\t')
        start = int(values[3])
        end = int(values[4])
        length = end - start + 1
        lengthsX.append(length)
    
        if(values[0] == 'Chromosome'):
            startsChr.append(start)
            endsChr.append(end)
            lengthsChr.append(length)

    
for defline, sequence in parse_fasta(open('Escherichia_coli_str_k_12_substr_mg1655_gca_000005845_2.GCA_000005845.2.27.dna.toplevel.fa', 'r')):
    if(defline[0:6] == '>Chrom'):
        for i in xrange(0, len(startsChr)):
            targetSeq = sequence[startsChr[i]:endsChr[i]+1]
            Gcount = targetSeq.count('G')
            Ccount = targetSeq.count('C')
            GCcount = (Gcount + Ccount)/float(lengthsChr[i])
            GCcountsY.append(GCcount)

x = lengthsX
y = GCcountsY
plt.scatter(x, y)
plt.xlabel('Length')
plt.ylabel('GC count')
plt.title('Length and GC Count Relation')









