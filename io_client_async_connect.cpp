#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <csignal>
#include <unistd.h>
#include <fcntl.h>
  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <iostream>
#include <errno.h>

int main(int argc, char*argv[])
{
    int s = socket(AF_INET, SOCK_STREAM, 0);
	
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("192.168.72.129");
	servaddr.sin_port = htons(10003);
	
	int old_fd = fcntl(s, F_GETFL, 0);
	if(fcntl(s, F_SETFL, old_fd | O_NONBLOCK) < 0 )
	{
		std::cerr << "Setting fd error! errorno:"  << errno << std::endl;
		close(s);
		return -1;
	}
	
    int ret = connect(s, (struct sockaddr*)&servaddr, sizeof(servaddr));
	if(	ret == 0)
	{
		std::cout << "connect successed" << std::endl;
	}
    else
	{
		if(errno == EINPROGRESS)
		{
		    struct timeval tv;
	        tv.tv_sec = 15;
	        tv.tv_usec = 0;
	
			fd_set rfds;
	        FD_ZERO(&rfds);
	        FD_SET(s, &rfds);
			
			ret = select(1, NULL, &rfds, NULL, &tv); // judge write fds
			if(ret < 0)
			{
				//if errno == EINTR 
				std::cerr << "connect error! errno:"  << errno << std::endl;
			}
			else
			{
				int error = -1;
				int errlen = sizeof(error);
				if(getsockopt(s, SOL_SOCKET, SO_ERROR, &error, (socklen_t*)&errlen) < -1)
				{
					std::cerr << "getsockopt error! errorno:"  << errno << std::endl;
		            close(s);
		            return -1;
				}
				
				if(error == 0)
				{
					std::cout << "connect successed" << std::endl;
				}
				else
				{
					//errno = error;
					std::cout << "connect error! errno:"  << errno << std::endl;
				}
			}
			
		}
		else
		{
			std::cerr << "connect error! errno:"  << errno << std::endl;
		}
		
	}		
	
	int n = 0;
	char buff[256] = {0};
	while((n = read(s, buff, sizeof(buff))) > 0)
	{
		buff[n] = 0;
		fputs(buff, stdout);
	}
	close(s);
    return 0;
}
