#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>

const wchar_t* driverName = L"MaidenRootkit";
const wchar_t* path = L"C:\\MaidenRootkit\\x64\\Debug\\RootkitDriver.sys";

bool load(SC_HANDLE sh, const wchar_t* driverName, const wchar_t* path) {
	printf("Loading driver as a service...\n");

	SC_HANDLE rh = CreateService(
		sh,
		driverName,
		driverName,
		SERVICE_ALL_ACCESS,
		SERVICE_KERNEL_DRIVER,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		path,
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
	load(sh, driverName, path);
	printf("\n");

	printf("[1] Start service\n");
	start(sh, driverName);
	printf("\n");

	printf("[2] Stop service\n");
	stop(sh, driverName);
	printf("\n");

	printf("[3] Delete service\n");
	delete(sh, driverName);
	printf("\n");

	CloseServiceHandle(sh);

	return 0;
}
