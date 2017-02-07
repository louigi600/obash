DATE = `date '+%Y-%m-%d-%H%M'`
FILES = README.txt COPYING Makefile obfuscated_bash.c interpreter.c functions.h functions.c recreate_interpreter_header

default:	all

interpreter.h:	interpreter.c functions.c
	./recreate_interpreter_header

obash:	obash.c interpreter.h functions.h functions.c
	$(CC) obash.c -o obash -lssl -lcrypto
	

all:	obash
	
	
clean:	 
	rm -f obash *.x interpreter.h
	

backup:	
	tar cpzf old/bk_$(DATE).tgz $(FILES) obash.c from_my_to_distributable.patch 
	

distributable:	clean	
	mkdir -p distributable/obash
	cp -p $(FILES) testme distributable/obash
	( cd distributable/obash ; ln -s obfuscated_bash.c obash.c ; patch -p1 < ../../from_my_to_distributable.patch ; rm *.orig )
	

