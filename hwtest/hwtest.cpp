// hwtest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
//#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "process.h"
#include <vector>
#include <array>
#include <iostream>

using namespace std;


#ifdef __cplusplus
extern "C" {
#endif

#include "HpcAsmLib.h"

#ifdef __cplusplus
} //end extern "C"
#endif

void NanoSecSpin(int t) {
	for (int i = 0; i < t / 10; i++) {
		int s;
		s = i;
	}

}

std::string GetLastErrorAsString(DWORD error_code)
{
	if (error_code == 0) {
		return std::string(); //No error message has been recorded
	}

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	LocalFree(messageBuffer);

	return message;
}

typedef struct _AsmInfoBlk {
	int			idx;
	struct _AsmInfo		asm_info_ary[F_LOOP];
} AsmInfoBlk, * pAsmInfoBlk;

typedef struct _ThrdsAry {
	int		idx;
	HANDLE	thrds[];
} ThrdsAry, * pThrdsAry;

class ThrdMgmt {

public:
	bool	obj_init_ok = true;
	static	int		test_sem;

	static unsigned __stdcall func1(void* p);
	static unsigned __stdcall func2(void* p);

	static ThrdsAry thrds_ary;

	struct _AsmInfoBlk	asm_info_blk = { 0 };

	ThrdMgmt() {
		do {
			if (SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS) == ERROR)
			{
				obj_init_ok = false;
				break;
			}

			if (!(SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST)))
			{
				obj_init_ok = false;
				break;
			}
			if (SetThreadAffinityMask(GetCurrentThread(), PROC_AFF_MN) == false) {
				obj_init_ok = false;
				break;
			}

		} while (false);
	}
	~ThrdMgmt() {
		// Scan list
		obj_init_ok = false;
	}

};

int ThrdMgmt::test_sem = 0;

ThrdsAry ThrdMgmt::thrds_ary = { 0, {0,0} };

class Thrd {
	DWORD	affinity_mask;
	class Thrd* lthis;

public:
	class	ThrdMgmt& rtm;
	HANDLE	h_thread = 0;

	struct _AsmInfoBlk	asm_info_blk = { 0 };


	bool	obj_init_ok = false;

	Thrd(unsigned(__stdcall* thrd_func)(void*), DWORD	aff_mask, class ThrdMgmt& tm) : affinity_mask(aff_mask), rtm(tm) {
		do {
			h_thread = (HANDLE)_beginthreadex(nullptr, STACK_SZ_TEST, thrd_func, this, CREATE_SUSPENDED, 0);
			if (h_thread == (HANDLE)HFILE_ERROR) {
				h_thread = 0;
				break;
			}
			if (SetThreadAffinityMask(h_thread, affinity_mask) == false) {
				if (h_thread != 0) _endthreadex(0);
				break;
			}

			if (!(SetThreadPriority(h_thread, THREAD_PRIORITY_BELOW_NORMAL)))
			{
				_endthreadex(0);
				break;
			}

			rtm.thrds_ary.thrds[rtm.thrds_ary.idx++] = h_thread;

			obj_init_ok = true;
		} while (false);
	}

	bool ThrdResume() {
		bool rc = true;
		if (ResumeThread(h_thread) == (DWORD)-1) rc = false;
		return rc;
	}

	~Thrd() {
		if (h_thread != 0) _endthreadex(0);
	}

};

unsigned __stdcall ThrdMgmt::func1(void* p) {

	for (int i = 0; i < F1_LOOP; i++) {
		AsmRead(&ThrdMgmt::test_sem, &(((class Thrd*)p)->asm_info_blk.asm_info_ary[i]));
		NanoSecSpin(NANO_SEC_BACK_OFF_2);
	}
	return 0;
}

unsigned __stdcall ThrdMgmt::func2(void* p) {

	for (int i = 0; i < F2_LOOP; i++) {
		AsmInc(&ThrdMgmt::test_sem, &(((class Thrd*)p)->asm_info_blk.asm_info_ary[i]));
		NanoSecSpin(NANO_SEC_BACK_OFF_1);
	}
	return 0;
}

int main()
{
	DWORD wrc;

	std::cout << "Test to determine machines memory interlace characteristics\n";

	// Init subsysems with RAII
	class ThrdMgmt ptm;
	if ((ptm.obj_init_ok) == false) exit(1);
	class Thrd th1(&ThrdMgmt::func1, PROC_AFF1, ptm);
	if (!th1.obj_init_ok) exit(1);
	class Thrd th2(&ThrdMgmt::func2, PROC_AFF2, ptm);
	if (!th2.obj_init_ok) exit(1);

	// Start pinned threads
	th1.ThrdResume();
	th2.ThrdResume();

	// Wait
	if ((wrc = WaitForMultipleObjectsEx(2, (const HANDLE*)(ptm.thrds_ary.thrds), true, INFINITE, false)) > 1) {
		std::cout << "Wait fail \n";
	}
	if (wrc == WAIT_FAILED) {
		std::cout << "Error Code " << GetLastErrorAsString(GetLastError()) << "\n";
	}
	else {

		std::cout << "WMO rc: " << std::hex << wrc << "\n";

		std::cout << "Back Offs in ns: " << NANO_SEC_BACK_OFF_1 << "\n";
		std::cout << "Back Offs in ns: " << NANO_SEC_BACK_OFF_2 << "\n";

		for (int i = 0; i < F_LOOP; i++) {
			std::cout << "Sem Value: " << th1.asm_info_blk.asm_info_ary[i].test_value << " Time Stamp: " << th1.asm_info_blk.asm_info_ary[i].tmr_cnt << "\n";
		}

	}


	return(0);
}

