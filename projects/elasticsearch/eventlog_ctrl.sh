#! /bin/bash

ES_HOME=/opt/elk
E_VER=2.3.2
L_VER=2.3.2
K_VER=4.5.0

E_PATH=elasticsearch-$E_VER
K_PATH=kibana-$K_VER-linux-x64

CMD=$1

sudo mkdir -p $ES_HOME -m 0777
sudo chown -R $USER $ES_HOME

cp eventlog_import.py $ES_HOME

function download() {
    cd $ES_HOME
    if [ ! -f elasticsearch-$E_VER.tar.gz ]; then
        wget https://download.elastic.co/elasticsearch/release/org/elasticsearch/distribution/tar/elasticsearch/$E_VER/$E_PATH.tar.gz
        [ $? -ne 0 ] && exit 1
    fi

    if [ ! -f kibana-$K_VER-linux-x64.tar.gz ]; then
        wget https://download.elastic.co/kibana/kibana/$K_PATH.tar.gz
        [ $? -ne 0 ] && exit 1
    fi

    if [ ! -f logstash-$L_VER.tar.gz ]; then
        wget https://download.elastic.co/logstash/logstash/logstash-$L_VER.tar.gz
        [ $? -ne 0 ] && exit 1
    fi

}

function install_elasticsearch() {
    cd $ES_HOME
    echo "Install $E_PATH and plugins ..."

    if [ ! -r $E_PATH ]; then
        echo "decompress tar and install $E_PATH ..."
        tar xzvf $E_PATH.tar.gz
        [ $? -ne 0 ] && exit 1
    fi

    cd $E_PATH
    bin/plugin list | grep license >/dev/null 2>&1
    [ $? -ne 0 ] && bin/plugin install license && [ $? -ne 0 ] && exit 1

    bin/plugin list | grep graph >/dev/null 2>&1
    [ $? -ne 0 ] && bin/plugin install graph &&   [ $? -ne 0 ] && exit 1

    bin/plugin list | grep graph >/dev/null 2>&1
    [ $? -ne 0 ] && bin/plugin install marvel-agent && [ $? -ne 0 ] && exit 1


    echo
    bin/plugin list
}

function install_kibana() {
    cd $ES_HOME
    echo "Install $K_PATH and plugins ..."

    if [ ! -r $K_PATH ]; then
        tar xzvf $K_PATH.tar.gz
        [ $? -ne 0 ] && exit 1
    fi

    cd $K_PATH
    bin/kibana plugin -l | grep graph >/dev/null 2>&1
    [ $? -ne 0 ] && bin/kibana plugin -i elasticsearch/graph/$E_VER && [ $? -ne 0 ] && exit 1

    bin/kibana plugin -l | grep graph >/dev/null 2>&1
    [ $? -ne 0 ] && bin/kibana plugin -i elasticsearch/marvel/$E_VER && [ $? -ne 0 ] && exit 1

    echo "Installed plugins in /opt/elk/$K_PATH"
    bin/kibana plugin -l
}


function install() {
    install_elasticsearch
    install_kibana

#    if [ ! -r logstash-$L_VER ]; then
#        tar xzvf logstash-$L_VER.tar.gz
#        [ $? -ne 0 ] && exit 1
#    fi
}


function start_es() {
    cd $ES_HOME/$E_PATH
    count=3
    restart=0

    echo "Start $count elasticsearch nodes..."
    while [ $count -ne 0 ]; do
        count=$(($count-1))
        if [ -f data/$count.pid ]; then
            [ $restart -eq 0 ] && continue

            echo "elasticsearch node $count already started"
            kill $(cat data/$count.pid)
            while [ /bin/true ]; do
                sleep 0.5
                [ ! -f data/$count.pid ] && break;
                kill -0 $(cat data/$count.pid)
                [ $? -eq 0 ] && break
                echo "wait elasticsearch node $count quit"
            done
        fi
        echo "Start elasticsearch node $count"
        bin/elasticsearch -d -p data/$count.pid
    done
}

function start_kibana() {
    cd $ES_HOME/$K_PATH
    bin/kibana
}

function start_eventlog() {
    cd $ES_HOME
    echo "Start eventlog collector ..."
    if [ -f eventlog.pid ]; then
        # kill sudo process
        sudo kill $(cat eventlog.pid)
        sleep 1
    fi

    # use sudo since pcap need root permission
    sudo python ./eventlog_import.py &
    echo $! > eventlog.pid
    ps -ef | grep python
}

function start() {
    start_es
    start_eventlog
    start_kibana
}

[ -z "$1" ] && echo "Usage $0 <download|install|start>" && exit 0

$CMD
