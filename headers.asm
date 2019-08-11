* generated Sat Aug 10 21:44:12 2019

	case on

dummy	START
	END

scan_headers	START

cp	equ 5

	phb
	tsc
	tcd
	phd
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

	lda (cp)
	ora #$2020
	cmp #$656b	; 'ke'
	bne _1
	ldy #2
	lda (cp),y
	ora #$2020
	cmp #$7065	; 'ep'
	bne _6
	ldy #4
	lda (cp),y
	ora #$2000
	cmp #$612d	; '-a'
	bne _6
	ldy #6
	lda (cp),y
	ora #$2020
	cmp #$696c	; 'li'
	bne _6
	ldy #8
	lda (cp),y
	ora #$2020
	cmp #$6576	; 've'
	bne _6
	longa off
	sep #$20
	ldy #10
	lda (cp),y
	cmp #$3a	; ':'
	bne _6
	ldx #267	; 'keep-alive:'
_6	anop
	rts
_1	anop
	cmp #$6f63	; 'co'
	beq *+5
	brl _7
	ldy #2
	lda (cp),y
	ora #$2020
	cmp #$6e6e	; 'nn'
	bne _8
	ldy #4
	lda (cp),y
	ora #$2020
	cmp #$6365	; 'ec'
	bne _12
	ldy #6
	lda (cp),y
	ora #$2020
	cmp #$6974	; 'ti'
	bne _12
	ldy #8
	lda (cp),y
	ora #$2020
	cmp #$6e6f	; 'on'
	bne _12
	longa off
	sep #$20
	ldy #10
	lda (cp),y
	cmp #$3a	; ':'
	bne _12
	ldx #523	; 'connection:'
_12	anop
	rts
_8	anop
	cmp #$746e	; 'nt'
	bne _22
	ldy #4
	lda (cp),y
	ora #$2020
	cmp #$6e65	; 'en'
	bne _22
	ldy #6
	lda (cp),y
	ora #$0020
	cmp #$2d74	; 't-'
	bne _22
	ldy #8
	lda (cp),y
	ora #$2020
	cmp #$656c	; 'le'
	bne _16
	ldy #10
	lda (cp),y
	ora #$2020
	cmp #$676e	; 'ng'
	bne _19
	ldy #12
	lda (cp),y
	ora #$2020
	cmp #$6874	; 'th'
	bne _19
	longa off
	sep #$20
	ldy #14
	lda (cp),y
	cmp #$3a	; ':'
	bne _19
	ldx #1039	; 'content-length:'
_19	anop
	rts
_16	anop
	cmp #$7974	; 'ty'
	bne _22
	ldy #10
	lda (cp),y
	ora #$2020
	cmp #$6570	; 'pe'
	bne _22
	longa off
	sep #$20
	ldy #12
	lda (cp),y
	cmp #$3a	; ':'
	bne _22
	ldx #1293	; 'content-type:'
_22	anop
	rts
_7	anop
	cmp #$6f68	; 'ho'
	bne _23
	ldy #2
	lda (cp),y
	ora #$2020
	cmp #$7473	; 'st'
	bne _25
	longa off
	sep #$20
	ldy #4
	lda (cp),y
	cmp #$3a	; ':'
	bne _25
	ldx #773	; 'host:'
_25	anop
	rts
_23	anop
	cmp #$6564	; 'de'
	bne _26
	ldy #2
	lda (cp),y
	ora #$2020
	cmp #$7470	; 'pt'
	bne _28
	ldy #4
	lda (cp),y
	ora #$0020
	cmp #$3a68	; 'h:'
	bne _28
	ldx #1542	; 'depth:'
_28	anop
	rts
_26	anop
	cmp #$6172	; 'ra'
	bne _31
	ldy #2
	lda (cp),y
	ora #$2020
	cmp #$676e	; 'ng'
	bne _31
	ldy #4
	lda (cp),y
	ora #$0020
	cmp #$3a65	; 'e:'
	bne _31
	ldx #1798	; 'range:'
_31	anop
	rts
	END
