#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <getopt.h>
#include <stdbool.h>


void print_help(char *command)
{
	printf("Programa que simula a un publicador, el cual va a enviar contenido...\n");
	printf("Uso:\n %s -i <ip> -p [port] -t [topico] -c [contenido]\n", command);
	printf(" %s -i <ip> -t [topico] -c [contenido] (Puerto por defecto: 8080)\n", command);
	printf(" %s -p [port] -t [topico] -c [contenido] (IP por defecto: 127.0.0.1)\n", command);
	printf(" %s -t [topico] -c [contenido] ... Si no se usan las opciones -i y -p ... (IP por defecto: 127.0.0.1 -- Puerto por defecto: 8080)\n", command);
	printf(" %s -h\n", command);
	printf("Opciones:\n");
	printf(" -h\t\t\tAyuda, muestra este mensaje\n");
	printf(" -i\t\t\tDireccion IP del broker al que se desea conectar\n");
	printf(" -p\t\t\tPuerto del broker al que se desea conectar\n");
	printf(" -t\t\t\tTopico con el que se identificara el publicador\n");
	printf(" -c\t\t\tContenido que enviara el publicador a los suscriptores con su mismo topico\n");
}

bool iflag = false; 
bool pflag = false;
bool tflag = false;
bool cflag = false;

int main(int argc,char *argv[]){
	
	int opt;
        char *topic;
        char *contenido;
        const char *ip;
	unsigned short int port ;
        int status;
       
        while ((opt = getopt (argc, argv, "hi:p:t:c:")) != -1){
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
				topic = optarg;
				tflag = true;
				break;
			case 'c':
				contenido = optarg;
				cflag = true;
				break;
			default:
				fprintf(stderr,"Uso: %s -t [topico] -c [contenido]\n",argv[0]);
				fprintf(stderr, "     %s -h\n", argv[0]);
				fprintf(stderr, "     %s -i <ip> -t [topico] -c [contenido]\n", argv[0]);
				fprintf(stderr, "     %s -p [port] -t [topico] -c [contenido]\n", argv[0]);
				fprintf(stderr, "     %s -i <ip> -p [port] -t [topico] -c [contenido]\n", argv[0]);
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
		fprintf(stderr, "     %s -t No ingreso el topico del publicador\n", argv[0]);
		return -1;
	}
	if(!cflag){
		fprintf(stderr, "     %s -t No ingreso el contenido del publicador\n", argv[0]);
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
	printf("Enviando contenido..... \n");
	char *linea_topic;
	linea_topic = (char *) malloc(strlen(topic) + 1);
	strcpy(linea_topic, topic);
	strcat(linea_topic," publicador");
	send(sfd,linea_topic,strlen(linea_topic)+1,0);
	
	
	pid_t pid = fork();
	
	if(pid == -1){
		perror("fork");	
      		return -1;
     	}
     
	if(pid == 0){
		printf("Cantidad de bytes del contenido : %ld\n",strlen(contenido));
		send(sfd,contenido,strlen(contenido)+1,0);
	}
	else{
		waitpid(pid,&status,0);
		while(1){
			
			char mensaje_exito[1024]={};
			if(recv(sfd,mensaje_exito,1024,0)<=0){
				break;
			}
			printf("%s\n",mensaje_exito);
			
			if(strcmp(mensaje_exito, "Contenido enviado con exito!") == 0 ){
				break;
			}
					
		}
		close(sfd);
	}
	
	return 0;	
}




