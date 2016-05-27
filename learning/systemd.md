systemd
=======

[systemd documents][1]

systemd is a system and service manager for Linux

systemd provides aggressive parallelization capabilities, uses **socket and D-Bus** activation for starting services, offers on-demand starting of daemons, **keeps track of processes using Linux cgroups**, supports **snapshotting** and restoring of the system state, maintains mount and automount points and implements an elaborate transactional dependency-based service control logic.

[1]: http://0pointer.de/blog/projects/systemd-docs.html


Concepts
--------
systemd provides a dependency system between various entities called **units** of 12 different types.
systemctl -t <automount|busname|device|mount||path|scope|service|slice|socket|swap|target|timer>

1.  *Service* - which start and control daemons and the processes
2.  *Socket*  - which encapsulate local IPC or network sockets in the system,
3.  *Target*  - which useful to group units, or provide well-known synchronization points during boot-up
4.  *Device*  - which expose kernel devices in systemd and may be used to implement device-based activation
5.  *Mount*   - which control mount points in the file system
6.  *Automount* which provide automount capabilities, for on-demand mounting of file systems as well as parallelized boot-up
7.  *Snapshot*  which can be used to temporarily save the state of the set of systemd units
8.  *Timer*   - which are useful for triggering activation of other units based on timers.
9.  *Swap*    - which encapsulate memory swap partitions or files of the operating system.
10. *Path*    - may be used to activate other services when file system objects change or are modified.
11. *Slice*   - which manage system processes (such as service and scope units) in a hierarchical tree for resource management purposes
12. *Scope*   - similar to service units, but manage foreign processes instead of starting them as well


### cgroups
*cgroups* can be used as an effective way to label processes after the service they belong to and be sure that the service cannot escape from the label, regardless how often it forks or renames itself. Furthermore this can be used to safely kill a service and all processes it created, again with no chance of escaping.

show cgroup information along the other process details
    ps xawf -eo pid,user,cgroup,args
    systemd-cgls


systemd usage
-------------


### systemd ###
Check systemd variables
    pkg-config systemd --variable=systemduserunitpath
    pkg-config systemd --variable=systemdsystemunitpath
    pkg-config systemd --variable=systemdsystemunitdir


### systemctl ###


* Three Levels of turn off



reload all unit
    systemctl daemon-reload
