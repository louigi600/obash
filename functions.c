/* Obfuscated Bash
// Copyright (C) 2017- Davide Rao: louigi600 (at) yahoo (dot) it
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version provided that no poit of the
//  AA License is violated.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
extern char prod_uuid[256];
extern char prod_serial[256];
extern char key[33];
extern char iv[17];
extern char *includes;
extern char *functions;
extern char *main_body;

int getkey (char *key)
{ FILE *filepointer;
  int i=0,rb=0;
  char uuid[40]="\0";
  char *s, *end;

/* attempt to open sys produtc uuid */
  if((filepointer=fopen(prod_uuid,"r"))==NULL)
  { 
/*  get uuid via sys failed using dmidecode */
    if((filepointer=popen("dmidecode -s system-uuid","r"))==NULL)
    { return(-1); /*failed running dmidecode*/ 
    }
/*  read uuid or exit on error */
    if((rb=fread(uuid,1,36,filepointer))!=36)
    { return(-2); /*could not read enough data from "dmidecode -s system-uuid"*/
    } else pclose(filepointer);
    
  } else
  {
/* get uuid from /sys .... */
    if((rb=fread(uuid,1,36,filepointer))!=36)
    { return(-3); /*could not read enough data from prod_uuid*/
    } else fclose(filepointer); 
  }

/* where you get the uuid is tracable via strace or by strings */
/* so now is a good time to manipulate the uuid so that  */
/* it does not match with what can be seen altought it's still  */
/* derived from there (scramble uuid) . Same applies to getiv below. */
/* strip "-" from the uuid into key */
  s=(char*)uuid;
  while (*s)
  { if ( *s != '-') key[i++]=*s;
    s++;
  }
  return strlen(key);
}

int getiv (char *iv)
{ FILE *filepointer;
  int i=0,rb=0;
  char serial[20]="\0";
  char *s, *end;

/* attempt to open sys produtc serial */
  if((filepointer=fopen(prod_serial,"r"))==NULL)
  { //printf("File open error. Will attempt to use dmidecode.\n");

/*  get serial via sys failed using dmidecode */
    if((filepointer=popen("dmidecode -s system-serial-number","r"))==NULL)
    { return(-1); /* failed running dmidecode */
    }
/*  read serial or exit on error */
    rb=fread(serial,1,16,filepointer);
    pclose(filepointer);
  } else
  {
/* read serial or exit on error */
    rb=fread(serial,1,16,filepointer);
    fclose(filepointer);
  }

/* dealing with not enough data in serial */
  if(rb<1)
  { /*if you gein in here nothing was read so migh as well just give up */
    printf("Insufficient data to identify.\n");
    exit(1);
  }

/* if necessary padding iv to reach the advised lenght for the chipher */
  if(rb!=16)
  { strncat(iv,serial,rb-1);
    if(rb<9)  strncat(iv,prod_serial,17-rb);
    strncat(iv,serial,17-rb);
  } else strcpy(iv,serial);
  return strlen(iv);
}

int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext)
{ EVP_CIPHER_CTX *ctx;
  int len;
  int ciphertext_len;

  /* Create and initialise the context */
  if(!(ctx = EVP_CIPHER_CTX_new())) exit(1);

  /* Initialise the encryption operation. IMPORTANT - ensure you use a key
   * and IV size appropriate for your cipher
   * In this example we are using 256 bit AES (i.e. a 256 bit key). The
   * IV size for *most* modes is the same as the block size. For AES this
   * is 128 bits */
  if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    exit(1);

  /* Provide the message to be encrypted, and obtain the encrypted output.
   * EVP_EncryptUpdate can be called multiple times if necessary*/
  if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
    exit(1);
  ciphertext_len = len;

  /* Finalise the encryption. Further ciphertext bytes may be written at
   * this stage.*/
  if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) exit(1);
  ciphertext_len += len;

  /* Clean up */
  EVP_CIPHER_CTX_free(ctx);

  return ciphertext_len;
}

int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext)
{ EVP_CIPHER_CTX *ctx;
  int len;
  int plaintext_len;

  /* Create and initialise the context */
  if(!(ctx = EVP_CIPHER_CTX_new())) exit(1);

  /* Initialise the decryption operation. IMPORTANT - ensure you use a key
   * and IV size appropriate for your cipher
   * In this example we are using 256 bit AES (i.e. a 256 bit key). The
   * IV size for *most* modes is the same as the block size. For AES this
   * is 128 bits */
  if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    exit(1);

  /* Provide the message to be decrypted, and obtain the plaintext output.
   * EVP_DecryptUpdate can be called multiple times if necessary*/
  if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
    exit(1);
  plaintext_len = len;

  /* Finalise the decryption. Further plaintext bytes may be written at
   * this stage.*/
  if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) exit(1);
  plaintext_len += len;

  /* Clean up */
  EVP_CIPHER_CTX_free(ctx);

  return plaintext_len;
}

char *unbase64(unsigned char *input, int length)
{ BIO *b64, *bmem;

  char *buffer = (char *)malloc(length);
  memset(buffer, 0, length);

  b64 = BIO_new(BIO_f_base64());
  bmem = BIO_new_mem_buf(input, length);
  bmem = BIO_push(b64, bmem);

  BIO_read(bmem, buffer, length);

  BIO_free_all(bmem);

  return buffer;
}

char *base64(const unsigned char *input, int length)
{ BIO *bmem, *b64;
  BUF_MEM *bptr;

  b64 = BIO_new(BIO_f_base64());
  bmem = BIO_new(BIO_s_mem());
  b64 = BIO_push(b64, bmem);
  BIO_write(b64, input, length);
  BIO_flush(b64);
  BIO_get_mem_ptr(b64, &bptr);

  char *buff = (char *)malloc(bptr->length);
  memcpy(buff, bptr->data, bptr->length-1);
  buff[bptr->length-1] = 0;

  BIO_free_all(b64);

  return buff;
}

int mk_sh_c ( char *infilename, char *key, char *iv )
{ unsigned char *plaintext, *ciphertext, *b64ctx;
  char *outfilename;
  FILE *infile,*outfile;
  int rb,wb,insize,ctsize,i; 
  char str[256]="\0";

/* outfilename is infilename suffixed with .c */
  outfilename=malloc(strlen(infilename)+2);
  strcpy(outfilename,infilename);
  strcat(outfilename,".c");

/* opening outfile for writing */
  if((outfile=fopen(outfilename,"wb"))==NULL)
  return(-1); /*failed opening intermediate c source file*/

/* opening infilename for reading, get size and rewind */
  if((infile=fopen(infilename,"r"))==NULL)
  return(-2); /*failed opening infile*/
  
  fseek(infile,0L,SEEK_END);
  insize=ftell(infile);
  rewind(infile);

/* allocating memory for plaintext */
  plaintext=malloc(insize);
  
/* allocating memory for ciphertext: its bigger then plaintext */
/* but i's not very predictable in size so using 2x to be conservative */
  ciphertext=malloc(2*insize);
  b64ctx=malloc(2*insize);

/* Initialise the openssl library */
  ERR_load_crypto_strings();
  OpenSSL_add_all_algorithms();
  OPENSSL_config(NULL);

/* reading infile into plaintext */
  if((rb=fread(plaintext,1,insize,infile))!=insize)
  return(-3); /*did not read the entire infile */ 

/* encripting plaintext into ciphertext */
  ctsize=encrypt (plaintext,insize,key,iv,ciphertext); 
  b64ctx=base64(ciphertext,ctsize);
  

//some feedback stuff ... can be removed after debugging 
  printf("input filename: %s\n",infilename);
  printf("input file size: %i\n",insize);
  printf("ciphertext size: %i\n",ctsize);
  printf("base64 encoded ciphertext: %i : %i whole lines\n",strlen(b64ctx),strlen(b64ctx)/65);
  printf("intermediate c generated filename: %s\n",outfilename);
//

/* writing the includes in the beginning of the outfile */
  fwrite(includes,1,strlen(includes),outfile);

/* wtriting the declaration of the static ctx_size variable */
  sprintf(str,"int ctx_len=%i;\n",ctsize);
  fwrite(str,1,strlen(str),outfile);

/* writing the declaration of the static ciphertext variable */
  fwrite("unsigned char *crypted_script=",1,30,outfile);
/* base64 outputs lines of 64 characters ... will quote each line */
  for (i=0;i<strlen(b64ctx)/65;i++)
  { fputc(34,outfile); 
    fwrite(b64ctx+(65*i),1,64,outfile); 
    fputc(92,outfile);
    fputc('n',outfile);
    fputc(34,outfile); 
    fputc(10,outfile); 
  }
  if((i*65)<<strlen(b64ctx))
  { fputc(34,outfile);
    fwrite(b64ctx+(65*i),1,strlen(b64ctx)-(65*i),outfile);
    fputc(92,outfile);
    fputc('n',outfile); 
    fputc(34,outfile);
    fputc(10,outfile);
  }

/* closing the definition of static ciphertext variable */
  fwrite(";\n",1,2,outfile);

/* writing the functions in the outfile */
  fwrite(functions,1,strlen(functions),outfile);
  fwrite("\n",1,1,outfile);

/* writing the main in the outfile */ 
  fwrite(main_body,1,strlen(main_body),outfile);

/* closing and freeing stuff */
  fclose(outfile);
  fclose(infile);
  free(outfilename);
  free(plaintext);
  free(ciphertext);
  free(b64ctx);
  EVP_cleanup();
  ERR_free_strings();
  return(0);
}
