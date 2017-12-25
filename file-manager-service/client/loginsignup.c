#include <stdio.h>
#include <my_global.h>
#include <mysql.h>
int signup(char* name,char* pw){
  char query[100];
  MYSQL *con = mysql_init(NULL);
  
  if (con == NULL) 
  {
      fprintf(stderr, "%s\n", mysql_error(con));
      exit(1);
  }  

  if (mysql_real_connect(con, "localhost", "root", "16101996", 
           "testdb", 0, NULL, 0) == NULL) 
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
  sprintf(query,"INSERT INTO(name,pass) User VALUES ('%s','%s')",name,pw);
  if (mysql_query(con,query)) {
        finish_with_error(con);
    }
  return 1; 
  mysql_close(con);
    
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

  if (mysql_real_connect(con, "localhost", "root", "16101996","testdb", 0, NULL, 0) == NULL) 
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

void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);        
}

void main()
{
  char name[100],pw[100];
  printf("nhap id: ");
  scanf("%s",name);
  printf("nhap pass: ");
  scanf("%s",pw);
  printf("ketqua CheckLogin:%d\n",checklogin(name,pw));
  printf("ketqua SignUp:%d\n",signup(name,pw));


}
