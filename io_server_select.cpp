#include <sys/types.h>
#include <sys/socket.h>
#include <csignal>
#include <unistd.h>
  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <sys/select.h>
#include <errno.h>

bool g_isStop = false;

void signal_handle(int signal_id)
{
    g_isStop = true;
}

int main(int argc, char*argv[])
{
	signal(SIGINT, signal_handle);
	signal(SIGTSTP, signal_handle); //ctrl + z
		
    int s = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(10003);
    bind(s, (struct sockaddr*)&servaddr, sizeof(servaddr));
	listen(s, 5);
	
	int connfd;
	char buff[256] = {0};
	
	struct timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(s, &rfds);
	
	int ret = -1;
	int n = 0;
	while(!g_isStop)
	{
		int ret = select(1, &rfds, NULL, NULL, &tv);
		if(ret < 0)
		{
			if(errno = EINTR) continue;
			else break;
		}
		else if (ret == 0) //that means all fds timeout
		{
			continue;
		}
		else
		{
			if(FD_ISSET(s, &rfds)) // judge listing socket can read
			{
		       connfd = accept(s, NULL, NULL);
	           while((n = read(connfd, buff, sizeof(buff))) > 0)
	           {
	           	buff[n] = 0;
	           	fputs(buff, stdout);
	           }
	           close(connfd);
			}
		}
	}
    return 0;
}
