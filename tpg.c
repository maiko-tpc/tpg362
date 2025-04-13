/////////////////////////////////////////////////////////
//
//  Communication with Pfeiffer TPG361/362 via ethernet
//  2022.09.23 by MAIKo dev. group
//
/////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>

//const int wait_time=10000000; // measure interval in us
const int wait_time_sec=10; // measure interval in s


int main(int argc, char** argv){

  /* set IP address */
  char ip_address[256];
  int host_flag=0;
  
  if(argc==2){
    if(argv[1][0]=='1'){
      sprintf(ip_address, "%s", argv[1]);
    }
    else{
      /* get IP address by hostname */
      struct sockaddr_in ip;
      struct hostent *host;
      host = gethostbyname(argv[1]);
      if( host != NULL ){
	printf("hostname: %s\n", argv[1]);
	ip.sin_addr =  *(struct in_addr *)(host->h_addr_list[0]);
	sprintf(ip_address, "%s", inet_ntoa(ip.sin_addr));
	host_flag=1;
      }
      if( host ==NULL){
	printf("Unknown hostname: %s\n", argv[1]);
	exit(1);
      }
    }
  }
  
  if(argc!=2){
    sprintf(ip_address, "172.16.213.163");  // RCNP maikotpg
  }
  
  printf("IP address: %s\n", ip_address);

  // Make socket
  int sd;
  struct sockaddr_in addr;
  if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    return -1;
  }
  
  // Set the host info
  addr.sin_family = AF_INET;
  addr.sin_port = htons(8000); // scaned by nmap command
  addr.sin_addr.s_addr = inet_addr(ip_address);

  // Connect to the module 
  int connect_res = connect(sd, (struct sockaddr *)&addr,
			    sizeof(struct sockaddr_in));
  if(connect_res <0){
    printf("TCP connection error: %d\n", connect_res);
    return -1;
  }

  time_t timer;
  struct tm *local;
  int year, month, day, hour, min, sec;

  // Set the output file name
  timer = time(NULL);
  local = localtime(&timer);
  year = local->tm_year + 1900;
  month = local->tm_mon + 1;
  day = local->tm_mday;
  
  char fname[512];
  sprintf(fname, "%d%02d%02d.dat", year, month, day);
  printf("output file: %s\n", fname);

  FILE *output;
  
  char tmp[128], data1[128], data2[128];
  int data_length=0;
  int res;

  while(1){
    output = fopen(fname, "a");

    // Get Unix time
    timer = time(NULL);  
    fprintf(output, "%ld  ", timer);
    local = localtime(&timer);
    year = local->tm_year + 1900;
    month = local->tm_mon + 1;
    day = local->tm_mday;
    hour = local->tm_hour;
    min = local->tm_min;
    sec = local->tm_sec;    
    printf("%d/%02d/%02d %02d:%02d:%02d\n",
	   year, month, day, hour, min, sec);
    
    // Send Command for CH1
    res=send(sd, "PR1\r\n", 5, 0);

    // Get acknowledge
    data_length=recv(sd, &tmp, 128, 0);
    
    // Request for data transmission <ENQ>
    res=send(sd, "\x05", 2, 0);

    // Get the data and decode
    data_length=recv(sd, &data1, 128, 0);
    printf("data1: %s", data1);
    for(int i=0; i<10; i++){
      fprintf(output, "%c", data1[i+3]);
    }
    fprintf(output, "  ");
    
    
    // Send Command for CH2
    res=send(sd, "PR2\r\n", 5, 0);

    // Get acknowledge
    data_length=recv(sd, &tmp, 128, 0);
    
    // Request for data transmission <ENQ>
    res=send(sd, "\x05", 2, 0);

    // Get the data and decode
    data_length=recv(sd, &data2, 128, 0);
    printf("data2: %s\n", data2);
    for(int i=0; i<10; i++){
      fprintf(output, "%c", data2[i+3]);
    }
    fprintf(output, "\n");
    fclose(output);
    
    // Wait for the next measurement
    //usleep(wait_time);
    sleep(wait_time_sec);
  }
  
  close(sd);

  return 0;
}
