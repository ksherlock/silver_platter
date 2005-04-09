       case on
	mcopy /usr/local/ainclude/m16.SmartStacks


root   start
       end

SWAP16 start
ntohs  entry
htons  entry

	DefineStack
	EndLocals
	FixStack
       BegParms
value  Word
       EndParms
	BeginStack

       lda <value
       xba
       tay

       EndStack
       tya
       rtl

       end

SWAP32 start
ntohl  entry
htonl  entry

	DefineStack
	EndLocals
	FixStack
       BegParms
value  LongWord
       EndParms
	BeginStack

       lda <value
       xba
       tax
       lda <value+2
       xba
       tay

       EndStack
       tya
       rtl

       end
