#include <sys/types.h>
#include <sys/termio.h>
#include <fcntl.h>

struct termios mytermio, savtermio;

int termfd = -1;

int
EchoOff()
{
	if(termfd < 0) {
		termfd = open("/dev/tty",O_RDWR);
	}

	if(ioctl(termfd,TCGETS,&savtermio)) {
		perror("Can't get termio info");
		return 1;
	}
	mytermio = savtermio;
	mytermio.c_lflag &= (0xff - ECHO);
	if(ioctl(termfd,TCSETS,&mytermio)) {
		perror("Can't set echo off");
		return 1;
	}

	return 0;
}

int
EchoOn()
{
	if(termfd < 0) {
		termfd = open("/dev/tty",O_RDWR);
	}
	if(ioctl(termfd,TCGETS,&savtermio)) {
		perror("Can't get termio info");
		return 1;
	}
	mytermio = savtermio;
	mytermio.c_lflag |= ECHO;
	if(ioctl(termfd,TCSETS,&mytermio)) {
		perror("Can't change echo status");
		return 1;
	}

	return 0;
}

int
EchoRestore()
{
	if(ioctl(termfd,TCSETS,&savtermio)) {
		perror("Error restoring Echo");
		return 1;
	}

	return 0;
}



