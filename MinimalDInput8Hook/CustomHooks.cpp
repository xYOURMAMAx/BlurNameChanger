#include "stdafx.h"
#include "CustomHooks.h"

typedef BOOL(WINAPI*GetUserNameA_t)(
	LPSTR                 lpBuffer,
	LPDWORD               pcbBuffer);

GetUserNameA_t OriginalGetUserNameA;

BOOL WINAPI GetUserNameA_Wrapper(
	LPSTR                 lpBuffer,
	LPDWORD               pcbBuffer
)
{
	HANDLE hFile = CreateFile(
		L"Driver",
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	try {
		if (hFile == INVALID_HANDLE_VALUE)
			throw FALSE;

		DWORD fileSize = GetFileSize(hFile, NULL);

		if (fileSize == INVALID_FILE_SIZE) {
			CloseHandle(hFile);
			throw FALSE;
		}

		const DWORD bufferSize = 16; // Max amount of characters supported
		char* buffer = new char[bufferSize + 1];

		DWORD bytesRead;
		if (ReadFile(hFile, buffer, bufferSize, &bytesRead, NULL) == FALSE) {
			CloseHandle(hFile);
			throw FALSE;
		}

		buffer[bytesRead] = '\0';

		CloseHandle(hFile);

		if (*pcbBuffer < bytesRead + 1) {
			throw FALSE;
		}

		strcpy_s(lpBuffer, *pcbBuffer, buffer);

		*pcbBuffer = static_cast<DWORD>(bytesRead);

		return TRUE;

	} catch (bool result) {
		// Use default driver name on any error
		const char* defaultUsername = "Driver";
		size_t defaultUsernameLength = strlen(defaultUsername);

		if (*pcbBuffer < defaultUsernameLength + 1)
			return false;

		strcpy_s(lpBuffer, *pcbBuffer, defaultUsername);

		*pcbBuffer = static_cast<DWORD>(defaultUsernameLength);

		return TRUE;
	}
}

void SetupHooks()
{
	// Create a console for Debug output
	// AllocConsole();

	// Setup hooks here, see examples below
	OriginalGetUserNameA = HookFunction("ADVAPI32.dll", "GetUserNameA", &GetUserNameA_Wrapper);
}

