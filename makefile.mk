#for use iwith dmake(1)
# for compiling

CFLAGS	+=  $(DEFINES) -v #-O
LDLIBS += -l /usr/local/lib/liborca -l /usr/local/lib/libdfa
CFLAGS += -I /usr/local/include/

OBJS	= httpnda.o tools.o server.o error.o file.o time.o \
 mime.o config.o ctrl.o toolbox.o header.o log.o \
 mangle.o ftype.o applesingle.o utils.o pointer.o \
 globals.o methods.o headers.o  string.o \
 put.o propfind.o options.o mkcol.o lock.o \
 tcp.o membuffer.o macbinary.o volumes.o

ROBJS = http.r errors.r

silverplatter: $(OBJS) $(ROBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@
	catrez -d $@ $(ROBJS)
	chtyp -t nda $@

config.o: config.c config.h toolbox.h
error.o: error.c server.h
file.o: file.c server.h
header.o: header.c server.h
httpnda.o: httpnda.c httpnda.h rez.h
server.o: server.c server.h httpnda.h config.h
toolbox.o: toolbox.c toolbox.h
tools.o: tools.c httpnda.h
applesingle.o: applesingle.c applesingle.h server.h
macbinary.o: macbinary.c macbinary.h server.h
put.o: put.c config.h server.h
lock.o: lock.c
globals.o: globals.c globals.h

propfind.o: propfind.c server.h
options.o: options.c server.h
mkcol.o: mkcol.c server.h
membuffer.o: membuffer.c membuffer.h
string.o: string.c server.h
volumes.o: volumes.c

tcp.o: tcp.c
pointer.o: pointer.asm

# resource files
http.r: http.rez rez.h
errors.r: errors.rez html/err400.html html/err403.html \
    html/err404.html html/err409.html \
	html/err500.html html/err501.html

#dfa table
methods.o: methods.txt
	dfa -f omf -o $@ methods methods.txt


headers.o: headers.txt
	dfa -f omf -o $@ headers headers.txt

clean:
	$(RM) *.o *.root *.r

clobber: clean
	$(RM) -f httpnda
