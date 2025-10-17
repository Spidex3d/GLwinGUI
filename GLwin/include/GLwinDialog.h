#pragma once
#include <windows.h>
#include <string>


#ifdef __cplusplus
extern "C" {
#endif

	// Open a standard file open dialog and print the selected file path
	std::string GLwinOpenDialog();
	// Open a standard file save dialog and print the selected file path
	std::string GLwinSaveDialog();


	

#ifdef __cplusplus
}
#endif


