#include "serial.h"

#include <unistd.h>
#include <bcm2835.h>
#include <stdlib.h>



#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>


#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>
#include <termios.h> 
#include <ctype.h>
#include <sys/ioctl.h>
#include <limits.h>
#include <limits.h>
#include <pthread.h>
#include <poll.h>


int speed;

//const char *serialPort="/dev/ttyAMA0";
const char *serialPort="/dev/ttyS0";

int sd,status;

struct termios options;

struct timeval start_program, end_point;


void beginSerial(int serialSpeed)
{
	switch(serialSpeed){
		case     50:	speed =     B50 ; break ;
		case     75:	speed =     B75 ; break ;
		case    110:	speed =    B110 ; break ;
		case    134:	speed =    B134 ; break ;
		case    150:	speed =    B150 ; break ;
		case    200:	speed =    B200 ; break ;
		case    300:	speed =    B300 ; break ;
		case    600:	speed =    B600 ; break ;
		case   1200:	speed =   B1200 ; break ;
		case   1800:	speed =   B1800 ; break ;
		case   2400:	speed =   B2400 ; break ;
		case   9600:	speed =   B9600 ; break ;
		case  19200:	speed =  B19200 ; break ;
		case  38400:	speed =  B38400 ; break ;
		case  57600:	speed =  B57600 ; break ;
		case 115200:	speed = B115200 ; break ;
		default:	speed = B230400 ; break ;
			
	}

	if ((sd = open(serialPort, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK)) == -1){
		fprintf(stderr,"Unable to open the serial port %s - \n", serialPort);
		exit(-1);
	}
    
	fcntl (sd, F_SETFL, O_RDWR) ;
    
	tcgetattr(sd, &options);
	cfmakeraw(&options);
	cfsetispeed (&options, speed);
	cfsetospeed (&options, speed);

	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_oflag &= ~OPOST;

	tcsetattr (sd, TCSANOW, &options);

	ioctl (sd, TIOCMGET, &status);

	status |= TIOCM_DTR;
	status |= TIOCM_RTS;

	ioctl (sd, TIOCMSET, &status);
	
	usleep (10000);
	
	gettimeofday(&start_program, NULL);
}


int availableSerialByte()
{
    int nbytes = 0;
    if (ioctl(sd, FIONREAD, &nbytes) < 0)  {
		fprintf(stderr, "Failed to get byte count on serial.\n");
        exit(-1);
    }
    return nbytes;
}

void println(const char *message)
{
	const char *newline="\r\n";
	char msg[512] = {'\0'};
	sprintf(msg,"%s%s",message,newline);
	msg[strlen(message)+strlen(newline)]='\0';
	write(sd,msg,strlen(msg));
	printf("(write serial data):\r\n\033[1;34;40m%s\033[0m",msg);
}


void print(char* message, int len)
{
	write(sd,message,len);
}

char readSerial()
{
	unsigned char c;
	read(sd,&c,1);
    return c;
}


int readSerialBuffer(char* message,int len)
{
	int result;
    result = read(sd,message,len);
	return result;
}




long millis()
{
	long elapsedTime;
	// stop timer
    gettimeofday(&end_point, NULL);

    // compute and print the elapsed time in millisec
    elapsedTime = (end_point.tv_sec - start_program.tv_sec)*1000.0;      // sec to ms
    elapsedTime += (end_point.tv_usec - start_program.tv_usec)/1000.0;   // us to ms
    return elapsedTime;
}
