#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <signal.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <my_global.h>
#include <mysql.h>
#define PORT 20000
#define BACKLOG 5
#define LENGTH 512
void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);        
}

int checklogin(char* name,char* pw){
  void finish_with_error(MYSQL *con)
  {
      fprintf(stderr, "%s\n", mysql_error(con));
      mysql_close(con);
      exit(1);        
  }
  MYSQL *con = mysql_init(NULL);
  
    if (con == NULL) 
    {
      fprintf(stderr, "%s\n", mysql_error(con));
      exit(1);
    }  

  if (mysql_real_connect(con, "localhost", "root", "123456","ltm", 0, NULL, 0) == NULL) 
    {
      finish_with_error(con);
    }

  if (mysql_query(con, "SELECT * FROM Users")) 
  {
    finish_with_error(con);
  }
  
  MYSQL_RES *result = mysql_store_result(con);
  
  if (result == NULL) 
    {
    finish_with_error(con);
    }

  int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;
  while ((row = mysql_fetch_row(result))) 
    { 
    for(int i = 0; i < num_fields; i++) 
      { 
          if(strcmp(row[i],name)==0 && strcmp(row[i+1],pw)==0) return 1;          
      } 
           
    }

  mysql_free_result(result);      
  mysql_close(con);
  return 0;
}

int signup(char* name,char* pw){
  char query[100];
  MYSQL *con = mysql_init(NULL);
  
  if (con == NULL) 
  {
      fprintf(stderr, "%s\n", mysql_error(con));
      exit(1);
  }  

  if (mysql_real_connect(con, "localhost", "root", "123456", 
           "ltm", 0, NULL, 0) == NULL) 
  {
      finish_with_error(con);
  }

  if (mysql_query(con, "SELECT name FROM Users")) 
  {
    finish_with_error(con);
  }
  
  MYSQL_RES *result = mysql_store_result(con);
  
  if (result == NULL) 
    {
    finish_with_error(con);
    }
       
  int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;
  while ((row = mysql_fetch_row(result))) 
    { 
    for(int i = 0; i < num_fields; i++) 
      { 
          if(strcmp(row[i],name)==0) return 0;          
      } 
           
    }
  sprintf(query,"INSERT INTO Users(name,pass) VALUES ('%s','%s')",name,pw);
  if (mysql_query(con,query)) {
        finish_with_error(con);
    }
  return 1; 
  mysql_close(con);
    
}
void error(const char *msg)
{
  perror(msg);
  exit(1);
}

typedef int boolean;
//enum { FALSE, TRUE };


void send_message(char *sdbuf, int sockfd){
  if(send(sockfd, sdbuf, sizeof(sdbuf), 0) < 0){
    fprintf(stderr, "ERROR: Failed to send message. (errno = %d)\n", errno);
  }
  printf("Ok message from Client was Sent!\n");
}

void receive_message(int nsockfd, char *revbuf){
  bzero(revbuf, LENGTH);
  int fr_block_sz = 0;
  if(fr_block_sz = recv(nsockfd, revbuf, LENGTH, 0) > 0);
  if(fr_block_sz < 0){
    if (errno == EAGAIN){
      printf("recv() timed out.\n");
    }
    else{
      fprintf(stderr, "recv() failed due to errno = %d\n", errno);
      exit(1);
    }
  }
  printf("Ok received message from client!\n");
}

void send_file(char *fs_name, int nsockfd){
  /* Send index to client */
  char sdbuf[LENGTH]; // Send buffer
  printf("[Server] Sending %s to the Client...", fs_name);
  FILE *fs = fopen(fs_name, "r");
  if(fs == NULL){
    fprintf(stderr, "ERROR: File %s not found on server. (errno = %d)\n", fs_name, errno);
    exit(1);
  }
  bzero(sdbuf, LENGTH);
  int fs_block_sz;
  while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs))>0){
    if(send(nsockfd, sdbuf, fs_block_sz, 0) < 0){
      fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fs_name, errno);
      exit(1);
    }
    bzero(sdbuf, LENGTH);
  }
  printf("Ok sent to client!\n");
  close(nsockfd);
  printf("[Server] Connection with Client closed. Server will wait now...\n");
  while(waitpid(-1, NULL, WNOHANG) > 0);
  fclose(fs);
}

void receive_file(char *fr_name, int nsockfd){
  char revbuf[LENGTH];
  printf("[Client] Receiveing file from Server and saving it as %s...\n", fr_name);
  FILE *fr = fopen(fr_name, "w");
  if(fr == NULL)
    printf("File %s Cannot be opened.\n", fr_name);
  else{
    bzero(revbuf, LENGTH);
    int fr_block_sz = 0;
    while((fr_block_sz = recv(nsockfd, revbuf, LENGTH, 0)) > 0){
      int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
      if(write_sz < fr_block_sz){
        error("File write failed.\n");
      }
      bzero(revbuf, LENGTH);
      if (fr_block_sz == 0 || fr_block_sz != 512){
        break;
      }
    }
    if(fr_block_sz < 0){
      if (errno == EAGAIN){
        printf("recv() timed out.\n");
      }
      else{
        fprintf(stderr, "recv() failed due to errno = %d\n", errno);
      }
    }
    printf("Ok received from client!\n");
    close(nsockfd);
    printf("[Server] Connection with Client closed. Server will wait now...\n");
    while(waitpid(-1, NULL, WNOHANG) > 0);
    fclose(fr);
  }
}

void main(){
  /* Defining Variables */
  int sockfd;
  int nsockfd;
  int num;
  int sin_size;
  struct sockaddr_in addr_local; /* client addr */
  struct sockaddr_in addr_remote; /* server addr */
  char revbuf[LENGTH]; // Receiver buffer
  int auth_success;

  /* Get the Socket file descriptor */
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
    fprintf(stderr, "ERROR: Failed to obtain Socket Descriptor. (errno = %d)\n", errno);
    exit(1);
  }
  else
    printf("[Server] Obtaining socket descriptor successfully.\n");

  /* Fill the client socket address struct */
  addr_local.sin_family = AF_INET; // Protocol Family
  addr_local.sin_port = htons(PORT); // Port number
  addr_local.sin_addr.s_addr = INADDR_ANY; // AutoFill local address
  bzero(&(addr_local.sin_zero), 8); // Flush the rest of struct

  /* Bind a special Port */
  if( bind(sockfd, (struct sockaddr*)&addr_local, sizeof(struct sockaddr)) == -1 ){
    fprintf(stderr, "ERROR: Failed to bind Port. (errno = %d)\n", errno);
    exit(1);
  }
  else
    printf("[Server] Binded tcp port %d in addr 127.0.0.1 sucessfully.\n",PORT);

  /* Listen remote connect/calling */
  if(listen(sockfd,BACKLOG) == -1){
    fprintf(stderr, "ERROR: Failed to listen Port. (errno = %d)\n", errno);
    exit(1);
  }
  else
    printf ("[Server] Listening the port %d successfully.\n", PORT);

  while(1){
    sin_size = sizeof(struct sockaddr_in);

    /* Wait a connection, and obtain a new socket file despriptor for single connection */
    if ((nsockfd = accept(sockfd, (struct sockaddr *)&addr_remote, &sin_size)) == -1){
        fprintf(stderr, "ERROR: Obtaining new Socket Despcritor. (errno = %d)\n", errno);
      exit(1);
    }
    else printf("[Server] Server has got connected from %s.\n", inet_ntoa(addr_remote.sin_addr));

    bzero(revbuf, LENGTH);   
    receive_message(nsockfd, revbuf);
    printf("%s\n", revbuf);
    if(revbuf[0]== '8') {
      char *name,*password;
      name=strtok(&revbuf[2]," ");
      password=strtok(NULL," ");
      printf("%s\n",name);
      printf("%s\n",password);
      int auth_success = signup(name,password);
      write(nsockfd, &auth_success, sizeof(int));
    } 

    if(revbuf[0]== '9') {
      char *name,*password;
      name=strtok(&revbuf[2]," ");
      password=strtok(NULL," ");
      printf("%s\n",name);
      printf("%s\n",password);
      int auth_success = checklogin(name,password);
      write(nsockfd, &auth_success, sizeof(int));
    }
 
    if(auth_success){
      bzero(revbuf, LENGTH);
      receive_message(nsockfd, revbuf);
      /* List all file name */
      if(revbuf[0] == '1'){
        /* Update File list to index.txt and send back to client*/
        system("ls Files > index.txt");
        send_file("index.txt", nsockfd);
      }

      /* Client want download a file */
      else if(revbuf[0] == '2'){
        char file_url[100] = "./Files/";
        strcat(file_url, &revbuf[2]);
        int str;
        str = strlen(file_url);
        printf("%d\t%s\n",str,file_url);
        //file_url[str+1] = '\0';
        send_file(file_url, nsockfd);
      }

      /* Client want upload a file*/
      else if(revbuf[0] == '3'){
        char file_url[100] = "./Files/";
        strcat(file_url, &revbuf[2]);
        receive_file(file_url, nsockfd);
      }
    }
  }
}
