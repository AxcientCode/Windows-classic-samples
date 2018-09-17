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
#include <fstream>

///////////////////////////////////////////////////////////////////////////////

static const wchar_t g_pwcBeforeFileName[] = L"VolumeBitmapBefore.bin";
static const wchar_t g_pwcDifferencesFileName[] = L"DifferencesFound.bin";

// Initialize the writer
HRESULT STDMETHODCALLTYPE CReplibitWriter::Initialize() {
    _tprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    HRESULT hr = CVssWriter::Initialize(guidReplibitWriterId,  // WriterID
                                        pwcWriterName,         // wszWriterName
                                        VSS_UT_USERDATA,       // ut
                                        VSS_ST_OTHER);         // st
    if (FAILED(hr)) {
        _tprintf(TEXT(__FUNCTION__ ": CVssWriter::Initialize failed!. (0x%08lx)\r\n"), GetLastError());
        return hr;
    }

    // subscribe for events
    hr = CVssWriter::Subscribe();
    if (FAILED(hr)) {
        _tprintf(TEXT(__FUNCTION__ ": CVssWriter::Subscribe failed!. (0x%08lx)\r\n"), GetLastError());
    }

    _tprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return hr;
}

// OnIdentify is called as a result of the requestor calling
// GatherWriterMetadata
bool STDMETHODCALLTYPE CReplibitWriter::OnIdentify(IN IVssCreateWriterMetadata *pMetadata) {
    _tprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));

    // Set the restore method to restore if can replace
    HRESULT hr = pMetadata->SetRestoreMethod(VSS_RME_RESTORE_IF_CAN_REPLACE, NULL, NULL, VSS_WRE_NEVER, false);
    if (FAILED(hr)) {
        _tprintf(TEXT(__FUNCTION__ ": SetRestoreMethod failed. (0x%08lx)\r\n"), GetLastError());
        return false;
    }

    hr = pMetadata->AddComponent(VSS_CT_FILEGROUP, NULL, L"VolumeBitmap", L"Volume bitmaps", NULL, 0, false, true, true,
                                 true);
    if (FAILED(hr)) {
        _tprintf(TEXT(__FUNCTION__ ": AddComponent failed. (0x%08lx)\r\n"), GetLastError());
        return false;
    }

#if 0
    WCHAR pwcVolumeName[MAX_PATH + 1] = { L'\0' };
    HANDLE hVolume = INVALID_HANDLE_VALUE;
    if (INVALID_HANDLE_VALUE != (hVolume = FindFirstVolume(pwcVolumeName, MAX_PATH))) {
        do {
            hr = pMetadata->AddFilesToFileGroup(NULL, L"VolumeBitmap", pwcVolumeName, g_pwcBeforeFileName, false, NULL);
            if (FAILED(hr)) {
                _tprintf(TEXT(__FUNCTION__ ": AddFilesToFileGroup failed. (0x%08lx)\r\n"), GetLastError());
                return false;
            }
        } while (FindNextVolume(hVolume, pwcVolumeName, MAX_PATH));
        FindVolumeClose(hVolume);
    }
#endif

    _tprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
}

// This function is called as a result of the requestor calling PrepareForBackup
// this indicates to the writer that a backup sequence is being initiated
bool STDMETHODCALLTYPE CReplibitWriter::OnPrepareBackup(_In_ IVssWriterComponents *) {
    _tprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    _tprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
}

// This function is called after a requestor calls DoSnapshotSet
// time-consuming actions related to Freeze can be performed here
bool STDMETHODCALLTYPE CReplibitWriter::OnPrepareSnapshot() {
    _tprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    _tprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
}

CWriterVolume *CReplibitWriter::GatherVolumeInformation(LPCWSTR pwcVolume) {
    // _tprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    HANDLE hVolume = INVALID_HANDLE_VALUE;
    std::wstring wsVolumeName(pwcVolume);

    // remove trailing backlash
    if (wsVolumeName[wsVolumeName.size() - 1] == L'\\') {
        wsVolumeName.pop_back();
    }

    // get volume handle
    hVolume = CreateFile(wsVolumeName.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                         NULL, NULL);

    if (hVolume != INVALID_HANDLE_VALUE) {
        // gather volume info
        NTFS_VOLUME_DATA_BUFFER nvdbData;
        if (!GetNTFSvolumeData(hVolume, nvdbData)) {
            CloseHandle(hVolume);
            // _tprintf(TEXT(__FUNCTION__ ": End. \r\n"));
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
        CChunkBitmap *pBitmap = new CChunkBitmap(llBitmapSize, 0);
        if (!GetAllocationBitmap(hVolume, pBitmap, CBSS_ALL_ZEROS)) {
            CloseHandle(hVolume);
            // _tprintf(TEXT(__FUNCTION__ ": End. \r\n"));
            delete pBitmap;
            return nullptr;
        }

        return new CWriterVolume(wsVolumeName, hVolume, &nvdbData, llNumberOfClusters, llBitmapSize, pBitmap);
    } else {
        _tprintf(TEXT(__FUNCTION__ L": CreateFile failed. (0x%08lx)\r\n"), GetLastError());
    }
    // _tprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return nullptr;
}

bool CReplibitWriter::InitializeWriterVolumes() {
    // _tprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    m_uVolumeCount = CVssWriter::GetCurrentVolumeCount();
    m_ppwcVolumeArray = CVssWriter::GetCurrentVolumeArray();
    for (UINT i = 0; i < m_uVolumeCount; i++) {
        _tprintf(L"Volume found: %s \r\n", m_ppwcVolumeArray[i]);

        CWriterVolume *pWriterVolume = GatherVolumeInformation(m_ppwcVolumeArray[i]);
        if (pWriterVolume != nullptr) {
            m_volumeVector.push_back(pWriterVolume);
        }
    }
    // _tprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return m_uVolumeCount == m_volumeVector.size();
}

bool CReplibitWriter::CleanupWriterVolumes() {
    // _tprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
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
    // _tprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
}

// This function is called after a requestor calls DoSnapshotSet
// here the writer is expected to freeze its store
bool STDMETHODCALLTYPE CReplibitWriter::OnFreeze() {
    _tprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    InitializeWriterVolumes();
    _tprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
}

// This function is called after a requestor calls DoSnapshotSet
// here the writer is expected to thaw its store
bool STDMETHODCALLTYPE CReplibitWriter::OnThaw() {
    _tprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    _tprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
}

// Test files that go inside the snapshot
void CreateFileInDeviceTests(const std::wstring &wsDevice) {
    _tprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    // Test 1: regular set end of file
    HANDLE hFile = INVALID_HANDLE_VALUE;
    const wchar_t pwcRegular[] = L"1-regular.txt";
    const LONG lFileSize = 10L * 1024L * 1024L;
    if (INVALID_HANDLE_VALUE != (hFile = CreateFileInDevice(CREATE_ALWAYS, wsDevice, pwcRegular))) {
        MoveFilePointer(hFile, lFileSize);
        TruncateFile(hFile);
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
        _tprintf(TEXT(__FUNCTION__ ": %s creation succeeded. \r\n"), pwcRegular);
    }

    // Test 2: sparse file
    const wchar_t pwcSparse[] = L"2-sparse.txt";
    if (INVALID_HANDLE_VALUE != (hFile = CreateFileInDevice(CREATE_ALWAYS, wsDevice, pwcSparse))) {
        MoveFilePointer(hFile, lFileSize);
        TruncateFile(hFile);
        SetSparseFlag(hFile, TRUE);
        MoveFilePointer(hFile, lFileSize / 2);
        TruncateFile(hFile);
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
        _tprintf(TEXT(__FUNCTION__ ": %s creation succeeded. \r\n"), pwcSparse);
    }
    _tprintf(TEXT(__FUNCTION__ ": End. \r\n"));
}

// This function is called after a requestor calls DoSnapshotSet
bool STDMETHODCALLTYPE CReplibitWriter::OnPostSnapshot(_In_ IVssWriterComponents *) {
    _tprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    HRESULT hr;

    for (auto it = m_volumeVector.begin(); it != m_volumeVector.end(); it++) {
        // re-add trailing backlash in local copy
        std::wstring wsVolumeName((*it)->m_wsVolumeName);

        if (wsVolumeName[wsVolumeName.size() - 1] != L'\\') {
            wsVolumeName.push_back(L'\\');
        }

        LPCWSTR pwcSnapshotDevice = NULL;  // result pointer
        hr = GetSnapshotDeviceName(wsVolumeName.c_str(), &pwcSnapshotDevice);

        if (FAILED(hr)) {
            _tprintf(TEXT(__FUNCTION__ ": GetSnapshotDeviceName failed. (0x%08lx)\r\n"), hr);
            continue;
        }

        _tprintf(L"Snapshot found: %s \r\n", pwcSnapshotDevice);

        CWriterVolume *pWriterVolume = GatherVolumeInformation(pwcSnapshotDevice);
        if (pWriterVolume != nullptr) {
            m_snapshotVector.push_back(pWriterVolume);
        }

        CreateFileInDeviceTests(pwcSnapshotDevice);

        // store bitmap as seen from live volume before snapshot was taken inside snapshot
        std::wstring wsFullPath;
        GetFullPath(pwcSnapshotDevice, g_pwcBeforeFileName, wsFullPath);
        if (!(*it)->m_pBitmap->BitmapSave(wsFullPath)) {
            _tprintf(TEXT(__FUNCTION__ ": BitmapSave failed. (0x%08lx)\r\n"), hr);
        } else {
            _tprintf(TEXT(__FUNCTION__ ": %s creation succeeded. \r\n"), wsFullPath.c_str());
        }
    }
    _tprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return m_snapshotVector.size() == m_volumeVector.size();
}

// This function is called to abort the writer's backup sequence.
// This should only be called between OnPrepareBackup and OnPostSnapshot
bool STDMETHODCALLTYPE CReplibitWriter::OnAbort() {
    _tprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    _tprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
}

// This function is called as a result of the requestor calling BackupComplete
bool STDMETHODCALLTYPE CReplibitWriter::OnBackupComplete(_In_ IVssWriterComponents *) {
    _tprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    _tprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
}

// This function is called at the end of the backup process.  This may happen as
// a result of the requestor shutting down, or it may happen as a result of
// abnormal termination of the requestor.
bool STDMETHODCALLTYPE CReplibitWriter::OnBackupShutdown(_In_ VSS_ID) {
    _tprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    const size_t zMessageLength = 128;
    char pcMessage[zMessageLength] = {'\0'};

    // Recalculate and compare
    for (auto it = m_snapshotVector.begin(); it != m_snapshotVector.end(); it++) {
        // TODO: consider using handle obtained before
        // open snapshot again
        HANDLE hSnapshot = INVALID_HANDLE_VALUE;
        if (INVALID_HANDLE_VALUE !=
            (hSnapshot = CreateFile((*it)->m_wsVolumeName.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL, OPEN_EXISTING, NULL, NULL))) {
            // assumes volume has not changed size
            // get allocation bitmap again (now with writer-added data)
            CChunkBitmap *pBitmapAfter = new CChunkBitmap((*it)->m_llBitmapSize, 0);
            if (GetAllocationBitmap(hSnapshot, pBitmapAfter, CBSS_ALL_ZEROS)) {
                size_t zIndex = it - m_snapshotVector.begin();
                CChunkBitmap *pBitmapBefore = m_volumeVector[zIndex]->m_pBitmap;
                std::wstring wsFullPath;
                GetFullPath(m_volumeVector[zIndex]->m_wsVolumeName, g_pwcDifferencesFileName, wsFullPath);
                *pBitmapAfter -= *pBitmapBefore;
                if (pBitmapAfter->BitmapSave(wsFullPath)) {
                    _tprintf(TEXT(__FUNCTION__ ": %s written.\r\n"), wsFullPath.c_str());
                } else {
                    strerror_s(pcMessage, zMessageLength, errno);
                    _tprintf(TEXT(__FUNCTION__ ": Could not create file %s. (%S)\r\n"), wsFullPath.c_str(), pcMessage);
                }
            } else {
                _tprintf(TEXT(__FUNCTION__ ": GetAllocationBitmap failed for %s. (0x%08lx)\r\n"),
                        (*it)->m_wsVolumeName.c_str(), GetLastError());
            }
            delete pBitmapAfter;
            CloseHandle(hSnapshot);
        } else {
            _tprintf(TEXT(__FUNCTION__ ": CreateFile failed for %s. (0x%08lx)\r\n"), (*it)->m_wsVolumeName.c_str(),
                    GetLastError());
        }
    }
    CleanupWriterVolumes();
    _tprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
}

// This function is called as a result of the requestor calling PreRestore
// This will be called immediately before files are restored
bool STDMETHODCALLTYPE CReplibitWriter::OnPreRestore(_In_ IVssWriterComponents *) {
    _tprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    _tprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
}

// This function is called as a result of the requestor calling PreRestore
// This will be called immediately after files are restored
bool STDMETHODCALLTYPE CReplibitWriter::OnPostRestore(_In_ IVssWriterComponents *) {
    _tprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    _tprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
}
