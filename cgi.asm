* generated Tue Sep  3 20:57:30 2019

	case on

dummy	START
	END

scan_cgi	START

cp	equ 5

	phb
	tsc
	phd
	tcd
	pei cp+1
	plb
	plb
	jsr _action
	rep #$20
	lda 1
	sta 5
	lda 3
	sta 7
	pld
	pla
	pla
	txa
	plb
	rtl

_action	anop
	ldx #0

	longi on
	longa on
	lda (cp)
	cmp #$7061	; 'ap'
	bne _1
	ldy #2
	lda (cp),y
	cmp #$6c70	; 'pl'
	bne _10
	ldy #4
	lda (cp),y
	cmp #$7365	; 'es'
	bne _3
	ldy #6
	lda (cp),y
	cmp #$6e69	; 'in'
	bne _6
	ldy #8
	lda (cp),y
	cmp #$6c67	; 'gl'
	bne _6
	ldy #10
	lda (cp),y
	cmp #$0065	; 'e\x00'
	bne _6
	ldx #1	; 'applesingle\x00'
_6	anop
	rts
_3	anop
	cmp #$6465	; 'ed'
	bne _10
	ldy #6
	lda (cp),y
	cmp #$756f	; 'ou'
	bne _10
	ldy #8
	lda (cp),y
	cmp #$6c62	; 'bl'
	bne _10
	ldy #10
	lda (cp),y
	cmp #$0065	; 'e\x00'
	bne _10
	ldx #2	; 'appledouble\x00'
_10	anop
	rts
_1	anop
	cmp #$616d	; 'ma'
	bne _11
	ldy #2
	lda (cp),y
	cmp #$6263	; 'cb'
	bne _15
	ldy #4
	lda (cp),y
	cmp #$6e69	; 'in'
	bne _15
	ldy #6
	lda (cp),y
	cmp #$7261	; 'ar'
	bne _15
	ldy #8
	lda (cp),y
	cmp #$0079	; 'y\x00'
	bne _15
	ldx #3	; 'macbinary\x00'
_15	anop
	rts
_11	anop
	cmp #$7372	; 'rs'
	bne _18
	ldy #2
	lda (cp),y
	cmp #$6372	; 'rc'
	bne _18
	longa off
	sep #$20
	ldy #4
	lda (cp),y
	cmp #$00	; '\x00'
	bne _18
	ldx #4	; 'rsrc\x00'
_18	anop
	rts
	END
