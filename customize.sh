#! /bin/bash


################################################################################
############            Variables used in this script            ###############
# ChangeList
#
# 1.5.3 optimize cc-mode of emacs

VERSION=1.5.3
WATERMARK="generated by customize.sh"
WATERMARK_END="--------------------------------"

OPT_VERBOSE=0
OPT_CFGSSH=1
OPT_CFGNETWORK=0

DISTRIB_ID=$(lsb_release -i | sed 's/.*:[ \t]*//')
DISTRIB_RELEASE=$(lsb_release -r | sed 's/.*:[ \t]*//')

function die() {
    echo -e "\033[31mError: $1\033[0m"
    exit 1
}

[ -z "$DISTRIB_ID" ] && die "Cannot get distribution ID"

MYLOG=/tmp/customize.log
> $MYLOG

BASHRC=.bashrc
BASHFUNC=.functions
BASHALIASES=.bash_aliases
PROFILE=.bash_profile
if [ $DISTRIB_ID == "Ubuntu" ]; then
    PROFILE=.profile
fi

FULLPATH_BASHRC=$HOME/$BASHRC
FULLPATH_PROF=$HOME/$PROFILE
FULLPATH_ALIAS=$HOME/$BASHAILASES
FULLPATH_FUNC=$HOME/$BASHFUNC

[ ! -f $FULLPATH_BASHRC ] && die "$FULLPATH_BASHRC not exists"
########## END ##########

################################################################################
##################               Functions                  ####################

function usage {
    printf "Usage: $0 [ -vh ] [ --no-ssh ]\n"

    exit 0
}


function err() {
    echo -e "\033[31mError: $1\033[0m" | tee -a $MYLOG
}

function warn() {
    echo -e "\033[33mWarn: $1\033[0m" | tee -a $MYLOG
}

function notice() {
    echo -e "\033[32m$1\033[0m" | tee -a $MYLOG
}

function copy_files()
{
    notice "copy files to $HOME"
    /bin/cp -f temp/{.functions,.bash_aliases,.emacs,.gdbinit} ~/
    if [ $? -ne 0 ]; then
        echo "error occur during copy files"
        exit 1
    fi
}

function config_emacs() {
    local version=$(emacs --version | sed -n '1p' | sed -n 's|^GNU Emacs \([0-9.]*\)|\1|p')
    local major=$(echo $version | cut -d. -f1)
    mkdir -p ~/.emacs.d
    cp -f temp/php-mode.el ~/.emacs.d
    notice "config emacs $version"
}

function config_bash()
{
    notice "config bash $BASH_VERSION"
    isnew=0
    renew=0
    version=

    grep "$WATERMARK" $FULLPATH_BASHRC > /dev/null
    if [ $? -ne 0 ]; then
        notice "$FULLPATH_BASHRC not configured"
        isnew=1
    else
	version=$(grep "$WATERMARK" $FULLPATH_BASHRC | sed -ne 's/.*version://p')
        warn "$FULLPATH_BASHRC already configured, version is $version"
        if [ "$version" != $VERSION ]; then
            notice "version changed to $VERSION, $FULLPATH_BASHRC need to reconfigure"
            renew=1
            sed -i "/$WATERMARK/,/$WATERMARK_END/d" $FULLPATH_BASHRC
        fi
    fi

    if [ $isnew -eq 1 -o $renew -eq 1 ]; then
	echo "# $WATERMARK @time:$(date +%D) @version:$VERSION" >> $FULLPATH_BASHRC
        echo . ~/$BASHFUNC >> $FULLPATH_BASHRC
        if [ $DISTRIB_ID == "CentOS" ]; then
            echo . ~/.bash_aliases >> $FULLPATH_BASHRC
        fi

        if [ $OPT_CFGSSH -gt 0 ]; then
            # ssh agnent startup
            cat temp/ssh-agent.bashrc >> $FULLPATH_BASHRC
        fi
        echo "# $WATERMARK_END" >> $FULLPATH_BASHRC
    fi
    #sed -ne '/generated/,/$WATERMARK_END/p' $FULLPATH_BASHRC
}

function config_network()
{
    local value=
    local reject=
    local hostname=
    local ip=
    local prefixlen=24
    local gateway=
    local dnsserver=
    local dnssearch=

    [ $OPT_CFGNETWORK -eq 0 ] && return

    read -p "Input the hostname: " hostname
    read -p "Using DHCP boot method for ip assign (y/n): " value
    if [ "$value" == "y" ]; then
	read -p "Input the reject ip or subnet, such as 192.168.0.0/16, 10.0.0.5: " reject

        echo
        echo ----------------
        echo hostname $hostname
        echo reject $reject

    else
	read -p "Input the ip address: " ip
	read -p "Input the prefix length, default is 24: " value
	[ ! -z "$value" ] && prefixlen=$value
	read -p "Input the default gateway: " gateway
	read -p "Input the dns server: " dnsserver
	read -p "Input the dns search: " dnssearch

	echo
	echo ----------------
	echo hostname $hostname
	echo ip $ip
	echo prefixlen $prefixlen
	echo gateway $gateway
	echo dnsserver $dnsserver
	echo dnssearch $dnssearch
    fi

    exit 1
}

function config_ldap()
{
    local value=
    local binddn=
    local hostname=
    local ip=
    local prefixlen=24
    local gateway=
    local dnsserver=
    local dnssearch=

    [ $OPT_CFGLDAP -eq 0 ] && return

    read -p "Input bind DN: " binddn
    if [ -z "$binddn" ]; then
        echo $binddn > ~/.ldaprc
    fi
    read -p "Using DHCP boot method for ip assign (y/n): " value
    if [ "$value" == "y" ]; then
	read -p "Input the reject ip or subnet, such as 192.168.0.0/16, 10.0.0.5: " reject

        echo
        echo ----------------
        echo hostname $hostname
        echo reject $reject

    else
        read -p "Input the ip address: " ip
        read -p "Input the prefix length, default is 24: " value
        [ ! -z "$value" ] && prefixlen=$value
    fi
}

function config_git()
{
    version=
    which git 2>&1 >/dev/null
    if [ $? -eq 0 ]; then
        notice "config git"
        source temp/git.config
    fi
}

function copy_sshkeys()
{
    TYPE=$1
    SRC=temp/RSA_key
    DST=$HOME/.ssh/id_rsa
    AUTHORIZED=$HOME/.ssh/authorized_keys
    if [ "$1" == "DSA" ]; then
	SRC=temp/DSA_key
	DST=$HOME/.ssh/id_dsa
    fi

    mkdir -p $HOME/.ssh
    if [ -f $DST ]; then
	diff $SRC $DST >/dev/null 2>&1
	if [ $? -ne 0 ]; then
	    warn "Backup $DST"
	    mkdir -p $HOME/.ssh/bak
	    mv $DST $HOME/.ssh/bak
	else
	    warn "Identical $TYPE key"
	fi
    else
	cp $SRC $DST
    fi

    grep "`cat $SRC.pub`" $AUTHORIZED >/dev/null 2>&1
    if [ $? -eq 0 ]; then
	warn "$TYPE public key already add to authorized_keys"
    else
	cat $SRC.pub >> $AUTHORIZED
    fi
    chmod 400 $DST

#    shopt -s nocasematch
#    shopt -u nocasematch
}

function config_sshd()
{
    [ $OPT_CFGSSH -eq 0 ] && return
    if [ ! -d ~/.ssh ]; then
	warn "No .ssh directory"
	return
    fi

    cp temp/.ssh/config ~/.ssh/

    SERVICE_NAME=ssh
    if [ $DISTRIB_ID == "CentOS" ]; then
	SERVICE_NAME=sshd
    fi

    if [ $UID -eq 0 ]; then
	notice "config ssh server"
	grep '^UseDNS' /etc/ssh/sshd_config > /dev/null
	if [ $? -ne 0 ]; then
	    echo "UseDNS no" >> /etc/ssh/sshd_config
	    service $SERVICE_NAME restart
	else
	    warn "ssh server already configure"
	fi
    fi
    # copy ssh keys
    # copy_sshkeys RSA
    copy_sshkeys DSA
}
########## END ##########


################################################################################
##################           Parse Command Line             ####################

if ! my__options=$(getopt -u -o vh -l no-ssh,help,with-network -- "$@")
then
    exit 1
fi
set -- $my__options
while [ $# -gt 0 ]
do
    case $1 in
        -h|--help)
            usage;;
        -v)
            OPT_VERBOSE=$(($OPT_VERBOSE+1));
            shift;;
        --no-ssh)
            OPT_CFGSSH=0;
            shift;;
        --with-network)
            OPT_CFGNETWORK=1;
            shift;;
        (--) shift; break;;
        (-*) echo "error - unrecognized option $1" 1>&2; exit 1;;
        (*) usage;;
    esac
    shift
done
########## END ##########
echo "Customizing for $DISTRIB_ID $DISTRIB_RELEASE" | tee $MYLOG

copy_files
config_emacs
config_bash
config_git
config_sshd

# done
source $FULLPATH_BASHRC
source $FULLPATH_PROF
