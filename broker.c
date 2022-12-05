#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <limits.h>
#include <getopt.h>
#include <stdbool.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <getopt.h>


#define MAX 100
#define SIZE_NAME 200

typedef struct Client{
	int cfd;
	char name[40];
	char tipo[15];
	char direccionip [25];
	unsigned short int puerto;
}Client;

Client client[MAX] = {};
size_t cnt = 0;

sem_t sem1;
sem_t sem2;
sem_t sem3;

char thenamefile[SIZE_NAME];


void msg_producer(char *mensaje,int cgd){
	char buffer[100]={};
	strcpy(buffer,mensaje);
	
	if(send(cgd,buffer,strlen(buffer),0)<=0){
		printf("Error al enviar la info\n");
	 }
}

void guardar_log(char *mensaje){
	
	printf("%s\n",mensaje);
	char elbuffer[1024]={};
	
	 pid_t  pid;
     	     int estado=0;
     	     
     	     pid = fork();
     	     
	     if (pid < 0) {     
		  printf("ERROR: El proceso hijo ha fallado\n");
		  exit(1);
	     }
	     else if (pid == 0) {
	     
	     
	          strcpy(elbuffer,"echo ");
	          strcat(elbuffer,"'");
	          strcat(elbuffer,mensaje);
	          strcat(elbuffer,"'");
	          strcat(elbuffer," >> ");
	          strcat(elbuffer,thenamefile);
	          
	          
	     	  char *write_cmd[] = { "/bin/sh", "-c", elbuffer, NULL };          
		  if (execv("/bin/sh", write_cmd) < 0) {     
		       printf("ERROR: Comando invalido\n");
		       exit(1);
		  }
		  
	     }
	     else {                                  
		  while (wait(&estado) != pid);
	     }
}

void broadcast(char *msg,Client c){
	 
	 time_t timepu;
	 time(&timepu);
	
	struct tm *mitiempo3 = localtime(&timepu);
	char *timetext3 = asctime(mitiempo3);
        timetext3[24] = '\0';
	//Aqui mensaje  
	
	char mensaje_envio[200];
	sprintf(mensaje_envio,"%s : Publicador %s enviando contenido... Cantidad de bytes del contenido : %ld",timetext3,c.name,strlen(msg));
	guardar_log(mensaje_envio);
	
	//Fin mensaje
	 
        
	size_t i;
	
	sem_wait(&sem2);
	///Inicio seccion critica
	/****************************************/	
	for(i=0;i<cnt;i++){
		       char *linea_copy;
		       char *linea_copy_dos;
		       
	               linea_copy = (char *) malloc(strlen(client[i].name) + 1);
	               linea_copy_dos = (char *) malloc(strlen(c.name) + 1);
	               
	               strcpy(linea_copy, client[i].name);
	               strcpy(linea_copy_dos, c.name);
	               
	               char *token = strtok_r(linea_copy, "/", &linea_copy);
			char *token_dos = strtok_r(linea_copy_dos, "/", &linea_copy_dos);
	               
	               bool flag = false;
	               
	              if(token != NULL && token_dos != NULL){
			       	while(token != NULL && token_dos != NULL) {
		    				
		    				if(strcmp(token, token_dos) == 0){
		    					flag = true;
		    				}
		    				else if(strcmp(token, "#") == 0){
		    					flag = true;
		    					break;
		    				}
		    				else{
		    					flag = false;
		    					break;
		    				}
		    				
		    				
		    				
		    				token = strtok_r(linea_copy, "/", &linea_copy);
		    				token_dos = strtok_r(linea_copy_dos, "/", &linea_copy_dos);
					}
	               	}
	                              
	               if(flag){
	               	if(send(client[i].cfd,msg,strlen(msg),0)<=0){
					break;
			        }
	               }	
	}
	///Fin seccion critica
	/****************************************/	
	sem_post(&sem2);
	
	char mensaje_exito[200];
	sprintf(mensaje_exito,"%s : Publicador %s ha ennviado su contenido exitosamente!",timetext3,c.name);
	guardar_log(mensaje_exito);
		
	msg_producer("Contenido enviado con exito!",c.cfd);
	
}

void *pthread_run(void *arg){
	Client cl = *(Client*)(arg);
	if(strcmp(cl.tipo, "suscriptor") == 0){
		while(1){
			char buf[1024]={};
			int ret = recv(cl.cfd,buf,1024-strlen(buf),0);
			if(ret <= 0){
				sem_wait(&sem3);
				///Inicio seccion critica
			       /****************************************/	
				size_t i;
				for(i=0;i<cnt;i++){
					if(client[i].cfd == cl.cfd){
						client[i] = client[cnt-1];
						
						--cnt;
						
						time_t timepu;
						time(&timepu);
		
						struct tm *mitiempo3 = localtime(&timepu);
						char *timetext3 = asctime(mitiempo3);
						timetext3[24] = '\0';
						
						//Aqui mensaje
						
						char mensaje_desconec[200];
						sprintf(mensaje_desconec,"%s : Un %s se desconecto correctamente: ip <%s> puerto [%hu] topico -> %s",timetext3,cl.tipo,cl.direccionip,cl.puerto,cl.name); 
						guardar_log(mensaje_desconec);
						
						//Fin mensaje
			
						break;
					}	
				}
			       ///Fin seccion critica
			      /****************************************/	
				sem_post(&sem3);
				close(cl.cfd);
				return NULL;
			}
		}
	}
	else{
		while(1){
			char buf[1024]={};
			int ret = recv(cl.cfd,buf,1024-strlen(buf),0);
			if(ret <= 0){
			   	time_t timepu;
				time(&timepu);
		
				struct tm *mitiempo3 = localtime(&timepu);
				char *timetext3 = asctime(mitiempo3);
				timetext3[24] = '\0';
				
				//Aqui mensaje
				
				char mensaje_desconec[200];
				sprintf(mensaje_desconec,"%s : Un %s se desconecto correctamente: ip <%s> puerto [%hu] topico -> %s",timetext3,cl.tipo,cl.direccionip,cl.puerto,cl.name); 
				guardar_log(mensaje_desconec);
			
				//Fin mensaje
				
				close(cl.cfd);
				return NULL;
			}else{
				broadcast(buf,cl);
			}
		}
	}
	
}


void print_help(char *command)
{
	printf("Programa que simula a un broker, en la espera de recibir y gestionar a un suscriptor o a un publicador...\n");
	printf("Uso:\n %s -i <ip> -p [port]\n", command);
	printf(" %s -i <ip> (Puerto por defecto: 8080)\n", command);
	printf(" %s -p [port] (IP por defecto: 127.0.0.1)\n", command);
	printf(" %s Si no se usa ninguna opcion... (IP por defecto: 127.0.0.1 -- Puerto por defecto: 8080)\n", command);
	printf(" %s -h\n", command);
	printf("Opciones:\n");
	printf(" -h\t\t\tAyuda, muestra este mensaje\n");
	printf(" -i\t\t\tDireccion IP que utilizara el broker\n");
	printf(" -p\t\t\tPuerto que utilizara el broker\n");
}

bool iflag = false; 
bool pflag = false;

int main(int argc,char *argv[]){
	int opt;
	const char *ip;
	unsigned short int port;
        while ((opt = getopt (argc, argv, "hi:p:")) != -1){
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
			default:
				fprintf(stderr,"Uso: %s\n",argv[0]);
				fprintf(stderr, "     %s -h\n", argv[0]);
				fprintf(stderr, "     %s -i <ip> \n", argv[0]);
				fprintf(stderr, "     %s -p [port]\n", argv[0]);
				fprintf(stderr, "     %s -i <ip> -p [port]\n", argv[0]);
				return -1;
		}
	}
	
	if(!iflag){
		ip= "127.0.0.1";
	}

	if(!pflag){
		port = 8080;
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

	int ret = bind(sfd,(struct sockaddr*)(&addr),addrlen);
	if(ret == -1){
		perror("bind");
		return -1;
	}
	if(listen(sfd,10)==-1){
		perror("listen");
		return -1;
	}
	//Tiempo de inicio del broker
	time_t timep;
	time(&timep);
	
	struct tm *mitiempo = localtime(&timep);
	
	sprintf(thenamefile, "%d-%d-%d_%d:%d:%d.log",mitiempo->tm_mday,mitiempo->tm_mon,mitiempo->tm_year+1900,mitiempo->tm_hour,mitiempo->tm_min,mitiempo->tm_sec ); 
	char *timetext = asctime(mitiempo);
	timetext[24] = '\0';	
	
	char mensaje_inicio[200];
	sprintf(mensaje_inicio, "%s : Inicializacion del broker: ip <%s> puerto [%hu]",timetext,ip,port); 
	guardar_log(mensaje_inicio);
	
	//Semaforos Inicializacion
	 sem_init(&sem1, 0, 1);
	 sem_init(&sem2, 0, 1);
         sem_init(&sem3, 0, 1);
	//Fin de Inicializacion
	
	
	while(1){
		struct sockaddr_in caddr;
		socklen_t len = sizeof(caddr);
		
		int cfd = accept(sfd,(struct sockaddr*)(&caddr),&len);
		if(cfd == -1){
			perror("accept");
			return -1;
		}
		char buf[1024]={};
		
		recv(cfd,buf,1024-strlen(buf),0);
		
		char *lineas_copy;
		char *eltopico;
		char *elname;
		
		lineas_copy = (char *) malloc(strlen(buf) + 1);
		strcpy(lineas_copy, buf);
		
		char *tokens = strtok_r(lineas_copy, " ", &lineas_copy);
		eltopico = (char *) malloc(strlen(tokens) + 1);
		strcpy(eltopico, tokens);
		
		
		tokens = strtok_r(lineas_copy, " ", &lineas_copy);
		elname = (char *) malloc(strlen(tokens) + 1);
		strcpy(elname, tokens);
		
		char *elip = inet_ntoa(caddr.sin_addr);
		unsigned short int elpuertox = ntohs(caddr.sin_port);
		
			
		
		time_t timepo;
		time(&timepo);
	
		struct tm *mitiempo = localtime(&timepo);
		char *timetext2 = asctime(mitiempo);
		timetext2[24] = '\0';
		
		Client cl4;
		
		
		if(strcmp(elname, "publicador") == 0){
			cl4.cfd = cfd;
			strcpy(cl4.name , eltopico);
			strcpy(cl4.tipo , elname);
			strcpy(cl4.direccionip, elip);
			cl4.puerto = elpuertox;
		}
		else{
			sem_wait(&sem1);
			///Inicio seccion critica
	        	/****************************************/
			strcpy(client[cnt].name , eltopico);
			strcpy(client[cnt].tipo , elname);
			strcpy(client[cnt].direccionip , elip);
			client[cnt].cfd =  cfd;
			client[cnt].puerto =  elpuertox;
		
			cl4 = client[cnt];
			cnt++;
		
			///Fin seccion critica
			/****************************************/	
			
			sem_post(&sem1);
			
		}
		
		//Aqui mensaje
		char mensaje_conec[200];
		sprintf(mensaje_conec, "%s : Un %s se conectó correctamente: ip <%s> puerto [%hu] topico -> %s",timetext2,elname,elip,elpuertox,eltopico); 
		guardar_log(mensaje_conec);
		//Fin mensaje
		
		pthread_t id;
		
		ret = pthread_create(&id,NULL,pthread_run,(void*)(&cl4));
		
		if(ret != 0){
			printf("pthread_create:%s\n",strerror(ret));
			continue;
		}
		
	}
	return 0;
}


