#! /usr/bin/env python

import os
import sys
import time
import string
import stat
import subprocess
import threading

remote_hosts = {
    'mail':"10.227.1.210",
    'rhel':"10.227.1.224",
    'samba4':"10.227.1.207",
    'ldap1':"10.227.1.202",
    'centos0':"10.227.1.214",
    'centos1':"10.227.1.208",
    'centos2':"10.227.1.209",
    'centos3':"10.227.1.248",
    'centos4':"10.227.1.121"
}

remote_hosts = { 'remote':'10.228.254.119' }

local_ipaddr = '10.227.1.234'
local_depot = '/root/mylibrary'
local_script= '/tmp/distributed_customize.sh'
remote_dest = '/tmp/mylibrary'
ssh_options = '-oStrictHostKeyChecking=no -oUserKnownHostsFile=/dev/null'

def write_script():
    rmdir='[ -d %s ] && rm -rf %s\n'%(remote_dest,remote_dest)
    sshexec='scp %s -pr %s:%s %s >/tmp/scpcopylist.txt && cd %s && ./customize.sh\n' % (
        ssh_options,local_ipaddr,local_depot,remote_dest,remote_dest)

    with open(local_script, 'w') as fp:
        fp.write('#! /bin/bash\n\n')
        fp.write(rmdir)
        fp.write(sshexec)
        fp.write(rmdir)
        fp.write('rm -f %s\n' % local_script)

    os.chmod(local_script, stat.S_IREAD|stat.S_IEXEC)

def config_host(*args):
    thr=threading.current_thread()
    host=args[0]
    logstr=''

    print 'Thread %s started, copy script to remote host' % thr.name
    command='scp -p %s %s %s:%s' % (ssh_options, local_script, host, local_script)
    p = subprocess.Popen(command.split(' '), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    outstr,err = p.communicate()
    if p.returncode != 0:
        print outstr
        return False

    command='ssh %s %s %s' % (ssh_options,host,local_script)
    command_list=command.split(' ')
    print 'Run script on remote host [ %8s - %s ]' % (thr.name,host)

    p = subprocess.Popen(command_list, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    outstr,err = p.communicate()
    if p.returncode != 0:
        print 'error in thread %s - %s' % (thr.name,outstr)
        return False

    print 'Thread %s finished on %s'%(thr.name,host)
    return True

if __name__ == '__main__':
    write_script()
    for k,v in remote_hosts.items():
        th = threading.Thread(target=config_host,name=k,args=(v,))
        th.start()

    time.sleep(2)
    while threading.active_count() > 1:
        time.sleep(0.5)
