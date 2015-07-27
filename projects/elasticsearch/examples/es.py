#! /usr/bin/env python

import os
import sys
import getopt
import random
import httplib
import simplejson
import time
import fnmatch
from pprint import pprint
from datetime import date,datetime
import elasticsearch
from elasticsearch import Elasticsearch

opt_size = 10
opt_source = False

def usage():
  print 'Usage: ./elasticsearch <get|update|search> [ --id id ] -f body'
  sys.exit(2)

class Feedback(object):
  host = '127.0.0.1'
  index='result_'+date.today().strftime('%Y%m%d')
  index_pattern = 'result_*'

  def __init__(self, doctype='feedback'):
    self.server = Elasticsearch([{'host': self.host}])

    self.doctype = doctype
  

  def get(self, docid):
    try:
      doc = self.server.get(index=self.index, doc_type=self.doctype, id=docid)
      pprint(doc)
    except elasticsearch.ElasticsearchException as e:
      print e.info

  def update(self, docid, content):
    try:
      res = self.server.update(index=self.index, doc_type=self.doctype, id=docid, body=content)
      print res
    except elasticsearch.ElasticsearchException as e:
      print e.info

  def search(self, content):
    global opt_size, opt_source
    try:
      res = self.server.search(index=self.index, doc_type=self.doctype, body=content, _source=opt_source, size=opt_size)
      pprint(res, width=120)
    except elasticsearch.ElasticsearchException as e:
      print e.info

def loadfile(filename):
  content = None
  with open(filename, 'r') as f:
    content = f.read()
    f.close()
  return content

def main():
  if len(sys.argv) < 3:
    usage()
  command = sys.argv[1]
  opt_doc=''
  opt_id=''
  global opt_source, opt_size
  opt_body='feedback'
  try:
    opts,args = getopt.getopt(sys.argv[2:], "f:", ["id=", "doc=", "source", "size="])
  except getopt.GetoptError as err:
    print str(err)
    usage()
  for o, a in opts:
    if o in ("--id"):
      opt_id=a
    elif o in ("--doc"):
      opt_doc=a
    elif o in ("--source"):
      opt_source=True
    elif o in ("--size"):
      opt_size=int(a)
    elif o in ("-f"):
      opt_body = loadfile(a)
    else:
      usage()

  fb = Feedback(opt_doc)
  if (command == 'get'):
    fb.get(opt_id)
  elif (command == 'update'):
    if opt_id and opt_body:
      fb.update(opt_id, opt_body)
  elif (command == 'search'):
    if opt_body:
      fb.search(opt_body)
  else:
    usage()

if __name__ == '__main__':
  main()
