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
#include <ctype.h>
#include <stdbool.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/buffer.h>
#include "functions.h"
#include "functions.c"
#include "interpreter.h"

typedef struct option
{ char option_shortname;
  enum option_types { flag = 0, parameter = 1, } option_type;
  char option_helptext[80];
  bool option_flag_status;
  char option_param[80];
} option;
char prod_uuid[256]="/sys/devices/virtual/dmi/id/product_uuid";
char prod_serial[256]="/sys/devices/virtual/dmi/id/product_serial";
char key[33]="\0";
char iv[17]="\0";

/* getopt extension functions for using an array of option as defined above */
bool flag_status (char shortname,  option *oa, int oalen)
{ int i;

  for (i=0;i<oalen;i++)
    if(oa[i].option_shortname==shortname && oa[i].option_flag_status) return(true);
  return(false);
}

int set_flag (char shortname,  option *oa, int oalen)
{ int i;

  for (i=0;i<oalen;i++)
    if(oa[i].option_shortname==shortname) oa[i].option_flag_status=true;
}

int set_param (char shortname, char *parameter, option *oa, int oalen)
{ int i;

  for (i=0;i<oalen;i++)
    if(oa[i].option_shortname==shortname)
    { strcpy(oa[i].option_param,parameter);
      oa[i].option_flag_status=true;
    }
}

/*return the element of optionarray containing the shortname flag/parameter */
int who_has_shortname (char shortname,  option *oa, int oalen)
{int i;

  for (i=0;i<oalen;i++)
    if(oa[i].option_shortname==shortname && oa[i].option_flag_status) return(i);

  return(-1);
}

int get_param (char *param,  char shortname,  option *oa, int oalen)
{ int i;

  for (i=0;i<oalen;i++)
  { if(oa[i].option_shortname==shortname && oa[i].option_flag_status)
    { strcpy(param,oa[i].option_param);
      return(i);
    }
  }
  sprintf(param,"\0");
  return(-1);
}

int usage(char *name, char *message, option *oa,int oalen)
{ int i;
  char str[80]="\0",options[80]="\0";

  for (i=0;i<oalen;i++)
  { if(oa[i].option_type==0) sprintf(str,"[-%c] ",oa[i].option_shortname);
    else  sprintf(str,"[-%c <value>] ",oa[i].option_shortname);
    strcat(options,str);
  }

  printf("Usage:\n%s %s <input filename>\n",name,options);

  for (i=0;i<oalen;i++)
  { if(oa[i].option_type==0) printf("-%c \t\t: %s\n",oa[i].option_shortname,oa[i].option_helptext);
    else printf("-%c <value>\t: %s\n",oa[i].option_shortname,oa[i].option_helptext);
  }

  if(strlen(message)>0)
  { fprintf(stderr,"%s\n",message);
    exit(1);
  } else exit(0);
}
/* getopt extentsion ends here */

int main(int argc, char *argv[])
{ FILE *infile;
//  char string[256]="\0";
  int ret;
  char *str;
  static const char *copyright="Obfuscated Bash\n"
  "Copyright (C) 2017- Davide Rao: louigi600 (at) yahoo (dot) it\n"
  "\nThis program is free software; you can redistribute it and/or modify\n"
  "it under the terms of the GNU General Public License as published by\n"
  "the Free Software Foundation; either version 2 of the License, or\n"
  "(at your option) any later version provided that no poit of the\n"
  "AA License is violated.\n";
/* these variables are for getopt and the extension */
  int c;
  opterr = 0;
  option optionarray[4] = {
    {'c',0,"Cleanup intermediate c files on success.",false,""},
    {'h',0,"How this help message.",false,""},
    {'o',1,"Output filename.",false,""},
    {'r',0,"Create a static reusable binary.",false,""},
  };
//  char optstring[80]="\0";   
  char optstring[256]="\0";   
  int i;
/* str is already defines as pointer to char and there is a mallog allocating 256 bytes for it, we should be ok to use the one already defined */
//  char str[80]="\0";
/* end variables for getopt */
  char input_filename[256]="\0",output_filename[256]="\0";
  str=malloc(256);

/*parsing options */
  for (i=0;i<sizeof(optionarray)/sizeof(option);i++)
  { if(optionarray[i].option_type==0) sprintf(str,"%c",optionarray[i].option_shortname);
    else  sprintf(str,"%c:",optionarray[i].option_shortname);
    strcat(optstring,str);
  }

  while ((c = getopt (argc, argv, optstring)) != -1)
    switch (c)
    { case 'c':
      case 'r':
        set_flag(c, optionarray, sizeof(optionarray)/sizeof(option));
        break;
      case 'h':
        usage(argv[0],"\0",optionarray,sizeof(optionarray)/sizeof(option));
      case 'o':
        set_param(c,optarg,optionarray,sizeof(optionarray)/sizeof(option));
        break;
      case '?':
        if (optopt == 'o') sprintf(str,"\nERROR: option `-%c' requires an argument.", optopt);
        else if (isprint (optopt))  sprintf(str,"\nERROR: unknown option `-%c'.", optopt);
        else  sprintf(str,"\nERROR: nknown option character `\\x%x'.", optopt);

        usage(argv[0],str,optionarray,sizeof(optionarray)/sizeof(option));
      default:
        abort ();
    }

/* doing some sanity checks */
  if(optind==argc) usage(argv[0],"\nERROR: no input file was provided.",optionarray,sizeof(optionarray)/sizeof(option));
  sprintf(input_filename,"%s",argv[optind]);

  if(!flag_status('o',optionarray,sizeof(optionarray)/sizeof(option))) sprintf(output_filename,"%s.x",argv[optind]);
  else get_param(output_filename,'o',optionarray,sizeof(optionarray)/sizeof(option));
  printf("Output filename: %s\n",output_filename);
/* finished parsing options */  

/* making sure input file is readable and then immediately closing it */
  if((infile=fopen(input_filename,"r"))==NULL)
  { printf("Error opening %s.\n",input_filename);
    exit(1);
  } 
  fclose(infile);

  getkey(key);
  getiv(iv);
  if((ret=mk_sh_c(input_filename,key,iv))<0)
  printf("Failed: %i/n",ret);
  else printf("Created %s.c\n",input_filename);
  sprintf(str,"sleep 1 ; sync ;cc %s.c -o %s -lssl -lcrypto && strip %s",input_filename,output_filename,output_filename);
//  printf("%s\n",str); 
//  exit(0);
  printf("Compiling %s.c ... ",input_filename); 
  if(system(str)!=0) 
  { printf("failed\n");
    exit(1);
  } else printf("done\nOutput file is %s.x\n",input_filename);

/* if -c flag was issued cleaning up intermediate c file */
  if(flag_status('c',optionarray,sizeof(optionarray)/sizeof(option))) 
  { printf("Cleaning up intermediate c file: %s.c ... ",input_filename);
    sprintf(str,"rm -f %s.c",input_filename);
    if(system(str)!=0)
    { printf("failed\n");
      exit(1);
    } else printf("done\n");
  }
  return(0);
  printf("%s\n",copyright);
}
