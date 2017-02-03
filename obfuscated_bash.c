/* Obfuscated Bash
// Copyright (C) 2017- Davide Rao: louigi600 (at) yahoo (dot) it
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version provided that no poit of the 
// AA License is violated.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/buffer.h>
#include "functions.h"
#include "functions.c"
#include "interpreter.h"

char prod_uuid[256]="/sys/devices/virtual/dmi/id/product_uuid";
char prod_serial[256]="/sys/devices/virtual/dmi/id/product_serial";
char key[33]="\0";
char iv[17]="\0";

int main(int argc, char *argv[])
{ FILE *infile;
  char string[256]="\0";
  int ret;
  char *str;
  static const char *copyright="Obfuscated Bash\n"
  "Copyright (C) 2017- Davide Rao: louigi600 (at) yahoo (dot) it\n"
  "\nThis program is free software; you can redistribute it and/or modify\n"
  "it under the terms of the GNU General Public License as published by\n"
  "the Free Software Foundation; either version 2 of the License, or\n"
  "(at your option) any later version provided that no poit of the\n"
  "AA License is violated.\n";

/* making sure input file is readable and then immediately closing it */
  if((infile=fopen(argv[1],"r"))==NULL)
  { printf("Error opening %s.\n",argv[1]);
    exit(1);
  } 
  fclose(infile);

  str=malloc(256);
  getkey(key);
  getiv(iv);
  if((ret=mk_sh_c(argv[1],key,iv))<0)
  printf("Failed: %i/n",ret);
  else printf("Created %s.c\n",argv[1]);
  sprintf(str,"sleep 1 ; sync ;cc %s.c -o %s.x -lssl -lcrypto && strip %s.x",argv[1],argv[1],argv[1]);
//  printf("%s\n",str); 
  printf("Compiling %s.c ... ",argv[1]); 
  if(system(str)!=0) 
  { printf("failed\n");
    exit(1);
  } else printf("done\nOutput file is %s.x\n",argv[1]);
  return(0);
}
