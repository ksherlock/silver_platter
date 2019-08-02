	case off

	copy /lang/orca/libraries/ainclude/e16.control

	mcopy M16.SmartStacks
	mcopy ctrl.mac

root	start
	end


* void SetCtlTextByID(Window *, LongWord, Word, Ref)
SetCtlTextByID	start

	DefineStack

cPtr	LongWord
cHand	LongWord
	endlocals

	FixStack

	begparms
ref	LongWord
refType	Word
id	LongWord
win	LongWord
	endparms

	BeginStack

	lda <ref
	ora <ref+2
	bne gotstr
	lda #NullStr
	sta <ref
	lda #^NullStr
	sta <ref+2
	lda #0
	sta <refType

gotstr	anop

	lda <win
	ora <win+2
	bne gotwin

	pha
	pha
	_FrontWindow
	pla
	sta <win
	pla
	sta <win+2
	bcc gotwin
	brl exit

gotwin	anop

	pha
	pha
	~GetCtlHandleFromID <win,<id
	pla
	sta <cHand
	pla
	sta <cHand+2
	bcs exit

	pha
	pha
	_GetPort

	~SetPort <win

	ldy #2
	lda [<cHand]
	sta <cPtr
	lda [<cHand],y
	sta cPtr+2

	clc
	lda #octlRect
	adc <cPtr
	tax
	lda #0
	adc <cPtr+2
	
	pha
	phx

	_EraseRect

	~SetCtlMoreFlags #$1000,<cHand

	lda <refType
	and #%11
	asl a
	tax
	jsr (table,x)

	phy
	~SetCtlValue *,<cHand
	~SetCtlTitle <ref,<cHand

	_SetPort
exit	anop
	EndStack
	rtl

table	anop
	dc i2'pstring'
	dc i2'cstring'
	dc i2'gstring'
	dc i2'wtf'

pstring	anop
	lda [<ref]
	and #$00ff
	tay
	lda #1
	clc
	adc <ref
	sta <ref
	lda #0
	adc <ref+2
	sta <ref+2
	rts

cstring	anop
	ldy #-1
l	anop
	iny
	lda [<ref],y
	and #$00ff
	bne l
	rts

gstring	anop
	lda [<ref]
	tay
	lda #2
	clc
	adc <ref
	sta <ref
	lda #0
	adc <ref+2
	sta <ref+2
	rts
wtf	anop
	lda #NullStr
	sta <ref
	lda #^NullStr
	sta <ref+2
	ldy #0
	rts

NullStr	dc i1'0'

	end
