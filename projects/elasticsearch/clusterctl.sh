#! /bin/bash

CLUSTER_PATH=/opt/cluster
CLUSTER_CONF=$CLUSTER_PATH/elasticsearch.yml

ELASTICSEARCH=/elasticsearch/bin/elasticsearch
OPTS="-Des.config=/data/elasticsearch.yml"
IMAGE="lijing/e4"
K4IMAGE="lijing/k4"

# https://registry.hub.docker.com/u/dockerfile/elasticsearch
# echo -e "path:\n  logs: /data/log\n  data: /data/data" > /opt/cluster/elasticsearch.yml
#IMAGE="dockerfile/elasticsearch"

function nodeadd {
    local n=$1
    local f=$2
    local CMD="docker.io run -d -h node$n --name node$n -v /opt/cluster/$n:/data"
    if [ -n "$f" -a "$f" == "master" ]; then
	CMD="$CMD -p 9200:9200 -p 9300:9300"
    fi
    CMD="$CMD $IMAGE $ELASTICSEARCH"
    echo $CMD
    $CMD
}

function nodectl {
    local cmd=$1
    shift
    for n in $@
    do
	local CMD="docker.io $cmd node$n"
	echo $CMD
	$CMD
    done
}

function find_all {
    local all=''
    for i in `ls $CLUSTER_PATH`
    do
	[ ! -d $CLUSTER_PATH/$i ] && continue
	all="$all $i"
    done
    echo $all
}

function find_unused_dir {
    local max=-1
    for i in `ls $CLUSTER_PATH`
    do
	[ ! -d $CLUSTER_PATH/$i ] && continue
	if [ $i -gt $max ]; then 
	    max=$i
	fi
    done
    echo $(($max+1))
}

function stopall {
    local all=$(find_all)
    nodectl stop $all
}
function startall {
    local all=$(find_all)
    nodectl start $all
}

if [ -n "$1" ]; then
    case $1 in
	add)
	    num=$(find_unused_dir)
	    dir=$CLUSTER_PATH/$num
	    mkdir $dir
	    if [ $? -eq 0 ]; then
		ln -f $CLUSTER_CONF $dir/
		if [ $num -eq 0 ]; then
		    nodeadd $num master
		else
		    nodeadd $num
		fi
		echo "success add node to $dir"
	    fi
	    ;;
	destroy)
	    stopall
	    for i in `ls $CLUSTER_PATH`
	    do
		[ ! -d $CLUSTER_PATH/$i ] && continue
		rm -rf $CLUSTER_PATH/$i
	    done	    

	    ids=$(docker.io ps -a | grep elastic | awk '{ print $1 }')
	    for i in $(echo $ids)
	    do
		docker.io rm -f $i
	    done
            ;;
	stop)
	    stopall
            ;;
	start)
	    startall
            ;;
	shell)
	    docker.io run --rm -it $IMAGE /bin/bash
	    exit 0
	    ;;
esac    
fi

sleep 1 && docker.io ps
