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
    hr = Subscribe();
    if (FAILED(hr)) wprintf(TEXT(__FUNCTION__ ": CVssWriter::Subscribe failed!. (0x%08lx)\r\n"), GetLastError());

    wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return hr;
}

// OnIdentify is called as a result of the requestor calling
// GatherWriterMetadata
bool STDMETHODCALLTYPE CReplibitWriter::OnIdentify(IN IVssCreateWriterMetadata *pMetadata) {
    wprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));

    // Set the restore method to restore if can replace
    HRESULT hr = pMetadata->SetRestoreMethod(VSS_RME_RESTORE_IF_CAN_REPLACE, NULL, NULL, VSS_WRE_ALWAYS, false);
    if (FAILED(hr)) {
        wprintf(TEXT(__FUNCTION__ ": SetRestoreMethod failed. (0x%08lx)\r\n"), GetLastError());
        return false;
    }

    // TODO: add component for every NTFS volume

    // add simple FileGroup component
    hr = pMetadata->AddComponent(VSS_CT_FILEGROUP, NULL, L"FilesComponent", L"My FileGroup", NULL, 0, false, true, true,
                                 true);
    if (FAILED(hr)) {
        wprintf(TEXT(__FUNCTION__ ": AddComponent failed. (0x%08lx)\r\n"), GetLastError());
        return false;
    }

    // add some sample files to the group
    hr = pMetadata->AddFilesToFileGroup(NULL, L"FilesComponent", L"D:\\MyFiles", L"*.dat", false, NULL);
    if (FAILED(hr)) {
        wprintf(TEXT(__FUNCTION__ ": AddFilesToFileGroup failed. (0x%08lx)\r\n"), GetLastError());
        return false;
    }

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

bool CReplibitWriter::InitializeWriterVolumes() {
    wprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));

    HANDLE hVolume = INVALID_HANDLE_VALUE;
    WCHAR pwcVolumeName[MAX_PATH + 1] = {0};

    m_uVolumeCount = GetCurrentVolumeCount();
    m_ppwcVolumeArray = GetCurrentVolumeArray();
    for (UINT i = 0; i < m_uVolumeCount; i++) {
        // for some reason //wprintf prints chars and not wchars
        wprintf(L"Volume found: %s \r\n", m_ppwcVolumeArray[i]);

        // get wstring length
        size_t zLength = wcslen(m_ppwcVolumeArray[i]);

        // make local copy
        errno_t zErrno = memcpy_s(pwcVolumeName,             // local buffer
                                  MAX_PATH * sizeof(WCHAR),  // buffer size
                                  m_ppwcVolumeArray[i],      // VSS buffer
                                  zLength * sizeof(WCHAR));  // size

        // remove trailing backlash
        if (pwcVolumeName[zLength - 1] == L'\\') {
            pwcVolumeName[zLength - 1] = L'\0';
        }

        // get volume handle
        hVolume = ::CreateFile(pwcVolumeName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                               NULL, NULL);

        NTFS_VOLUME_DATA_BUFFER nvdbData;

        if (!GetNTFSvolumeData(hVolume, nvdbData)) {
            CloseHandle(hVolume);
            continue;
        }

        LONGLONG llNumberOfClusters =
            (nvdbData.NumberSectors.QuadPart * nvdbData.BytesPerSector) / nvdbData.BytesPerCluster;
        if (((nvdbData.NumberSectors.QuadPart * nvdbData.BytesPerSector) % nvdbData.BytesPerCluster) != 0) {
            llNumberOfClusters++;
        }

        LONGLONG llBitmapSize = llNumberOfClusters / 8;
        if (llNumberOfClusters % 8 != 0) {
            llBitmapSize++;
        }

        if (hVolume != INVALID_HANDLE_VALUE) {
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
                CloseHandle(hVolume);
                continue;
            }

            m_volumesVector.push_back(new CWriterVolume(zLength, pwcVolumeName, hVolume, &nvdbData, llNumberOfClusters,
                                                        llBitmapSize, outBuffer));

        } else {
            wprintf(TEXT(__FUNCTION__ L": CreateFile failed. (0x%08lx)\r\n"), GetLastError());
        }
    }

    wprintf(TEXT(__FUNCTION__ ": End. \r\n"));

    return m_uVolumeCount == m_volumesVector.size();
}

bool CReplibitWriter::CleanupWriterVolumes() {
    wprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    for (auto it = m_volumesVector.begin(); it != m_volumesVector.end(); it++) {
        delete *it;
        *it = nullptr;
    }
    m_uVolumeCount = 0;
    m_ppwcVolumeArray = NULL;
    m_volumesVector.clear();
    wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
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

// This function is called after a requestor calls DoSnapshotSet
bool STDMETHODCALLTYPE CReplibitWriter::OnPostSnapshot(_In_ IVssWriterComponents *) {
    wprintf(TEXT(__FUNCTION__ ": Begin. \r\n"));
    HRESULT hr;
    for (UINT i = 0; i < m_uVolumeCount; i++) {
        LPCWSTR *ppwcSnapshotDevice = NULL;
        hr = GetSnapshotDeviceName(m_volumesVector[i]->m_pwcVolumeName, ppwcSnapshotDevice);

        if (FAILED(hr)) {
            wprintf(TEXT(__FUNCTION__ ": GetSnapshotDeviceName failed. (0x%08lx)\r\n"), hr);
            continue;
        }

        WCHAR pwcVolumeName[MAX_PATH + 1] = {0};

        // get wstring length
        size_t zLength = wcslen(*ppwcSnapshotDevice);

        // make local copy
        errno_t zErrno = memcpy_s(pwcVolumeName,             // local buffer
                                  MAX_PATH * sizeof(WCHAR),  // buffer size
                                  *ppwcSnapshotDevice,       // VSS buffer
                                  zLength * sizeof(WCHAR));  // size
        VSS_E_OBJECT_NOT_FOUND;
        wprintf(L"Snapshot found: %s \r\n", *ppwcSnapshotDevice);
    }
    wprintf(TEXT(__FUNCTION__ ": End. \r\n"));
    return true;
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
