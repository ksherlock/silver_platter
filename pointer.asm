	case	off
	copy 13/ainclude/e16.memory

	mcopy M16.SmartStacks
	mcopy pointer.mac	
	
dummy	start
	end
	
PointerData	Data Memory
MemID	ds 2
	end

* void PointerStartUp(Word)
PointerStartUp	start Memory
	
	using PointerData

	DefineStack
	EndLocals
	FixStack

    BegParms
mid	Word
    EndParms

	BeginStack

	lda <mid
	sta >MemID
	
	EndStack
	rtl
	end

* void * NewPointer(LongWord size)
NewPointer	start Memory

	using PointerData

	DefineStack
	
ptr	LongWord
h	LongWord
	EndLocals

	FixStack

    BegParms
size  LongWord
    EndParms

	BeginStack

	stz <ptr
	stz <ptr+2
	
	lda <size
	clc
	adc #6
	sta <size
	lda #0
	adc <size+2
	sta <size+2
	
	pha
	pha
	~NewHandle <size,>MemID,#attrNoSpec+attrLocked,0
	pla
	sta <h
	pla
	sta <h+2
	bcs exit
	
	ldy #2
	lda [<h]
	sta <ptr
	lda [<h],y
	sta <ptr+2
	
	lda <h
	sta [<ptr]
	lda <h+2
	sta [<ptr],y
	iny
	iny
	lda #1
	sta [<ptr],y
	
	lda <ptr
	clc
	adc #6
	sta <ptr
	lda #0
	adc <ptr+2
	sta <ptr+2
	
exit	anop

	ldy <ptr
	ldx <ptr+2

	EndStack
	tya
	rtl
	
	end
	
	
DisposePointer	start Memory

	DefineStack

	EndLocals

	FixStack

    BegParms
ptr  LongWord
    EndParms

	BeginStack

	lda <ptr
	ora <ptr+2
	beq exit

	lda <ptr
	sec
	sbc #6
	sta <ptr
	lda <ptr+2
	sbc #0
	sta <ptr+2

	ldy #2
	lda [<ptr],y
	pha
	lda [<ptr]
	pha

	_DisposeHandle

exit	anop
	EndStack
	rtl
	
	end

* void RetainPointer(Pointer)
RetainPointer	start Memory


	DefineStack

	EndLocals

	FixStack

    BegParms
ptr  LongWord
    EndParms

	BeginStack

	lda <ptr
	ora <ptr+2
	beq exit

	lda <ptr
	sec
	sbc #6
	sta <ptr
	lda <ptr+2
	sbc #0
	sta <ptr+2

	ldy #4
	lda [<ptr],y
	inc a
	sta [<ptr],y

exit	anop

	EndStack
	rtl
	
	end

* void ReleasePointer(Pointer)
ReleasePointer	start Memory


	DefineStack

	EndLocals

	FixStack

    BegParms
ptr  LongWord
    EndParms

	BeginStack

	lda <ptr
	ora <ptr+2
	beq exit

	lda <ptr
	sec
	sbc #6
	sta <ptr
	lda <ptr+2
	sbc #0
	sta <ptr+2

	ldy #4
	lda [<ptr],y
	dec a
	sta [<ptr],y
	bne exit

*	now we must dispose

	dey
	dey
	lda [<ptr],y
	pha
	lda [<ptr]
	pha
	_DisposeHandle
	
exit	anop
	EndStack
	
	rtl
	
	end
