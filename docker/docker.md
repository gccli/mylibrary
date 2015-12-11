Introductioin & Concepts
========================

Docker is a platform for developers and sysadmins to develop, ship, and run applications.
Docker consists of:
    The Docker Engine - our lightweight and powerful open source container virtualization technology combined with a work flow for building and containerizing your applications.
    Docker Hub - our SaaS service for sharing and managing your application stacks

cgroups - control groups
------------------------
cgroups is a Linux kernel feature that limits, accounts for, and isolates the resource usage (CPU, memory, disk I/O, network, etc.) of a collection of processes. Docker relies on cgroups to control and isolate resource limits.

Containers
----------
A container is a runtime instance of a docker image.
Docker containers wrap up a piece of software in a complete filesystem that contains everything it needs to run: code, runtime, system tools, system libraries – anything you can install on a server.
Containers running on a single machine all share the same operating system kernel so they start instantly and make more efficient use of RAM.

### Data volumes ###
A data volume is a specially-designated directory within one or more containers that bypasses the Union File System. Data volumes are designed to persist data, independent of the container’s life cycle. Docker therefore never automatically delete volumes when you remove a container, nor will it “garbage collect” volumes that are no longer referenced by a container.

### Union file systems ###
UnionFS, are file systems that operate by creating layers, making them very lightweight and fast. Docker uses union file systems to provide the building blocks for containers.


Images
------
Docker images are the basis of containers. An Image is an ordered collection of root filesystem changes and the corresponding execution parameters for use within a container runtime. An image typically contains a union of layered filesystems stacked on top of each other. An image does not have state and it never changes.
Images are constructed from layered filesystems so they can share common files, making disk usage and image downloads much more efficient.


Dockerfile
==========
A [Dockerfile][1] is a text document that contains all the commands you would normally execute manually in order to build a Docker image.
[1]: https://docs.docker.com/articles/dockerfile_best-practices
[2]: http://docs.docker.com/engine/reference/builder/
Docker can build images automatically by reading the instructions from a Dockerfile

Recommendations
---------------

1. Containers should be ephemeral
   By "ephemeral", we mean that it can be stopped and destroyed and a new one built and put in place with an absolute minimum of set-up and configuration.
2. Avoid installing unnecessary packages
3. Run only one process per container
4. Minimize the number of layers

Instructions
------------

See [Dockerfile reference][2] for details

### RUN ###
`RUN <command>` (shell form, the command is run in a shell - /bin/sh -c)
`RUN ["executable", "param1", "param2"]` (exec form)


### CMD ###
`CMD ["executable","param1","param2"]` (exec form, this is the preferred form)
`CMD ["param1","param2"]` (as default parameters to ENTRYPOINT)
`CMD command param1 param2` (shell form)

There can only be one `CMD` instruction in a Dockerfile. If you list more than one CMD then only the last CMD will take effect.

**The main purpose of a CMD is to provide defaults for an executing container.**

> If CMD is used to provide default arguments for the ENTRYPOINT instruction, both the CMD and ENTRYPOINT instructions should be specified with the JSON array format.


### LABEL ###

`LABEL <key>=<value> <key>=<value> <key>=<value> ...`
The `LABEL` instruction adds metadata to an image. A `LABEL` is a key-value pair. To include spaces within a `LABEL` value, use quotes and backslashes as you would in command-line parsing.


### EXPOSE ###
`XPOSE <port> [<port>...]`
The `EXPOSE` instruction informs Docker that the container listens on the specified network ports at runtime. `EXPOSE` does not make the ports of the container accessible to the host. To do that, you must use either the `-p` flag to publish a range of ports or the `-P` flag to publish all of the exposed ports. You can expose one port number and publish it externally under another number.

To set up port redirection on the host system, see using the `-P` flag. The Docker network feature supports creating networks without the need to expose ports within the network, for detailed information see the overview of this feature).

### ENV ###
    ENV <key> <value>
    ENV <key>=<value> ...

The ENV instruction sets the environment variable <key> to the value <value>. This value will be in the environment of all “descendent” Dockerfile commands and can be replaced inline in many as well.

The ENV instruction has two forms. The first form, ENV <key> <value>, will set a single variable to a value. The entire string after the first space will be treated as the <value> - including characters such as spaces and quotes.

The second form, ENV <key>=<value> ..., allows for multiple variables to be set at one time. Notice that the second form uses the equals sign (=) in the syntax, while the first form does not. Like command line parsing, quotes and backslashes can be used to include spaces within values.


### ADD ###
`ADD <src>... <dest>`
`ADD ["<src>",... "<dest>"]` (this form is required for paths containing whitespace)

The `ADD` instruction copies new files, directories or remote file URLs from `<src>` and adds them to the filesystem of the container at the path `<dest>`.

`ADD` obeys the following rules:
* The `<src>` path must be inside the context of the build; because the first step of a docker build is to send the context directory (and subdirectories) to the docker daemon.
* If `<src>` is a directory, the entire contents of the directory are copied, including filesystem metadata.
* If `<src>` is a local tar archive in a recognized compression format (identity, gzip, bzip2 or xz) then it is unpacked as a directory.


### USER ###

`USER daemon`
The USER instruction sets the user name or UID to use when running the image and for any RUN, CMD and ENTRYPOINT instructions that follow it in the Dockerfile.


[Install in Ubuntu][3]
======================
[3]: https://docs.docker.com/engine/installation/ubuntulinux/

## Update your apt sources ##

1. Add the new gpg key.
sudo apt-key adv --keyserver hkp://p80.pool.sks-keyservers.net:80 --recv-keys 58118E89F3A912897C070ADBF76221572C52609D
2. Add following lines to `/etc/apt/sources.list.d/docker.list` for Ubuntu Wily 15.10
    deb https://apt.dockerproject.org/repo ubuntu-wily main

sudo apt-get update
sudo apt-get install linux-image-extra-$(uname -r)
sudo apt-get install docker-engine


### Create a Docker group ###

To avoid having to use sudo when you use the `docker` command, create a Unix group called `docker` and add users to it. When the `docker` daemon starts, it makes the ownership of the Unix socket read/writable by the `docker` group.
    sudo usermod -aG docker lijing


Registry
========
[daocloud]: http://www.oschina.net/news/57894/daocloud
A registry is a storage and content delivery system, holding named Docker images.

    docker run -d -p 5000:5000 --restart=always --name registry -v $HOME/tmp/notes/certs:/certs -e REGISTRY_HTTP_TLS_CERTIFICATE=/certs/server.pem -e REGISTRY_HTTP_TLS_KEY=/certs/serverkey.pem registry:2


Command line
============

Container
---------

   docker run
   -p flag will bind the specified port to all interfaces on the host machine. But you can also specify a binding to a specific interface
   --name flag, name your container by using the

   docker ps -l
 + An Interactive Container
   docker run -t -i ubuntu:14.04 /bin/bash
   -i, --interactive=false    Keep stdin open even if not attached
   -t, --tty=false            Allocate a pseudo-tty

### docker logs ###

    -f, --follow=false        Follow log output
    -t, --timestamps=false    Show timestamps

### Looking at Container's processes ###

    $ docker top zk0

### Inspecting our Web Application Container ###

    $ docker inspect zk0
    $ docker inspect -f '{{ .NetworkSettings.IPAddress }}' zk0

Linking Containers
------------------

   --link=[]                  Add link to another container (name:alias)

   docker run --name my-mysql -e MYSQL_ROOT_PASSWORD=my-mysqlpw -d mysql
   # Connect to MySQL from an application in another Docker container
   docker run --name my-app --link my-mysql:mysql -d app-that-uses-mysql
   # Connect to MySQL from the MySQL command line client
   docker run -it --link my-mysql:mysql --rm mysql sh -c 'exec mysql -h"$MYSQL_PORT_3306_TCP_ADDR" -P"$MYSQL_PORT_3306_TCP_PORT" -uroot -p"$MYSQL_ENV_MYSQL_ROOT_PASSWORD"'
   # Container shell access
   The docker exec command allows you to run commands inside a Docker container
   docker exec -it my-mysql bash

   + Environment Variables
     the ENV commands in the source container's Dockerfile
     the -e, --env and --env-file options on the docker run command when the source container is started

** Managing Data in Containers

Docker Registry
===============
The Registry is a stateless, highly scalable server side application that stores and lets you distribute Docker images.
* tightly control where your images are being stored
* fully own your images distribution pipeline
* integrate image storage and distribution tightly into your in-house development workflow
