/**
* simple http Server
* Seminar Systemmodellierung
* date: 04-20-2003
* author: Tassilo Glander
*         Christian Hentschel
* http://wendtstud2.hpi.uni-potsdam.de/sysmod-seminar/SS2003/presentations/gruppe-1/httpServer.c
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>


#define MAXLENGTH 500
#define MAXLINES 50
#define PORT 8080
#define DOC_ROOT "htdocs/"

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

int testa_extencao(char p[],char e[])
{
	if(
		(localiza(p,e)>-1)
		&&
		(strlen(p)-localiza(p,e))==strlen(e)
	)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

char *mime_content_type(char *name){
	char *buf;
	/* Text */
	printf("%d\n",testa_extencao(name, ".txt"));
	printf("%d\n",testa_extencao(name, ".html"));
    if ( testa_extencao(name, ".txt")|| testa_extencao(name, ".h")|| testa_extencao(name, ".c")|| testa_extencao(name, ".cpp")){
        buf = "text/plain";
    } else if ( testa_extencao(name, ".css" )){
        buf = "text/css";
    } else if ( testa_extencao(name, ".js" )){
        buf = "text/javascript";
    } else if ( testa_extencao(name, ".xml")|| testa_extencao(name, ".xsl")){
        buf = "text/xml";
    } else if ( testa_extencao(name, ".xhtm")|| testa_extencao(name, ".xhtml")|| testa_extencao(name, ".xht")){
        buf = "application/xhtml+xml";
    } else if ( testa_extencao(name, ".html")|| testa_extencao(name, ".htm")|| testa_extencao(name, ".shtml")|| testa_extencao(name, ".hts")){
        buf = "text/html";
	/* Images */
    } else if ( testa_extencao(name, ".gif" )){
        buf = "image/gif";
    } else if ( testa_extencao(name, ".png" )){
        buf = "image/png";
    } else if ( testa_extencao(name, ".bmp" )){
        buf = "application/x-MS-bmp";
    } else if ( testa_extencao(name, ".jpg" )|| testa_extencao(name, ".jpeg" )|| testa_extencao(name, ".jpe" )|| testa_extencao(name, ".jpz" )){
        buf = "image/jpeg";
	/* Audio & Video */
    } else if ( testa_extencao(name, ".wav" )){
        buf = "audio/wav";
    } else if ( testa_extencao(name, ".wma" )){
        buf = "audio/x-ms-wma";
    } else if ( testa_extencao(name, ".wmv" )){
        buf = "audio/x-ms-wmv";
    } else if ( testa_extencao(name, ".au" )|| testa_extencao(name, ".snd" )){
        buf = "audio/basic";
    } else if ( testa_extencao(name, ".midi" )|| testa_extencao(name, ".mid" )){
        buf = "audio/midi";
    } else if ( testa_extencao(name, ".mp3" )|| testa_extencao(name, ".mp2" )){
        buf = "audio/x-mpeg";
	} else if ( testa_extencao(name, ".rm" ) || testa_extencao(name, ".rmvb" )|| testa_extencao(name, ".rmm" )){
        buf = "audio/x-pn-realaudio";
    } else if ( testa_extencao(name, ".avi" )){
        buf = "video/x-msvideo";
    } else if ( testa_extencao(name, ".3gp" )){
        buf = "video/3gpp";
    } else if ( testa_extencao(name, ".mov" )){
        buf = "video/quicktime";
    } else if ( testa_extencao(name, ".wmx" )){
        buf = "video/x-ms-wmx";
	} else if ( testa_extencao(name, ".asf" ) || testa_extencao(name, ".asx" )){
        buf = "video/x-ms-asf";
    } else if ( testa_extencao(name, ".mp4" )|| testa_extencao(name, ".mpg4" )){
        buf = "video/mp4";
    } else if ( testa_extencao(name, ".mpe" ) || testa_extencao(name, ".mpeg" )|| testa_extencao(name, ".mpg" )|| testa_extencao(name, ".mpga" )){
	buf = "video/mpeg";
	/* Documents */
    } else if ( testa_extencao(name, ".pdf" )){
        buf = "application/pdf";
    } else if ( testa_extencao(name, ".rtf" )){
        buf = "application/rtf";
    } else if ( testa_extencao(name, ".doc" ) || testa_extencao(name, ".dot" )){
	buf = "application/msword";
    } else if ( testa_extencao(name, ".xls" ) || testa_extencao(name, ".xla" )){
	buf = "application/msexcel";
    } else if ( testa_extencao(name, ".hlp" ) || testa_extencao(name, ".chm" )){
	buf = "application/mshelp";
    } else if ( testa_extencao(name, ".swf" ) || testa_extencao(name, ".swfl" )|| testa_extencao(name, ".cab" )){
	buf = "application/x-shockwave-flash";
    } else if ( testa_extencao(name, ".ppt" ) || testa_extencao(name, ".ppz" )|| testa_extencao(name, ".pps" )|| testa_extencao(name, ".pot" )){
	buf = "application/mspowerpoint";
	/* Binary & Packages */
    } else if ( testa_extencao(name, ".zip" )){
        buf = "application/zip";
    } else if ( testa_extencao(name, ".rar" )){
        buf = "application/x-rar-compressed";
    } else if ( testa_extencao(name, ".gz" )){
        buf = "application/x-gzip";
    } else if ( testa_extencao(name, ".jar" )){
        buf = "application/java-archive";
    } else if ( testa_extencao(name, ".tgz" ) || testa_extencao(name, ".tar" )){
	buf = "application/x-tar";
    } else {
	buf = "application/octet-stream";
    }
    printf("%s\n",buf);
    return buf;
}

int main (int argc, char * argv[])
{
   int newSocket = 0, ServerSocket = 0;
   struct sockaddr_in servAddr, clientAddr;

   //******************************socket()
   ServerSocket = socket(PF_INET, SOCK_STREAM, 0);
   //******************************socket()
   if (0 > ServerSocket)
   {
      perror("cannot open socket ");
      return -1;
   }


   /* bind server port */
   servAddr.sin_family = AF_INET;
   servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   servAddr.sin_port = htons(PORT);

   //******************************bind()
   if(0 > bind(ServerSocket, (struct sockaddr *) &servAddr, sizeof(servAddr)))
   //******************************bind()
   {
      perror("cannot bind port ");
      return -1;
   }

   //******************************listen()
   listen (ServerSocket, 5);
   //******************************listen()

   while (1)
   {
      int pid;
      int clientLen = sizeof(clientAddr);

      //******************************accept()
      newSocket = accept(ServerSocket, (struct sockaddr *) &clientAddr, &clientLen);
      //******************************accept()

      if (0 > newSocket)
      {
         perror("cannot accept connection ");
         return -1;
      }

      pid = fork();

      if (0 == pid)     //child thread
      {
         char *buf[MAXLINES], *method, *path, *npath, *version;

         method  = (char*)malloc(50*sizeof(char));
         path    = (char*)malloc(400*sizeof(char));
         npath    = (char*)malloc(400*sizeof(char));
         version = (char*)malloc(50*sizeof(char));

         if(-1 == readRequest(newSocket, buf))                 //read request of web-browser
         {
            httpError(newSocket, 400, "Bad Request");
            close(newSocket);
         }

         if (-1 == parseCommand(buf[0], method, path, version)) //extract command, path and version
         {
            httpError(newSocket, 400, "Bad Request");
            close(newSocket);
         }
	 sprintf(npath,"%s%s",DOC_ROOT,path);
	 strcpy(path,npath);
         if(strcmp("GET", method)==0)                 //found GET method -> create response
         {
            int fd = open(path, O_RDONLY);
            if(-1 == fd)            //requested file was not found - send 404 error
            {
	       char *erro;
	       erro    = (char*)malloc(400*sizeof(char));
		sprintf(erro,"File \"%s\" not found!",path);
               httpError(newSocket, 404, erro);
               close(newSocket);
            }
            else                                      //found file
            {
               char *tmp, *responseString, *mimetype;
               int i=0;

               tmp=(char*)malloc(sizeof(char));
               //get file size for content length
               while(1==read(fd, tmp, 1))
               {
                  i++;
               }
               lseek(fd, 0, SEEK_SET);
	       mimetype=mime_content_type(path);
               responseString=(char*)malloc((100)*sizeof(char));
               sprintf(responseString, "HTTP/1.0 200 OK\r\nContent-type: %s\r\nContent-length: %d\r\n\r\n", mimetype, i);

               write(newSocket, responseString, strlen(responseString));
               while(1==read(fd, tmp, 1))
               {
                  write(newSocket, tmp, 1);
               }
               close(fd);

               close(newSocket);
            }
         }
         else                                         //found an unsupported HTTP method
         {
            httpError(newSocket, 501, "Not implemented");
            close(newSocket);
         }
      }
      else if (0 > pid) //error occured while creating new thread
      {
         perror("error in thread creation ");
      }
   }
}


/*
 * methods writes HTTP Error-File with given error-code and error-description to given Socket
 *
 * params:  sd - Socketdescriptor
 *          errorCode - error code to send
 *          description - string that explains error code
 * returns: 0 (a void return type would create curious compiler warning!)
 */
int httpError(int sd, int errorCode, char *description)
{
   char *errorstring = (char*)malloc(500 * sizeof(char));
   char *body = (char*)malloc(400*sizeof(char));

   sprintf(body, "<html><head><title>Simple WebServer - ERROR</title></head><body><h1>HTTP/1.0 %d</h1><h3>%s</h3></body></html>", errorCode, description);
   sprintf(errorstring, "HTTP/1.0 %d %s \r\nContent-type: text/html\r\nContent-length: %d\r\n\r\n%s", errorCode, description, strlen(body), body);

   write(sd, errorstring, strlen(errorstring));

   return 0;
}


/*
 * method reads strings from filedescriptor sd
 * strings that are seperated by a '\n' are stored seperately into an array of char pointers
 * each string may have a length of up to MAXLENGTH chars
 *
 * params:  sd - filedescriptor to read from
 *          buf - array of char ptrs to store separated strings in
 * returns: number of strings read from filedescriptor on success
 *          -1 on failure (strings longer than MAXVALUE chars)
 */
int readRequest(int sd, char * * buf)
{
   char *ch, *s;
   int i=0, j=0;

   ch = (char*)malloc(sizeof(char));
   s  = (char*)malloc(MAXLENGTH*sizeof(char));

   while(-1 != read(sd, ch, 1))   //as longs as still characters in Socket
   {
      if(*ch == '\r')
      {
         //do nothing
      }
      else if (*ch == '\n')      //indicates end of line in stream -> terminate string in array
      {
         if(0 >= i)             //end of line reached but no character was read from stream -> indicates end of request!
         {
            free(s);             //frees memory used for string
            free(ch);            //frees memory used for single char
            return j;
         }
         else                    //reached end of line -> terminate string, write string ptr to array and allocate memory for new string
         {
            s[i+1]='\0';         //terminate string
            buf[j]=s;            //write ptr to string in array
            s = (char*)malloc(MAXLENGTH*sizeof(char));    //allocate new memory for next string
            i=0;
            j++;
         }
      }
      else
      {
         if ((MAXLENGTH-1) < i)
         {
            return -1;     //line is longer than allocated memory!
         }
         s[i]=*ch;
         i++;
      }
   }
}

/*
 * method extracts http method, requested filename and http version from given
 *
 * params:  comline - the string to be parsed
 *          method, path, version - separated substrings are store here
 * returns: 0 on success
 *         -1 on error (comline's syntax does not correspond to HTTP1.1 specs)
 */

int parseCommand(char *comline, char *method, char *path, char *version)
{
   int i=0, j=0;

   method[0]  ='\0';
   path[0]    ='\0';
   version[0] ='\0';

   //extract method
   while((strlen(comline)>i)&&(' ' != comline[i]))
   {
      method[j]=comline[i];
      j++;
      i++;
   }
   method[j]='\0';      //terminate method
   i++;
   j=0;

   //extract path
   i++;                 //very first character of path is '/' - indicates root - we don't need it for filename
   while((strlen(comline)>i)&&(' ' != comline[i]))
   {
      path[j]=comline[i];
      j++;
      i++;
   }
   path[j]='\0';        //terminate path
   i++;
   j=0;

   //extract HTTP-version
   while(strlen(comline)>i)
   {
      if(' '==comline[i])     //another (third) whitespace - illegal commandline!!
      {
         return -1;
      }
      else
      {
         version[j]=comline[i];
         j++;
         i++;
      }
   }
   version[j]='\0';     //terminate version

   if( (strlen(method)>0) && (strlen(version)>0) )
   {
      return 0;
   }
   else
   {
      return -1;
   }
}
