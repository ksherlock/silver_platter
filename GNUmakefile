#for use Golden Gate
# for compiling
MAKEFLAGS += --no-builtin-rules
.SUFFIXES:


CC = occ
AS = occ
CHTYP = iix chtyp
COPYFORK = iix copyfork
CFLAGS	=  -v #-O
ASFLAGS = 
LDFLAGS = -M
LDLIBS =

.PHONY: all clean clobber
all: silverplatter
clean:
	$(RM) -r o
clobber: clean
	$(RM) silverplatter


OBJS	= o/httpnda.a o/tools.a o/server.a o/error.a o/file.a o/time.a \
 o/mime.a o/config.a o/ctrl.a o/toolbox.a o/header.a o/log.a \
 o/mangle.a o/ftype.a o/applesingle.a o/utils.a o/pointer.a \
 o/globals.a o/methods.a o/headers.a  o/string.a \
 o/put.a o/propfind.a o/options.a o/mkcol.a o/lock.a \
 o/tcp.a o/membuffer.a o/macbinary.a o/volumes.a o/xstring.a

ROBJ = o/http.r

HTML = html/err400.html html/err403.html html/err404.html html/err409.html html/err416.html \
 html/err500.html html/err501.html

silverplatter: $(OBJS) $(ROBJ)
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@
	$(COPYFORK) $(ROBJ) $@ -r
	$(CHTYP) -t nda $@


o:
	mkdir o


o/config.a: config.c config.h toolbox.h
o/error.a: error.c server.h
o/file.a: file.c server.h
o/header.a: header.c server.h
o/httpnda.a: httpnda.c httpnda.h rez.h
o/server.a: server.c server.h httpnda.h config.h
o/toolbox.a: toolbox.c toolbox.h
o/tools.a: tools.c httpnda.h
o/applesingle.a: applesingle.c applesingle.h server.h
o/macbinary.a: macbinary.c macbinary.h server.h
o/put.a: put.c config.h server.h
o/lock.a: lock.c
o/globals.a: globals.c globals.h

o/propfind.a: propfind.c server.h
o/options.a: options.c server.h
o/mkcol.a: mkcol.c server.h
o/membuffer.a: membuffer.c membuffer.h
o/string.a: string.c server.h
o/volumes.a: volumes.c

o/tcp.a: tcp.c
o/pointer.a: pointer.asm

o/methods.a : methods.asm
o/headers.a : headers.asm

o/%.a : %.c | o
	$(CC) -c $(CFLAGS) -o $@ $<

o/%.a : %.asm | o
	$(AS) -c $(ASMFLAGS) -o $@ $<
	$(RM) $(patsubst %.a,%.root,$@)


# resource files
o/http.r: http.rez rez.h $(HTML) | o
	$(CC) -c -o $@ $<

o/errors.r: errors.rez $(HTML) | o
	$(CC) -c -o $@ $<

#dfa table
# o/methods.a: methods.txt
# 	iix dfa/dfa -f omf -o $@ methods $<


# o/headers.a: headers.txt
# 	iix dfa/dfa -f omf -o $@ headers $<

