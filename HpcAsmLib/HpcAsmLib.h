#pragma once

#define		F_LOOP		10000
#define		F1_LOOP  F_LOOP
#define		F2_LOOP  F_LOOP

#define STACK_SZ_TEST 5000000

#define	PROC_AFF_MN     1
#define	PROC_AFF1		4
#define	PROC_AFF2		32

#define NANO_SEC_BACK_OFF_1		100
#define NANO_SEC_BACK_OFF_2		100


typedef struct _AsmInfo {
	unsigned long long		tmr_cnt;
	DWORD					test_value;
} AsmInfo, * pAsmInfo;

int  __fastcall AsmRead(int*, AsmInfo*);
void __fastcall AsmInc(int*, AsmInfo*);

