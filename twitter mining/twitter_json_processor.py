# -*- coding: utf-8 -*-
"""
Created on Mon Apr 25 11:52:18 2016

@author: kevinkunkle
"""

import json
import fileinput
import sys
import xlwt

# number of inputted files
num_files = 1
# number of json objects per file
num_dates = 30
# number of items per json object
num_items = 6

item_list = ['DateString',
		'OpenSharePrice',
		'HighSharePrice',
		'LowSharePrice',
		'CloseSharePrice',
		'TradeVolume']
  
input_string = ""

for line in fileinput.input('#donaldtrump/apr16_#donaldtrump.json.txt'):
    if not (line[:5] == 'Since' or line[:5] == 'Until' or line[:5] == 'Downl' or line[:5] == 'No mo'): 
        input_string += line

input_string = input_string.replace("u\'", '"')
input_string = input_string.replace("\'", '"')

print input_string[:100]

parsed_json = json.loads(input_string)

#print parsed_json

test_list = [[] for i in range(10)]

test_dict = {'heyoo':23}

test_list[0] = test_dict

print test_list