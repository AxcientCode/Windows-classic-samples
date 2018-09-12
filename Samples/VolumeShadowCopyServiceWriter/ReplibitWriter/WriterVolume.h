
#ifndef _WRITER_VOLUME_H
#define _WRITER_VOLUME_H

#include <windows.h>
#include <ChunkBitmap.h>

class CWriterVolume {
   public:
    CWriterVolume();
    CWriterVolume(std::wstring wsVolumeGuid, HANDLE hVolume, NTFS_VOLUME_DATA_BUFFER* pnvdbData,
                  LONGLONG llClusterCount, LONGLONG llBitmapSize, CChunkBitmap* pBitmap);

    ~CWriterVolume();

   private:
    static const size_t m_zMaxVolumeNameLength = MAX_PATH + 1;

   public:
    std::wstring m_wsVolumeName;
    HANDLE m_hVolume;
    NTFS_VOLUME_DATA_BUFFER m_nvdbData;
    LONGLONG m_llClusterCount;
    LONGLONG m_llBitmapSize;
    CChunkBitmap* m_pBitmap;
};

#endif
