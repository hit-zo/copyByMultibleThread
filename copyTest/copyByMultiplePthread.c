#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <pthread.h>

int param_check(int argc, const char* sfile, int pronum) {
	if (argc < 3) {
		printf("parameters not enough\n");
		exit(0);
	}
	if ((access(sfile,F_OK)) != 0) {
		printf("filetype error\n");
		exit(0);
	}
	if (pronum <= 0 || pronum > 100) {
		printf("pronum error\n");
		exit(0);
	}
	return 0;
}
int file_block(const char* sfile, int pronum) {
	int fd = open(sfile, O_RDONLY);
	int fsize = lseek(fd, 0, SEEK_END);
	printf("file no %d\n", fsize / pronum + 1);
	if (fsize % pronum == 0)
		return fsize / pronum;
	else
		return fsize / pronum + 1;
}

void* job(void* arg) {
	int block_size = atoi((char*)(arg+2));
	int copy_pos = atoi((char*)(arg+3));
	char buffer[block_size];
	bzero(buffer,sizeof(buffer));
	ssize_t readlen;
	int sfd = open((char*)(arg), O_RDONLY);
	int dfd = open((char*)(arg+1), O_RDWR|O_CREAT, 0664);
	//change offset
	lseek(sfd, copy_pos, SEEK_SET);
	lseek(dfd, copy_pos, SEEK_SET);
	//copy
	readlen = read(sfd, buffer, sizeof(buffer));
	write(dfd, buffer, readlen);
	close(sfd);
	close(dfd);
}
int main(int argc, char** argv) {

	//initialize parameters
	int pronum;
	if (argv[3] == 0) 
		pronum = 3;
	else
		pronum = atoi(argv[3]);
	param_check(argc, argv[1], pronum);
	int blocksize;
	blocksize = file_block(argv[1], pronum);
	
	//pthread create
	pthread_t tid;
	int err;
	int flags = 0;
	while(flags < pronum) {
		int pos;
		pos = flags * blocksize;
		char str_blocksize[50];
		char str_pos[50];
		bzero(str_blocksize, sizeof(str_blocksize));
		bzero(str_pos,sizeof(str_pos));
		sprintf(str_blocksize, "%d", blocksize);
		sprintf(str_pos, "%d", pos);
		printf("TID 0x%x cop_pos %d copy_size %d\n", (unsigned int)pthread_self(), pos, blocksize);
		char* arg[4]={argv[1], argv[2], str_blocksize, str_pos};

		if((err=pthread_create(&tid, NULL, job, (void*)arg)) > 0) {
			printf("pthread create error: %s\n", strerror(err));
			exit(0);
		}
		printf("thread No %d\n", ++flags);
	}
	return 0;
}
