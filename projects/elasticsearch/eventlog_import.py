#! /usr/bin/env python

import sys
import signal
import getopt

import pcap
import socket
import json
from datetime import date, datetime, timedelta, tzinfo
import elasticsearch
from elasticsearch import Elasticsearch

import dpkt


import time
current_milli_time = lambda: int(round(time.time() * 1000))

def usage():
    print >>sys.stderr, 'usage: %s [-i device] [pattern]' % sys.argv[0]
    sys.exit(1)


class IndexMgr(object):
    index_pattern='event_*'
    doc_feedback='log'
    def __init__(self, *args, **kwargs):
        self.server = kwargs.get('host', 'localhost')
        self.es = Elasticsearch([{'host': self.server}])

    def feedback_create(self):
        self.es.indices.delete(index=self.current(), ignore=[400, 404])
        self.es.indices.create(index=self.current(), ignore=[400])

        mapping = {
            "log" : {
                "_timestamp": {
                    "enabled": True
                },
                "properties" : {
                    "vendor" : {
                        "type" : "string",
                        "index" : "not_analyzed"
                    },
                    "URL" : {
                        "type" : "string",
                        "index" : "not_analyzed"
                    },
                    "source" : {
                        "type" : "string",
                        "index" : "not_analyzed"
                    },
                    "appid_action" : {
                        "type" : "string",
                        "index" : "not_analyzed"
                    }
                }
            }
        }

        rt=self.es.indices.put_mapping(index=self.current(), doc_type=self.doc_feedback, body=mapping)
        mp=self.es.indices.get_mapping(index=self.current())
        print 'Index created', rt, '\n'

    def delete(self):
        self.es.indices.delete(index=self.index_pattern, ignore=[400, 404])

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

    def current(self):
        index = 'event_{0}'.format(date.today().strftime('%Y%m%d'))
        return index


def decode_udp ( pc ) :
    for ts, pkt in pc:
        eth = dpkt.ethernet.Ethernet(pkt)
        if eth.type == dpkt.ethernet.ETH_TYPE_IP :
            ip = eth.data
            if ip.p == dpkt.ip.IP_PROTO_UDP :
                udp = ip.data
                yield (ip.src, udp.sport, udp.data, ip.v)
        elif eth.type == dpkt.ethernet.ETH_TYPE_IP6 :
            ip = eth.data
            if ip.nxt == dpkt.ip.IP_PROTO_UDP :
                udp = ip.data
                yield (ip.src, udp.sport, udp.data, ip.v)
        else:
            pass


def signal_handler(signal, frame):
    sys.exit(0)


# pip install elasticsearch dpkt pypcap
class TZ(tzinfo):
    def utcoffset(self, dt): return timedelta(hours=+8)
    def dst(self, dt): return timedelta(hours=+8)

def main():
    index_manager = IndexMgr(host = '127.0.0.1')
    pcap_filter = 'port 10087'

    opts, args = getopt.getopt(sys.argv[1:], 'i:d')
    name = 'lo'
    for o, a in opts:
        if o == '-i': name = a
        elif o == '-d':
            index_manager.delete();
            index_manager.feedback_create();
        else: usage()

    pc = pcap.pcap(name)
    pc.setfilter(pcap_filter)

    signal.signal(signal.SIGINT, signal_handler)
    actions = {
        2:'DownloadREQ',
        3:'Download',
        4:'Upload',
        6:'DeleteREQ',
        7:'Delete',
        9:'RenameREQ',
        9:'Rename',
        128:'AddrbookListREQ',
        129:'AddrbookList',
        131:'AddrbookDetailREQ',
        131:'AddrbookDetail',
        132:'Addrbook Save',

        257:'SFShow',
        265:'SFMainPage',
        277:'SFHomePage',
        267:'SFHover',
        269:'SFLookup'
    }
    vendors = {1:"Qihoo", 2:"Salesforce"}
    nact = 0

    for src, sport, data, ip_version in decode_udp(pc):
        dl = len(data)
        try:
            jd = json.loads(data[:dl-1])
        except:
            print 'parse json error:'
            print data
            continue

        nact = jd['appid_action'];
        jd['datetime'] = datetime.fromtimestamp(jd['datetime'], tz=TZ()).isoformat()
        if ((nact == 256 or nact > 263) and jd['appid_action']%2==0):
            continue
        jd['appid_action'] = actions.get(jd['appid_action'], 'Unknown {0}'.format(jd['appid_action']))
        jd['vendor'] = vendors.get(jd['vendor'], 'Unknown')

        print '{0} from {1}:{2} received {3} bytes'.format(jd['datetime'], socket.inet_ntoa(src), sport, dl)
        index_manager.feedback_insert(current_milli_time(), jd)

if __name__ == '__main__':
    main()
