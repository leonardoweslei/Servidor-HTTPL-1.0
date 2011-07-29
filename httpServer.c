/**
 * 
 * @author   Servidor Http simples
 * @author 	 Leonardo Weslei Diniz<leonardoweslei@gmail.com>
 * @abstract 
 * 			 Este programa visa implementar um servidor HTTP simples
 * 			 foram usados sockets pra escutar a porta 8080, o fork para atender as requisições, memoria compartilhada para armazenas a quantidade de processos finalizados e em execução, semaforos para controlar o acesso as memorias e aos arquivos de log e de processos, e sinais para limpar as memorias remover semáforos e matar processos filhos.
 * 			 Na compilação existem varios warnings mas não influenciam no funcionamento do programa.
 * 			 Para sair do servidor é só dar um ctrl+c ou um kill no pid do programa.
 * 			 é possivel personalizar a quantidade maxima de processos, arquivo padrão de erro, modo de gravação de log, pasta padrão de documentos web, porta padrão e nome do servidor.
 * 			 Foi usado um arquivo externo para recuperar os mimetypes este arquivo vou criado a partir do /etc/mime.types.
 * 			 dentro dos arquivos de erros é possível usar variáveis como _DATE_ para data do servidor, _TIME_ para hora do servidor, _SERVER_ para nome do servidor, _ERROR_CODE_ para código do erro e _ERROR_DESC_ para descrição do erro.
 * 			 O formato de hora e data também é personalizável, assim como nome de arquivo de log e o formato da data do nome do arquivo de log.
 * 			 para compilar use gcc httpServer.c -lpthread -o httpServer
 * 			 para executar ./httpServer
 * 			 para sair dê um kill ou um ctrl+c no processo
 * @final    09/12/2010 15:37:49
 * @name 	 httpServer.c
 * @version  1.0
 * http://wendtstud2.hpi.uni-potsdam.de/sysmod-seminar/SS2003/presentations/gruppe-1/httpServer.c
 * http://geekrepublic.wordpress.com/2007/09/04/ler-linhas-de-um-arquivo-de-texto-de-maneira-portavel/
 * http://www.guiadohardware.net/comunidade/linguagem-variavel/220628/
 * http://code.google.com/p/icebit/source/browse/trunk/src/urldecode.c?spec=svn1&r=1
 * http://code.google.com/p/icebit/source/browse/trunk/src/urldecode.h?spec=svn1&r=1
 * http://www.facom.ufu.br/~faina/BCC_Crs/INF09-1S2009/Prjt_SO1/semaphor.html
 * http://www.portugal-a-programar.org/forum/index.php?topic=26238.0
 * http://kb.globalscape.com/KnowledgebaseArticle10141.aspx
 * http://www.cesarkallas.net/arquivos/apostilas/programacao/c_c%2B%2B/c/PL05files.PDF
 * http://www.linuxsecurity.com.br/info/unix/permissoes.unix.txt
 * http://www.unix.com/pt/programming/45037-how-implement-sigkill-sigterm-print-message.html
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <stddef.h>
#include <malloc.h>
#include <errno.h>              /* errno and error codes */
#include <unistd.h>             /* for gettimeofday(), getpid() */
#include <sys/wait.h>           /* for wait() */
#include <signal.h>             /* for kill(), sigsuspend(), others */
#include <sys/shm.h>            /* for shmget(), shmat(), shmctl() */
#include <sys/sem.h>            /* for semget(), semop(), semctl() */
#include <sys/ipc.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <netinet/in.h>
#define MAXLENGTH 500
#define MAXLINES 50
#define SERVER_NAME "HTTPLD rc1.0"
#define SERVER_PORT 8080
#define DOCUMENT_ROOT "htdocs"
#define DEFAULT_FILE_ERROR "htdocs/erro.html"
#define FILE_LOG "log.txt"
#define FILE_LOG_DATE "%d-%m-%Y"
#define GRAVA_LOG 1
#define MAX_CONN 5
#define TDATE "%d/%m/%Y"
#define THOUR "%H:%M:%S %p"

char* fgetline(FILE* file)
{
	char* line = 0;
	fpos_t fpos;
	int size = 0, read = 0;
	if(feof(file)) return 0;
	fgetpos(file, &fpos);
	read = fscanf(file, "%*[^\n]%n", &size);
	if(read > 0 || read == EOF) return 0;
	if(!size)
	{
		fsetpos(file, &fpos);
		fgetc(file);
		return calloc(1, 1);
	}
	line = malloc(size + 1);
	if(!line) return 0;
	fsetpos(file, &fpos);
	read = fread(line, 1, size, file);
	if(read != size)
	{
		free(line);
		return 0;
	}
	fgetc(file);
	line[size] = 0;
	return line;
}
char *lista_dir(char *path);
char * str_replace(char *str, char *old, char *new)
{
  int i, count = 0;
  int newlen = strlen(new);
  int oldlen = strlen(old);
  
  for (i = 0; str[i]; ++i)
    if (strstr(&str[i], old) == &str[i])
      ++count, i += oldlen - 1;
  
  char *ret = (char *) calloc(i + 1 + count * (newlen - oldlen), sizeof(char));
  if (!ret) return;
  
  i = 0;
  while (*str)
    if (strstr(str, old) == str)
      strcpy(&ret[i], new),
      i += newlen,
      str += oldlen;
    else
      ret[i++] = *str++;
    
  ret[i] = '\0';

  return ret;
}
int localiza(char s[],char t[])
{
    int i,j,k;

    for(i=0;s[i]!='\0';i++) {
     for(j=i,k=0;t[k]!='\0' && s[j]==t[k];j++,k++)
       ;
     if(k>0 && t[k]=='\0')
         return i;
    }
    return -1;
}
char from_hex(char ch);
char to_hex(char code);
char *url_encode(char *str);
char *url_decode(char *str);
/* Converts a hex character to its integer value */
char from_hex(char ch)
{
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}
/* Converts an integer value to its hex character*/
char to_hex(char code)
{
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}
/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_encode(char *str)
{
  char *pstr = str, *buf = malloc(strlen(str) * 3 + 1), *pbuf = buf;
  while (*pstr) {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~') 
      *pbuf++ = *pstr;
    else if (*pstr == ' ') 
      *pbuf++ = '+';
    else 
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}
////////
/* Returns a url-decoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_decode(char *str) {
  char *pstr = str, *buf = malloc(strlen(str) + 1), *pbuf = buf;
  while (*pstr) {
    if (*pstr == '%') {
      if (pstr[1] && pstr[2]) {
        *pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
        pstr += 2;
      }
    } else if (*pstr == '+') { 
      *pbuf++ = ' ';
    } else {
      *pbuf++ = *pstr;
    }
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}
int testa_extencao(char p[],char e[])
{
	if((localiza(p,e)>-1)&&(strlen(p)-localiza(p,e))==strlen(e))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
char *mime_content_type(char *name)
{
    char *ext,*t,*t2;
    FILE *arq;
    int flag=0;
    ext    = (char*)malloc(50*sizeof(char));
    t    = (char*)malloc(50*sizeof(char));
    t2    = (char*)malloc(5000*sizeof(char));
    arq=fopen("mime.types","r");
    if (arq == NULL){ perror("Erro ao abrir arquivo de mimetypes"); return "application/octet-stream"; }
    while ( fscanf( arq, "%s %s", t,t2) != EOF && flag==0 )
    {
		sprintf(ext,".%s",t);
		if(testa_extencao(name, ext))
		{
			return t2;
			flag=1;
			fclose(arq);
		}
    }
    fclose(arq);
    return "application/octet-stream";
}

int gravalog(char * log)
{
	 time_t now;
	 char timebuf[128];
	 int i;
	 now = time(NULL);
	 strftime(timebuf, sizeof(timebuf), FILE_LOG_DATE, localtime(&now));
	 char arq[1000];
	 strcpy(arq,timebuf);
	 strcat(arq,"_");
	 strcat(arq,FILE_LOG);
	 FILE *arq_log;
	 arq_log=fopen(arq, "a+");
	 if(arq_log==NULL) return -1;
	 fwrite(log, 1, strlen(log), arq_log);
	 fclose(arq_log);
	 return 0;
}
int gravap(int p)
{
	 FILE *arq_p;
	 char buf[128];
	 arq_p=fopen("process.list", "a+");
	 sprintf(buf,"%d\n",p);
	 if(arq_p==NULL) return -1;
	 fwrite(buf, 1, strlen(buf), arq_p);
	 fclose(arq_p);
	 return 0;
}

int excluip(int p)
{
	 FILE *arq_p;
	 char buf[128];
	 char processlist[MAX_CONN*2][10],tam=0,i;
	 arq_p=fopen("process.list", "r");
	 if(arq_p==NULL) return -1;	 
	 while ( fscanf( arq_p, "%s", processlist[tam]) != EOF )
	 {
		tam++;
	 }
	 fclose(arq_p);
	 arq_p=fopen("process.list", "w");
	 sprintf(buf,"%d",p);
	 for(i=0;i<tam;i++)
	 {
		if(strcmp(processlist[i],buf)!=0)fprintf(arq_p,"%s\n",processlist[i]);
	 }
	 fclose(arq_p);
	 return 0;
}

int stopserver=0;
int idm1,idm2,idm3;
key_t chave1 = SERVER_NAME"m1";
key_t chave2 = SERVER_NAME"m2";
key_t chave3 = SERVER_NAME"m3";
int *nprocessos,*processados;
sem_t *sem1;
sem_t *sem2;
sem_t *sem3;
sem_t *sem4;

static void stop_server(int signo) /* o argumento indica o sinal recebido */
{
	sem_wait(sem1);
	sem_wait(sem2);
	sem_wait(sem3);
	sem_wait(sem4);
	printf("Sinal recebido: %d\n", signo);
	printf("Iniciando processo de desligamento do servidor %s...\n", SERVER_NAME);
	int i=0,x;
	int t=0;
	
	FILE *arq_p;
	char processlist[MAX_CONN*2][10],tam=0;
	arq_p=fopen("process.list", "a+");
	fclose(arq_p);
	arq_p=fopen("process.list", "r");
	while ( fscanf( arq_p, "%s", processlist[tam]) != EOF )
		tam++;
	fclose(arq_p);
	for(i=0;i<tam;i++)
	{
		x=atoi(processlist[i]);
		printf("Matando processo %d: ",x);
		if(kill(x,SIGKILL))
		{
			printf("OK");
		}else
		{
			printf("FALHOU");
		}
		printf("\n");
	}
	remove("process.list");
	printf("Excluindo memoria %d: ",idm1);
	if ( shmctl( idm1, IPC_RMID, NULL) == -1 )
	{
		printf("FALHOU\n");
	}else
	{
		printf("OK\n");
	}
	printf("Excluindo memoria %d: ",idm2);
	if ( shmctl( idm2, IPC_RMID, NULL) == -1 )
	{
		printf("FALHOU\n");
	}else
	{
		printf("OK\n");
	}
	printf("Excluindo memoria %d: ",idm3);
	if ( shmctl( idm3, IPC_RMID, NULL) == -1 )
	{
		printf("FALHOU\n");
	}else
	{
		printf("OK\n");
	}
	printf("Fechando semaforo %s: ",SERVER_NAME"s1");
	if(sem_close(sem1)>=0)
	{
		printf("OK\n");
	}else
	{
		printf("FALHOU\n");
	}
	printf("Fechando semaforo %s: ",SERVER_NAME"s2");
	if(sem_close(sem2)>=0)
	{
		printf("OK\n");
	}else
	{
		printf("FALHOU\n");
	}
	printf("Fechando semaforo %s: ",SERVER_NAME"s3");
	if(sem_close(sem3)>=0)
	{
		printf("OK\n");
	}else
	{
		printf("FALHOU\n");
	}
	printf("Fechando semaforo %s: ",SERVER_NAME"s4");
	if(sem_close(sem4)>=0)
	{
		printf("OK\n");
	}else
	{
		printf("FALHOU\n");
	}
	printf("Removendo semaforo %s: ",SERVER_NAME"s1");
	if(sem_unlink (SERVER_NAME"s1")>=0)
	{
		printf("OK\n");
	}else
	{
		printf("FALHOU\n");
	}
	printf("Removendo semaforo %s: ",SERVER_NAME"s2");
	if(sem_unlink (SERVER_NAME"s2")>=0)
	{
		printf("OK\n");
	}else
	{
		printf("FALHOU\n");
	}
	printf("Removendo semaforo %s: ",SERVER_NAME"s3");
	if(sem_unlink (SERVER_NAME"s3")>=0)
	{
		printf("OK\n");
	}else
	{
		printf("FALHOU\n");
	}
	printf("Removendo semaforo %s: ",SERVER_NAME"s4");
	if(sem_unlink (SERVER_NAME"s4")>=0)
	{
		printf("OK\n");
	}else
	{
		printf("FALHOU\n");
	}
	printf("matando processo principal %d...\n",getpid());
	printf("Desligando servidor %s agora!\n", SERVER_NAME);
	if(kill(getpid(),SIGKILL))
	{
		printf("OK\n");
	}else
	{
		printf("FALHOU\n");
	}
	stopserver=1;
	return;
}
int main(int argc,char * argv[])
{
	int newSocket = 0, ServerSocket = 0;
	int pid=0;
	struct sockaddr_in servAddr, clientAddr;
	//int nprocessos=0,processados=0;
	// memoria compartilhada com semaforos
	
	if (chave1 == -1) {perror("Erro ao criar a chave1"); stop_server(SIGABRT);}
	if (chave2 == -1) {perror("Erro ao criar a chave2"); stop_server(SIGABRT);}
	if (chave3 == -1) {perror("Erro ao criar a chave3"); stop_server(SIGABRT);}
	//Cria a memoria compartilhada
	idm1 = shmget( chave1, sizeof(int *), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	idm2 = shmget( chave2, sizeof(int *), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	idm3 = shmget( chave3, sizeof(int *), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	if (idm1 == -1) {perror("Erro ao criar memoria1");stop_server(SIGABRT);}
	if (idm2 == -1) {perror("Erro ao criar memoria2");stop_server(SIGABRT);}
	if (idm3 == -1) {perror("Erro ao criar memoria3");stop_server(SIGABRT);}
	nprocessos = (int *) shmat(idm1, NULL, NULL);
	processados = (int *) shmat(idm2, NULL, NULL);
	if ( (int)(nprocessos) == -1 ){ perror("Erro ao obter endereço da mem1");stop_server(SIGABRT); }
	if ( (int)(processados) == -1 ){ perror("Erro ao obter endereço da mem2");stop_server(SIGABRT); }
	sem1 = sem_open(SERVER_NAME"s1", O_RDWR|O_CREAT, 0600, 1);
	sem2 = sem_open(SERVER_NAME"s2", O_RDWR|O_CREAT, 0600, 1);
	sem3 = sem_open(SERVER_NAME"s3", O_RDWR|O_CREAT, 0600, 1);
	sem4 = sem_open(SERVER_NAME"s4", O_RDWR|O_CREAT, 0600, 1);
	if(sem1==SEM_FAILED){printf("Erro na criaçao do semaforo1!\n");exit(-1);}
	if(sem2==SEM_FAILED){printf("Erro na criaçao do semaforo2!\n");exit(-1);}
	if(sem3==SEM_FAILED){printf("Erro na criaçao do semaforo3!\n");exit(-1);}
	if(sem4==SEM_FAILED){printf("Erro na criaçao do semaforo4!\n");exit(-1);}
	//pro SIGKILL funfar
	static struct sigaction act;
	act.sa_handler = stop_server;
	act.sa_flags    = 0;
	sigfillset(&(act.sa_mask));
	
	signal(SIGINT, SIG_IGN);
	/*
	if (signal(SIGINT, stop_server) == SIG_ERR) {
		fprintf(stderr, "SIGINT\n");
	}
	Nao funciona nem pro SIGTERM e nem pro SIGKILL
	if (signal(SIGKILL, stop_server) == SIG_ERR) {
		fprintf(stderr, "SIGKILL\n");
	}*/
	sigaction( SIGINT  , &act, NULL );
	sigaction( SIGTERM, &act, NULL );
	sigaction( SIGKILL  , &act, NULL );

	*nprocessos=0;
	*processados=0;
	ServerSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (ServerSocket<0)
	{
		perror("Nao foi possivel cria o socket!");
		return -1;
	}
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(SERVER_PORT);
	if(bind(ServerSocket, (struct sockaddr *) &servAddr, sizeof(servAddr))<0)
	{
		perror("Nao foi possivel alocar a porta para o socket!");
		return -1;
	}
	listen (ServerSocket, 5);
	while (1)
	{	
		char *log = (char*)malloc(5000*sizeof(char));
		char *date = (char*)malloc(12*sizeof(char));
		char *hour = (char*)malloc(10*sizeof(char));
		time_t t=time(NULL);
		struct tm *tm;
		tm=localtime(&t);
		if(stopserver==1)
		{
			stop_server(SIGABRT);
		}
		int clientLen = sizeof(clientAddr);
		newSocket = accept(ServerSocket, (struct sockaddr *) &clientAddr, &clientLen);
		if (newSocket<0)
		{
			perror("Nao foi possivel aceitar o pedido!");
			return -1;
		}
		pid = fork();
		if (pid==0)
		{
			char *buf, *method, *path, *npath, *version,*pperm;
			int notc=0, stat=0, intt=0;
			DIR *dp;
			struct stat perm;
			method  = (char*)malloc(50*sizeof(char));
			path    = (char*)malloc(400*sizeof(char));
			npath    = (char*)malloc(400*sizeof(char));
			buf    = (char*)malloc(400*sizeof(char));
			pperm    = (char*)malloc(4*sizeof(char));
			read(newSocket, buf,255);
			//extrair metodo caminho e protocolo do pedido recebido
			method = strtok(buf, " ");
			path = strtok(NULL, " ");
			version = strtok(NULL, " ");
			sem_wait (sem1);
			sem_wait (sem2);
			sem_wait (sem4);
			gravap(getpid());
			sem_post (sem4);
			*nprocessos=*nprocessos+1;
			printf("Pedido numero %d recebido %s.\nProcesso filho numero %d cuidara da requisicao.\n",*processados+1,path,getpid());
			printf("Numero de processos em execucao: %d.\nNumero de requisicoes processadas:%d\n",*nprocessos,*processados);
			sem_post (sem2);
			sem_post (sem1);
			if(*nprocessos>MAX_CONN)
			{			
				char *erro;
				erro    = (char*)malloc(100*sizeof(char));
				sprintf(erro,"Numero maximo de conex&otilde;es atingidas!");
				posta_arquivo(newSocket, 401, erro,"","htdocs/404.html","");
				sem_wait (sem1);
				sem_wait (sem2);
				sem_wait (sem4);
				excluip(getpid());
				sem_post (sem4);
				*nprocessos=*nprocessos-1;
				*processados=*processados+1;
				sem_post (sem2);
				sem_post (sem1);
				strftime(date, 51, TDATE, tm);
				strftime(hour, 51, THOUR, tm);
				sprintf(log,"Acesso em %s usando %s as %s do dia %s tratado pelo processo %d fechado com codigo %d - %s\n",path,method,hour,date,getpid(),401,erro);
				sem_wait (sem3);
				if(GRAVA_LOG==1)gravalog(log);
				sem_post (sem3);
				close(newSocket);
				exit(-1);
			}
			if(!path || !method || !version)
			{
				posta_arquivo(newSocket, 400, "Bad Request","","","");
				strftime(date, 51, TDATE, tm);
				strftime(hour, 51, THOUR, tm);

				sprintf(log,"Acesso em %s usando %s as %s do dia %s tratado pelo processo %d fechado com codigo %d - %s\n",path,method,hour,date,getpid(),400,"Bad Request");
				sem_wait(sem3);
				if(GRAVA_LOG==1){
					if(gravalog(log)==-1)
					{
						printf("Impossivel Gravar log!\n");
					}
				}
				sem_post(sem3);
				close(newSocket);
				exit(-1);
			}
			sprintf(path,"%s",url_decode(path));
			sprintf(npath,"%s%s",DOCUMENT_ROOT,path);
			dp=opendir(npath);
			stat=lstat(npath,&perm);
			sprintf(pperm,"%04o",perm.st_mode & 07777);
			if(dp!=NULL)
			{
				if(npath[strlen(npath)-1]=='/')
				{
					strcat(npath,"index.html");
					if(!fopen(npath,"r"))
					{
						sprintf(npath,"%s%s",DOCUMENT_ROOT,path);
						notc=1;
					}else
					{			
						strftime(date, 51, TDATE, tm);
						strftime(hour, 51, THOUR, tm);
						sprintf(log,"Encaminhando acesso para %s usando %s as %s do dia %s tratado pelo processo %d.\n",npath,method,hour,date,getpid());
						sem_wait (sem3);
						if(GRAVA_LOG==1)gravalog(log);
						sem_post (sem3);
					}
				}
				else
				{
					char *tmp, *responseString, *mimetype, *indexof;
					responseString=(char*)malloc((10000)*sizeof(char));
					indexof=(char*)malloc((10000)*sizeof(char));
					mimetype="text/html";
					sprintf(indexof, "<script>location.href='%s/';</script>\r\n",path);
					sprintf(responseString, "HTTP/1.0 200 OK\r\nContent-type: %s\r\nContent-length: %d\r\n\r\n",mimetype, strlen(indexof)*sizeof(char));
					write(newSocket, responseString, strlen(responseString));
					write(newSocket, indexof, strlen(indexof));
					sem_wait (sem1);
					sem_wait (sem2);
					sem_wait (sem4);
					excluip(getpid());
					sem_post (sem4);
					*nprocessos=*nprocessos-1;
					*processados=*processados+1;
					sem_post (sem2);
					sem_post (sem1);
					strftime(date, 51, TDATE, tm);
					strftime(hour, 51, THOUR, tm);
					sprintf(log,"Redirecionando %s para a pasta %s/, usando %s as %s do dia %s tratado pelo processo %d.\n",path,path,method,hour,date,getpid());
					sem_wait (sem3);
					if(GRAVA_LOG==1)gravalog(log);
					sem_post (sem3);
					close(newSocket);
					exit(0);
				}
			}
			if(strcmp("GET", method)==0)
			{
				int fd = open(npath, O_RDONLY);
				if(!stat && pperm[1]=='0')            //requested file was not found - send 404 error
				{
					char *erro;
					erro    = (char*)malloc(400*sizeof(char));
					sprintf(erro,"acess danied",path);
					posta_arquivo(newSocket, 401, erro,"","htdocs/404.html","");
					sem_wait (sem1);
					sem_wait (sem2);
					sem_wait (sem4);
					excluip(getpid());
					sem_post (sem4);
					*nprocessos=*nprocessos-1;
					*processados=*processados+1;
					sem_post (sem2);
					sem_post (sem1);
			
					strftime(date, 51, TDATE, tm);
					strftime(hour, 51, THOUR, tm);
					sprintf(log,"Acesso em %s usando %s as %s do dia %s tratado pelo processo %d fechado com codigo %d - %s\n",path,method,hour,date,getpid(),401,erro);
					sem_wait (sem3);
					if(GRAVA_LOG==1)gravalog(log);
					sem_post (sem3);
					close(newSocket);
					exit(0);
				}else if(-1 == fd)            //requested file was not found - send 404 error
				{
					char *erro;
					erro    = (char*)malloc(400*sizeof(char));
					sprintf(erro,"File not found",path);
					posta_arquivo(newSocket, 404, erro,"","htdocs/404.html","");
					sem_wait (sem1);
					sem_wait (sem2);
					sem_wait (sem4);
					excluip(getpid());
					sem_post (sem4);
					*nprocessos=*nprocessos-1;
					*processados=*processados+1;
					sem_post (sem2);
					sem_post (sem1);
			
					strftime(date, 51, TDATE, tm);
					strftime(hour, 51, THOUR, tm);
					sprintf(log,"Acesso em %s usando %s as %s do dia %s tratado pelo processo %d fechado com codigo %d - %s\n",path,method,hour,date,getpid(),404,erro);
					sem_wait (sem3);
					if(GRAVA_LOG==1)gravalog(log);
					sem_post (sem3);
					close(newSocket);
					exit(0);
				}
				else
				{
					char *tmp,*responseString, *mimetype, *indexof;
					int i=0;
					tmp=(char*)malloc(sizeof(char));
					if(notc==1)
					{
						mimetype="text/html";
						responseString=(char*)malloc((10000)*sizeof(char));
						indexof=(char*)malloc((100000)*sizeof(char));
						sprintf(indexof,"%s",lista_dir(path));
						sprintf(responseString, "HTTP/1.0 200 OK\r\nContent-type: %s\r\nContent-length: %d\r\n\r\n",mimetype, strlen(indexof)*sizeof(char));
						write(newSocket, responseString, strlen(responseString));
						write(newSocket, indexof, strlen(indexof));
						sem_wait (sem1);
						sem_wait (sem2);
						sem_wait (sem4);
						excluip(getpid());
						sem_post (sem4);
						*nprocessos=*nprocessos-1;
						*processados=*processados+1;
						sem_post (sem2);
						sem_post (sem1);
			
						strftime(date, 51, TDATE, tm);
						strftime(hour, 51, THOUR, tm);
						sprintf(log,"Acesso em %s usando %s as %s do dia %s tratado pelo processo %d fechado com codigo %d - %s %s\n",path,method,hour,date,getpid(),200,"OK Listando diretorio",path);
						sem_wait (sem3);
						if(GRAVA_LOG==1)gravalog(log);
						sem_post (sem3);
						close(newSocket);
						exit(0);
					}
					else
					{
						while(read(fd, tmp, 1)==1)
						{
							i++;
						}
						lseek(fd, 0, SEEK_SET);
						mimetype=mime_content_type(npath);
						responseString=(char*)malloc((100)*sizeof(char));
						sprintf(responseString, "HTTP/1.0 200 OK\r\nContent-type: %s\r\nContent-length: %d\r\n\r\n", mimetype, i);
						write(newSocket, responseString, strlen(responseString));
						while(read(fd, tmp, 1)==1)
						{
							write(newSocket, tmp, 1);
						}
						close(fd);
						sem_wait (sem1);
						sem_wait (sem2);
						sem_wait (sem4);
						excluip(getpid());
						sem_post (sem4);
						*nprocessos=*nprocessos-1;
						*processados=*processados+1;
						sem_post (sem2);
						sem_post (sem1);
			
						strftime(date, 51, TDATE, tm);
						strftime(hour, 51, THOUR, tm);
						sprintf(log,"Acesso em %s usando %s as %s do dia %s tratado pelo processo %d fechado com codigo %d - %s\n",path,method,hour,date,getpid(),200,"OK");
						sem_wait (sem3);
						if(GRAVA_LOG==1)gravalog(log);
						sem_post (sem3);
						close(newSocket);
						exit(0);
					}
				}
			}
			else
			{
				posta_arquivo(newSocket, 501, "Not implemented","","","");
				sem_wait (sem1);
				sem_wait (sem2);
				sem_wait (sem4);
				excluip(getpid());
				sem_post (sem4);
				*nprocessos=*nprocessos-1;
				*processados=*processados+1;
				sem_post (sem2);
				sem_post (sem1);
			
				strftime(date, 51, TDATE, tm);
				strftime(hour, 51, THOUR, tm);
				sprintf(log,"Acesso em %s usando %s as %s do dia %s tratado pelo processo %d fechado com codigo %d - %s\n",path,method,hour,date,getpid(),501,"Not implemented");
				sem_wait (sem3);
				if(GRAVA_LOG==1)gravalog(log);
				sem_post (sem3);
				close(newSocket);
				exit(0);
			}
		}
		else if (pid<0)
		{
			perror("Erro na criacao do processo!");
		}
	}
}
int posta_arquivo(int socket, int codigo, char *descricao_codigo,char *mensagem,char *arquivo,char *mime)
{
	char *msg = (char*)malloc(1000*sizeof(char));
	char *tmp = (char*)malloc(50*sizeof(char));
	char *date = (char*)malloc(12*sizeof(char));
	char *hour = (char*)malloc(10*sizeof(char));
	char c;
	char *linha = (char*)malloc(1000*sizeof(char));
	FILE *arq;
	int i=0;
	time_t t=time(NULL);
	struct tm *tm;
	tm=localtime(&t);
	strftime(date, 51, TDATE, tm);
	strftime(hour, 51, THOUR, tm);
	if(strlen(arquivo)>1)arq = fopen(arquivo, "r");
	else arq = fopen(DEFAULT_FILE_ERROR, "r");
	while(!feof(arq))
	{
		fread(&c,sizeof(char), 1,arq);
		i++;
	}
	linha=(char*)malloc(i*sizeof(char));
	fseek(arq,0,SEEK_SET);
	sprintf(tmp, "%d",codigo);
	fread(linha,sizeof(char), i,arq);
	linha=str_replace(linha,"_SERVER_",SERVER_NAME);
	linha=str_replace(linha,"_DATE_",date);
	linha=str_replace(linha,"_TIME_",hour);
	linha=str_replace(linha,"_ERROR_CODE_",tmp);
	linha=str_replace(linha,"_ERROR_DESC_",descricao_codigo);
	if(strlen(mensagem)<=1)
	{
		mensagem=" ";
	}
	linha=str_replace(linha,"_MSG_",mensagem);
	if(strlen(mime)<=1)
	{
		mime="text/html";
	}
	sprintf(msg, "HTTP/1.0 %d %s\r\nContent-type: %s\r\nContent-length: %d\r\n\r\n",codigo, descricao_codigo,mime, strlen(linha));
	write(socket, msg, strlen(msg));
	write(socket, linha, strlen(linha));
	fclose(arq);
	return 0;
}

char *lista_dir(char *path)
{
	DIR *dp;
	struct stat perm;
	char *indexof=(char*)malloc((10000)*sizeof(char));
	char *npath=(char*)malloc((10000)*sizeof(char));
	char *t=(char*)malloc((10000)*sizeof(char));
	char *date=(char*)malloc((30)*sizeof(char));
	sprintf(npath, "%s%s",DOCUMENT_ROOT,path);
	sprintf(indexof, "\
<html>\n\
	<head>\n\
		<title>%s -Index of /%s</title>\n\
	</head>",SERVER_NAME,path);
	
	sprintf(indexof, "\
%s\n\
	<body>\n\
		<h1>Index of %s</h1>\n\
		<br />\n\
		<hr />\n\
		<pre>\n\
			<table border=\"0\" width=\"80%c\">",indexof, path,'%');
	sprintf(indexof, "\
%s\n\
				<tr>\n\
					<td>Nome</td>\n\
					<td>Tipo</td>\n\
					<td>Permissao</td>\n\
					<td>Tamanho</td>\n\
					<td>Modificado em</td>\n\
				</tr>",indexof);
	struct dirent *namelist;
	dp = opendir (npath);
	while (namelist=readdir(dp))
	{
		sprintf(t, "%s%s",npath,namelist->d_name);
		lstat(t,&perm);
		strftime(date,30,TDATE" "THOUR,localtime(&perm.st_mtime));
		sprintf(indexof, 
			"%s\n\
				<tr>\n\
					<td>\n\
						<a href=\"%s%s\">%s</a>\n\
					</td>\n\
					<td>%s</td>\n\
					<td>%04o</td>\n\
					<td>%ld Kb</td>\n\
					<td>%s</td>\n\
				</tr>",indexof,path,namelist->d_name,namelist->d_name,(opendir(t)==NULL?mime_content_type(t):"dir"),perm.st_mode & 07777,perm.st_size/1024,date);
	}
	sprintf(indexof, 
"%s\n\
			</table>\n\
			<hr />\n\
		</pre>\n\
		<address>%s</address>\n\
	</body>\n\
</html>",indexof,SERVER_NAME);
	closedir(dp);
	return indexof;
}
