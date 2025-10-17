#include <windows.h>
#include "../GLwinDialog.h"
#include "../GLwinLog.h"

// Standard Open File Dialog
std::string GLwinOpenDialog()
{
	OPENFILENAME ofn;       // common dialog box structure
	
	wchar_t filename[MAX_PATH];      // buffer for file name

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;  // If you have a window to center over, put its handle here
	ofn.lpstrFile = filename;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrFilter = L"All\0*.*\0Text\0*.TXT\0SpxSpl\0*.spl\0SpxSplh\0*.splh";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;	

	if (GetOpenFileName(&ofn) == TRUE) {
		// Convert wchar_t* to std::string
		std::wstring ws(ofn.lpstrFile);
		std::string str(ws.begin(), ws.end());
		return str;
	}
	else {
		DWORD err = CommDlgExtendedError();
		if (err != 0) {
			// Handle the error as needed
			GLWIN_LOG_ERROR("GetOpenFileName failed with error code: " << err << std::endl);

			return "";
		}
	}

	
}
// Standard Save File Dialog
std::string GLwinSaveDialog()
{
	OPENFILENAME ofn;
	wchar_t filename[MAX_PATH];

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;  // Set this to your window handle if you want to parent the dialog
	ofn.lpstrFile = filename;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(filename) / sizeof(wchar_t);
	ofn.lpstrFilter = L"All\0*.*\0Text\0*.TXT\0SpxSpl\0*.spl\0SpxSplh\0*.splh";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	// For save dialog, typically OFN_OVERWRITEPROMPT is used
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

	if (GetSaveFileName(&ofn) == TRUE) {
		// Convert wchar_t* to std::string
		std::wstring ws(ofn.lpstrFile);
		std::string str(ws.begin(), ws.end());
		return str;
	}
	else {
		DWORD err = CommDlgExtendedError();
		if (err != 0) {
			GLWIN_LOG_ERROR("GetSaveFileName failed with error code: " << err << std::endl);
		}
		return "";
	}
}
