
all:install

debuka: debuka.c
	gcc -g -o $@ $<

install: debuka
	cp debuka ${HOME}/bin/
