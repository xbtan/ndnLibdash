#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <deque>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define TRACE_PERIOD (2)
std::deque<int> trace;
char* dst_ip;

void SetBandwidth(int);

void timer(int sig)
{
    if ( SIGALRM == sig )
    {
	SetBandwidth(trace.front());
        trace.pop_front();
 	alarm(TRACE_PERIOD);
    }
}

void SetBandwidth(int band)
{
    char command[256] = {0};
    sprintf(command, "tcset --device eth0 --rate %dk --network %s",band, dst_ip);
    std::cout << command << std::endl;
    system(command);  
}

void StopShape()
{
  system("tcdel --device eth0");
}

int main(int argc, char* argv[])
{
    if ( argc < 3 )
    {
 	printf("sudo ./trace_gen dst_ip trace_file\n");
 	return -1;
    }
    printf("%s %s\n", argv[1], argv[2]);
    dst_ip = argv[1];
    char buffer[256] = {0};
    std::ifstream is(argv[2]);
    if ( !is.good() )    
       return -1;
    while ( !is.eof() )
    {
	is.getline(buffer, 256);
 	trace.push_back(atoi(buffer));      
    }
   
    signal(SIGALRM, timer);
    alarm(TRACE_PERIOD);
    getchar();
    StopShape(); 
    return 0;
}
