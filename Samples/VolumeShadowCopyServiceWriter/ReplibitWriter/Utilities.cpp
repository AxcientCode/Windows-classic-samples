#include "stdafx.h"
#include "Utilities.h"

bool GetNTFSvolumeData(HANDLE m_hVolume, NTFS_VOLUME_DATA_BUFFER& m_nvdbData) {
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
        wprintf(TEXT(__FUNCTION__) L"DeviceIoControl for FSCTL_GET_NTFS_VOLUME_DATA failed. (0x%08lx) \r\n",
                GetLastError());
    }
    return false;
}
