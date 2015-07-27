#! /usr/bin/env python

import os
import sys
import time
import string
import re
import getopt
import subprocess

from collections import OrderedDict

class Smaps(object):
    addrsize = 0
    addrmaps = OrderedDict()

    rss_total=0
    rss_shared=0
    rss_shared_clean=0
    rss_shared_dirty=0
    rss_private=0
    rss_private_clean=0
    rss_private_dirty=0
    def __init__(self, cmaps):
        if not cmaps or cmaps == '':
            return 

        pattern='(^[0-9a-f]+-[0-9a-f]+).* (.*)$'
        for m in re.finditer(pattern, cmaps, re.M):
            addr = m.group(1)
            if (len(addr) > self.addrsize):
                self.addrsize = len(addr)
            s = self.section(m.group(2), cmaps[m.start():])
            self.rss_shared_clean += s.shared_clean
            self.rss_shared_dirty += s.shared_dirty

            self.rss_private_clean += s.private_clean
            self.rss_private_dirty += s.private_dirty

            self.addrmaps[addr] = s

        self.rss_private = self.rss_private_clean+self.rss_private_dirty
        self.rss_shared = self.rss_shared_clean+self.rss_shared_dirty
        self.rss_total = (self.rss_shared+self.rss_private)

    class section(object):
        proc=''
        shared_clean=0
        shared_dirty=0
        private_clean=0
        private_dirty=0

        def __init__(self, proc, text):
            self.proc=proc
            pattern='^Shared_Clean:\s*([0-9]*)'
            self.shared_clean = self.retrive(text, pattern)
            pattern='^Shared_Dirty:\s*([0-9]*)'
            self.shared_dirty = self.retrive(text, pattern)
            pattern='^Private_Clean:\s*([0-9]*)'
            self.private_clean = self.retrive(text, pattern)
            pattern='^Private_Dirty:\s*([0-9]*)'
            self.private_dirty = self.retrive(text, pattern)

        def retrive(self, text, pattern):
            m = re.search(pattern, text, re.M)
            if m:
                return int(m.group(1))

    @staticmethod
    def dump(smaps, verbose=0, pid=0):
        if verbose > 0:
            for k,v in smaps.addrmaps.items():
                print '%-*s %-15s %-3d %-3d %-3d %-3d' % (
                    smaps.addrsize,k,
                    os.path.basename(v.proc)[0:14].rstrip('.'),
                    v.shared_clean,v.shared_dirty,v.private_clean,v.private_dirty)
        print '%-8d %-8d %8d %9d %13d %14d' % (
            pid, smaps.rss_total,
            smaps.rss_shared,
            smaps.rss_private,
            smaps.rss_shared_dirty,
            smaps.rss_private_dirty)        

class Processes(object):
    prog = ''
    def __init__(self, prog):
        self.prog = prog

    def monitor(self):
        while True:
            smaps_list = OrderedDict()
            pids = self.get_pid_list()
            for i in pids:
                pid = i.strip('\n')
                smaps_list[pid] = self.analyze_pid(pid)

            for k,v in smaps_list.items():
                Smaps.dump(v,pid=int(k))
            Smaps.dump(self.merge(smaps_list))
            time.sleep(interval)

    def merge(self, smaps_list):
        merged = Smaps('')
        for k,v in smaps_list.items():
            merged.rss_total += v.rss_total
            merged.rss_shared += v.rss_shared
            merged.rss_private += v.rss_private
            merged.rss_shared_dirty += v.rss_shared_dirty
            merged.rss_private_dirty += v.rss_private_dirty

        return merged

    def get_pid_list(self):
        command_list = ['pidof', self.prog]
        p = subprocess.Popen(command_list, stdout=subprocess.PIPE)
        outstr,err = p.communicate()
        if p.returncode != 0:
            print outstr
            sys.exit()
        pids = outstr.split(' ')
        return pids

    def analyze_pid(self, pid):
        smaps_file='/proc/%s/smaps' % pid
        smaps_buffer = ''
        with open(smaps_file, 'r') as fp:
            smaps_buffer=fp.read()
            fp.close()

        smaps = Smaps(smaps_buffer)        
        return smaps

def usage():
    print './smaps.py [ -v ] [ -n ] [ -i int ] <pid|name>'
    sys.exit(1)

if __name__=='__main__':
    if len(sys.argv) < 2:
        usage()
        sys.exit(1)

    verbose=0
    interval=1
    isname=False
    opts, args = getopt.getopt(sys.argv[1:], "vni:")
    for o, a in opts:
        if o == "-v":
            verbose+=1
        elif o == "-i":
            interval = float(a)
        elif o == "-n":
            isname = True
        else:
            usage()

    print 'PID      Total      Shared   Private  Shared_Dirty  Private_Dirty'
    print '--------------------------------------------------------------------------------'

    if isname:
        process = Processes(args[0])
        process.monitor()
        sys.exit(0)

    while True:
        smaps_file='/proc/%s/smaps' % args[0]
        smaps_buffer = ''
        with open(smaps_file, 'r') as fp:
            smaps_buffer=fp.read()
            fp.close()

        smaps = Smaps(smaps_buffer)
        Smaps.dump(smaps, verbose)
        time.sleep(interval)
