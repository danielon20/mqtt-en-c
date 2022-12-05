#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>



void print_help(char *command)
{
	printf("Programa que simula a un suscriptor, en la espera de recibir contenido...\n");
	printf("Uso:\n %s -i <ip> -p [port] -t [topico]\n", command);
	printf(" %s -i <ip> -t [topico] (Puerto por defecto: 8080)\n", command);
	printf(" %s -p [port] -t [topico] (IP por defecto: 127.0.0.1)\n", command);
	printf(" %s -t [topico] ... Si no se usan las opciones -i y -p ... (IP por defecto: 127.0.0.1 -- Puerto por defecto: 8080)\n", command);
	printf(" %s -h\n", command);
	printf("Opciones:\n");
	printf(" -h\t\t\tAyuda, muestra este mensaje\n");
	printf(" -i\t\t\tDireccion IP del broker al que se desea conectar\n");
	printf(" -p\t\t\tPuerto del broker al que se desea conectar\n");
	printf(" -t\t\t\tTopico con el que se identificara el suscriptor\n");
}
bool iflag = false; 
bool pflag = false;
bool tflag = false;

int main(int argc,char *argv[]){
	
	int opt;
	const char *ip;
	unsigned short int port ;
	char *topico;
	
        while ((opt = getopt (argc, argv, "hi:p:t:")) != -1){
		switch(opt)
		{
			case 'h':
				print_help(argv[0]);
				return 0;
			case 'i':
				ip = optarg;
				iflag = true;
				break;
			case 'p':
				port = atoi(optarg);
				pflag = true;
				break;
			case 't':
				topico = optarg;
				tflag = true;
				break;
			default:
				fprintf(stderr,"Uso: %s -t [topico]\n",argv[0]);
				fprintf(stderr, "     %s -h\n", argv[0]);
				fprintf(stderr, "     %s -i <ip> -t [topico]\n", argv[0]);
				fprintf(stderr, "     %s -p [port] -t [topico]\n", argv[0]);
				fprintf(stderr, "     %s -i <ip> -p [port] -t [topico]\n", argv[0]);
				return -1;
		}
	}
	
	if(!iflag){
		ip= "127.0.0.1";
	}

	if(!pflag){
		port = 8080;
	}
	if(!tflag){
		fprintf(stderr, "     %s -t No ingreso el topico del suscriptor\n", argv[0]);
		return -1;
	}
	
	int sfd = socket(AF_INET,SOCK_STREAM,0);
	if(sfd == -1){
		perror("socket");
		return -1;
	}
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);
	socklen_t addrlen = sizeof(addr);

	int ret = connect(sfd,(const struct sockaddr*)(&addr),addrlen);
	if(ret == -1){
		perror("connect");
		return -1;
	}
	
	strcat(topico," suscriptor");
	send(sfd,topico,strlen(topico)+1,0);
	
		while(1){
			char contenido[1024]={};
			if(recv(sfd,contenido,1024,0)<=0){
				break;
			}
			printf("%s\n",contenido);
		}

	close(sfd);
	return 0;	
}




