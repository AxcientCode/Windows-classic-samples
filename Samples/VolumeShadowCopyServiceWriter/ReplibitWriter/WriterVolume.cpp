#include "stdafx.h"
#include "WriterVolume.h"

CWriterVolume::CWriterVolume()
    : m_hVolume(INVALID_HANDLE_VALUE), m_llClusterCount(0), m_llBitmapSize(0), m_pBitmap(nullptr) {
    ZeroMemory(&m_nvdbData, sizeof(NTFS_VOLUME_DATA_BUFFER));
}

CWriterVolume::CWriterVolume(std::wstring wsVolumeName, HANDLE hVolume, NTFS_VOLUME_DATA_BUFFER* pnvdbData,
                             LONGLONG llClusterCount, LONGLONG llBitmapSize, CChunkBitmap* pBitmap)
    : m_wsVolumeName(wsVolumeName),
      m_hVolume(hVolume),
      m_llClusterCount(llClusterCount),
      m_llBitmapSize(llBitmapSize),
      m_pBitmap(pBitmap) {
    memcpy(&m_nvdbData, pnvdbData, sizeof(NTFS_VOLUME_DATA_BUFFER));
}

CWriterVolume::~CWriterVolume() {
    if (m_hVolume != INVALID_HANDLE_VALUE) {
        CloseHandle(INVALID_HANDLE_VALUE);
        m_hVolume = INVALID_HANDLE_VALUE;
    }

    if (m_pBitmap) {
        delete m_pBitmap;
        m_pBitmap = nullptr;
    }
}
