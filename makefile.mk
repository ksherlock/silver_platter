#for use iwith dmake(1)
# for compiling

CFLAGS	+=  $(DEFINES) -v #-O
LDLIBS += -l /usr/local/lib/libk -l /usr/local/lib/liborca
CFLAGS += -I /usr/local/include/

OBJS	= httpnda.o tools.o server.o error.o file.o time.o \
 mime.o config.o methods.o ctrl.o toolbox.o header.o log.o \
 mangle.o ftype.o applesingle.o utils.o
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

# resource files
http.r: http.rez rez.h
errors.r: errors.rez html/err400.html html/err403.html html/err404.html \
	html/err500.html html/err501.html

#dfa table
#methods.o: methods.txt
#	dfa -f omf -o $@ methods methods.txt

clean:
	$(RM) *.o *.root *.r

clobber: clean
	$(RM) -f httpnda
