	case on
	copy 13/ainclude/e16.gsos
	mcopy 13/ainclude/m16.gsos
	mcopy 13/orcainclude/m16.orca

	mcopy M16.util
	mcopy M16.SmartStacks

root	start
	end

SWAP16	start
ntohs	entry
htons	entry

	DefineStack
	EndLocals
	FixStack
	BegParms
value	Word
	EndParms
	BeginStack

	lda <value
	xba
	tay

	EndStack
	tya
	rtl

	end

SWAP32	start
ntohl	entry
htonl	entry

	DefineStack
	EndLocals
	FixStack
	BegParms
value	LongWord
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



* returns new length
* Word ConvertCRLF(char * data, Word length)
ConvertCRLF start


	DefineStack
	
offset	Word
	endlocals

	FixStack

	begparms
data	LongWord
length	Word

	endparms

	BeginStack

	ldy #0
	stz <offset
	ldx <length
	beq exit
	
loop	anop
	lda [<data],y
	cmp #$0a0d
	beq crlf
	and #$00ff
	cmp #$000a
	bne store
	lda #$000d
store	anop
	phy
	ldy <offset
	short m
	sta [<data],y
	long m
	inc <offset
	ply
	iny
	dex
	beq exit
	bra loop
	
	
crlf	anop
	lda #$000d
	phy
	short m
	ldy <offset
	sta [<data],y
	long m
	inc <offset
	ply
	iny
	iny
	dex
	beq exit
	dex
	beq exit
	bra loop

exit	ldx <offset

	EndStack
	txa
	rtl
	end





* pascal Word GetHFSInfo(GSString255Ptr, hfsInfo *)
GETHFSINFO start

	using OptionData

	DefineStack

err	Word

	endlocals

	FixStack

	begparms
result	LongWord
path	LongWord
	endparms

	BeginStack

	stz <err

	ldax <path
	stax Info_path

	_GetFileInfoGS InfoDCB
	bcs i_err
	lda Info_fileType
	cmp #$000f
	beq dir

* make sure there's enough data...
	lda optionData+oDataSize
	cmp #34
	bcc fake_it

* only prodos/hfs/appleshare need apply...
	lda optionData+oFileSysID
	cmp #proDOSFSID
	beq legit
	cmp #hfsFSID
	beq legit
	cmp #appleShareFSID
	beq legit
	bra fake_it

legit	anop

	ldx #32-2
	ldy #32-2

cloop	anop
	lda optionData+oFileType,x
	sta [<result],y
	dey
	dey
	dex
	dex
	bpl cloop

	brl exit
	
* directory... ick
dir	anop
	lda #badStoreType
	brl exit


* a get file info error occurred.  check if it's ok or not.
i_err	anop
	cmp #buffTooSmall
	beq fake_it
	sta <err
	bra exit

* create a fake entry
fake_it	anop

* first, zero out everything.

	lda #0
	ldx #32-2

zloop	anop
	sta optionData+oFileType,x
	dex
	dex
	bpl zloop

	lda Info_fileType
	ldx Info_auxType
	cmp #$000f
	beq dir


	cmp #0
	beq bin
	cmp #4
	beq txt
	cmp #$b3
	beq s16
	cmp #$ff
	beq sys

* store as 'p' <filetype> <aux lo> <aux hi>
default anop

	pha
	txa
	xba
	tax
	pla

	and #$00ff
	xba
	ora #'p'


* a = ftype, x = auxtype
store	anop
	stax optionData+oFileType
	ldax #'sodp'
	stax optionData+oCreator

	bra legit

bin	anop
	cpx #0
	bne default
	ldax #'ANIB'
	bra store

txt	anop
	cpx #0
	bne default
	ldax #'TXET'
	bra store

s16	anop
	cpx #0
	bne default
	ldax #'61SP'
	bra store

sys	anop
	ldax #'SYSP'
	bra store

exit	anop

	ldx <err
	EndStack
	txa
	rtl

	end
	
	
* pascal Word GetFSTID(path)
GETFSTID	start

	using OptionData

	DefineStack

	endlocals

	FixStack

	begparms
path	LongWord
	endparms

	BeginStack

	ldax <path
	stax Info_path
	
	_GetFileInfoGS InfoDCB
	ldx #-1
	bcc ok
	cmp #buffTooSmall
	bne exit
	
ok	anop
	lda optionData+oDataSize
	cmp #2
	bcc exit
	
	ldx optionData+oFileSysID


exit	anop


	EndStack
	
	txa
	rtl

	end	
	
	

OptionData	PRIVDATA

* struct optionData
* {
*   Word fileSysID;
*   LongWord fileType;
*   LongWord creator;
*   Word finderFlags;
*   LongWord iconLoc;
*   Word fileWindow;
* };

oDataSize gequ 2
oFileSysID gequ 4
oFileType gequ 6
oCreator gequ 10


optionData anop
	dc i2'54'
	ds 52

InfoDCB	anop
	dc i2'8'
Info_path	ds 4
	ds 2
Info_fileType	ds 2
Info_auxType	ds 4
	ds 2
	ds 8
	ds 8
	dc i4'optionData'
	end




