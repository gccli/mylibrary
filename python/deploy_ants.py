#! /usr/bin/env python

import os
import sys
import time
import string
import re
import fnmatch
import getopt
import shutil
import subprocess

ants_process_number = 12
ants_loglevel = 'DEBUG'
cca_loglevel = {'Framework':1, 'Feedback':2}

def copy_libs():
    dstdir = '%s/lib' % dest_prefix

    tmpdir = '%s/package/lib' % cca_src
    shutil.copytree(tmpdir, dstdir)

    tmpdir = '%s/bin/X64_LINUX_dbg' % cca_src
    for s in os.listdir(tmpdir):
        if fnmatch.fnmatch(s, 'libCCA*'):
            d = '%s/%s' % (dstdir,s)
            s = '%s/%s' % (tmpdir,s)
            print 'link %s to %s' % (s,d)
            os.link(s,d)


    cwd = os.getcwd()
    os.chdir(dest_libdir)
    for s in os.listdir('.'):
        if fnmatch.fnmatch(s, '*_debug.so'):
            d = string.replace(s, '_debug.so', '.so')
            os.symlink(s, d)

    os.chdir(cwd)

def copy_config():
    dstdir = '%s/config' % dest_prefix
    tmpdir = '%s/package/config' % cca_src
    shutil.copytree(tmpdir, dstdir)

    tmpdir = '%s/config/analytics.esg.config' % cca_src
    shutil.copy(tmpdir, dest_cca_config)
    shutil.copy(ants_config, dest_config)
    os.remove('%s/make-analytics.wcg.config' % dest_confdir)
    os.remove('%s/ant_server.conf' % dest_confdir)

def config_ants():
    tmpfile = dest_prefix + '/ants_server'
    os.link(ants_binary, tmpfile)

    tmpfile = '%s/defaultDatabases/binary' % cca_src
    os.symlink(tmpfile, dest_dbdir)

    with open('%s/.antsrc' % dest_prefix , 'w') as fp:
        fp.write('export MY_ANTS_DEBUG=1\n')
        libs='%s:%s/Authentium' % (dest_libdir,dest_libdir)
        fp.write('export LD_LIBRARY_PATH=%s\n' % libs)
        fp.close()
        
    with open('/tmp/config_ants.sh', 'w') as fp:
        fp.write('sed -i "s#/var/lib/CCA#%s#" %s\n' % (dest_prefix,dest_config))
        fp.write('sed -i "s#<Analysis_Processes>[0-9]*#<Analysis_Processes>%d#" %s\n' % (ants_process_number,dest_config))
        fp.write('sed -i "s#<Log_Level>.*<#<Log_Level>%s<#" %s\n' % (ants_loglevel,dest_config))
        fp.write('sed -i "s#^MagicDbPath.*#MagicDbPath = %s/magic.mgc#" %s\n' % (dest_confdir,dest_cca_config))
        fp.write('sed -i "s#^AEMapPath.*#AEMapPath = %s/AEMap.config#" %s\n' % (dest_confdir,dest_cca_config))

        for k,v in cca_loglevel.items():
            fp.write('sed -i "/^\[%s/{N;N;N;s/loglevel.*\\n/loglevel = %d\\n/}" %s\n' % (k,v,dest_cca_config));

        fp.close()
    print 'Run script "/tmp/config_ants.sh" to config analytic server'
    subprocess.call(['sh', '/tmp/config_ants.sh'])

def usage():
    print '%s [ -fv ] [ -s=dir ] [ -d dir ]' % sys.argv[0]
    sys.exit(1)

if __name__=='__main__':
    if len(sys.argv) < 2:
        usage()
        sys.exit(1)

    cca_src = '%s/src' % os.environ['P4ROOT']
    dest_prefix=''
    verbose=0
    force=False
    opts, args = getopt.getopt(sys.argv[1:], "s:d:fv")
    for o, a in opts:
        if o == "-s":
            cca_src = a
        elif o == "-d":
            dest_prefix = a
        elif o == "-v":
            verbose += 1
        elif o == "-f":
            force = True
        else:
            usage()

    if not os.access(cca_src, os.X_OK):
        print 'CCA source directory "%s" not exists' % cca_src
        sys.exit(1)

    if os.access(dest_prefix, os.F_OK):
        if force:
            shutil.rmtree(dest_prefix)
            os.mkdir(dest_prefix)
        else:
            yes = raw_input('File "%s" exists, remove it? (y/n)')
            if (yes == 'y'):
                shutil.rmtree(dest_prefix)
                os.mkdir(dest_prefix)
            else:
                sys.exit(0)
    else:
        os.mkdir(dest_prefix)

    if not os.access(dest_prefix, os.F_OK):
        print 'Please specify analytic server directory'
        sys.exit(1)

    print 'Begin to deploy analytic server to "%s" ...' % dest_prefix

    ants_src = '%s/analytic_server' % cca_src
    ants_binary = '%s/server/ants_server' % ants_src
    ants_config = '%s/tools/ant_server.conf' % ants_src

    dest_dbdir = '%s/databases' % dest_prefix
    dest_confdir = '%s/config' % dest_prefix
    dest_libdir = '%s/lib' % dest_prefix
    dest_config = '%s/server.xml' % dest_prefix
    dest_cca_config = '%s/analytics.config' % dest_confdir

    copy_libs()
    copy_config()
    config_ants()
