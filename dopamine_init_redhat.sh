#!/bin/bash
#
# chkconfig: - 85 15
# description: Dopamine is a document-oriented PACS
# processname: dopamine
# config: /etc/dopamine/dopamine_conf.ini
# pidfile: /var/run/dopamine/dopamine.pid

# Source function library.
. /etc/init.d/functions

# Source networking configuration.
. /etc/sysconfig/network

# Check that networking is up.
[ "$NETWORKING" = "no" ] && exit 0

dopamine="/usr/local/bin/dopamine"
prog=$(basename $dopamine)
lockfile=/var/lock/subsys/${prog}
pidfile=/var/run/dopamine/dopamine.pid

start() {
        echo -n "Starting ${prog}: "
        if [ -s ${pidfile} ]
        then
            retval=1
            echo -n "Already running !" && warning
            echo
        else
            nohup ${dopamine} > /var/log/dopamine.log &
            retval=$?
            pid=$!
            [ ${retval} -eq 0 ] && touch ${lockfile} && success || failure
            echo ${pid} > ${pidfile}
        fi
        return $retval
}

stop() {
        echo -n "Shutting down ${prog}: "
        killproc -p ${pidfile} ${prog}
        retval=$?
	echo
        [ $retval -eq 0 ] && touch $lockfile
        return $retval
}

case "$1" in
    start)
        start
        ;;
    stop)
        stop
        ;;
    status)
        status ${prog}
        ;;
    restart)
        stop
        start
        ;;
    reload)
        stop
        start
        ;;
    condrestart)
        [ -f ${lockfile} ] && restart || :
        ;;
esac
exit $?
