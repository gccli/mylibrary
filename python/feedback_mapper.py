#! /usr/bin/env python
import os
import sys

import logging
import struct
import datetime
from pprint import pprint as pp

attribute_type={1:'URL',2:'Reffer',3:'User-Agent',4:'CustomID',5:'UserID',6:'ProductName',7:'ProductVersion',
                8:'Filename',9:'URLHash',10:'ResultAttribute',11:'Result',12:'TrafficDirection'}


def feedback_gen_chunk_list(content, contlen):
    chunklist=[]
    logging.debug("Begin parse chunk list, length is %d, %d", contlen, len(content))

    remaining=contlen
    count=0
    while remaining > 0:
        chunk={}

        remaining -= struct.calcsize('IHHI')
        fmt = 'IHHI%ds' % remaining

        ts,ctype,num,attrlen,content=struct.unpack(fmt, content)
        chunk['time'] = ts
        chunk['type'] = ctype
        chunk['number'] = num
        chunk['attribute_length'] = attrlen

        fmt = '%ds%ds' % (attrlen,remaining-attrlen)
        attrbuf,content=struct.unpack(fmt, content)
        remaining -= attrlen
        count=count+1
        logging.debug("Begin parse chunk[%d] with %d feed, attrbute length is %d", count, num, len(attrbuf))

        while attrlen > 0 and attrbuf and len(attrbuf) > 0:

            fmt='=bH'
            fixed=struct.calcsize(fmt)
            attrlen -= fixed

            fmt += ('%ds' % attrlen)
            t,l,attrbuf=struct.unpack(fmt,attrbuf)
            logging.debug("  got attrbute '%s', length is %d", attribute_type[t] if attribute_type[t] else 'Unknown',l)
            if t==1:
                chunk['url'] = attrbuf[:l]
            elif t==3:
                chunk['user_agent'] = attrbuf[:l]
            elif t==4:
                chunk['custom_id'] = attrbuf[:l]
            elif t==5:
                chunk['user_id'] = attrbuf[:l]
            attrlen -=l
            attrbuf=attrbuf[l:]

        remaining -= 4
        fmt = 'I%ds'%(remaining)
        feedcntlen,content=struct.unpack(fmt,content)
        remaining -= feedcntlen
        fmt = '%ds%ds'%(feedcntlen,remaining)
        feedcontent,content=struct.unpack(fmt,content)
        chunklist.append(chunk)
    return chunklist

def feedback_gen_buffer(buff, size):
    result={}
    fmt='6s8sH'
    size -= struct.calcsize(fmt)
    fmt += ('%ds' % size)

    logging.info("Begin parse buffer, buffer len is %d", size)
    fbver,libver,engine_vp_num,buff=struct.unpack(fmt, buff)
#    logging.debug( '--------',fbver,'--------',libver,'--------',engine_vp_num)

    result['feedback_version'] = fbver
    result['cca_version'] = libver
    result['engine_number'] = engine_vp_num

    result['engine_array'] = []
    for i in range(engine_vp_num):
        fmt='4s4s8s'
        size -= struct.calcsize(fmt)
        fmt += ('%ds' % size)
        engname,engver,dbver,buff=struct.unpack(fmt, buff)
 #       logging.debug('--------',engname,'--------',engver,'--------',dbver)
        result['engine_array'].append({'name':engname,'version':engver,'dbversion':dbver})

    size -= 8
    fmt='II%ds' % size
    ts,contlen,buff=struct.unpack(fmt, buff)
    result['time'] = ts
    result['buffer_length'] = contlen
    # parse the chunk list
    
    content = buff[:contlen]
    result['chunk_list'] = feedback_gen_chunk_list(content, contlen)

    size -= contlen
    return result,size

def feedback_parser(filename):
    st = os.stat(filename)
    if not st:
        return None

    buff_list = []

    file_len=st.st_size
    file_buf=''
    with open(filename, 'rb') as f:
        file_buf=f.read()
        f.close()
    
    remaining=file_len
    while remaining > 0:
        buf,remaining = feedback_gen_buffer(file_buf, remaining)
        file_buf=file_buf[file_len-remaining:]
        buff_list.append(buf)
        logging.info("End parse buffer version %s, cca version %s, buffer length %d",
                      buf['feedback_version'], buf['cca_version'], buf['buffer_length'])


    return buff_list

def useragent_map(bufflist):
    useragent={}

    for buff in bufflist:
        for chunk in buff['chunk_list']:
            if chunk.has_key('user_agent'):
                ua=chunk['user_agent']
                if not useragent.has_key(ua):
                    useragent[ua]=1
                else:
                    useragent[ua]+=1
    pp(useragent)

if __name__ == '__main__':
    import getopt
    verbose=0

    try:
        opts,args = getopt.getopt(sys.argv[1:], "v")#, ["verbose"])
    except getopt.GetoptError as err:
        print str(err)
        sys.exit(2)

    if len(args) <= 0:
        print 'feedbackparser.py [-v] filename'
        sys.exit(2)

    for o, a in opts:
        if o in ("-v"):
            verbose+=1
        else:
            sys.exit(2)

    logging.basicConfig(format='%(levelname)-8s%(message)s', level=(logging.INFO if verbose==0 else logging.DEBUG))
    bufflist = feedback_parser(args[0])
    logging.info("buffer list size is %d", len(bufflist))

    useragent_map(bufflist)
