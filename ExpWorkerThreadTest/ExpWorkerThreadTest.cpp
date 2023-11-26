#include <ntddk.h>

// KIRQL currentIrql = KeGetCurrentIrql();
// KdPrint(("Current IRQL: %d\n", KeGetCurrentIrql()));

UNICODE_STRING Name = RTL_CONSTANT_STRING(L"\\Device\\ExpWorkerThreadTest");
UNICODE_STRING SymbolicLink = RTL_CONSTANT_STRING(L"\\??\\ExpWorkerThreadTest");

NTSTATUS Create(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS Close(PDEVICE_OBJECT DeviceObject, PIRP Irp);
void DriverUnload(PDRIVER_OBJECT DriverObject);

extern "C" NTSTATUS
DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);
	
	DriverObject->DriverUnload = DriverUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = Create;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = Create;

	PDEVICE_OBJECT DeviceObject;
	NTSTATUS status = IoCreateDevice(
		DriverObject,
		0,
		&Name,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&DeviceObject
	);

	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed to create a device object!"));
		return status;
	}

	status = IoCreateSymbolicLink(&SymbolicLink, &Name);

	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed to create a symbolic link!"));
		IoDeleteDevice(DeviceObject);
		return status;
	}

	return STATUS_SUCCESS;
}

NTSTATUS Create(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return NTSTATUS();
}

NTSTATUS Close(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return NTSTATUS();
}


void DriverUnload(PDRIVER_OBJECT DriverObject) {
	IoDeleteSymbolicLink(&SymbolicLink);
	IoDeleteDevice(DriverObject->DeviceObject);
}