#/bin/bash
APP_HOME="/opt/calmonitor"
EXEC_INTERVAL=5

checkpid() {
    pid=`ps -ef|grep malserl|grep -v grep |awk '{print $2}'`
    if [ "x$pid" != "x" ]; then 
        kill -9 $pid 
    fi
}

start() {  
    checkpid
    while [ 1 ] 
    do 
    {
	${APP_HOME}/bin/malserl /dev/ttyS0
        sleep ${EXEC_INTERVAL}
    }
    done
}     
stop() {  
    checkpid  
}
status() {  
    pid=`ps -ef|grep malserl|grep -v grep | awk '{print $2}'`
    if [ "x$pid" != "x" ];  then  
       echo "Program is running! (pid=$psid)"  
    else 
        echo "Program is not running"  
    fi  
}
case "$1" in  
       'start')  
          start  
          ;;  
       'stop')  
         stop  
         ;;  
       'restart')  
         stop  
         start  
         ;;  
       'status')  
         status  
         ;;  
       'info')  
         info  
         ;;  
      *)  
         echo "Usage: $0 {start|stop|restart|status|info}"  
         exit 1  
    esac  
    exit 0  
