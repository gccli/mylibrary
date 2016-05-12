systemd
=======

[systemd documents][1]

systemd is a system and service manager for Linux

systemd provides aggressive parallelization capabilities, uses **socket and D-Bus** activation for starting services, offers on-demand starting of daemons, **keeps track of processes using Linux cgroups**, supports **snapshotting** and restoring of the system state, maintains mount and automount points and implements an elaborate transactional dependency-based service control logic.

[1]: http://0pointer.de/blog/projects/systemd-docs.html


Concepts
--------
systemd provides a dependency system between various entities called **units** of 12 different types.
