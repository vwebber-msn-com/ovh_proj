; Lock Free Queue Semaphore in x64 Assembly
; Victor Webber 
;
;The LOCK prefix is valid only with the
;following instructions and only when they modify memory:
;
;  BTS, BTR, BTC -> bit test and change instructions
;  XCHG, XADD, CMPXCHG -> exchange instructions
;  INC, DEC, NOT, NEG -> single operand instructions
;  ADD, ADC, SUB, SBB, AND, OR, XOR -> two operand arithmetic and logic
;

.code

AsmInfo		STRUCT
	tstmp	DQ	?
	tval	DD	?
AsmInfo		ENDS



PUBLIC AsmInc
AsmInc PROC
	lfence
	inc DWORD PTR [rcx]
	mov		rcx, rdx
	rdtsc	
	sal		rdx, 32
	or		rax, rdx
	mov		[rcx.AsmInfo.tstmp], rax
	mov		[rcx.AsmInfo.tval], 0
	ret
AsmInc ENDP

PUBLIC AsmRead
AsmRead PROC
	xor		rax, rax
	lfence
	xadd	DWORD PTR [rcx], eax
	mov		[rdx.AsmInfo.tval], eax
	mov		rcx, rdx
	rdtsc	
	sal		rdx, 32
	or		rax, rdx
	mov		[rcx.AsmInfo.tstmp], rax
	ret
AsmRead ENDP

END









