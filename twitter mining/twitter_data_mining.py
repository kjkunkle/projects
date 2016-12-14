# -*- coding: utf-8 -*-
"""
Created on Sat Mar  5 17:28:16 2016

@author: kevinkunkle
"""

import sys
import jsonpickle
import json
import os
import tweepy
import xlwt
from collections import Counter
from nltk.tokenize import RegexpTokenizer

ckey = "5Dge3i3FVPNmCB4xNKEVjFv7r";
csecret = "6fZIXbEQsZglhAVESMcg96pxKAFn1NT8uOeMvd8vqDnKcaINr9";
atoken = "35451374-f4miynm2eydQVEK0nP8k7K4cW4iCVvkdG1YgiNMpH";
asecret = "iYsyQTO0IJKKEN6J8xCeHw0JNtjAR7PEEM9wBcDByIWrb";


# Replace the API_KEY and API_SECRET with your application's key and secret.
auth = tweepy.OAuthHandler(ckey, csecret)
auth.set_access_token(atoken, asecret)

api = tweepy.API(auth)
 
api = tweepy.API(auth, wait_on_rate_limit=True,
				   wait_on_rate_limit_notify=True)
 
if (not api):
    print ("Can't Authenticate")
    sys.exit(-1)


############################## INPUT GOES BELOW ###############################

searchQuery = str(sys.argv[1])  # this is what we're searching for
maxTweets = 10000 # the max number of tweets you want to pull
tweetsPerQry = 100  # this is the max the API permits
fName = 'tweets.txt' # We'll store the tweets in a text file.
excel_file =  str(sys.argv[1]) + '/apr' + sys.argv[2] + sys.argv[1] + '.xls'
since="2016-04-" + sys.argv[2]
until="2016-04-" + str(int(sys.argv[2])+1)

###############################################################################


# If results from a specific ID onwards are reqd, set since_id to that ID.
# else default to no lower limit, go as far back as API allows
sinceId = None

# If results only below a specific ID are, set max_id to that ID.
# else default to no upper limit, start from the most recent tweet matching the search query.
max_id = -1L

tweetCount = 0
print("Downloading max {0} tweets".format(maxTweets))
print 'Since: ' + since
print 'Until: ' + until

search_results = [[] for i in range(maxTweets+100)];
i = 0
with open(fName, 'w') as f:
    while tweetCount < maxTweets:
        try:
            if (max_id <= 0):
                if (not sinceId):
                    new_tweets = api.search(q=searchQuery, count=tweetsPerQry, since=since, until=until, lang="en")
                else:
                    new_tweets = api.search(q=searchQuery, count=tweetsPerQry, since=since, until=until,
                                            since_id=sinceId, lang="en")
            else:
                if (not sinceId):
                    new_tweets = api.search(q=searchQuery, count=tweetsPerQry, since=since, until=until,
                                            max_id=str(max_id - 1), lang="en")
                else:
                    new_tweets = api.search(q=searchQuery, count=tweetsPerQry, since=since, until=until,
                                            max_id=str(max_id - 1),
                                            since_id=sinceId, lang="en")
            if not new_tweets:
                print("No more tweets found")
                break
            for tweet in new_tweets:
                #f.write(jsonpickle.encode(tweet._json, unpicklable=False) +
                search_results[i] = tweet._json
                i += 1
            tweetCount += len(new_tweets)
            print("Downloaded {0} tweets".format(tweetCount))
            max_id = new_tweets[-1].id
        except tweepy.TweepError as e:
            # Just exit if any error
            print("some error : " + str(e))
            print("sinceID = ", sinceId)
            print("maxID = ", max_id)
            break

#print ("Downloaded {0} tweets, Saved to {1}".format(tweetCount, excel_file))
print search_results

"""
for i in range(0, tweetCount):
    print "text: ", search_results[i]["text"]
    print "created_at: ", search_results[i]["created_at"]
    print "favorite_count: ", search_results[i]["favorite_count"]
    print "retweet_count: ", search_results[i]["retweet_count"]
    print "followers_count: ", search_results[i]["user"]["followers_count"]
    print "friends_count: ", search_results[i]["user"]["friends_count"]
    print "coordinates: ", search_results[i]["coordinates"]
    print 'statuses_count: ', search_results[i]["user"]['statuses_count']
    print "source: ", search_results[i]["source"]
"""
    
    
book = xlwt.Workbook(encoding="utf-8")

sheet1 = book.add_sheet("Sheet 1")

# row, col
sheet1.write(0,0, "Text")
sheet1.write(0,1, "Tweet Time")
sheet1.write(0,2, "# of Favorites")
sheet1.write(0,3, "# of Retweets")
sheet1.write(0,4, "# of Followers")
sheet1.write(0,5, "# Following")
sheet1.write(0,6, "Geo Coordinates")
sheet1.write(0,7, "Total Tweets")
sheet1.write(0,8, "URL source")
sheet1.write(0,13, "User Location")


tokenizer = RegexpTokenizer(r'\w+')
hashtag_tokenizer = RegexpTokenizer(r'#\w+')

words = []
hashtags = []
for i in range(1, tweetCount+1):
    sheet1.write(i,0, search_results[i-1]["text"])
    #for w in search_results[i-1]["text"].split():
    #    words += w.lower()
    words += tokenizer.tokenize(search_results[i-1]["text"].lower())
    hashtags += hashtag_tokenizer.tokenize(search_results[i-1]["text"].lower())
    sheet1.write(i,1, search_results[i-1]["created_at"])
    sheet1.write(i,2, search_results[i-1]["favorite_count"])
    sheet1.write(i,3, search_results[i-1]["retweet_count"])
    sheet1.write(i,4, search_results[i-1]["user"]["followers_count"])
    sheet1.write(i,5, search_results[i-1]["user"]["friends_count"])
    if(search_results[i-1]["coordinates"] != None):
        sheet1.write(i,6, str(search_results[i-1]["coordinates"]["coordinates"]))
    sheet1.write(i,7, search_results[i-1]["user"]['statuses_count'])
    sheet1.write(i,8, search_results[i-1]["source"])
    if(search_results[i-1]["user"]['location'] != None):
        sheet1.write(i,13, search_results[i-1]["user"]['location'])

c = Counter(words)
h = Counter(hashtags)

sheet1.write(0,9, "Most common tokens (all converted to lowercase)")
sheet1.write(0,10, "# of tokens found")
i = 1
for word in c.most_common(1000):
    sheet1.write(i,9, word[0])
    sheet1.write(i,10, word[1])
    i += 1
    
sheet1.write(0,11, "Most common hashtags")
sheet1.write(0,12, "# of hashtags found")
i = 1
for word in h.most_common(300):
    sheet1.write(i,11, word[0])
    sheet1.write(i,12, word[1])
    i += 1

book.save(excel_file)

