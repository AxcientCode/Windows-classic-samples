#include "stdafx.h"
#include "WriterVolume.h"

CWriterVolume::CWriterVolume()
    : m_zLength(0), m_hVolume(INVALID_HANDLE_VALUE), m_llNumberOfRecords(0), m_llBitmapSize(0), m_pBitmap(NULL) {
    ZeroMemory(m_pwcVolumeName, m_zMaxVolumeNameLength);
    ZeroMemory(&m_nvdbData, sizeof(NTFS_VOLUME_DATA_BUFFER));
}

CWriterVolume::CWriterVolume(size_t zLength, WCHAR* pwcVolumeName, HANDLE hVolume, NTFS_VOLUME_DATA_BUFFER* pnvdbData,
                             LONGLONG llNumberOfRecords, LONGLONG llBitmapSize, PVOLUME_BITMAP_BUFFER pBitmap)
    : m_zLength(zLength),
      m_hVolume(hVolume),
      m_llNumberOfRecords(llNumberOfRecords),
      m_llBitmapSize(llBitmapSize),
      m_pBitmap(pBitmap) {
    memcpy(m_pwcVolumeName, pwcVolumeName, zLength);
    memcpy(&m_nvdbData, pnvdbData, sizeof(NTFS_VOLUME_DATA_BUFFER));
}

CWriterVolume::~CWriterVolume() {
    if (m_hVolume != INVALID_HANDLE_VALUE) {
        CloseHandle(INVALID_HANDLE_VALUE);
        m_hVolume = INVALID_HANDLE_VALUE;
    }

    if (m_pBitmap) {
        free(m_pBitmap);
        m_pBitmap = NULL;
    }
}
