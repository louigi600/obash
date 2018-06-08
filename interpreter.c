/* Obfuscated Bash
// Copyright (C) 2017- Davide Rao: louigi600 (at) yahoo (dot) it
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version provided that no poit of the 
// AA License is violated 
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/* This is not directly used but is manupulated by recreate_interpreter_header to make it into interpreter.h */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/buffer.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#gv char *includes;
#gv char *functions;
#gv char *main_body;
#gv char prod_serial[256]="/sys/devices/virtual/dmi/id/product_serial";
#gv char prod_uuid[256]="/sys/devices/virtual/dmi/id/product_uuid";
#gv char iv[17]="\0";
#gv char key[33]="\0";

int main(int argc, char *argv[])
{ char str[256]="\0";
  int rb,pid,status,len;
  char *ctx, *plaintext;
  int pipefd;
  char pipename[256]="\0";
  int i,j;
  int ret;
  static const char *copyright="Obfuscated Bash\n"
  "Copyright (C) 2017- Davide Rao: louigi600 (at) yahoo (dot) it\n"
  "\nThis program is free software; you can redistribute it and/or modify\n"
  "it under the terms of the GNU General Public License as published by\n"
  "the Free Software Foundation; either version 2 of the License, or\n"
  "(at your option) any later version provided that no poit of the\n"
  "AA License is violated.\n";
  
/* needs fixing for reusable binary */
  if(strlen(uuid)==0) getuuid(uuid);  
  makekey(key,uuid);
//  printf("Retreaved uuid: %s\n",uuid);
  if(strlen(serial)==0) getserial(serial);
//  printf("Retreaved serial: >%s<\n",serial);
  makeiv(iv,serial);
//  printf("Used key: >%s<\n",key);
//  printf("Used iv: >%s<\n",iv);

/* NB: crypted_script is a variable not defined in here but is injectec into the trasition c source before compilinh the binary version */
//  printf("length of the crypted script: %i\n",strlen(crypted_script));

  ctx=malloc(strlen(crypted_script));
  plaintext=malloc(strlen(crypted_script));

/* Initialise the openssl library */
  ERR_load_crypto_strings();
  OpenSSL_add_all_algorithms();
  OPENSSL_no_config();

  ctx=unbase64(crypted_script,strlen(crypted_script));
  rb=decrypt(ctx,ctx_len,key,iv,plaintext);

//  printf("size of decrypted script: %i\n",rb);

/* creating named pipe (FIFO)*/
  sprintf(pipename,"/tmp/%i",getpid());
  if(mkfifo(pipename, 0666)!=0)
  { unlink(pipename); /* assuming it failed because a file by that name exists */
    if(mkfifo(pipename, 0666)!=0)
    { printf("Aborting: could not create named pipe %s\n",pipename);
      exit(1);
    }
  }

/* forking so that the parent writes to pipe and child reads from pipe */
  switch (pid=fork()) 
  { case -1: /* Handle fork error */
      printf("Error forking interpreter.\n");
      break;

//       printf("Child: own pid: %i\n",getpid());
//       printf("Child: Parent's pid: %i\n",getppid());
    case 0:  /* Child - reads from named pipe */
       printf("\0");
       enum { MAX_ARGS = 64 };
       char *args[MAX_ARGS];
       char arg2[15]="\0";
       char **argp=args;
       args[0]="bash";
       args[1]="-c";
       sprintf(arg2,"source %s",pipename);
       argp[2]=arg2;
       argp[3]=argv[0];
       if (argc==1) args[4]=NULL;
       else
       { for(i=1;i<argc;i++)
         {  argp[i+3]=argv[i];
         }
//         printf("Current counter: %i\n",i);
         args[i+3]=NULL; 
       }

//       printf("Arguments passed to %s: %i\n",argv[0],argc);
//       printf("Child: bash arg[2]: %s len:%i\n",args[2],strlen(args[2]) );
       fflush(stdout);
       execvp("bash",args);
       printf("Interpreter crashed.\n");
       break;

    default: /* Parent - writes to named pipe */
//       printf("Parent: own pid: %i\n",getpid());
//       printf("Parent: Child's pid: %i\n",pid);
/* opening named pipe for writing */
       pipefd=open(pipename, O_WRONLY);  
/* writing plaintext in the pipe for child to execute */
       write(pipefd,plaintext,rb);
/* closing named pipe else the child will hang waithing for writer to end */
       close(pipefd);
       break;
  }
  
//  printf("Parent waiting for child to terminate\n");
//  printf("Parent:  child terminated\n");
/* removing the pipe as by this time child has read it out and started execution */
  unlink(pipename);
/* cleaning up */
  EVP_cleanup();
  ERR_free_strings(); 
  free(ctx);
  free(plaintext);
/* waiting for the child to terminate else it's stdin wil be bust */
  waitpid(pid,&status,0);
  if(WIFEXITED(status)!=0)
  { ret=WEXITSTATUS(status);
  }else
  if(WIFSIGNALED(status)!=0)
  { ret=128+WTERMSIG(status);
  }else
  { ret=255;
  }
  return(ret);
}

