// Hello.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

// Notes -
// This is for the Postal III (Sep 29, 2009 prototype) port on Xbox 360.
// 
// Issues-
// Unfortunately, the particles have to be removed, and some jank things I had to patch.
// Something also seems to call KeBugCheck in Engine_360.dll sometimes.
// Below, I've attempted to comment and explain what each patch is for.
// I've left my failed, or test patches in here commented out. 

__declspec(naked) void _x887453D8_Patch() {
	__asm {

		// Additional nullptr check.
		cmplwi cr6, r3, 0
		beq cr6, jEnd

		// Original instructions.
		lwz       r11, 0(r3)
		mr        r4, r30
		lwz       r10, 0x1B4(r11)
		mtctr     r10
		bctrl

		jEnd:

		// Jmp after patch.
		lis r0, 0x8874
		ori r0, r0, 0x53EC
		mtctr r0
		bctr
	}
}

__declspec(naked) void _x88745498_Patch() {
	__asm {
		// Additional nullptr check.
		cmplwi cr6, r3, 0
		beq cr6, jEnd

		// Original instructions.
		lwz       r11, 0(r3)
		lwz       r10, 0x164(r11)
		mtctr     r10
		bctrl

		jEnd:

		// Jmp after patch.
		lis r0, 0x8874
		ori r0, r0, 0x54A8
		mtctr r0
		bctr
	}
}

// Fix for the random KeBugCheck call.
void KeBugCheck_Enginedll_Patch() {
	DWORD ReturnAddress = 0;
	__asm mflr ReturnAddress

	//DbgPrint("Engine.dll attempted to call KeBugCheck: lr=0x%X\n", ReturnAddress);
}

Detour<int> sub_874E2968_Detour;
int sub_874E2968_Hook(int r3, int r4, int r5) {

	return 0;

	// This has a 50/50 chance of working.
	// it's 1am, give me a break.

	// Note from the future:
	// This was an attempt to see if I could filter out what effects would play.
	// Unfortunately, this did not work, at some point it'd just crash in the plugin.
	// I gave up and decided to disable the effect system entirely.
	// It seems to be particles that are just reading random things, or aren't initialized correctly.
	// They have a lot of random stuff passed in, I also saw this when patching the interactable object crashes. 

	//if (r3 == 0 || !MmIsAddressValid((void*)r3)) {
	//	DbgPrint("Hook2 called: 0x%X\n", r3);
	//	return 0;
	//}
	//
	//DWORD unkStruct = *(DWORD*)(r3 + 0x40);
	//
	//if (!unkStruct || !MmIsAddressValid((void*)unkStruct)) {
	//	DbgPrint("Hook2.5 called: 0x%X\n", r3);
	//	return 0;
	//}
	//
	//DWORD unkStruct1 = *(DWORD*)(unkStruct + 0x280); // Count? 
	//DWORD unkStruct2 = *(DWORD*)(unkStruct + 0x274); 
	//DWORD unkStruct3 = *(DWORD*)(unkStruct + 0x300);
	//
	//void* pCombined1 = (void*)(unkStruct2 + (unkStruct1 * 0x4));
	//void* pCombined2 = (void*)(unkStruct3 + (unkStruct1 * 0x4));
	//
	//if (!MmIsAddressValid(pCombined1) || !MmIsAddressValid(pCombined2)) {
	//	DbgPrint("Hook2.6 called: 0x%X\n", r3);
	//	return 0;
	//}
	//
	//return sub_874E2968_Detour.CallOriginal(r3, r4, r5);
}

// Test hook before I decided to disable the particles entirely.
Detour<int> ParticleSystem_StopEmission_Detour;
int ParticleSystem_StopEmission_Hook(int r3, int r4, int r5, int r6) { // 8738D620

	if (r3 == 0 || !MmIsAddressValid((void*)r3)) {
		//DbgPrint("Hook called: 0x%X\n", r3);
		return 0;
	}
	 
	return ParticleSystem_StopEmission_Detour.CallOriginal(r3, r4, r5, r6);
}

// Hook to catch and patch modules loaded after client_360.dll
Detour<int> sub_82796C60_Detour;
int sub_82796C60_Hook(int r3, int r4, int r5) {
	// 82796C60
	DWORD ReturnAddress = 0;
	__asm mflr ReturnAddress

	// Wait for server dll to call, we load before it.
	if (ReturnAddress == 0x862D2738) {

		// This patches some "script?" issues.
		// Interactable objects* 
		PatchInJump((DWORD*)0x887453D8, _x887453D8_Patch, false);
		PatchInJump((DWORD*)0x88745498, _x88745498_Patch, false);

		// Disables some interactble objects, like doors.
		//*(QWORD*)0x88745368 = 0x386000004E800020;

		// Some test, probably breaks a lot.
		//*(QWORD*)0x885E4E10 = 0x386000004E800020;
		//*(DWORD*)0x885E4E4C = 0x60000000;
	}

	//DbgPrint("Call from: 0x%X\n", ReturnAddress);

	return sub_82796C60_Detour.CallOriginal(r3, r4, r5);
}

VOID OnAttachProcess() {
	DbgPrint("Hello from HPatch.dll!\n");
	DbgPrint("Applying patches!\n");

	*(DWORD*)0x874956CC = 0x60000000; // Call to spawn particle when shooting?
	*(DWORD*)0x87294930 = 0x60000000; // Unk crash when walking 

	*(DWORD*)0x87492018 = 0x4E800020; // Patch crash when loading
	//*(short*)0x874920A4 = 0x4800; // ^ Another way to patch this.

	*(short*)0x8738D424 = 0x4800; // Jump over some weird loop
	*(short*)0x8751DF08 = 0x4800; // Jump over some other weird thing.

	// If things really start to break, try to remove this.
	// These seem to also fix a crash in maps, idk.
	*(short*)0x874942A4 = 0x4800; // Fix a loading hang.
	*(DWORD*)0x874BAC88 = 0x60000000; // Fix a crash fixing the loading hang.

	// Jump back to a valid function epilogue when we call KeBugCheck...
	*(DWORD*)0x8615E22C = 0x4BFFFF94;

	// I don't even remember what these are.
	// They're from when I was trying to fix particle crashes.
	//*(short*)0x8749C970 = 0x4800; 
	//*(DWORD*)0x872F5AB4 = 0x60000000;
	//*(QWORD*)0x874E2968 = 0x386000004E800020;

	// Remove most stuff from the particle system.
	*(QWORD*)0x87383C90 = 0x386000004E800020;
	*(QWORD*)0x8738DF30 = 0x386000004E800020;
	*(QWORD*)0x8738D988 = 0x386000004E800020;
	*(QWORD*)0x87277558 = 0x386000004E800020;
	*(QWORD*)0x8738E030 = 0x386000004E800020;
	*(QWORD*)0x8738DDF0 = 0x386000004E800020;
	*(QWORD*)0x8738D620 = 0x386000004E800020;

	// This is a pretty jank fix, but it does work. 
	// Crash in a call to KeFreeTls, probably important...
	*(DWORD*)0x8A92EE34 = 0x60000000; // Crash on respawn fix

	// 8ab06178

	//ParticleSystem_StopEmission_Detour.SetupDetour(0x8738D620, ParticleSystem_StopEmission_Hook);
	//sub_874E2968_Detour.SetupDetour(0x874E2968, sub_874E2968_Hook);

	sub_82796C60_Detour.SetupDetour(0x82796C60, sub_82796C60_Hook);
	PatchInJump((DWORD*)0x8638E3C4, KeBugCheck_Enginedll_Patch, false);
}

VOID OnDetachProcess() {}

// This is the main entry point to your xex file.
BOOL WINAPI DllMain(HANDLE hInst, DWORD dwReason, LPVOID lpReserved) {
	if (dwReason == DLL_PROCESS_ATTACH) OnAttachProcess();
	if (dwReason == DLL_PROCESS_DETACH) OnDetachProcess();

	return TRUE;
} 