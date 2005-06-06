       case on
	copy /lang/orca/libraries/ainclude/e16.gsos
	mcopy /lang/orca/libraries/ainclude/m16.gsos
	mcopy /usr/local/ainclude/m16.SmartStacks
	mcopy /usr/local/ainclude/m16.util


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





* pascal Word GetHFSInfo(GSString255Ptr, hfsInfo *)
GETHFSINFO start

	DefineStack

err	Word

	endlocals

	FixStack

	begparms
result	LongWord
path	LongWord
	endparms

	BeginStack

* struct optionData
* {
*   Word fileSysID;
*   LongWord fileType;
*   LongWord creator;
*   Word finderFlags;
*   LongWord iconLoc;
*   Word fileWindow;
* };


oDataSize equ 2
oFileSysID equ 4
oFileType equ 6
oCreator equ 10


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
