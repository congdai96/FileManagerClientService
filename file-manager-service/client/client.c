#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <signal.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define PORT 20000
#define LENGTH 512
#define SERVER_ADDR "127.0.0.1"

typedef int boolean;
enum { FALSE, TRUE };
boolean sign_up();
boolean sign_in();
void get_file_names();
void download_file();

void error(const char *msg)
{
  perror(msg);
  exit(1);
}

int server_has_that_file(char *fname, char *str) {
  FILE *fp;
  int line_num = 1;
  int find_result = 0;
  char temp[512];

  if((fp = fopen(fname, "r")) == NULL) {
   return(-1);
  }

  while(fgets(temp, 512, fp) != NULL) {
    /* Remove newline character */
    size_t ln = strlen(temp) - 1;
    if (*temp && temp[ln] == '\n') temp[ln] = '\0';
    /* Compare */
    if(strcmp(temp, str) == 0) {
      find_result++;
    }
  }

  if(fp) {
    fclose(fp);
  }
  return find_result;
}

void send_message(char *sdbuf, int sockfd){
  if(send(sockfd, sdbuf, strlen(sdbuf), 0) < 0){
    fprintf(stderr, "ERROR: Failed to send message. (errno = %d)\n", errno);
  }
  printf("Ok message from Client was Sent!\n");
}

void send_file(char *fs_name, int sockfd){
  char sdbuf[LENGTH];
  printf("[Client] Sending %s to the Server... ", fs_name);
  FILE *fs = fopen(fs_name, "r");
  if(fs == NULL){
    printf("ERROR: File %s not found.\n", fs_name);
    exit(1);
  }

  bzero(sdbuf, LENGTH);
  int fs_block_sz;
  while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs)) > 0){
    if(send(sockfd, sdbuf, fs_block_sz, 0) < 0){
      fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fs_name, errno);
      break;
    }
    bzero(sdbuf, LENGTH);
  }
  printf("Ok File %s from Client was Sent!\n", fs_name);
}


void *receive_message(int sockfd, char *revbuf){
  bzero(revbuf, LENGTH);
  int fr_block_sz = 0;
  if(fr_block_sz = recv(sockfd, revbuf, LENGTH, 0) > 0);
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

void receive_file(char *fr_name, int sockfd){
  char revbuf[LENGTH];
  printf("[Client] Receiveing file from Server and saving it as %s...\n", fr_name);
  FILE *fr = fopen(fr_name, "w");
  if(fr == NULL)
    printf("File %s Cannot be opened.\n", fr_name);
  else{
    bzero(revbuf, LENGTH);
    int fr_block_sz = 0;
    while((fr_block_sz = recv(sockfd, revbuf, LENGTH, 0)) > 0){
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
      printf("Ok received from server!\n");
      fclose(fr);
  }
}

void print_files_name(){
  int c;
  FILE *file;
  file = fopen("index.txt", "r");
  printf("Danh sach file do server quan ly:\n");
  if (file) {
    while ((c = getc(file)) != EOF)
      putchar(c);
    fclose(file);
  }
}

boolean sign_up(int sockfd){
  char str1[50],str2[50];
  printf("Nhap user name :");
  scanf("%s",str1);
  printf("Nhap password:");
  scanf("%s",str2);
  char* remess;
  char message[100] = "8 ";
  strcat(message,str1);
  strcat(message," ");
  strcat(message,str2);
  send_message(message, sockfd);
  bzero(message, 100);
  char buffer[50];
  int ret;
  read(sockfd, &ret, sizeof(ret));
  if(ret == 1) printf("Successfully signup.\n");
  else printf("Failed signup.\n");
  return ret;
}

boolean sign_in(int sockfd){
  char str1[50],str2[50];
  printf("Nhap user name :");
  scanf("%s",str1);
  printf("Nhap password:");
  scanf("%s",str2);
  char* remess;
  char message[100] = "9 ";
  strcat(message,str1);
  strcat(message," ");
  strcat(message,str2);
  send_message(message, sockfd);
  bzero(message, 100);
  char buffer[50];
  int ret;
  read(sockfd, &ret, sizeof(ret));
  if(ret == 1) printf("Successfully signin.\n");
  else printf("Failed signin.\n");
  return ret;
}

void main(){
  boolean login_status;

  char file_name[50];
  int sockfd;
  int nsockfd;
  char revbuf[LENGTH];
  struct sockaddr_in remote_addr;

  /* Get the Socket file descriptor */
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    fprintf(stderr, "ERROR: Failed to obtain Socket Descriptor! (errno = %d)\n",errno);
    exit(1);
  }

  /* Fill the socket address struct */
  remote_addr.sin_family = AF_INET;
  remote_addr.sin_port = htons(PORT);
  inet_pton(AF_INET, SERVER_ADDR, &remote_addr.sin_addr);
  bzero(&(remote_addr.sin_zero), 8);  

  /* Try to connect the remote */
  if (connect(sockfd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1){
    fprintf(stderr, "ERROR: Failed to connect to the host! (errno = %d)\n",errno);
    exit(1);
  }
  else printf("[Client] Connected to server at port %d...ok!\n", PORT);

  /* Nguoi dung phai dang nhap de tiep tuc */
  printf("Hay dang nhap, dang ki de tiep tuc chuong trinh: \n");
  printf("1. Dang ki.\n");
  printf("2. Dang nhap.\n");
  printf("0. Thoat chuong trinh.\n");
  printf("Lua chon cua ban: ");
  int n;
  do{
    scanf("%d", &n);
    switch(n){
      case 1:
        
        // ret = recv(sockfd, buffer, LENGTH, 0, NULL); 
        // printf("%s\n",buffer);
        //strcat(str1,str2);
        //send_message(str1, sockfd);
        //signup(str1,str2);
        //printf("%d\n", signup(str1,str2));
        //if(signup(str1,str2)==1) login_status = sign_up();
        login_status = sign_up(sockfd);
        break;
      case 2:
        login_status = sign_in(sockfd);
        break;
      case 0:
        exit(0);
      default:
        printf("Nhap sai, xin nhap lai: ");
    }
  }while(!login_status);
  if(login_status){
    printf("Ban muon dung chuc nang nao: \n");
    printf("1. Danh sach file cua server.\n");
    printf("2. Down load file.\n");
    printf("3. Upload to server.\n");
    printf("0. Thoat chuong trinh.\n");
    printf("Lua chon cua ban: ");
    do{
      scanf("%d", &n);
      switch(n){
        case 1:
          send_message("1", sockfd); //i want to see all file name
          receive_file("index.txt", sockfd);
          print_files_name();
          break;
        case 2:
          printf("Nhap ten file ban muon download: ");
          scanf("%s", file_name);
          if(server_has_that_file("index.txt", file_name)){
            char message[50] = "2 ";
            strcat(message, file_name);
            send_message(message, sockfd);
            receive_file(file_name, sockfd);
          }
          else printf("File ban nhap khong ton tai.\n");
          break;
        case 3:
          printf("Nhap ten file ban muon upload: ");
          scanf("%s", file_name);
          char message[50] = "3 ";
          strcat(message, file_name);
          send_message(message, sockfd);
          send_file(file_name, sockfd);
        case 0:
          exit(0);
        default:
          printf("Nhap sai, xin nhap lai: ");
      }
    }while(n != 0);
  }
  close (sockfd);
}
