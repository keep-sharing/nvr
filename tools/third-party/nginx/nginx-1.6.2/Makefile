
default:	build

#clean:
#	rm -rf Makefile objs
clean:
	-rm -f objs/nginx
	-rm -f objs/nginx.8
	-rm -f objs/src/core/*.o
	-rm -f objs/src/event/*.o objs/src/event/modules/*.o
	-rm -f objs/src/http/*.o objs/src/http/modules/*.o
	-rm -f objs/src/os/unix/*.o

build:
	$(MAKE) -f objs/Makefile
	$(MAKE) -f objs/Makefile manpage
	$(MAKE) -f objs/Makefile install
	
install:
	$(MAKE) -f objs/Makefile install

upgrade:
	./temp/sbin/nginx -t

	kill -USR2 `cat ./temp/logs/nginx.pid`
	sleep 1
	test -f ./temp/logs/nginx.pid.oldbin

	kill -QUIT `cat ./temp/logs/nginx.pid.oldbin`
