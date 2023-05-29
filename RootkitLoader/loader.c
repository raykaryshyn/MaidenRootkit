#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>

const wchar_t* serviceName = L"MaidenRootkit";
const wchar_t* driverName = L"RootkitDriver";

bool load(SC_HANDLE sh, const wchar_t* serviceName, const wchar_t* driverName) {
	wchar_t currentDir[MAX_PATH];
	wchar_t filePath[MAX_PATH];

	if (GetCurrentDirectoryW(MAX_PATH, currentDir) > 0) {
		swprintf(filePath, MAX_PATH, L"%s\\%s.sys", currentDir, driverName);
		wprintf(L"Driver file path: %s\n", filePath);
	}
	else {
		printf("Failed to get the current directory\n");
		return false;
	}

	printf("Loading driver as a service...\n");

	SC_HANDLE rh = CreateService(
		sh,
		serviceName,
		serviceName,
		SERVICE_ALL_ACCESS,
		SERVICE_KERNEL_DRIVER,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		filePath,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL);

	if (!rh) {
		if (GetLastError() == ERROR_SERVICE_EXISTS) {
			printf("Service already exists.\n");
			return true;
		}
		else {
			printf("CreateService failed. Error = %lu\n", GetLastError());
			return false;
		}
	}

	printf("Driver loaded.\n");
	return true;
}

bool start(SC_HANDLE sh, const wchar_t* driverName) {
	SC_HANDLE rh;

	rh = OpenService(
		sh,
		driverName,
		SERVICE_ALL_ACCESS
	);

	if (rh) {
		if (0 == StartService(rh, 0, NULL)) {
			if (ERROR_SERVICE_ALREADY_RUNNING == GetLastError()) {
				printf("Service already running.\n");
			}
			else {
				printf("StartService failed. Error = %lu\n", GetLastError());
				CloseServiceHandle(rh);
				return false;
			}
		}
		else {
			printf("Service started successfully.\n");
		}

		CloseServiceHandle(rh);
		return true;
	}

	printf("OpenService failed. Error = %lu\n", GetLastError());
	return false;
}

bool stop(SC_HANDLE sh, const wchar_t* driverName) {
	SC_HANDLE rh;

	rh = OpenService(
		sh,
		driverName,
		SERVICE_ALL_ACCESS
	);

	if (rh) {
		SERVICE_STATUS  serviceStatus;
		if (ControlService(rh, SERVICE_CONTROL_STOP, &serviceStatus)) {
			printf("Service stopped successfully.\n");
		}
		else {
			printf("ControlService failed. Error = %lu\n", GetLastError());
			CloseServiceHandle(rh);
			return false;
		}

		CloseServiceHandle(rh);
		return true;
	}

	printf("OpenService failed. Error = %lu\n", GetLastError());
	return false;
}

bool delete(SC_HANDLE sh, const wchar_t* driverName) {
	SC_HANDLE rh;

	rh = OpenService(
		sh,
		driverName,
		SERVICE_ALL_ACCESS
	);

	if (rh) {
		if (DeleteService(rh)) {
			printf("Service deleted successfully.\n");
		}
		else {
			printf("DeleteService failed. Error = %lu\n", GetLastError());
			CloseServiceHandle(rh);
			return false;
		}

		CloseServiceHandle(rh);
		return true;
	}

	printf("OpenService failed. Error = %lu\n", GetLastError());
	return false;
}

int main() {
	SC_HANDLE sh = OpenSCManager(
		NULL,
		NULL,
		SC_MANAGER_ALL_ACCESS);

	if (!sh) {
		printf("Open SC Manager failed. Error = %lu\n", GetLastError());
		return 0;
	}

	printf("[0] Load driver\n");
	load(sh, serviceName, driverName);
	printf("\n");

	printf("[1] Start service\n");
	start(sh, serviceName);
	printf("\n");

	printf("[2] Stop service\n");
	stop(sh, serviceName);
	printf("\n");

	printf("[3] Delete service\n");
	delete(sh, serviceName);
	printf("\n");

	CloseServiceHandle(sh);

	return 0;
}
