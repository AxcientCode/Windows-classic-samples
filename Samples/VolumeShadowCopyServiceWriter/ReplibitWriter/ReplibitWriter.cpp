/*
**++
**
** Copyright (c) 2018 eFolder Inc
**
**
** Module Name:
**
**	swriter.cpp
**
*/

///////////////////////////////////////////////////////////////////////////////
// Includes

#include "stdafx.h"
#include "Utilities.h"
#include "ReplibitWriter.h"

///////////////////////////////////////////////////////////////////////////////

static const wchar_t g_pwcBeforeFileName[] = L"VolumeBitmapBefore.bin";
static const wchar_t g_pwcDifferencesFileName[] = L"DifferencesFound.txt";

// Initialize the writer
HRESULT STDMETHODCALLTYPE CReplibitWriter::Initialize() {
    wprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    HRESULT hr = CVssWriter::Initialize(guidReplibitWriterId,  // WriterID
                                        pwcWriterName,         // wszWriterName
                                        VSS_UT_USERDATA,       // ut
                                        VSS_ST_OTHER);         // st
    if (FAILED(hr)) {
        wprintf(TEXT(__FUNCTION__ ": CVssWriter::Initialize failed!. (0x%08lx)\r\n"), GetLastError());
        return hr;
    }

    // subscribe for events
    hr = CVssWriter::Subscribe();
    if (FAILED(hr)) wprintf(TEXT(__FUNCTION__ ": CVssWriter::Subscribe failed!. (0x%08lx)\r\n"), GetLastError());

    wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return hr;
}

// OnIdentify is called as a result of the requestor calling
// GatherWriterMetadata
bool STDMETHODCALLTYPE CReplibitWriter::OnIdentify(IN IVssCreateWriterMetadata *pMetadata) {
    wprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));

    // Set the restore method to restore if can replace
    HRESULT hr = pMetadata->SetRestoreMethod(VSS_RME_RESTORE_IF_CAN_REPLACE, NULL, NULL, VSS_WRE_NEVER, false);
    if (FAILED(hr)) {
        wprintf(TEXT(__FUNCTION__ ": SetRestoreMethod failed. (0x%08lx)\r\n"), GetLastError());
        return false;
    }

    hr = pMetadata->AddComponent(VSS_CT_FILEGROUP, NULL, L"VolumeBitmap", L"Volume bitmaps", NULL, 0, false, true, true,
                                 true);
    if (FAILED(hr)) {
        wprintf(TEXT(__FUNCTION__ ": AddComponent failed. (0x%08lx)\r\n"), GetLastError());
        return false;
    }

#if 0
    WCHAR pwcVolumeName[MAX_PATH + 1] = { L'\0' };
    HANDLE hVolume = INVALID_HANDLE_VALUE;
    if (INVALID_HANDLE_VALUE != (hVolume = FindFirstVolume(pwcVolumeName, MAX_PATH))) {
        do {
            hr = pMetadata->AddFilesToFileGroup(NULL, L"VolumeBitmap", pwcVolumeName, g_pwcBeforeFileName, false, NULL);
            if (FAILED(hr)) {
                wprintf(TEXT(__FUNCTION__ ": AddFilesToFileGroup failed. (0x%08lx)\r\n"), GetLastError());
                return false;
            }
        } while (FindNextVolume(hVolume, pwcVolumeName, MAX_PATH));
        FindVolumeClose(hVolume);
    }
#endif

    wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
}

// This function is called as a result of the requestor calling PrepareForBackup
// this indicates to the writer that a backup sequence is being initiated
bool STDMETHODCALLTYPE CReplibitWriter::OnPrepareBackup(_In_ IVssWriterComponents *) {
    wprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
}

// This function is called after a requestor calls DoSnapshotSet
// time-consuming actions related to Freeze can be performed here
bool STDMETHODCALLTYPE CReplibitWriter::OnPrepareSnapshot() {
    wprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
}

PVOLUME_BITMAP_BUFFER GetVolumeAllocationBitmap(HANDLE hVolume, LONGLONG llBitmapSize) {
    // wprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    STARTING_LCN_INPUT_BUFFER inBuffer = {0};
    DWORD dwOutBufferSize = (DWORD)(sizeof(VOLUME_BITMAP_BUFFER) + llBitmapSize);
    DWORD dwBytesReturned = 0;
    PVOLUME_BITMAP_BUFFER outBuffer = (PVOLUME_BITMAP_BUFFER)malloc(dwOutBufferSize);
    ZeroMemory(outBuffer, dwOutBufferSize);

    if (!DeviceIoControl(hVolume,                            // handle to volume
                         FSCTL_GET_VOLUME_BITMAP,            // dwIoControlCode
                         &inBuffer,                          // input buffer
                         sizeof(STARTING_LCN_INPUT_BUFFER),  // size of input buffer
                         outBuffer,                          // output buffer
                         dwOutBufferSize,                    // size of output buffer
                         &dwBytesReturned,                   // number of bytes returned
                         NULL) &&
        ERROR_MORE_DATA != GetLastError())  // OVERLAPPED structure
    {
        wprintf(TEXT(__FUNCTION__ L": DeviceIoControl failed for FSCTL_GET_VOLUME_BITMAP. (0x%08lx)\r\n"),
                GetLastError());
        free(outBuffer);
        // wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
        return NULL;
    }
    // wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return outBuffer;
}

CWriterVolume *CReplibitWriter::GatherVolumeInformation(LPCWSTR pwcVolume) {
    // wprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    HANDLE hVolume = INVALID_HANDLE_VALUE;
    WCHAR pwcVolumeName[MAX_PATH + 1] = {0};

    // get wstring length
    size_t zLength = wcslen(pwcVolume);

    // make local copy
    errno_t zErrno = memcpy_s(pwcVolumeName,             // local buffer
                              MAX_PATH * sizeof(WCHAR),  // buffer size
                              pwcVolume,                 // VSS buffer
                              zLength * sizeof(WCHAR));  // size

    // remove trailing backlash
    if (pwcVolumeName[zLength - 1] == L'\\') {
        pwcVolumeName[zLength - 1] = L'\0';
        zLength--;
    }

    // get volume handle
    hVolume =
        ::CreateFile(pwcVolumeName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);

    if (hVolume != INVALID_HANDLE_VALUE) {
        // gather volume info
        NTFS_VOLUME_DATA_BUFFER nvdbData;
        if (!GetNTFSvolumeData(hVolume, nvdbData)) {
            CloseHandle(hVolume);
            // wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
            return nullptr;
        }

        // calculate number of clusters based on sectors and size
        LONGLONG llNumberOfClusters =
            (nvdbData.NumberSectors.QuadPart * nvdbData.BytesPerSector) / nvdbData.BytesPerCluster;
        if (((nvdbData.NumberSectors.QuadPart * nvdbData.BytesPerSector) % nvdbData.BytesPerCluster) != 0) {
            llNumberOfClusters++;
        }

        // calculate bitmap size
        LONGLONG llBitmapSize = llNumberOfClusters / 8;
        if (llNumberOfClusters % 8 != 0) {
            llBitmapSize++;
        }

        // get bitmap
        PVOLUME_BITMAP_BUFFER pvbBuffer = GetVolumeAllocationBitmap(hVolume, llBitmapSize);
        if (!pvbBuffer) {
            CloseHandle(hVolume);
            // wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
            return nullptr;
        }

        return new CWriterVolume(zLength, pwcVolumeName, hVolume, &nvdbData, llNumberOfClusters, llBitmapSize,
                                 pvbBuffer);
    } else {
        wprintf(TEXT(__FUNCTION__ L": CreateFile failed. (0x%08lx)\r\n"), GetLastError());
    }
    // wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return nullptr;
}

bool CReplibitWriter::InitializeWriterVolumes() {
    // wprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    m_uVolumeCount = GetCurrentVolumeCount();
    m_ppwcVolumeArray = GetCurrentVolumeArray();
    for (UINT i = 0; i < m_uVolumeCount; i++) {
        wprintf(L"Volume found: %s \r\n", m_ppwcVolumeArray[i]);

        CWriterVolume *pWriterVolume = GatherVolumeInformation(m_ppwcVolumeArray[i]);
        if (pWriterVolume != nullptr) {
            m_volumeVector.push_back(pWriterVolume);
        }
    }
    // wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return m_uVolumeCount == m_volumeVector.size();
}

bool CReplibitWriter::CleanupWriterVolumes() {
    // wprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    for (auto it = m_volumeVector.begin(); it != m_volumeVector.end(); it++) {
        delete *it;
        *it = nullptr;
    }
    for (auto it = m_snapshotVector.begin(); it != m_snapshotVector.end(); it++) {
        delete *it;
        *it = nullptr;
    }
    m_uVolumeCount = 0;
    m_ppwcVolumeArray = NULL;
    m_volumeVector.clear();
    m_snapshotVector.clear();
    // wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
}

// This function is called after a requestor calls DoSnapshotSet
// here the writer is expected to freeze its store
bool STDMETHODCALLTYPE CReplibitWriter::OnFreeze() {
    wprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    InitializeWriterVolumes();
    wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
}

// This function is called after a requestor calls DoSnapshotSet
// here the writer is expected to thaw its store
bool STDMETHODCALLTYPE CReplibitWriter::OnThaw() {
    wprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
}

void GetFullPath(LPCWSTR pwcSnapshot, const wchar_t *pwcFileName, wchar_t *pwcFullPath) {
    // wprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    // copy snapshot name to full path
    size_t zSnapshotLength = wcsnlen_s(pwcSnapshot, MAX_PATH);
    wcsncpy_s(pwcFullPath, MAX_PATH, pwcSnapshot, zSnapshotLength);

    // append trailing backlash
    pwcFullPath[zSnapshotLength++] = L'\\';
    pwcFullPath[zSnapshotLength] = L'\0';

    // append file name
    size_t zFileNameLength = wcsnlen_s(pwcSnapshot, MAX_PATH);
    wcsncat_s(pwcFullPath, MAX_PATH, pwcFileName, zFileNameLength);
    // wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
}

HANDLE CreateSnapshotFile(DWORD dwDispotision, LPCWSTR pwcSnapshot, const wchar_t *pwcFileName) {
    // wprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    WCHAR pwcFullPath[MAX_PATH + 1] = {0};

    GetFullPath(pwcSnapshot, pwcFileName, pwcFullPath);
    HANDLE hFile = ::CreateFile(pwcFullPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                dwDispotision, NULL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        wprintf(TEXT(__FUNCTION__ ": CreateFile failed. (0x%08lx)\r\n"), GetLastError());
    }
    // wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return hFile;
}

bool MoveFilePointer(HANDLE hFile, LONG lPosition) {
    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, lPosition, NULL, FILE_BEGIN)) {
        wprintf(TEXT(__FUNCTION__ ": SetFilePointer failed. (0x%08lx)\r\n"), GetLastError());
        // wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
        return false;
    }
    return true;
}

bool TruncateFile(HANDLE hFile) {
    if (!SetEndOfFile(hFile)) {
        wprintf(TEXT(__FUNCTION__ ": SetEndOfFile failed. (0x%08lx)\r\n"), GetLastError());
        // wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
        return false;
    }
    return true;
}

bool SetSparseFlag(HANDLE hFile, BOOL bSetSparse) {
    // wprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    // wprintf(L"Setting sparse flag to: %s \n", bSetSparse ? L"true" : L"false");
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
        wprintf(L"DeviceIoControl failed for FSCTL_SET_SPARSE. (0x%08lx)\n", GetLastError());
        // wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
        return false;
    }
    // wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
}

void FileTests(LPCWSTR pwcSnapshot) {
    // wprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));

    // Test 1: regular set end of file
    HANDLE hFile = INVALID_HANDLE_VALUE;
    const wchar_t pwcRegular[] = L"1-regular.txt";
    const LONG lFileSize = 1024L * 1024L;
    if (INVALID_HANDLE_VALUE != (hFile = CreateSnapshotFile(CREATE_ALWAYS, pwcSnapshot, pwcRegular))) {
        MoveFilePointer(hFile, lFileSize);
        TruncateFile(hFile);
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
        wprintf(TEXT(__FUNCTION__ ": %s creation succeeded. \r\n"), pwcRegular);
    }

    // Test 2: sparse file
    const wchar_t pwcSparse[] = L"2-sparse.txt";
    if (INVALID_HANDLE_VALUE != (hFile = CreateSnapshotFile(CREATE_ALWAYS, pwcSnapshot, pwcSparse))) {
        MoveFilePointer(hFile, lFileSize);
        TruncateFile(hFile);
        SetSparseFlag(hFile, TRUE);
        MoveFilePointer(hFile, lFileSize / 2);
        TruncateFile(hFile);
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
        wprintf(TEXT(__FUNCTION__ ": %s creation succeeded. \r\n"), pwcSparse);
    }
    // wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
}

// This function is called after a requestor calls DoSnapshotSet
bool STDMETHODCALLTYPE CReplibitWriter::OnPostSnapshot(_In_ IVssWriterComponents *) {
    wprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    HRESULT hr;
    WCHAR pwcVolumeName[MAX_PATH + 1] = {0};
    WCHAR pwcSnapshotName[MAX_PATH + 1] = {0};

    for (auto it = m_volumeVector.begin(); it != m_volumeVector.end(); it++) {
        // re-add trailing backlash in local copy
        wcsncpy_s(pwcVolumeName, MAX_PATH, (*it)->m_pwcVolumeName, (*it)->m_zLength);

        size_t zLength = wcslen(pwcVolumeName);
        if (pwcVolumeName[zLength - 1] != L'\\') {
            pwcVolumeName[zLength++] = L'\\';
            pwcVolumeName[zLength] = L'\0';
        }

        LPCWSTR pwcSnapshotDevice = NULL;  // result pointer
        hr = GetSnapshotDeviceName(pwcVolumeName, &pwcSnapshotDevice);

        if (FAILED(hr)) {
            wprintf(TEXT(__FUNCTION__ ": GetSnapshotDeviceName failed. (0x%08lx)\r\n"), hr);
            continue;
        }

        wprintf(L"Snapshot found: %s \r\n", pwcSnapshotDevice);

        CWriterVolume *pWriterVolume = GatherVolumeInformation(pwcSnapshotDevice);
        if (pWriterVolume != nullptr) {
            m_snapshotVector.push_back(pWriterVolume);
        }

        FileTests(pwcSnapshotDevice);

#if 0
        HANDLE hBitmap = INVALID_HANDLE_VALUE;
        if (INVALID_HANDLE_VALUE !=
            (hBitmap = CreateSnapshotFile(CREATE_ALWAYS, pwcSnapshotDevice, g_pwcBeforeFileName))) {
            // find elements that are in the snapshot but were not there in the live volume
            DWORD dwBytesWritten = 0;
            if (!WriteFile(hBitmap, (*it)->m_pBitmap->Buffer, (DWORD)((*it)->m_llBitmapSize), &dwBytesWritten, NULL) ||
                dwBytesWritten != (DWORD)((*it)->m_llBitmapSize)) {
                wprintf(TEXT(__FUNCTION__ ": WriteFile failed. (0x%08lx)\r\n"), GetLastError());
            }
            CloseHandle(hBitmap);
            hBitmap = INVALID_HANDLE_VALUE;
        }
#endif
    }
    wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return m_snapshotVector.size() == m_volumeVector.size();
}

// This function is called to abort the writer's backup sequence.
// This should only be called between OnPrepareBackup and OnPostSnapshot
bool STDMETHODCALLTYPE CReplibitWriter::OnAbort() {
    wprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
}

// This function is called as a result of the requestor calling BackupComplete
bool STDMETHODCALLTYPE CReplibitWriter::OnBackupComplete(_In_ IVssWriterComponents *) {
    wprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
}

// This function is called at the end of the backup process.  This may happen as
// a result of the requestor shutting down, or it may happen as a result of
// abnormal termination of the requestor.
bool STDMETHODCALLTYPE CReplibitWriter::OnBackupShutdown(_In_ VSS_ID) {
    wprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    const size_t zMessageLength = 128;
    wchar_t pwcMessage[zMessageLength] = {'\0'};
    wchar_t pwcFullPath[MAX_PATH + 1] = {'\0'};

    // Recalculate and compare
    for (auto it = m_snapshotVector.begin(); it != m_snapshotVector.end(); it++) {
        // TODO: consider using handle obtained before
        // open snapshot again
        HANDLE hSnapshot = INVALID_HANDLE_VALUE;
        PVOLUME_BITMAP_BUFFER pvbBuffer = NULL;
        if (INVALID_HANDLE_VALUE !=
            (hSnapshot = ::CreateFile((*it)->m_pwcVolumeName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                      OPEN_EXISTING, NULL, NULL))) {
            // assumes volume has not changed size
            // get allocation bitmap again (now with writer-added data)
            pvbBuffer = GetVolumeAllocationBitmap(hSnapshot, (*it)->m_llBitmapSize);

            size_t zIndex = it - m_snapshotVector.begin();
            BYTE *pBitmap = m_volumeVector[zIndex]->m_pBitmap->Buffer;
            HANDLE hDifferences = INVALID_HANDLE_VALUE;
            GetFullPath(m_volumeVector[zIndex]->m_pwcVolumeName, g_pwcDifferencesFileName, pwcFullPath);
            if (INVALID_HANDLE_VALUE !=
                (hDifferences = ::CreateFile(pwcFullPath, GENERIC_READ | GENERIC_WRITE,
                                             FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, NULL, NULL))) {
                LONGLONG llIteratorEnd = (*it)->m_llBitmapSize;
                BYTE *pSnapshot = pvbBuffer->Buffer;
                BYTE uResult;
                DWORD dwBytesWritten = 0;
                size_t zBytesToWrite = 0;
                for (LONGLONG i = 0; i < llIteratorEnd; i++) {
                    // if it is in snapshot but not in volume before snapshot
                    uResult = *(pSnapshot + i) & ~(*(pBitmap + i));
                    if (uResult) {
                        zBytesToWrite = swprintf_s(pwcMessage, zMessageLength, L"%-8lld 0x%02x\r\n", i * 8, uResult) *
                                        sizeof(WCHAR);
                        if (!WriteFile(hDifferences, pwcMessage, (DWORD)zBytesToWrite, &dwBytesWritten, NULL) ||
                            dwBytesWritten != (DWORD)zBytesToWrite) {
                            wprintf(TEXT(__FUNCTION__ ": WriteFile failed. (0x%08lx)\r\n"), GetLastError());
                            break;
                        }
                    }
                }
                CloseHandle(hDifferences);
                hDifferences = INVALID_HANDLE_VALUE;
                wprintf(TEXT(__FUNCTION__ ": %s written.\r\n"), pwcFullPath);
            } else {
                wprintf(TEXT(__FUNCTION__ ": CreateFile failed for %s. (0x%08lx)\r\n"), pwcFullPath, GetLastError());
            }
        }
        CloseHandle(hSnapshot);
        hSnapshot = INVALID_HANDLE_VALUE;
        free(pvbBuffer);
        pvbBuffer = NULL;
    }
    CleanupWriterVolumes();
    wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
}

// This function is called as a result of the requestor calling PreRestore
// This will be called immediately before files are restored
bool STDMETHODCALLTYPE CReplibitWriter::OnPreRestore(_In_ IVssWriterComponents *) {
    wprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
}

// This function is called as a result of the requestor calling PreRestore
// This will be called immediately after files are restored
bool STDMETHODCALLTYPE CReplibitWriter::OnPostRestore(_In_ IVssWriterComponents *) {
    wprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
}
