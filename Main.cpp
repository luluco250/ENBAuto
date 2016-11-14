#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
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
wstring cmdline;

//prototypes
void DoBeep(BeepType bt);
void CreateINI();
wstring s2ws(const string& s); //fuck wstrings
wstring GetDir(); //tfw this could be much easier
void NotepadTest();
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

struct EnumWindowsInfo { //struct for use with the enumwindowsproc callback
	DWORD id;
	HWND hwnd;
	EnumWindowsInfo(DWORD const ProcessID) : id(ProcessID) {}
};

BOOL WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	cmdline = pCmdLine; //get command line
	dir = GetDir(); //get the directory in which ENBAuto is located

	//NotepadTest(); //for testing functionality
	//return false;  //using notepad
	
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
		EnumWindowsInfo inject_winfo = EnumWindowsInfo(GetProcessId(injectorinfo.hProcess));
		EnumWindows(&EnumWindowsProc, reinterpret_cast<LPARAM>(&inject_winfo));

		wstring file_exe = s2ws(ini_config.Get("", "ExeName", _DEFAULTEXE_));
		SHELLEXECUTEINFO exeinfo;
		ZeroMemory(&exeinfo, sizeof(exeinfo));
		exeinfo.cbSize = sizeof(SHELLEXECUTEINFO);
		exeinfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		exeinfo.lpVerb = L"open";
		exeinfo.lpFile = file_exe.c_str();
		exeinfo.lpParameters = cmdline.c_str();
		exeinfo.nShow = SW_SHOWNORMAL;
		if (ShellExecuteEx(&exeinfo)) {//run game executable and wait
			WaitForSingleObject(exeinfo.hProcess, INFINITE); //this waits until the program process is terminated
		}
		//after the game executable's process has ended...
		PostMessage(inject_winfo.hwnd, WM_CLOSE, NULL, NULL);
	}
	return false; //end program
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
	EnumWindowsInfo *info = reinterpret_cast<EnumWindowsInfo*>(lParam);
	DWORD id;
	GetWindowThreadProcessId(hwnd, &id);
	if (info->id == id) { //if this process' id equals the id provided by the info
		info->hwnd = hwnd;
		return FALSE;
	}
	else return TRUE;
}

void NotepadTest() { //this is a function to test functionalities using notepad
	SHELLEXECUTEINFO notepadinfo;
	ZeroMemory(&notepadinfo, sizeof(SHELLEXECUTEINFO));
	notepadinfo.cbSize = sizeof(SHELLEXECUTEINFO);
	notepadinfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	notepadinfo.lpVerb = L"open";
	notepadinfo.lpFile = L"C:\\Windows\\system32\\notepad.exe";
	//notepadinfo.lpParameters = L"C:\\Users\\Lucas\\Documents\\test.txt";
	notepadinfo.lpParameters = cmdline.c_str(); //test command line arguments/parameters
	notepadinfo.nShow = SW_SHOWNORMAL;
	if (ShellExecuteEx(&notepadinfo)) {
		WaitForInputIdle(notepadinfo.hProcess, INFINITE);
		EnumWindowsInfo info = EnumWindowsInfo(GetProcessId(notepadinfo.hProcess));
		EnumWindows(&EnumWindowsProc, reinterpret_cast<LPARAM>(&info));
		//DoBeep(BeepClose);
		//PostMessage(info.hwnd, WM_CLOSE, NULL, NULL);
	}
}

void DoBeep(BeepType bt) { //feedback beeps
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
