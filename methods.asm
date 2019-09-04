* generated Sun Aug 11 00:10:32 2019

	case on

dummy	START
	END

scan_method	START

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
	cmp #$504f	; 'OP'
	bne _1
	ldy #2
	lda (cp),y
	cmp #$4954	; 'TI'
	bne _4
	ldy #4
	lda (cp),y
	cmp #$4e4f	; 'ON'
	bne _4
	ldy #6
	lda (cp),y
	cmp #$2053	; 'S '
	bne _4
	ldx #264	; 'OPTIONS '
_4	anop
	rts
_1	anop
	cmp #$4547	; 'GE'
	bne _5
	ldy #2
	lda (cp),y
	cmp #$2054	; 'T '
	bne _6
	ldx #516	; 'GET '
_6	anop
	rts
_5	anop
	cmp #$4548	; 'HE'
	bne _7
	ldy #2
	lda (cp),y
	cmp #$4441	; 'AD'
	bne _9
	longa off
	sep #$20
	ldy #4
	lda (cp),y
	cmp #$20	; ' '
	bne _9
	ldx #773	; 'HEAD '
_9	anop
	rts
_7	anop
	longa on
	cmp #$4f50	; 'PO'
	bne _10
	ldy #2
	lda (cp),y
	cmp #$5453	; 'ST'
	bne _12
	longa off
	sep #$20
	ldy #4
	lda (cp),y
	cmp #$20	; ' '
	bne _12
	ldx #1029	; 'POST '
_12	anop
	rts
_10	anop
	longa on
	cmp #$5550	; 'PU'
	bne _13
	ldy #2
	lda (cp),y
	cmp #$2054	; 'T '
	bne _14
	ldx #1284	; 'PUT '
_14	anop
	rts
_13	anop
	cmp #$4544	; 'DE'
	bne _15
	ldy #2
	lda (cp),y
	cmp #$454c	; 'LE'
	bne _18
	ldy #4
	lda (cp),y
	cmp #$4554	; 'TE'
	bne _18
	longa off
	sep #$20
	ldy #6
	lda (cp),y
	cmp #$20	; ' '
	bne _18
	ldx #1543	; 'DELETE '
_18	anop
	rts
_15	anop
	longa on
	cmp #$5254	; 'TR'
	bne _19
	ldy #2
	lda (cp),y
	cmp #$4341	; 'AC'
	bne _21
	ldy #4
	lda (cp),y
	cmp #$2045	; 'E '
	bne _21
	ldx #1798	; 'TRACE '
_21	anop
	rts
_19	anop
	cmp #$4f43	; 'CO'
	bne _22
	ldy #2
	lda (cp),y
	cmp #$4e4e	; 'NN'
	bne _23
	ldy #4
	lda (cp),y
	cmp #$4345	; 'EC'
	bne _25
	ldy #6
	lda (cp),y
	cmp #$2054	; 'T '
	bne _25
	ldx #2056	; 'CONNECT '
_25	anop
	rts
_23	anop
	cmp #$5950	; 'PY'
	bne _27
	longa off
	sep #$20
	ldy #4
	lda (cp),y
	cmp #$20	; ' '
	bne _27
	ldx #3333	; 'COPY '
_27	anop
	rts
_22	anop
	longa on
	cmp #$4150	; 'PA'
	bne _28
	ldy #2
	lda (cp),y
	cmp #$4354	; 'TC'
	bne _30
	ldy #4
	lda (cp),y
	cmp #$2048	; 'H '
	bne _30
	ldx #2310	; 'PATCH '
_30	anop
	rts
_28	anop
	cmp #$5250	; 'PR'
	bne _31
	ldy #2
	lda (cp),y
	cmp #$504f	; 'OP'
	bne _38
	ldy #4
	lda (cp),y
	cmp #$4946	; 'FI'
	bne _33
	ldy #6
	lda (cp),y
	cmp #$444e	; 'ND'
	bne _35
	longa off
	sep #$20
	ldy #8
	lda (cp),y
	cmp #$20	; ' '
	bne _35
	ldx #2569	; 'PROPFIND '
_35	anop
	rts
_33	anop
	longa on
	cmp #$4150	; 'PA'
	bne _38
	ldy #6
	lda (cp),y
	cmp #$4354	; 'TC'
	bne _38
	ldy #8
	lda (cp),y
	cmp #$2048	; 'H '
	bne _38
	ldx #2826	; 'PROPPATCH '
_38	anop
	rts
_31	anop
	cmp #$4b4d	; 'MK'
	bne _39
	ldy #2
	lda (cp),y
	cmp #$4f43	; 'CO'
	bne _41
	ldy #4
	lda (cp),y
	cmp #$204c	; 'L '
	bne _41
	ldx #3078	; 'MKCOL '
_41	anop
	rts
_39	anop
	cmp #$4f4d	; 'MO'
	bne _42
	ldy #2
	lda (cp),y
	cmp #$4556	; 'VE'
	bne _44
	longa off
	sep #$20
	ldy #4
	lda (cp),y
	cmp #$20	; ' '
	bne _44
	ldx #3589	; 'MOVE '
_44	anop
	rts
_42	anop
	longa on
	cmp #$4f4c	; 'LO'
	bne _45
	ldy #2
	lda (cp),y
	cmp #$4b43	; 'CK'
	bne _47
	longa off
	sep #$20
	ldy #4
	lda (cp),y
	cmp #$20	; ' '
	bne _47
	ldx #3845	; 'LOCK '
_47	anop
	rts
_45	anop
	longa on
	cmp #$4e55	; 'UN'
	bne _51
	ldy #2
	lda (cp),y
	cmp #$4f4c	; 'LO'
	bne _51
	ldy #4
	lda (cp),y
	cmp #$4b43	; 'CK'
	bne _51
	longa off
	sep #$20
	ldy #6
	lda (cp),y
	cmp #$20	; ' '
	bne _51
	ldx #4103	; 'UNLOCK '
_51	anop
	rts
	END
