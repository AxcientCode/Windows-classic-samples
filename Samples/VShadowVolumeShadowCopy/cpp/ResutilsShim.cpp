// Main header
#include "stdafx.h"

#define _CRESUTILSSHIM_IGNORE_ALIAS
#include "ResutilsShim.h"

// undefine our aliases before include with the real definitions
#undef ClusterIsPathOnSharedVolume
#undef ClusterPrepareSharedVolumeForBackup
#undef ClusterGetVolumePathName
#undef ClusterGetVolumeNameForVolumeMountPoint


#include <Resapi.h>


HMODULE CResutilsShim::m_hModule = NULL;
static PCLUSTER_IS_PATH_ON_SHARED_VOLUME lpfClusterIsPathOnSharedVolume = NULL;
static PCLUSTER_PREPARE_SHARED_VOLUME_FOR_BACKUP lpfClusterPrepareSharedVolumeForBackup = NULL;
static PCLUSTER_GET_VOLUME_PATH_NAME lpfClusterGetVolumePathName = NULL;
static PCLUSTER_GET_VOLUME_NAME_FOR_VOLUME_MOUNT_POINT lpfClusterGetVolumeNameForVolumeMountPoint = NULL;

CResutilsShim::CResutilsShim()
{
}

CResutilsShim::~CResutilsShim()
{
}

FARPROC CResutilsShim::LookupProc(LPCSTR procName)
{
	if (m_hModule == NULL)
		m_hModule = LoadLibraryW(L"resutils.dll");

	if (m_hModule != NULL)
	{
		FARPROC proc = GetProcAddress(m_hModule, procName);
		if (proc != NULL)
			return proc;
	}

	return (FARPROC)-1;
}

BOOL CResutilsShim::ClusterIsPathOnSharedVolumeImpl(LPCWSTR lpszPathName)
{
	// not set
	if (lpfClusterIsPathOnSharedVolume == NULL)
		lpfClusterIsPathOnSharedVolume = (PCLUSTER_IS_PATH_ON_SHARED_VOLUME)LookupProc("ClusterIsPathOnSharedVolume");

	// not present
	if (lpfClusterIsPathOnSharedVolume == (PCLUSTER_IS_PATH_ON_SHARED_VOLUME)-1)
		return FALSE;

	return (*lpfClusterIsPathOnSharedVolume)(lpszPathName);
}

DWORD CResutilsShim::ClusterPrepareSharedVolumeForBackupImpl(LPCWSTR lpszFileName, LPWSTR lpszVolumePathName, LPDWORD lpcchVolumePathName, LPWSTR lpszVolumeName, LPDWORD lpcchVolumeName)
{
	// not set
	if (lpfClusterPrepareSharedVolumeForBackup == NULL)
		lpfClusterPrepareSharedVolumeForBackup = (PCLUSTER_PREPARE_SHARED_VOLUME_FOR_BACKUP)LookupProc("ClusterPrepareSharedVolumeForBackup");

	// not present
	if (lpfClusterPrepareSharedVolumeForBackup == (PCLUSTER_PREPARE_SHARED_VOLUME_FOR_BACKUP)-1)
		return ERROR_INVALID_FUNCTION;

	return (*lpfClusterPrepareSharedVolumeForBackup)(lpszFileName, lpszVolumePathName, lpcchVolumePathName, lpszVolumeName, lpcchVolumeName);
}

BOOL CResutilsShim::ClusterGetVolumePathNameImpl(LPCWSTR lpszFileName, LPWSTR lpszVolumePathName, DWORD cchBufferLength)
{
	// not set
	if (lpfClusterGetVolumePathName == NULL)
		lpfClusterGetVolumePathName = (PCLUSTER_GET_VOLUME_PATH_NAME)LookupProc("ClusterGetVolumePathName");

	// not present
	if (lpfClusterGetVolumePathName == (PCLUSTER_GET_VOLUME_PATH_NAME)-1)
		return FALSE;

	return (*lpfClusterGetVolumePathName)(lpszFileName, lpszVolumePathName, cchBufferLength);
}

BOOL CResutilsShim::ClusterGetVolumeNameForVolumeMountPointImpl(LPCWSTR lpszVolumeMountPoint, LPWSTR lpszVolumeName, DWORD cchBufferLength)
{
	// not set
	if (lpfClusterGetVolumeNameForVolumeMountPoint == NULL)
		lpfClusterGetVolumeNameForVolumeMountPoint = (PCLUSTER_GET_VOLUME_NAME_FOR_VOLUME_MOUNT_POINT)LookupProc("ClusterGetVolumeNameForVolumeMountPoint");

	// not present
	if (lpfClusterGetVolumeNameForVolumeMountPoint == (PCLUSTER_GET_VOLUME_NAME_FOR_VOLUME_MOUNT_POINT)-1)
		return FALSE;

	return (*lpfClusterGetVolumeNameForVolumeMountPoint)(lpszVolumeMountPoint, lpszVolumeName, cchBufferLength);
}