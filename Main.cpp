#include <Windows.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "INIReader.h"
using namespace std;

#define _ININAME_ "ENBAuto.ini"
#define _DEFAULTEXE_ "FalloutNV.exe"
#define _DEFAULTINJECTOR_ "ENBInjector.exe"

enum BeepType {
	BeepInit = 0,
	BeepInjector = 1,
	BeepExecutable = 2,
	BeepError = 3,
	BeepClose = 4
};

//globals
wstring dir;
HWND hwnd_injector;

//prototypes
void DoBeep(BeepType bt);
void CreateINI();
wstring s2ws(const string& s); //fuck wstrings, fuck unicode, fuck everything
wstring GetDir(); //absolutely fuck LPWSTR, fucking worst type ever
void NotepadTest();
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

//fuck unicode

BOOL WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	dir = GetDir();
	INIReader ini_config(_ININAME_);

	if (ini_config.ParseError() < 0) {
		DoBeep(BeepError);
		CreateINI();
		return true;
	}

	if (ini_config.GetBoolean("", "DoBeep", true)) 
		DoBeep(BeepInit); //do initialization beep so the user know ENBAuto has loaded correctly

	wstring file_injector = dir + s2ws(ini_config.Get("", "InjectorName", _DEFAULTINJECTOR_));
	SHELLEXECUTEINFO injectorinfo;
	ZeroMemory(&injectorinfo, sizeof(SHELLEXECUTEINFO)); //memory needs to be cleared out first
	injectorinfo.cbSize = sizeof(SHELLEXECUTEINFO);
	injectorinfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	injectorinfo.lpVerb = L"open";
	injectorinfo.lpFile = file_injector.c_str();
	injectorinfo.nShow = SW_SHOWNORMAL;
	if (ShellExecuteEx(&injectorinfo)) {
		WaitForInputIdle(injectorinfo.hProcess, INFINITE); //wait for program to stabilize
		DWORD id_injector = GetProcessId(injectorinfo.hProcess);
		EnumWindows(&EnumWindowsProc, id_injector);

		wstring file_exe = s2ws(ini_config.Get("", "ExeName", _DEFAULTEXE_));
		SHELLEXECUTEINFO exeinfo;
		ZeroMemory(&exeinfo, sizeof(exeinfo));
		exeinfo.cbSize = sizeof(SHELLEXECUTEINFO);
		exeinfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		exeinfo.lpVerb = L"open";
		exeinfo.lpFile = file_exe.c_str();
		exeinfo.nShow = SW_SHOWNORMAL;
		if (ShellExecuteEx(&exeinfo)) {//run game executable and wait
			WaitForInputIdle(exeinfo.hProcess, INFINITE);
			WaitForSingleObject(exeinfo.hProcess, INFINITE);
		}
		//after the game executable's process has ended...
		PostMessage(hwnd_injector, WM_CLOSE, NULL, NULL);
	}
	return false; //end program
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
	DWORD id;
	GetWindowThreadProcessId(hwnd, &id);
	if (id == lParam) {
		hwnd_injector = hwnd;
		return FALSE;
	}
	else return TRUE;
}

void DoBeep(BeepType bt) {
	switch (bt) {
		case BeepInit:
			Beep(400, 100);
			Beep(500, 100);
			break;
		case BeepInjector:
			Beep(1000, 100);
			Beep(500, 100);
			break;
		case BeepExecutable:
			Beep(500, 100);
			Beep(1000, 100);
			break;
		case BeepError:
			Beep(2000, 100);
			Beep(2000, 100);
			break;
		case BeepClose:
			Beep(500, 66);
			Beep(500, 66);
			Beep(500, 66);
			break;
	}
}

void CreateINI() {
	ofstream ini("ENBAuto.ini"); //create new ini file
	ini << ";ENBAuto Config File\n"
		//<< "[ENBAuto]\n"
		<< "ExeName=" << _DEFAULTEXE_ << "\n"
		<< "InjectorName=" << _DEFAULTINJECTOR_ << "\n"
		<< "DoBeep=true\n"
		<< flush; //force file to be written before closing the program
	ini.close();
	MessageBox( //show popup telling a new file was created
		NULL, 
		L"No configuration file found, so a new one was created!", 
		L"ENBAuto", 
		MB_OK
	);
	ShellExecute( //open the ini file
		NULL,
		L"open",
		L"ENBAuto.ini",
		NULL,
		NULL,
		SW_SHOWNORMAL
	);
}

wstring s2ws(const string& s) {
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, NULL, s.c_str(), slength, NULL, NULL);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, NULL, s.c_str(), slength, buf, len);
	wstring r(buf);
	delete[] buf;
	return r;
}

wstring GetDir() {
	wchar_t buffer[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, buffer);
	return wstring(buffer) + wstring(L"\\");
}
