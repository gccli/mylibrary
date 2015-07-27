#! /usr/bin/env python

import sys
import getopt
import hashlib

BLOCK_SIZE = 0x10000
class MDtool(object):

    @staticmethod
    def sha1sum(filename):
        with open(filename, 'rb') as f:
            md = hashlib.sha1()
            while True:
                buf = f.read(BLOCK_SIZE)
                if not buf:
                    break
                md.update(buf)
            print md.hexdigest()

    @staticmethod
    def md5sum(filename):
        with open(filename, 'rb') as f:
            md = hashlib.md5()
            while True:
                buf = f.read(BLOCK_SIZE)
                if not buf:
                    break
                md.update(buf)
            print md.hexdigest()

def usage():
    print "Usage: mdtool.py [ -hsm ] filename"

def main():
    try:
        opts,args = getopt.getopt(sys.argv[1:], "sm", ["sha1", "md5"])
    except getopt.GetoptError as err:
        print str(err)
        usage()
        sys.exit(2)

    if len(args) <= 0:
        usage()
        sys.exit(2)

    for o, a in opts:
        if o in ("-s", "--sha1"):
            MDtool.sha1sum(args[0])
        elif o in ("-m", "--md5"):
            MDtool.md5sum(args[0])
        else:
            usage()
            sys.exit(2)

if __name__ == "__main__":
    main()
