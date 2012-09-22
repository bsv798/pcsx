! Copy a memory matrix into the internal one
.globl _mat_vec_load
! Number of cycles: ~11 cycles.
_mat_vec_load:
        fschg
        fmov            @r4+,xd0

        fmov            @r4+,xd2
        fmov            @r4+,xd4
        fmov            @r4+,xd6
        fmov            @r4+,xd8
        fmov            @r4+,xd10
        fmov            @r5+,xd12
        fmov            @r5+,xd14
		
        rts
		fschg

! Copy a memory matrix into the internal one
.globl _mat_zero_load
_mat_zero_load:
        fschg
        fmov            @r4+,xd0

        fmov            @r4+,xd2
        fldi0           fr0
        fmov            @r4+,xd4
        fldi0           fr1
        fmov            @r4+,xd6
        fmov            @r4+,xd8
        fmov            @r4+,xd10
        fmov            dr0,xd12
        fmov            dr0,xd14
		
        rts
		fschg

! loads a GTE matrix element
! r4  element
! r5  matrix address
! r6  low word offset
! r7  high word offset

.globl _LoadMatrixElement
_LoadMatrixElement:
	sts.l    pr,@-r15
	
	mov      r5, r0
	
	exts.w   r4, r1
	lds      r1, fpul
	float    fpul, fr0
	fmov.s   fr0, @(r0, r6)
	
	shlr16   r4
	exts.w   r4, r1
	lds      r1, fpul
	float    fpul, fr0
	fmov.s   fr0, @(r0, r7)
	lds.l    @r15+,pr
	rts	
	nop


		.align 4
! Transform a vector
! R4 addr of input vector and clipped transformed vector (ir)
! R5 addr of unclipped vector (mac)
! R6 <=0: clip to -32768  >0: clip to 0
! vector[3] is important for the translation:
!   0.0 -> no translation
!   x -> multiply translation with x
! returns gteFLAG in R0

.globl _asmTransformVector
_asmTransformVector:
	sts.l    pr,@-r15
	mov      #0, R1		! gteFLAG

! transform vector
	fmov.s   @R4+,fr4
	fmov.s   @R4+,fr5
	fmov.s   @R4+,fr6
	fmov.s   @R4, fr7
	ftrv     xmtrx,fv4

! store back into mac vector right shifted by 12
	add      #12, r5
	mova     float4096, r0
	fmov.s   @r0, fr0
	fmul     fr0, fr6
	fmov.s   fr6, @-r5
	fmul     fr0, fr5
	fmov.s   fr5, @-r5
	fmul     fr0, fr4
	fmov.s   fr4, @-r5

	mova     asmClip, R0
	jsr      @R0
	nop

! store back clipped vector
	fmov.s   fr6, @-r4
	fmov.s   fr5, @-r4
	fmov.s   fr4, @-r4

	lds.l    @r15+,pr
	rts
	mov      r1, r0
	

		.align 4
float4096: ! 1/4096
		.long	0x39800000


		.align 4
! NCLIP
! r4  addr of CP2D

.globl _asmNCLIP
_asmNCLIP:
	sts.l    pr,@-r15
	mov      #12*4, r0
	mov.w    @(r0, r4), r1	! gteSX0
	add      #4, r0
	mov.w    @(r0, r4), r2	! gteSX1
	add      #4, r0
	mov.w    @(r0, r4), r3	! gteSX2
	
	sub      r1, r3	! r3  (sx2-sx0)
	sub      r2, r1	! r1  (sx0-sx1)

	mov      #12*4+2, r0
	mov.w    @(r0, r4), r2	! gteSY0
	add      #4, r0
	mov.w    @(r0, r4), r5	! gteSY1
	add      #4, r0
	mov.w    @(r0, r4), r6	! gteSY2

	sub      r2, r5	! r5  (sy1-sy0)
	sub      r6, r2	! r2  (sy0-sy2)
	
	mul.l    r5, r3
	sts      macl, r3
	mul.l    r1, r2
	sts      macl, r2

	sub      r3, r2
	
	mov      #24*4, r0	! gteMAC0
	lds.l    @r15+,pr
	rts
	mov.l    r2, @(r0, r4)


! asmStore32(float *vec, u32 *addr)
! R4 addr of vector
! R5 addr of gteMAC/gteIR
.globl _asmStore32
_asmStore32:
	fmov.s   @R4+,fr0
	sts.l    pr,@-r15
	fmov.s   @R4+,fr1
	add      #12, R5
	fmov.s   @R4+,fr2

	ftrc     fr2, fpul
	sts.l    fpul, @-R5
	ftrc     fr1, fpul
	sts.l    fpul, @-R5
	ftrc     fr0, fpul
	lds.l    @r15+, pr
	rts
	sts.l    fpul, @-R5

! asmLoadVec(u32 *addr, float *vec)
! R4 addr of gteV[XYZ]
! R5 addr of vector
.globl _asmLoadVec
_asmLoadVec:
	mov.w    @R4+, R0
	sts.l    pr,@-r15
	mov.w    @R4+, R1
	add      #12, R5
	mov.w    @R4+, R2

	lds      r2, fpul
	float    fpul, fr2
	fmov.s   fr2, @-r5
	lds      r1, fpul
	float    fpul, fr1
	fmov.s   fr1, @-r5
	lds      r0, fpul
	float    fpul, fr0
	lds.l    @r15+, pr
	rts
	fmov.s   fr0, @-r5

! asmLoad16(u32 *addr, float *vec)
! R4 addr of gteIR
! R5 addr of vector
.globl _asmLoad16
_asmLoad16:
	mov.l    @R4+, R0
	exts.w   R0, R0
	sts.l    pr,@-r15
	mov.l    @R4+, R1
	exts.w   R1, R1
	add      #12, R5
	mov.w    @R4+, R2
	exts.w   R2, R2

	lds      r2, fpul
	float    fpul, fr2
	fmov.s   fr2, @-r5
	lds      r1, fpul
	float    fpul, fr1
	fmov.s   fr1, @-r5
	lds      r0, fpul
	float    fpul, fr0
	lds.l    @r15+, pr
	rts
	fmov.s   fr0, @-r5

! u32 asmColorCalc(float *ir_t, float *mac_t, u32 lim)
! R4 addr of input vector and clipped transformed vector (ir)
! R5 addr of unclipped vector (mac)
! R6 <=0: clip to -32768  >0: clip to 0
! ir[3] should be loaded with gteIR0
! returns gteFLAG in R0

.globl _asmColorCalc
_asmColorCalc:
	sts.l    pr,@-r15
	mov      #0, R1		! gteFLAG
	mov      R6, R7
	
! transform color vector
	fmov.s   @R4+,fr8	! r
	fmov.s   @R4+,fr9	! g
	fmov.s   @R4+,fr10	! b
	fmov.s   @R4, fr11	! gteIR0

	mov.l    fcVector_addr, R2
	fmov.s   @R2+,fr4
	fsub     fr8, fr4	! fr4 = rfc - r
	fmov.s   @R2+,fr5
	fsub     fr9, fr5	! fr5 = gfc - g
	fmov.s   @R2+,fr6
	fsub     fr10, fr6	! fr6 = bfc - b

	mova     asmClip, R0
	jsr      @R0
	mov      #0, R6

	fmul     fr11, fr4
	mova     floatx4096, r0
	fmul     fr11, fr5
	fmov.s   @r0, fr0
	fmul     fr11, fr6
	
	fmac     fr0, fr4, fr8    ! fr8 = fr0 * fr4 + fr8
	fmov     fr8, fr4
	fmac     fr0, fr5, fr9    ! fr9 = fr0 * fr5 + fr9
	fmov     fr9, fr5
	fmac     fr0, fr6, fr10   ! fr10 = fr0 * fr6 + fr10
	fmov     fr10, fr6
		
! store back into mac vector right shifted by 12
	add      #12, r5
	fmov.s   fr6, @-r5
	fmov.s   fr5, @-r5
	mova     asmClip, R0
	fmov.s   fr4, @-r5

	jsr      @R0
	mov      R7, R6

! store back clipped vector into ir
	fmov.s   fr6, @-r4
	fmov.s   fr5, @-r4
	fmov.s   fr4, @-r4
	lds.l    @r15+, pr
	rts
	mov      r1, r0
	
		.align 4
fcVector_addr:
		.long	_fcVector
floatx4096: ! 1/4096
		.long	0x39800000

! Clip vector to [-32768, 32767] (r6 <= 0) or [0, 32767] (r6 > 0)
! vector in fr4, fr5, fr6
! r1 gteFLAG
! deletes contents of fr0, fr1, r2
asmClip:
	sts.l    pr,@-r15
! check limits
	fldi0    fr0
	cmp/pl   r6
	bt       .L_Clip_L0
	mova     float32km, r0
	fmov.s   @r0, fr0
.L_Clip_L0:
	mova     float32kp, r0
	fmov.s   @r0, fr1

	fcmp/gt  fr4, fr0	
	bf       .L_Clip_L1
	fmov     fr0, fr4
	mov.l    oflow0, r2
	bra      .L_Clip_L2
	or       r2, r1	! gteFLAG
.L_Clip_L1:
	fcmp/gt  fr1, fr4
	bf       .L_Clip_L2
	mov.l    oflow0, r2
	fmov     fr1, fr4
	or       r2, r1	! gteFLAG

.L_Clip_L2:
	fcmp/gt  fr5, fr0	
	bf       .L_Clip_L3
	fmov     fr0, fr5
	mov.l    oflow1, r2
	bra      .L_Clip_L4
	or       r2, r1	! gteFLAG
.L_Clip_L3:
	fcmp/gt  fr1, fr5
	bf       .L_Clip_L4
	mov.l    oflow1, r2
	fmov     fr1, fr5
	or       r2, r1	! gteFLAG

.L_Clip_L4:
	fcmp/gt  fr6, fr0
	bf       .L_Clip_L5
	fmov     fr0, fr6
	mov.l    oflow2, r2
	bra      .L_Clip_L6
	or       r2, r1	! gteFLAG
.L_Clip_L5:
	fcmp/gt  fr1, fr6
	bf       .L_Clip_L6
	mov.l    oflow2, r2
	fmov     fr1, fr6
	or       r2, r1	! gteFLAG

.L_Clip_L6:
	lds.l    @r15+, pr
	rts
	nop
	
	.align	4
oflow0:
		.long	0x81000000
oflow1:
		.long	0x80800000
oflow2:
		.long	0x00400000
float32km:
		.long	0xc7000000
float32kp:
		.long	0x46fffe00
