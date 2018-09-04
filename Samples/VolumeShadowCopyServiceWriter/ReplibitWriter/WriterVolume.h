
#ifndef _WRITER_VOLUME_H
#define _WRITER_VOLUME_H

#include <windows.h>

class CWriterVolume {
   public:
    CWriterVolume();
    CWriterVolume(size_t zLength, WCHAR* pwcVolumeName, HANDLE hVolume, NTFS_VOLUME_DATA_BUFFER* pnvdbData,
                  LONGLONG llNumberOfRecords, LONGLONG llBitmapSize, PVOLUME_BITMAP_BUFFER pBitmap);

    ~CWriterVolume();

   private:
    static const size_t m_zMaxVolumeNameLength = MAX_PATH + 1;

   public:
    size_t m_zLength;
    WCHAR m_pwcVolumeName[m_zMaxVolumeNameLength];
    HANDLE m_hVolume;
    NTFS_VOLUME_DATA_BUFFER m_nvdbData;
    LONGLONG m_llNumberOfRecords;
    LONGLONG m_llBitmapSize;
    PVOLUME_BITMAP_BUFFER m_pBitmap;
};

#endif
