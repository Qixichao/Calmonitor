#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>

#define FALSE  -1
#define TRUE   0
/*******************************************************************
* Name:                  serialOpen
* Feature:                Open Serial and return File description
* Parameter:        fd: File Descr     port: Serial Number(ttyS0,ttyS1,ttyS2)
* Output:        Success 1, Fail 0
*******************************************************************/
int serialOpen(int fd,char* port)
{

    fd = open( port, O_RDWR|O_NOCTTY|O_NDELAY);
    if (FALSE == fd)
    {
        perror("Can't Open Serial Port");
        syslog(LOG_ERR, "Can't Open Serial Port");
        return(FALSE);
    }
    if(fcntl(fd, F_SETFL, 0) < 0)
    {
        printf("fcntl failed!\n");
        syslog(LOG_ERR, "fcntl failed!\n");
        return(FALSE);
    }
    else
    {
        printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
        syslog(LOG_INFO, "fcntl=%d\n",fcntl(fd, F_SETFL,0));
    }
    if(0 == isatty(STDIN_FILENO))
    {
        printf("standard input is not a terminal device\n");
        syslog(LOG_INFO, "standard input is not a terminal device\n");
        return(FALSE);
    }
    else
    {
        printf("isatty success!\n");
	syslog(LOG_INFO, "isatty success!\n");
    }
    printf("fd->open=%d\n",fd);
    syslog(LOG_INFO, "fd->open=%d\n",fd);
    return fd;
}
/*******************************************************************
* Name:                serialClose
* Feature:                Close Serial
* Parameter:        fd: File Descr     port: Serial Port(ttyS0)
* Output:        void
*******************************************************************/

void serialClose(int fd)
{
    close(fd);
}

int SERL0_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,char parity)
{

    int   i;
    //int   status;
    int   speed_arr[] = { B115200, B19200, B9600, B4800, B2400, B1200, B300};
    int   name_arr[] = {115200,  19200,  9600,  4800,  2400,  1200,  300};

    struct termios options;

    if( tcgetattr( fd,&options)  !=  0)
    {
        perror("SetupSerial 1");
        return(FALSE);
    }

    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
    {
        if  (speed == name_arr[i])
        {
            cfsetispeed(&options, speed_arr[i]);
            cfsetospeed(&options, speed_arr[i]);
        }
    }

    options.c_cflag |= CLOCAL;
    options.c_cflag |= CREAD;

    switch(flow_ctrl)
    {

        case 0 :
              options.c_cflag &= ~CRTSCTS;
              break;

        case 1 :
              options.c_cflag |= CRTSCTS;
              break;
        case 2 :
              options.c_cflag |= IXON | IXOFF | IXANY;
              break;
    }
    options.c_cflag &= ~CSIZE;
    switch (databits)
    {
        case 5:
                 options.c_cflag |= CS5;
                 break;
        case 6:
                 options.c_cflag |= CS6;
                 break;
        case 7:
                 options.c_cflag |= CS7;
                 break;
        case 8:
                 options.c_cflag |= CS8;
                 break;
        default:
                 fprintf(stderr,"Unsupported data size %d\n", databits);
                 return (FALSE);
    }
    switch (parity)
    {
        case 'n':
        case 'N':
                 options.c_cflag &= ~PARENB;
                 options.c_iflag &= ~INPCK;
                 break;
        case 'o':
        case 'O':
                 options.c_cflag |= (PARODD | PARENB);
                 options.c_iflag |= INPCK;
                 break;
        case 'e':
        case 'E':
                 options.c_cflag |= PARENB;
                 options.c_cflag &= ~PARODD;
                 options.c_iflag |= INPCK;
                 break;
        case 's':
        case 'S':
                 options.c_cflag &= ~PARENB;
                 options.c_cflag &= ~CSTOPB;
                 break;
        default:
                 fprintf(stderr,"Unsupported parity\n");
                 return (FALSE);
    }
    switch (stopbits)
    {
        case 1:
                 options.c_cflag &= ~CSTOPB; break;
        case 2:
                 options.c_cflag |= CSTOPB; break;
        default:
                 fprintf(stderr,"Unsupported stop bits\n");
                 return (FALSE);
    }

    options.c_oflag &= ~OPOST;

    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    options.c_cc[VTIME] = 1;
    options.c_cc[VMIN] = 1;

    tcflush(fd,TCIFLUSH);

    if (tcsetattr(fd,TCSANOW,&options) != 0)
    {
        perror("com set error!\n");
        return (FALSE);
    }
    return (TRUE);
}

int serialInit(int fd, int speed,int flow_ctrl,int databits,int stopbits,char parity)
{
    if (SERL0_Set(fd,speed,flow_ctrl,databits,stopbits,parity) == FALSE)
    {
        return FALSE;
    }
    else
    {
        return  TRUE;
    }
}
/*
int SERL0_Recv(int fd, char *rcv_buf,int data_len)
{
    int len,fs_sel;
    fd_set fs_read;

    struct timeval time;

    FD_ZERO(&fs_read);
    FD_SET(fd,&fs_read);

    time.tv_sec = 10;
    time.tv_usec = 0;

    fs_sel = select(fd+1,&fs_read,NULL,NULL,&time);
    printf("fs_sel = %d\n",fs_sel);
    if(fs_sel)
    {
        len = read(fd,rcv_buf,data_len);
        printf("I am right!(version1.2) len = %d fs_sel = %d\n",len,fs_sel);
        return len;
    }
    else
    {
        printf("Sorry,I am wrong!");
        return FALSE;
    }
}
*/
int serialSend(int fd, char *send_buf,int data_len)
{
    int len = 0;

    len = write(fd,send_buf,data_len);
    if (len == data_len )
    {
        return len;
    }
    else
    {

        tcflush(fd,TCOFLUSH);
        return FALSE;
    }

}
/*
int sig_child(int signo)
{
    pid_t pid;
    int stat;

}
*/
int main(int argc, char **argv)
{
    int fd=0;
    int err;
    int len;
    char send_buf[1024];
    char comm[1024];
    int serl_speed=9600;
    int serl_flow_ctrl=0;
    int serl_databit=8;
    int serl_stopbits=1;
    int exec_interval=0;
    char serl_parity[1]="N";
    char line[1024];
    FILE *cfile = fopen("/opt/calmonitor/etc/config.properties", "r");
    while(fgets(line, 100, cfile))
    {
        char *key;
        char *value;
        char *temp;
        temp=strtok(line, "=");
    	if(temp) key=temp;
    	temp=strtok(NULL, "=");
    	if(temp) value=temp;
 	syslog(LOG_INFO, "Get Parameter %s value is %s.", key, value);
        if(strcmp(key,"Command")==0) strncpy(comm, value, strlen(value));
        if(strcmp(key,"Serial_Speed")==0) serl_speed=atoi(value);
        if(strcmp(key,"Serial_Flow_Control")==0) serl_flow_ctrl=atoi(value);
        if(strcmp(key,"Serial_Databit")==0) serl_databit=atoi(value);
        if(strcmp(key,"Serial_Stopbit")==0) serl_stopbits=atoi(value);
	if(strcmp(key,"Execute_Interval")==0) exec_interval=atoi(value);
        if(strcmp(key,"Serial_Parity")==0) strncpy(serl_parity, value, 1);
    }
    fclose(cfile);
    if(argc < 2)
    {
        printf("Usage: %s /dev/ttySn \n",argv[0]);
        return FALSE;
    }

    if(argc >= 8)
    {
	strncpy(comm, argv[2], strlen(argv[2]));
	serl_speed=atoi(argv[3]);
	serl_flow_ctrl=atoi(argv[4]);
	serl_databit=atoi(argv[5]);
	serl_stopbits=atoi(argv[6]);
	serl_parity[0]=argv[7][0];
    }
    fd = serialOpen(fd,argv[1]);
    syslog(LOG_INFO, "Get Parameter value is %s %d %d %d %d %d %c.\n", comm, exec_interval, serl_speed,serl_flow_ctrl,serl_databit,serl_stopbits,serl_parity[0]);
    err = serialInit(fd,serl_speed,serl_flow_ctrl,serl_databit,serl_stopbits,serl_parity[0]);
    if(err==FALSE || fd==FALSE)
    {
	syslog(LOG_ERR, "Init serial port %s with parameters %d %d %d %d %d %c error.\n", argv[1],serl_speed,exec_interval, serl_flow_ctrl,serl_databit,serl_stopbits,serl_parity[0]);
	printf("Init serial port %s with parameters %d %d %d %d %d %c error.\n", argv[1],serl_speed,exec_interval, serl_flow_ctrl,serl_databit,serl_stopbits,serl_parity[0]);
	return FALSE;
    }
    syslog(LOG_INFO, "Set Port Exactly!\n");

    FILE *fstream = NULL;

    char comm_result[4096];

    memset(send_buf, 0, sizeof(send_buf));
    memset(comm_result,0,sizeof(comm_result));
    signal(SIGCHLD, SIG_IGN);
    if(NULL == (fstream = popen(comm,"r")))
    {
        syslog(LOG_ERR,"execute command %s failed: %s",comm, strerror(errno));
        return FALSE;
    }
    else
    {
	syslog(LOG_INFO,"execute command %s success.",comm);
    }


    while(NULL != fgets(send_buf, sizeof(send_buf), fstream))
    {
        if(strlen(send_buf) > 0)
	{
  	    strcat(comm_result, send_buf);
  	}
    }
    printf("Send %s successful\n", comm_result);
    len = serialSend(fd,comm_result,sizeof(comm_result));
    if(len>0)
    {
	syslog(LOG_INFO, "Send data %s successful.", comm_result);
        sleep(exec_interval);
    }
    else
    {
	syslog(LOG_ERR, "Send data %s failed.\n", comm_result);
       	    return FALSE;
    }
    serialClose(fd);
    pclose(fstream);
    return TRUE;

}
