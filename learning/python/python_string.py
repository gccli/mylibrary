#! /usr/bin/python

import sys
import random

# print 'There are seven sequence types: strings, Unicode strings, lists, tuples, bytearrays, buffers, and xrange objects\n'

def lambdafunc(i=None):
    list_appid = ['jingccli', 'inetlinuxdemo', 'inetlinux']
    https = 'https'
    path = '/2'
    get_server1 = lambda i: '%s://%s.appspot.com%s?' % (https, list_appid[i] if i is not None else random.choice(list_appid), path)
    get_server2 = lambda i: '%s://%s.appspot.com%s?' % (https, list_appid[i] if i is not None else random.choice(list_appid), path)
    server1 = get_server1(i)
    server2 = get_server2(i)
    print 
    print 'lambda function return:', server1, server2


def usingstr(i = None):
    ssl_ciphers = ':'.join(['ECDHE-ECDSA-AES256-SHA',
                            'ECDHE-RSA-AES256-SHA',
                            'DHE-RSA-CAMELLIA256-SHA',
                            'DHE-DSS-CAMELLIA256-SHA',
                            'DHE-RSA-AES256-SHA',
                            'DHE-DSS-AES256-SHA',
                            'ECDH-RSA-AES256-SHA',
                            'ECDH-ECDSA-AES256-SHA',
                            'CAMELLIA256-SHA',
                            'AES256-SHA',
                            'ECDHE-ECDSA-RC4-SHA',
                            'ECDHE-ECDSA-AES128-SHA',
                            'ECDHE-RSA-RC4-SHA',
                            'ECDHE-RSA-AES128-SHA',
                            'DHE-RSA-CAMELLIA128-SHA',
                            'DHE-DSS-CAMELLIA128-SHA',
                            'DHE-RSA-AES128-SHA',
                            'DHE-DSS-AES128-SHA',
                            'ECDH-RSA-RC4-SHA',
                            'ECDH-RSA-AES128-SHA',
                            'ECDH-ECDSA-RC4-SHA',
                            'ECDH-ECDSA-AES128-SHA',
                            'SEED-SHA',
                            'CAMELLIA128-SHA',
                            'RC4-SHA',
                            'RC4-MD5',
                            'AES128-SHA',
                            'ECDHE-ECDSA-DES-CBC3-SHA',
                            'ECDHE-RSA-DES-CBC3-SHA',
                            'EDH-RSA-DES-CBC3-SHA',
                            'EDH-DSS-DES-CBC3-SHA',
                            'ECDH-RSA-DES-CBC3-SHA',
                            'ECDH-ECDSA-DES-CBC3-SHA',
                            'DES-CBC3-SHA',
                            'TLS_EMPTY_RENEGOTIATION_INFO_SCSV'])


    print 'ssl_ciphers', 'type:', type(ssl_ciphers), 'length:', len(ssl_ciphers), \
        'id:', id(ssl_ciphers), '\n--------\n', ssl_ciphers, '\n--------\n'
    print 'ssl_ciphers[0:8]', ssl_ciphers[0:80]
    print 'DES-CBC3-SHA in ssl_ciphers:', 'DES-CBC3-SHA' in ssl_ciphers

usingstr()
lambdafunc()
