#pragma once

#ifndef _UTILITIES_H
#define _UTILITIES_H

#include <string>
#include <ChunkBitmap.h>

bool SetSparseFlag(HANDLE hFile, BOOL bSetSparse);
bool TruncateFile(HANDLE hFile);
bool MoveFilePointer(HANDLE hFile, LONG lPosition);
HANDLE CreateFileInDevice(DWORD dwDispotision, const std::wstring &wsDevice, const wchar_t *pwcFileName);
void GetFullPath(_In_ const std::wstring &wsDevice, _In_ const wchar_t *pwcFileName, _Out_ std::wstring &pwcFullPath);
bool GetAllocationBitmap(HANDLE hVolume, CChunkBitmap *pBitmap, CHUNK_BITMAP_SET_STATE iBitmapSetState);
bool GetNTFSvolumeData(HANDLE m_hVolume, NTFS_VOLUME_DATA_BUFFER &m_nvdbData);

#endif
