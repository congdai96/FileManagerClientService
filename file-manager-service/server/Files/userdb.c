#include <stdio.h>
#include <my_global.h>
#include <mysql.h>

void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);        
}

int main()
{
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
  
  if (mysql_query(con, "DROP TABLE IF EXISTS Users")) {
      finish_with_error(con);
  }
  
  if (mysql_query(con, "CREATE TABLE Users(name VARCHAR(20) NOT NULL, pass VARCHAR(20) NOT NULL)")) {      
      finish_with_error(con);
  }
  
  if (mysql_query(con, "INSERT INTO Users(name,pass) VALUES('Khoa','yeuquyen')")) {
       finish_with_error(con);
   }
  
  if (mysql_query(con, "INSERT INTO Users(name,pass) VALUES('Quyen','quenkhoa')")) {
       finish_with_error(con);
   }
  
  if (mysql_query(con, "INSERT INTO Users(name,pass) VALUES('Thai','bopdai')")) {
       finish_with_error(con);
   }
  
  if (mysql_query(con, "INSERT INTO Users(name,pass) VALUES('Lam','beo')")) {
      finish_with_error(con);
  }
  
  if (mysql_query(con, "INSERT INTO Users(name,pass) VALUES('Nam','hatxi')")) {
      finish_with_error(con);
  }
  
  if (mysql_query(con, "INSERT INTO Users(name,pass) VALUES('Duan','khocnhe')")) {
      finish_with_error(con);
  }
  
  if (mysql_query(con, "INSERT INTO Users(name,pass) VALUES('Thien','yeulam')")) {
     finish_with_error(con);
  }

  mysql_close(con);
  exit(0);
}
