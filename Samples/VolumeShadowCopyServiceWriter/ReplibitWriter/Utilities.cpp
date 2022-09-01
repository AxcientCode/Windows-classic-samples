#include "stdafx.h"
#include "Utilities.h"

bool GetNTFSvolumeData(HANDLE m_hVolume, NTFS_VOLUME_DATA_BUFFER &m_nvdbData) {
    DWORD dwBytesReturned;

    if (DeviceIoControl(m_hVolume,                        // handle to device
                        FSCTL_GET_NTFS_VOLUME_DATA,       // dwIoControlCode
                        NULL,                             // lpInBuffer
                        0,                                // nInBufferSize
                        &m_nvdbData,                      // output buffer
                        sizeof(NTFS_VOLUME_DATA_BUFFER),  // size of output buffer
                        &dwBytesReturned,                 // number of bytes returned
                        NULL))                            // OVERLAPPED structure
    {
        m_nvdbData.NumberSectors.QuadPart++;  // Add hidden sector
        return true;
    } else {
        _tprintf(TEXT(__FUNCTION__) L"DeviceIoControl for FSCTL_GET_NTFS_VOLUME_DATA failed. (0x%08lx) \r\n",
                GetLastError());
    }
    return false;
}

bool GetAllocationBitmap(HANDLE hVolume, CChunkBitmap *pBitmap, CHUNK_BITMAP_SET_STATE iBitmapSetState) {
    // Input
    const DWORD dwInBufferSize = sizeof(STARTING_LCN_INPUT_BUFFER);
    STARTING_LCN_INPUT_BUFFER
    sliBuffer = {0};

    // Output
    const DWORD dwOutHeaderSize = 2 * sizeof(LARGE_INTEGER);
    const DWORD dwOutBufferSize = (DWORD)((CB_CHUNK_SIZE * 16) + dwOutHeaderSize);
    DWORD dwBytesReturned;
    BOOL bResult = FALSE;
    PVOLUME_BITMAP_BUFFER
    lpOutBuffer = (PVOLUME_BITMAP_BUFFER)malloc(dwOutBufferSize);

    int64_t i64CopyStartingIndex, i64CopyLength;

    while (true) {
        ZeroMemory((PBYTE)lpOutBuffer, dwOutBufferSize);

        bResult = DeviceIoControl(hVolume,                  // handle to volume
                                  FSCTL_GET_VOLUME_BITMAP,  // dwIoControlCode
                                  &sliBuffer,               // input buffer
                                  dwInBufferSize,           // size of input buffer
                                  lpOutBuffer,              // output buffer
                                  dwOutBufferSize,          // size of output buffer
                                  &dwBytesReturned,         // number of bytes returned
                                  NULL);

        if (bResult || (!bResult && GetLastError() == ERROR_MORE_DATA)) 
        {
            i64CopyStartingIndex = lpOutBuffer->StartingLcn.QuadPart;  // copy start offset in bit index
            i64CopyLength = dwBytesReturned - dwOutHeaderSize;         // number of bitmap bytes to copy

            if (i64CopyStartingIndex + (i64CopyLength * 8) > pBitmap->GetNumberOfElements()) {
                // last time around the loop
                bResult = TRUE;
            }

            pBitmap->BitmapSetElementsFrom(&lpOutBuffer->Buffer[0],  // copy from
                                           (size_t)i64CopyLength, i64CopyStartingIndex, iBitmapSetState);

            sliBuffer.StartingLcn.QuadPart += i64CopyLength * 8LL;

            if (bResult) 
            {
                break;
            }
        }
        else
        {
            _tprintf(L"DeviceIoControl failed for FSCTL_GET_VOLUME_BITMAP. (0x%08lx)\n", GetLastError());
            break;
        }
    }

    if (lpOutBuffer != NULL) {
        free(lpOutBuffer);
        lpOutBuffer = NULL;
    }

    return bResult == TRUE;
}

void GetFullPath(_In_ const std::wstring &wsDevice, _In_ const wchar_t *pwcFileName, _Out_ std::wstring &pwcFullPath) {
    // _tprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    pwcFullPath = wsDevice;
    if (pwcFullPath[pwcFullPath.size() - 1] != L'\\') {
        pwcFullPath.push_back(L'\\');
    }
    pwcFullPath.append(pwcFileName);
    // _tprintf(TEXT(__FUNCTION__ ": End. \r\n"));
}

HANDLE CreateFileInDevice(DWORD dwDispotision, const std::wstring &wsDevice, const wchar_t *pwcFileName) {
    // _tprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    std::wstring wsFullPath;

    GetFullPath(wsDevice, pwcFileName, wsFullPath);
    HANDLE hFile = ::CreateFile(wsFullPath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL, dwDispotision, NULL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        _tprintf(TEXT(__FUNCTION__ ": CreateFile failed. (0x%08lx)\r\n"), GetLastError());
    }
    // _tprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return hFile;
}

bool MoveFilePointer(HANDLE hFile, LONG lPosition) {
    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, lPosition, NULL, FILE_BEGIN)) {
        _tprintf(TEXT(__FUNCTION__ ": SetFilePointer failed. (0x%08lx)\r\n"), GetLastError());
        // _tprintf(TEXT(__FUNCTION__ ": End. \r\n"));
        return false;
    }
    return true;
}

bool TruncateFile(HANDLE hFile) {
    if (!SetEndOfFile(hFile)) {
        _tprintf(TEXT(__FUNCTION__ ": SetEndOfFile failed. (0x%08lx)\r\n"), GetLastError());
        // _tprintf(TEXT(__FUNCTION__ ": End. \r\n"));
        return false;
    }
    return true;
}

bool SetSparseFlag(HANDLE hFile, BOOL bSetSparse) {
    // _tprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    // _tprintf(L"Setting sparse flag to: %s \n", bSetSparse ? L"true" : L"false");
    FILE_SET_SPARSE_BUFFER fssBuffer = {0};
    fssBuffer.SetSparse = bSetSparse;
    DWORD dwBytesReturned = 0;
    if (!DeviceIoControl(hFile,              // handle to a file
                         FSCTL_SET_SPARSE,   // dwIoControlCode
                         &fssBuffer,         // input buffer
                         sizeof(fssBuffer),  // size of input buffer
                         NULL,               // lpOutBuffer
                         0,                  // nOutBufferSize
                         &dwBytesReturned,   // number of bytes returned
                         NULL))              // OVERLAPPED structure
    {
        _tprintf(L"DeviceIoControl failed for FSCTL_SET_SPARSE. (0x%08lx)\n", GetLastError());
        // _tprintf(TEXT(__FUNCTION__ ": End. \r\n"));
        return false;
    }
    // _tprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
}
