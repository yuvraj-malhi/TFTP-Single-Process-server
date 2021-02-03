#include<time.h>
#include<stdio.h>
#include<error.h>
#include<errno.h>
#include<netdb.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<signal.h>
#include<sys/wait.h>
#include<sys/time.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#include<dirent.h>
#define directory "."

#define TIMEOUT  5
#define NAME_LEN 100
#define MODE_LEN 100
#define DATA_LEN 512

int verbose;
int L_PORT;
#define MSG_LEN 200
int C_PORT;
int please_wait;

typedef struct {
	unsigned short int opcode;
	char filename[NAME_LEN];
	char zero_0;
	char mode[MODE_LEN];
	char zero_1;
} Request;

typedef struct {
	unsigned short int opcode;
	unsigned short int block;
	char data[DATA_LEN + 1];
} Data;

typedef struct {
	unsigned short int opcode;
	unsigned short int block;
} Ack;

typedef struct {
	unsigned short int opcode;
	unsigned short int ecode;	// error code
	char msg[MSG_LEN];	
} Error;

typedef union {
	Request rrq;
	Data data;
	Ack ack;
	Error err;
}Message;


#define MAX_CLIENTS 500
struct client {
	int 	size;
	int 	state;
	char 	filename[NAME_LEN];
	FILE*	ptr;
	int 	fd;
	struct 	timeval last_time;
	struct 	timeval start_time;
	struct sockaddr_in clientaddress;
	int num_tries;
	int last_block;
};

struct client C[MAX_CLIENTS];
int count;		// Current index of last client



void sig_handler(int sign){
	please_wait = 0;
}

int ppid;
int child_pids[1000];
int check_file(char* file);

int L_FD;
void end(int sig)
{

	close(L_FD);
	for(int i=0; i<MAX_CLIENTS; i++)
		close(C[i].fd);

	printf("COMMAND: Terminating...\n");
	exit(0);
}

int get_block(struct client* cli, int block, int numxl){

	int offset = block-1;
	if(offset < 0){
		printf("SERVER REQUEST %d: Error, offset entered is %d\n", offset + 1,numxl);
		return -1;
	}
	offset *= DATA_LEN;
	fseek(cli->ptr, offset, SEEK_SET);
	Message x;
	
	x.data.opcode = 3;
	x.data.block  = block;
	x.data.opcode = htons(x.data.opcode);
	x.data.block  = htons(x.data.block);
	
	int fread_return = fread(x.data.data, sizeof(char), DATA_LEN, cli->ptr);
	x.data.data[fread_return] = '\0';
	
	int sendto_return = sendto(cli->fd, &x, sizeof(x) + (fread_return - 2 - DATA_LEN)*sizeof(char), 0, (struct sockaddr*)&(cli->clientaddress), sizeof(struct sockaddr));
	
	while(sendto_return < 0)
		sendto_return = sendto(cli->fd, &x, sizeof(x) + (fread_return - 2 - DATA_LEN)*sizeof(char), 0, (struct sockaddr*)&(cli->clientaddress), sizeof(struct sockaddr));

	cli->state = block;
	gettimeofday(&(cli->last_time), NULL);
	
	if(verbose)	
		printf("SERVER REQUEST %d: Sent the BLOCK - %d\n", numxl , block);
	
	if(fread_return < sizeof(char)*DATA_LEN){
		C[count].last_block = 1;
		return 0;
	}
	else
		return 1;
}

long int findSize(char file_name[]) 
{ 
	FILE* fp = fopen(file_name, "r"); 
	if (fp == NULL) 
        	return -1; 
	fseek(fp, 0L, SEEK_END); 
	long int res = ftell(fp); 
	fclose(fp); 
	return res; 
} 


void close_client(struct client* cli, int xx){
	fclose(cli->ptr);
	close(cli->fd);
	cli->state = -1;
	struct timeval result;
	gettimeofday(&result,NULL);
	result.tv_usec -= cli->start_time.tv_usec;
	result.tv_sec  -= cli->start_time.tv_sec;
	printf("\nSERVER REQUEST %d: File '%s' transferred at speed -> %f Mb/s\n",xx,cli->filename,(0.000001*(float)cli->size)/((float)(result.tv_sec)+((float)(result.tv_usec)/1000000)));
	printf("SERVER REQUEST %d: --------------- Request %d Done --------------\n\n", xx, xx);
}


void main(int argc, char ** argv)
{
	count = 1;
	C[0].state=-1;

	ppid = getpid();
	signal(SIGINT, end);	
	
	printf("\n------------------------- HOST INFO ---------------------------\n\n");

	char *IP, *IP2, host[100];
	struct hostent *hen;
	if(gethostname(host, sizeof(host)) == -1)
	{
		perror("GETHOSTNAME");
		exit(0);
	}

	if((hen = gethostbyname(host))==NULL)
	{
		perror("GETHOSTINFO");
		exit(0);
	}
	IP = inet_ntoa(*((struct in_addr*) hen->h_addr_list[0]));
	printf("HOST INFO: Name -> %s\n",host);
	printf("HOST INFO: IP   -> %s\n",IP);



	please_wait = 1;
	signal(SIGALRM, sig_handler);

	printf("\n------------------------ INITIALIZING -------------------------\n\n");
	struct sockaddr_in localbind, clientaddress;

	for(int ii=0; ii<1000; ii++)
		child_pids[ii]=1;

	int run=1;


	printf("COMMAND: Initialising server...\n");
	if(argc<2) 
	{ 
		printf("COMMAND: Error: Please enter port number as CLI. Exiting....\n"); 
		exit(0); 
	}


	if(argc>2) 
	{ 
		printf("COMMAND: Error: Invalid arguements specified. Exiting....\n"); 
		exit(0); 
	}

	L_PORT = atoi(argv[1]);
	printf("COMMAND: Dedicating listening port at: \t\t%d\n", L_PORT);
	if(L_PORT != 69 )
		printf("\nWARNING: Default port for client communication is PORT 69!!\n\n");


	char choice[100];
	printf("COMMAND: Do you want to print the status of each transfer continuously? (Y/N)\n> ");
	
	scanf("%[^\n]s",choice); getchar();
	
	if((choice[0]=='Y')||(choice[0]=='y'))
		verbose = 1;
	else
		verbose = 0;
	
	L_FD = socket(AF_INET, SOCK_DGRAM, 0);
	if(L_FD==-1) 
	{ 
		perror("LISTEN: Socket");
		exit(0); 
	}
	
	int tttt = 1; 
	setsockopt(L_FD, SOL_SOCKET, SO_REUSEADDR, &tttt , sizeof(int));


	localbind.sin_family       = AF_INET;
	localbind.sin_addr.s_addr  = htonl(INADDR_ANY);
	localbind.sin_port     = htons(L_PORT);
	if (bind(L_FD, (struct sockaddr *) &localbind, sizeof(localbind)) == -1)
	{ 
		perror("LISTEN: Bind");
		exit(0);
	}
	
	
	printf("COMMAND: Successfully initialised listen port\n");

	printf("\n------------------------ SERVER START -------------------------\n\n");
	
	char * tmp;
	int address_len, numbytes;
	Request * REQUEST;

	char file[NAME_LEN], mode[MODE_LEN];
	short signed int opcode;
	struct timeval current_time, select_time;

	fd_set SET;
	int max_fd, select_value;

	while(1)
	{
		select_time.tv_sec  = 0;
		select_time.tv_usec = 1;
		max_fd = L_FD;
		FD_ZERO (&SET);
		FD_SET  (L_FD,&SET);
		
		gettimeofday(&current_time,NULL);

		for(int i=0; i<count; i++)
		{
			if(C[i].state == -1) continue;
			if(current_time.tv_sec - C[i].last_time.tv_sec >=TIMEOUT)
			{
				printf("SERVER REQUEST %d: Timeout - Client not responding!\n",i);
				printf("SERVER REQUEST %d: --------------- Request %d Done --------------\n\n",i,i);
				close(C[i].fd);
				C[i].state = -1;
			}

			if(C[i].state!=-1)
			{
				if( max_fd<C[i].fd )
					max_fd = C[i].fd;

				FD_SET (C[i].fd,&SET);
			}

		}

		max_fd++;
		select_value = select(max_fd,&SET,NULL,NULL,&select_time);

		if(select_value == 0)
		{
			continue;
		}

		if(select_value < 0)
		{
			printf("SERVER: Error occured. Retrying..\n");
			continue;
		}


		if( FD_ISSET(L_FD, &SET) )
		{

			tmp = (char *)malloc( sizeof(Request) );
			memset(tmp, 0, sizeof(Request));
			address_len = sizeof(struct sockaddr);

		
			numbytes = recvfrom(L_FD, tmp, sizeof(Request), 0, (struct sockaddr *)&clientaddress, &address_len);
			if (numbytes < 0) 
			{
				free(tmp);
				continue;
			}


			REQUEST= (Request *)malloc(sizeof(Request));
			opcode=*(short signed int*)tmp;
			tmp = tmp + sizeof(REQUEST->opcode);
			strcpy(file,tmp);
			tmp = strchr(tmp, '\0')+1;		
			strcpy(mode,tmp);
			
			REQUEST->opcode = ntohs(opcode);
			REQUEST->zero_0 = '\0';
			REQUEST->zero_1 = '\0';
			
			strcpy(REQUEST->filename, file);
			strcpy(REQUEST->mode    , mode);
	
	
			if(REQUEST->opcode==1)
				printf("LISTEN: Request recieved:\n\t\t\tIPv4   <-> %s \n\t\t\tPORT   <-> %d\n\t\t\tMODE   <-> %s\n\
			FILE   <-> %s\n",inet_ntoa(clientaddress.sin_addr), ntohs(clientaddress.sin_port),REQUEST->mode, REQUEST->filename);
		
			else
			{
				printf("LISTEN: Invalid request recieved from:\n\t\t\tIPv4   <-> %s \n\t\t\tPORT   <-> %d\n\t\t\t\
					MODE   <-> %s\n", inet_ntoa(clientaddress.sin_addr), ntohs(clientaddress.sin_port),REQUEST->mode);
			
				printf("LISTEN: Error: Invalid request\n");
				continue;
			}
			printf("\nSERVER REQUEST %d: --------------- Request %d Start -------------\n\n", count, count);
	
			
			int check = check_file(REQUEST->filename);
			int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
			C[count].fd = sockfd;
			if(check == -1)
			{
				
				Error x;
				x.opcode = 5;
				x.ecode = 1;
				
				x.opcode = htons(x.opcode);
				x.ecode = htons(x.ecode);
		
				sprintf(x.msg, "File %s not found in %s directory\n", REQUEST->filename, directory);
				printf("SERVER REQUEST %d: Sending FILE NOT FOUND error message to client\n", count);
				sendto(sockfd, &x, sizeof(x), 0, (struct sockaddr*)&clientaddress, sizeof(struct sockaddr));
		
				printf("SERVER REQUEST %d: Error packet sent successfully\n", count);
				printf("\nSERVER REQUEST %d: --------------- Request %d Done --------------\n\n", count, count);
				close(sockfd);
				
			
				strcpy(C[count].filename,REQUEST->filename);
				C[count].state = -1;
				count++;
				continue;
			}
			else{
				strcpy(C[count].filename,REQUEST->filename);
				memcpy(&C[count].clientaddress, &clientaddress, sizeof(struct sockaddr_in));
				C[count].state      = 0;
				C[count].num_tries  = 0;
				C[count].last_block = 0;
				C[count].size       = findSize(REQUEST->filename);
				gettimeofday(&C[count].start_time, NULL);
				char* filename = C[count].filename;
				char newfile[NAME_LEN];
				newfile[0] = '\0';
				strcat(newfile, directory);
				strcat(newfile, "/");
				strcat(newfile, filename);
				C[count].ptr = fopen(newfile, "r");
		
				get_block(&C[count], 1,count);
				count++;
			}
	}

		for(int xx=0; xx<count; xx++)
			if(FD_ISSET(C[xx].fd,&SET))
			{
				
				Message x;
				int size_msg;
				int recvfrom_return = recvfrom(C[xx].fd, &x, sizeof(Message), 0, (struct sockaddr *)&(C[xx].clientaddress), &size_msg);
				
				while(recvfrom_return <= 0)
					recvfrom_return = recvfrom(C[xx].fd, &x, sizeof(Message), 0, (struct sockaddr *)&(C[xx].clientaddress), &size_msg);
					
				if(size_msg > sizeof(Message)){
					printf("SERVER REQUEST %d: Message Length Exceeded\n",xx);
					close_client(&C[xx], xx);
					continue;
				}
				
				x.ack.opcode = ntohs(x.ack.opcode);
				x.ack.block  = ntohs(x.ack.block);
				
				if(x.ack.opcode == 5){
					printf("SERVER REQUEST %d: received error\n",xx);
					close_client(&C[xx],xx);
					continue;
				}
				else if(x.ack.opcode == 4){
				
					if(verbose)	
					printf("SERVER REQUEST %d: ACK  for BLOCK - %d\n", xx, x.ack.block);
					if(x.ack.block == C[xx].state){
						if(C[xx].last_block == 1){
							close_client(&C[xx], xx);
							continue;
						}
													
						C[xx].state += 1;
						if(C[xx].state>(C[xx].size+1)/512)
							C[xx].last_block=1;
					
						get_block(&C[xx], x.ack.block + 1,xx);
						continue;
					}
					else{
						//printf("SERVER REQUEST %d: Discrepancy in acknowledgement\n",xx);
						
						if(C[xx].last_block == 1){
							close_client(&C[xx], xx);
							continue;
						}
						
						//C[xx].state = x.ack.block + 1;
						//get_block(&C[xx], x.ack.block + 1,xx);
						C[xx].state+=1;
						if(C[xx].state>(C[xx].size+1)/512)
							C[xx].last_block=1;
						get_block(&C[xx], C[xx].state ,xx);
						continue;
					}
				
				}
				
				
			}


	}	
	close(L_FD);
}

int check_file(char* file){

	DIR* d = opendir(directory);
	struct dirent* dir;
	
	if(d)
	{
		while((dir = readdir(d)) != NULL)
			if(strcmp(dir->d_name, file) == 0)
				return 0;
		
		closedir(d);
	}
	
	return -1;
}




