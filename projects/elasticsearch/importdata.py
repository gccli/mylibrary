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
from datetime import date, datetime
import elasticsearch
from elasticsearch import Elasticsearch
from progressbar import Bar, ProgressBar, Percentage#, FormatLabel

random_vt_avs = [
  {
    "Vendor" : "Avast",
    "MalwareName" : "Win32:Malware-gen"
  },
  {
    "MalwareName" : "Troj.W32.Swizzor",
    "Vendor" : "AegisLab"
  },
  {
    "MalwareName" : "PE:Trojan.Win32.Generic.17853AE6!394607334",
    "Vendor" : "Rising"
  },
  {
    "MalwareName" : "BScope.Trojan-Dropper.8612",
    "Vendor" : "VBA32"
  },
  {
    "Vendor" : "DrWeb",
    "MalwareName" : "BackDoor.Kuluoz.4"
  },
  {
    "MalwareName" : "W32/Trojan.TRHN-3234",
    "Vendor" : "Cyren"
  },
  {
    "Vendor" : "Avira",
    "MalwareName" : "TR/Kuluoz.A.460"
  },
  {
    "MalwareName" : "W32/Trojan3.LRQ",
    "Vendor" : "F-Prot"
  },
  {
    "Vendor" : "Sophos",
    "MalwareName" : "Troj/Wonton-JR"
  },
  {
    "Vendor" : "ESET-NOD32",
    "MalwareName" : "a variant of Win32/Kryptik.BVKP"
  }
]

random_modules = [
  {"name":"INJECT_CMD", "rating":10},
  {"name":"INJECT_AUTORUN", "rating":10},
  {"name":"PROC_WRITEFILE_APPDATA", "rating":10},
  {"name":"REG_AUTORUN", "rating":7},
  {"name":"PROC_POSSIBLE_INJECTED", "rating":6},
  {"name":"FS_APPDATA", "rating":4},
  {"name":"PROC_CREATE_CMD", "rating":4}
]

random_products = ["CWS/8.1", "WCG/8.0", "ESG/7.8", "RiskReason/3.0", "Tscope-Api/1.0"]
random_filetypes = ['EXE', 'Office', 'PDF', 'Archive', 'Media']
random_autoflags = ['Auto', 'HighRisk', 'HighPriority', 'LowPriority']
random_conditions = ["VirusTotal", "MaxScore", "TotalScore", "AlexaRank", "HitCount", "DigitalSignature"]

def usage():
  print 'Usage: %s [options] feedback' % (sys.argv[0])

class IndexMgr(object):
  index_pattern='result_*'
  doc_feedback='feedback'
  doc_percent='percent'
  doc_module='module'
  def __init__(self, *args, **kwargs):
    self.server = kwargs.get('host', 'localhost')
    self.es = Elasticsearch([{'host': self.server}])

  def feedback_create(self):
    self.es.indices.delete(index=self.current(), ignore=[400, 404])
    self.es.indices.create(index=self.current(), ignore=[400])

    mapping = {
      "feedback" : {
        "_timestamp" : {
          "enabled" : 'true',
          "path" : "tsapi.received"
        },

        "properties" : {
          "tsapi.product.name" : {
            "type" : "string",
            "index" : "not_analyzed"
          },
          "md5" : {
            "type" : "string",
            "index" : "not_analyzed"
          },
          "tag" : {
            "type" : "string",
            "index" : "not_analyzed"
          },
          "autoflag" : {
            "type" : "string",
            "index" : "not_analyzed"
          },
          "filetype" : {
            "type" : "string",
            "index" : "not_analyzed"
          },
          "assessment" : {
            "type" : "string",
            "index" : "not_analyzed"
          },
          "hit_modules.name" : {
            "type" : "string",
            "index" : "not_analyzed"
          }
        }
      }
    }
    rt=self.es.indices.put_mapping(index=self.current(), doc_type=self.doc_feedback, body=mapping)
    mapping = { 
      "percent" : {
        "properties" : {
          "timestamp" : {"type": "date"}
        }
      }
    }

    rt=self.es.indices.put_mapping(index=self.current(), doc_type=self.doc_percent, body=mapping)
    mapping = { 
      "module" : {
        "properties" : {
          "created" : {"type": "date"}
        }
      }
    }
    rt=self.es.indices.put_mapping(index=self.current(), doc_type=self.doc_module, body=mapping)
    mp=self.es.indices.get_mapping(index=self.current())  
    print 'Index created', rt, '\n'

  def delete(self):
    self.es.indices.delete(index='result_*', ignore=[400, 404])

  def module_insert(self, idd, doc):
    try:
      res = self.es.index(index=self.current(), doc_type=self.doc_module, id=idd, body=doc)
    except elasticsearch.ElasticsearchException as e:
      print 'Insert -', e.info

  def percent_insert(self, idd, doc):
    try:
      res = self.es.index(index=self.current(), doc_type=self.doc_percent, id=idd, body=doc)
    except elasticsearch.ElasticsearchException as e:
      print 'Insert -', e.info

  def feedback_get(self, idd):
    doc=None
    try:
      doc = self.es.get(index=self.current(), doc_type=self.doc_feedback, id=idd)
    except elasticsearch.ElasticsearchException as e:
      print 'Get -', e.info
    return doc

  def feedback_exists(self, idd):
    IsExists = False
    try:
      IsExists = self.es.exists(index=self.current(), doc_type=self.doc_feedback, id=idd)
    except elasticsearch.ElasticsearchException as e:
      print 'Exists -', e.info
    except: 
      pass
    return IsExists

  def feedback_insert(self, idd, doc):
    res = None
    try:
      res = self.es.index(index=self.current(), doc_type=self.doc_feedback, id=idd, body=doc)
    except elasticsearch.ElasticsearchException as e:
      print 'Insert -', e.info
    return res

  def feedback_update(self, idd, doc):
    res = None
    try:
      res = self.es.update(index=self.current(), doc_type=self.doc_feedback, id=idd, body=doc)
    except elasticsearch.ElasticsearchException as e:
      print 'Update (%s-%s)-' % (idd,doc), e.info
    return res

  def feedback_search(self, content, sz=10):
    res = None
    try:
      res = self.es.search(index=self.current(), doc_type=self.doc_feedback, body=content, size=sz)
    except elasticsearch.ElasticsearchException as e:
      print e.info
    return res

  def build_query(self, **kwargs):
    query = {"query" : {"filtered" : {"filter" : {"bool" : {"must" : [ ], "must_not": [ ]}}}}}
    must = query['query']['filtered']['filter']['bool']['must']
    must_not = query['query']['filtered']['filter']['bool']['must_not']

    dt_range = kwargs.get('daterange', None)
    vt_result = kwargs.get('vt_result', None)
    vt_detected = kwargs.get('vt_detected', None)
    tscp_score = kwargs.get('tscp_score', None)
    tag = kwargs.get('tag', None)
    assessment = kwargs.get('assessment', None)
    autoflag = kwargs.get('autoflag', None)
    not_gt_cca = kwargs.get('not_gt_cca', None)

    if dt_range:
      must.append({"range" : dt_range})
    if (vt_result == 0 or vt_result == 1):
      must.append({"term" : {"virustotal.result" : vt_result}})
    if vt_detected >= 0:
      must.append({"term" : {"virustotal.value" : vt_detected}})
    if tscp_score >= 0:
      must.append({"term" : {"threatscope.score" : tscp_score}})
    if tag:
      must.append({"term" : {"tag" : tag}})
    if assessment:
      must.append({"term" : {"assessment" : assessment}})
    if autoflag:
      must.append({"term" : {"autoflag" : autoflag}})

    if not_gt_cca >= 0:
#      must.append({"exists" : {"field": "threatscope.score"}})
      must_not.append({"range" : {"cca_result_count" : {"gt": 0 }}})

    return query

  def get_count(self, **kwargs):
    res = self.feedback_search(self.build_query(**kwargs), 0)
    return int(res['hits']['total'])

  def get_rules_count(self, **kwargs):
    agg = {"aggs" : {"modules" : {"terms" : {"field": "hit_modules.name"}}}}
    if len(kwargs) > 0:
      agg['query'] = self.build_query(**kwargs)['query']

    res = self.feedback_search(agg, 0)
    if (res and res['hits']['total'] > 0):
      return res['aggregations']['modules']['buckets']
    return []

  def current(self):
    index = 'result_{0}'.format(date.today().strftime('%Y%m%d'))
    return index

def isotime(ts):
  dt=datetime.fromtimestamp(ts)
  dt=datetime.isoformat(dt)
  return str(dt)

def fake_vt_result(tscp, randint=0):
  ret=0
  if (tscp == 0):
    if (randint <= 1):
      ret=1
  elif (tscp > 0 and tscp <=8):
    if (randint < 5):
      ret = 1
  else:
    if (randint < 8):
      ret = 1
  return ret

def parse_tsapi(idxmgr, filename):
  cache_sha1 = ''
  unique = 0
  with open(filename, 'r') as f:
    lines = f.readlines()
    print 'Process %s (%d lines):' % (filename, len(lines))
    widgets=[Percentage(), ' ', Bar()]
    pbar = ProgressBar(widgets=widgets, maxval=len(lines)).start()
    i=0
    for line in lines:
      dec = simplejson.loads(line)
      pbar.update(i)
      i+=1
      if not dec:
        break
      sha1=dec['id']
      if not sha1:
        continue
      if cache_sha1 == sha1:
        continue
      cache_sha1 = sha1
      if not idxmgr.feedback_exists(sha1):
        #product_version=dec['product_version']
        #product_name=dec['product_version'].split('/')[0]
        product_version = random_products[0]
        if (i%5 < 2):
          product_version = random_products[0]
        elif (i%5 == 2):
          product_version = random_products[1]
        else:
          product_version = random.sample(random_products, 1)[0]
#        ts = long(dec['timestamp'])-random.randint(-100000, 100000)
        ts = long(time.time())-random.randint(0, 200000)
        obj= {
          'tsapi': {
            'received':isotime(ts),
            'product':{
              'name':product_version.split('/')[0],
              'version':product_version.split('/')[1]
            }
          }
        }
        idxmgr.feedback_insert(sha1, obj)
        unique += 1
    # end parsing
    pbar.finish()
    f.close()

  return unique

def parse_sbx(idxmgr, filename):
  unique = 0
  with open(filename, 'r') as f:
    for line in f.readlines():
      dec = simplejson.loads(line)
      if not dec:
        break
      tscp_res=int(dec['rating'])
      vt_res=random.randint(0,2)

      obj={'scanned':isotime(int(dec['created_at'])),
           'md5':dec['md5_hex'],
           'filedesc':dec['file_desc'],
           'filelength':int(dec['length']),
           'threatscope': {'score':tscp_res, 'totalscore':int(dec['score'])}
         }

      if dec.has_key('cca_scan_result'):
        obj['cca_result'] = []
        cca_result_count = 0
        for r in dec['cca_scan_result'].split('],'):
          r=r.strip('[],')
          cca = {}
          for s in r.split(','):
            kv = s.split(':')
            cca[kv[0]] = kv[1] 
          cca_result_count+=1
          obj['cca_result'].append(cca)
          obj['cca_result_count'] = cca_result_count

      obj['filetype'] = random.sample(random_filetypes, 2)[0] if (unique%2 == 0) else random_filetypes[0]
      if (vt_res != 2):
        obj['virustotal'] = {}
        obj['virustotal']['result'] = vt_res
        if (vt_res == 0):
          val = random.randint(0, 2)
        else:
          val = random.randint(3, 9)
        obj['virustotal']['value'] = val
        obj['virustotal']['detected_avs'] = random.sample(random_vt_avs, val)

      if (tscp_res == 0):
        if (vt_res == 0):
          obj['TN']=True
          obj['tag']='TN'
        elif (vt_res == 1):
          obj['FN']=True
          obj['tag']='FN'
        obj['assessment'] = 'clean'
        obj['detected']=False
      else:
        if (vt_res == 0):
          obj['FP']=True
          obj['tag']='FP'
        elif (vt_res == 1):
          obj['TP']=True
          obj['tag']='TP'

        # http://www.elasticsearch.org/guide/en/elasticsearch/guide/current/_finding_multiple_exact_values.html#_contains_but_does_not_equal
        matched_rule_count = random.randint(1,3)
        obj['hit_modules']=random.sample(random_modules, matched_rule_count)
        obj['hit_modules_count']=matched_rule_count
        obj['detected']=True
        obj['assessment'] = 'malicious' if tscp_res >= 9 else 'suspicious'

      doc=idxmgr.feedback_get(dec['sha1_hex'])
      if doc:
        doc = doc['_source']
        obj['tsapi'] = doc['tsapi']
        res = idxmgr.feedback_insert(dec['sha1_hex'], obj)
        unique += 1
        if (res and tscp_res > 0):
          autoflag = random.sample(random_autoflags, 1)[0]
          conditions = []
          if (autoflag == "Auto"):
            conditions = random.sample(random_conditions, 1)
          elif (autoflag == "HighRisk"):
            conditions = random_conditions[3:]
          else:
            conditions = random.sample(random_conditions, random.randint(0,3))
          conditions_count = len(conditions)
          updateobj = {
            "script" : "ctx._source.autoflag = \"%s\"" % autoflag
          }
          res=idxmgr.feedback_update(dec['sha1_hex'], updateobj)
          if conditions_count > 0:
            updateobj = {
              "script" : "ctx._source.hit_conditions = \"%s\"" % conditions
            }
            res=idxmgr.feedback_update(dec['sha1_hex'], updateobj)
          updateobj = {
            "script" : "ctx._source.hit_conditions_count = %d" % conditions_count
          }
          res=idxmgr.feedback_update(dec['sha1_hex'], updateobj)
      else:
        pass
    f.close()
  return unique

def get_modules_score(idxmgr):
  allmodules = {}
  for r in idxmgr.get_rules_count():
    allmodules[r['key']] = 0
  for k,v in allmodules.iteritems():
    query = {
      "size": 1,
      "fields" : ["hit_modules.name", "hit_modules.rating"],
      "query" : {
        "filtered" : {
          "filter" : {
            "term" : {"hit_modules.name" : k}
          }
        }
      }
    }

    res = idxmgr.feedback_search(query, 1)
    if (res['hits']['total'] > 0):
      fields = res['hits']['hits'][0]['fields']
      i=0
      for n in fields['hit_modules.name']:
        if n == k:
          allmodules[k] = fields['hit_modules.rating'][i]
          break
        i += 1
  print 'Modules score:\n', allmodules, '\n'
  return allmodules

def calc_rules_before_day(idxmgr, day, to_day, rules):
  dtrange = {"tsapi.received" : {"lt": to_day}}

  p = idxmgr.get_count(daterange=dtrange, vt_result=1)
  n = idxmgr.get_count(daterange=dtrange, vt_result=0)
  if (p < 1 or n < 1):
    return {}

  for r in idxmgr.get_rules_count(daterange=dtrange):
    k = r['key']
    rules[k]['hit']['total'] = r['doc_count']
  for r in idxmgr.get_rules_count(daterange=dtrange, tag='FP'):
    k = r['key']
    rules[k]['FP']['total'] = r['doc_count']
    rules[k]['FPR']['total'] = round(100.0*r['doc_count']/n, 2)
  for r in idxmgr.get_rules_count(daterange=dtrange, tag='TP'):
    k = r['key']
    rules[k]['TP']['total'] = r['doc_count']
    rules[k]['DR']['total'] = round(100.0*r['doc_count']/p, 2)

  print '  Total malicious:%d clean:%d before "%s"' % (p,n,to_day.rstrip('|'))
  return rules

def do_calc(idxmgr, day, from_exp, to_exp, allrules):
  rules = {}
  for k,v in allrules.iteritems():
    rules[k] = {
      'name': k,
      'score': v,
      'hit': {},
      'FP': {},
      'TP': {},
      'DR': {},
      'FPR': {}
    }

  xmodules = calc_rules_before_day(idxmgr, day, to_exp, rules)
  if len(xmodules) > 0:
    rules = xmodules
  dtrange = {"tsapi.received" : {"gte": from_exp, "lt": to_exp}}
  p = idxmgr.get_count(daterange=dtrange, vt_result=1)
  n = idxmgr.get_count(daterange=dtrange, vt_result=0)
  print '  Date: "%s - %s":' % (from_exp.rstrip('|'), to_exp.rstrip('|')), '(malicious:%d, clean:%d)' % (p, n)
  if (p < 1 or n < 1):
    return False

  for r in idxmgr.get_rules_count(daterange=dtrange):
    k = r['key']
    rules[k]['hit']['today'] = r['doc_count']
  for r in idxmgr.get_rules_count(daterange=dtrange, tag='FP'):
    k = r['key']
    rules[k]['FP']['today'] = r['doc_count']
    rules[k]['FPR']['today'] = round(100.0*r['doc_count']/n, 2)
  for r in idxmgr.get_rules_count(daterange=dtrange, tag='TP'):
    k = r['key']
    rules[k]['TP']['today'] = r['doc_count']
    rules[k]['DR']['today'] = round(100.0*r['doc_count']/p, 2)

  for k,v in rules.iteritems():
    rules[k]['created'] = day
    rules[k]['name'] = k
    idd='%s_%s' % (day.strftime('%Y%m%d'), k)
    idxmgr.module_insert(idd, rules[k])
#    pprint(rules[k])

  dr,fpr,tp_count,tn_count,fp_count,fn_count={},{},{},{},{},{}
  count,extra_cca_percentage,extra_vt_percentage = {},{},{}

  autoflags = ['Auto']
  assessments = ['malicious', 'suspicious']
  scores = [str(i) for i in xrange(1,11)]
  partitions = autoflags + assessments + scores

  for x in partitions:
    if x in autoflags:
      tp_count[x] = idxmgr.get_count(daterange=dtrange, vt_result=1, autoflag=x)
      fp_count[x] = idxmgr.get_count(daterange=dtrange, vt_result=0, autoflag=x)
    elif x in scores:
      tp_count[x] = idxmgr.get_count(daterange=dtrange, vt_result=1, tscp_score=int(x))
      fp_count[x] = idxmgr.get_count(daterange=dtrange, vt_result=0, tscp_score=int(x))
    elif x in assessments:
      count[x] = idxmgr.get_count(daterange=dtrange, assessment=x)
      tp_count[x] = idxmgr.get_count(daterange=dtrange, vt_result=1, assessment=x)
      fp_count[x] = idxmgr.get_count(daterange=dtrange, vt_result=0, assessment=x)
      extra_cca = idxmgr.get_count(daterange=dtrange, assessment=x, not_gt_cca=0)
      extra_cca_percentage[x] = 100.0*extra_cca/count[x]

      extra_vt = idxmgr.get_count(daterange=dtrange, assessment=x, vt_detected=0)
      extra_vt_percentage[x] = 100.0*extra_vt/count[x]
    dr[x] = 100.0*float(tp_count[x])/p
    fpr[x] = 100.0*float(fp_count[x])/n

  for x in partitions:
    obj = {}
    obj['info'] = {}

    obj['timestamp'] = day
    obj['partition'] = x
    obj['info']['TP'] = tp_count[x]
    obj['info']['FP'] = fp_count[x]
    obj['info']['DR'] = dr[x]
    obj['info']['FPR'] = fpr[x]

    if extra_cca_percentage.has_key(x):
      obj['info']['cca_extra'] = extra_cca_percentage[x]
    if extra_vt_percentage.has_key(x):
      obj['info']['vt_extra'] = extra_vt_percentage[x]

    idd='%s_%s' % (day.strftime('%Y%m%d'), x)
    idxmgr.percent_insert(idd, obj)

def calculate(idxmgr, days=30):
  # http://www.elasticsearch.org/guide/en/elasticsearch/reference/current/mapping-date-format.html
  allmodules = get_modules_score(idxmgr)
  for i in range(days):
#    from_exp = 'now/d-{0}d'.format(i+1)
#    to_exp = 'now/d-{0}d'.format(i) if i>0 else 'now/d'
    day = date.fromtimestamp(time.time()-i*86400)
    today = date.fromtimestamp(time.time()+86400-i*86400)
    from_exp = '{0}||'.format(day.isoformat())
    to_exp = '{0}||'.format(today.isoformat())

    dtrange = {"tsapi.received" : {"lt": to_exp}}
    count = idxmgr.get_count(daterange=dtrange)
    print '\033[32mDocument count before "%s": %d\033[0m' %(date.fromtimestamp(time.time()+86400-i*86400), count)
    if (count == 0):
      break

    do_calc(idxmgr, day, from_exp, to_exp, allmodules)

def main():
  esserver='127.0.0.1'
  opt_sbx = None
  opt_delete = 0
  try:
    opts,args = getopt.getopt(sys.argv[1:], "h:d", ["sbx="])
  except getopt.GetoptError as err:
    print str(err)
    usage()
    sys.exit(2)

  for o, a in opts:
    if o in ("-h"):
      esserver = str(a)
    elif o in ("-d"):
      opt_delete = 1
    elif o in ("--sbx"):
      opt_sbx = str(a)
    else:
      usage()
      sys.exit(2)
  index_manager = IndexMgr(host = esserver)

#  calculate(index_manager)
#  sys.exit(0)

  if (opt_delete):
    index_manager.delete()
  if (opt_sbx):
    parse_sbx(index_manager, opt_sbx)
    sys.exit(0)
  time.sleep(1.0)
  index_manager.feedback_create()

  count = 0
  for f in os.listdir(args[0]):
    if fnmatch.fnmatch(f, 'tsapi*.log*'):
      count += parse_tsapi(index_manager, args[0]+'/'+f)
  print 'Insert/update %d documents for TSA' % count

  count = 0
  for f in os.listdir(args[0]):
    if fnmatch.fnmatch(f, 'cluster*'):
      count += parse_sbx(index_manager, args[0]+'/'+f)
  print 'Insert/update %d documents for SBX' % count
  calculate(index_manager)

if __name__ == '__main__':
  main()
