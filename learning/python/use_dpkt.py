#! /usr/bin/env python

import sys
import signal
import getopt

import pcap
import socket
import json
from datetime import date, datetime
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
                    "URL" : {
                        "type" : "string",
                        "index" : "not_analyzed"
                    },
                    "source" : {
                        "type" : "string",
                        "index" : "not_analyzed"
                    },
                    "datetime" : {
                        "type" : "date",
                        "format" : "epoch_second"
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

# event "{\"type\":1,\"vendor\":2,\"appid_action\":131,\"datetime\":1462965421,\"protocol\":1,\"URL\":\"yun.360.cn\\/contact\\/detail\",\"source\":\"10.18.240.131\"}


# pip install elasticsearch dpkt pypcap

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

    jd = json.JSONDecoder()
    for src, sport, data, ip_version in decode_udp(pc):
        dl = len(data)
        print 'from {0}:{1} received {2} bytes'.format(socket.inet_ntoa(src), sport, len(data))
        print json.dumps(data[:dl-1])
        index_manager.feedback_insert(current_milli_time(), data[:dl-1])

        #print json.loads(data)

if __name__ == '__main__':
    main()
