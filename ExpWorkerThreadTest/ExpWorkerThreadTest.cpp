#include <ntddk.h>

#define MY_DEVICE_TYPE 0x8123
#define IOCTL_WORKITEM_TEST CTL_CODE(MY_DEVICE_TYPE, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)

UNICODE_STRING Name = RTL_CONSTANT_STRING(L"\\Device\\ExpWorkerThreadTest");
UNICODE_STRING SymbolicLink = RTL_CONSTANT_STRING(L"\\??\\ExpWorkerThreadTest");

NTSTATUS CreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp);
void DriverUnload(PDRIVER_OBJECT DriverObject);

extern "C" NTSTATUS
DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);
	
	DriverObject->DriverUnload = DriverUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = CreateClose;

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

	KdPrint(("DriverEntry OK"));
	return STATUS_SUCCESS;
}

IO_WORKITEM_ROUTINE IoWorkItemRoutine;
void IoWorkItemRoutine(PDEVICE_OBJECT DeviceObject, PVOID Context) {
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Context);
	KdPrint(("Current IRQL: %d, Current PID: %lu", KeGetCurrentIrql(), PsGetCurrentProcessId()));
}

void CreateTestWorkItem(PDEVICE_OBJECT DeviceObject) {
	do {
		PIO_WORKITEM WorkItem = IoAllocateWorkItem(DeviceObject);
		if (NULL == WorkItem) {
			KdPrint(("Failed to allocate a work item!"));
			break;
		}

		IoQueueWorkItem(
			WorkItem,
			IoWorkItemRoutine,
			DelayedWorkQueue,
			NULL
		);

		IoFreeWorkItem(WorkItem);
	} while (false);
}

NTSTATUS CreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
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