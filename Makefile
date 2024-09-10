obj-m += wordsmith.o
wordsmith-y := ws_db.o ws_main.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) LLVM=1 modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
