libactor: actor.c actor.h message.c message.h process.c process.h node.c node.h distributer.c distributer.h
	clang -fblocks -c actor.c message.c process.c node.c distributer.c
	ar rc libactor.a actor.o message.o process.o node.o distributer.o
	ranlib libactor.a
	rm actor.o message.o process.o node.o distributer.o

test: test.c
	clang -o test test.c -fblocks -L. -lactor
