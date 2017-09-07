#pragma once
class CResutilsShim
{
public:
	CResutilsShim();
	~CResutilsShim();

public:
	static BOOL ClusterIsPathOnSharedVolumeImpl(
		LPCWSTR lpszPathName);

	static DWORD ClusterPrepareSharedVolumeForBackupImpl(
		LPCWSTR lpszFileName,
		LPWSTR lpszVolumePathName,
		LPDWORD lpcchVolumePathName,
		LPWSTR lpszVolumeName,
		LPDWORD lpcchVolumeName
	);

	static BOOL ClusterGetVolumePathNameImpl(
		LPCWSTR lpszFileName,
		LPWSTR lpszVolumePathName,
		DWORD cchBufferLength
	);

	static BOOL ClusterGetVolumeNameForVolumeMountPointImpl(
		LPCWSTR lpszVolumeMountPoint,
		LPWSTR lpszVolumeName,
		DWORD cchBufferLength
	);

protected:
	static HMODULE m_hModule;
	static FARPROC LookupProc(LPCSTR procName);

};

#define ClusterIsPathOnSharedVolume CResutilsShim::ClusterIsPathOnSharedVolumeImpl
#define ClusterPrepareSharedVolumeForBackup CResutilsShim::ClusterPrepareSharedVolumeForBackupImpl
#define ClusterGetVolumePathName CResutilsShim::ClusterGetVolumePathNameImpl
#define ClusterGetVolumeNameForVolumeMountPoint CResutilsShim::ClusterGetVolumeNameForVolumeMountPointImpl