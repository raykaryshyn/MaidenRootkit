#include <ntddk.h>
#include <wdf.h>

PDEVICE_OBJECT g_RootkitDevice;
const WCHAR deviceLinkBuffer[] = L"\\DosDevices\\Rootkit";
const WCHAR deviceNameBuffer[] = L"\\Device\\Rootkit";

typedef struct _IDTR {
	USHORT Limit;
	ULONG_PTR Base;
} IDTR;

typedef struct _IDT_ENTRY {
	USHORT OffsetLow;
	USHORT Selector;
	UCHAR IST;
	UCHAR TypeAttr;
	USHORT OffsetMiddle;
	ULONG OffsetHigh;
	ULONG Reserved;
} IDT_ENTRY;

IDTR GetIDTBaseAddress() {
	IDTR idtr = { 0 };
	__sidt(&idtr);
	return idtr;
}

NTSTATUS OnStubDispatch(
	_In_ PDEVICE_OBJECT DeviceObject,
	_In_ PIRP Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(
		Irp,
		IO_NO_INCREMENT);

	DbgPrint("Received IOCTL request!\n");

	return STATUS_SUCCESS;
}

VOID OnUnload(_In_ PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);

	UNICODE_STRING deviceLinkUnicodeString;

	RtlInitUnicodeString(&deviceLinkUnicodeString, deviceLinkBuffer);
	IoDeleteSymbolicLink(&deviceLinkUnicodeString);

	if (g_RootkitDevice != NULL)
	{
		IoDeleteDevice(g_RootkitDevice);
		g_RootkitDevice = NULL;
	}

	DbgPrint("Unloaded the rootkit!\n");
}

NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT theDriverObject,
	_In_ PUNICODE_STRING theRegistryPath
)
{
	UNREFERENCED_PARAMETER(theRegistryPath);

	NTSTATUS ntStatus;

	UNICODE_STRING deviceNameUnicodeString;
	UNICODE_STRING deviceLinkUnicodeString;

	int i;

	RtlInitUnicodeString(&deviceNameUnicodeString, deviceNameBuffer);
	RtlInitUnicodeString(&deviceLinkUnicodeString, deviceLinkBuffer);

	ntStatus = IoCreateDevice(
		theDriverObject,
		0,
		&deviceNameUnicodeString,
		0x00001234,
		0,
		TRUE,
		&g_RootkitDevice);

	if (!NT_SUCCESS(ntStatus) || g_RootkitDevice == NULL)
	{
		DbgPrint("Failed to create IO device (status: 0x%X)\n", ntStatus);
		return ntStatus;
	}

	ntStatus = IoCreateSymbolicLink(
		&deviceLinkUnicodeString,
		&deviceNameUnicodeString);

	if (!NT_SUCCESS(ntStatus))
	{
		DbgPrint("Failed to create symbolic link (status: 0x%X)\n", ntStatus);
		IoDeleteDevice(g_RootkitDevice);
		return ntStatus;
	}

	DbgPrint("Loaded the rootkit!\n");

	theDriverObject->DriverUnload = OnUnload;

	IDTR idtr = GetIDTBaseAddress();
	DbgPrint("IDT Base Address: 0x%p\n", (PVOID)idtr.Base);
	DbgPrint("Number of Entries in IDT: %u\n", (unsigned int)((idtr.Limit / sizeof(IDT_ENTRY)) + 1));

	DbgPrint("Maximum IRP Value: %d\n", IRP_MJ_MAXIMUM_FUNCTION);
	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		DbgPrint("IRP[%d]\n", i);
		theDriverObject->MajorFunction[i] = OnStubDispatch;
	}

	return ntStatus;
}