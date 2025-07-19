 
#include "acpi.h"
#include "printf.h"

typedef struct {
    UINT64(*GetRSDP)();
    
    void*(*Allocate)(INT64);
    void(*Free)(void*);

    INT64(*GetThreadID)();
    void(*CreateTask)(void*, void*);
    void(*Sleep)(INT64);
    void(*Stall)(INT64);

    void(*InstallIDT)(UINT64, void*, void*);
    void(*UninstallIDT)(UINT64);

    UINT64(*GetTimeMS)();

    void(*Print)(const char*);

    void(*OutB)(int, char);
    void(*OutW)(int, short);
    void(*OutD)(int, int);

    char(*InB)(int);
    short(*InW)(int);
    int(*InD)(int);

    void(*PCIeOutB)(int, int, int, int, int, char);
    void(*PCIeOutW)(int, int, int, int, int, short);
    void(*PCIeOutD)(int, int, int, int, int, int);
    void(*PCIeOutQ)(int, int, int, int, int, long long);

    char(*PCIeInB)(int, int, int, int, int);
    short(*PCIeInW)(int, int, int, int, int);
    int(*PCIeInD)(int, int, int, int, int);
    long long(*PCIeInQ)(int, int, int, int, int);


} UHOSKernel;

UHOSKernel* Kernel = NULL;

ACPI_STATUS AcpiOsInitialize() {
    return AE_OK;
}

ACPI_STATUS AcpiOsTerminate() {
    return AE_OK;
}

ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer() {
    printf("AcpiOsGetRootPointer called\n");
    return Kernel->GetRSDP();
}

ACPI_STATUS AcpiOsPredefinedOverride(const ACPI_PREDEFINED_NAMES* PredefinedObject, ACPI_STRING* NewValue) {
    printf("AcpiOsPredefinedOverride called: PredefinedObject=%p, NewValue=%p\n", PredefinedObject, NewValue);
    *NewValue = NULL;
    return AE_OK;
}

ACPI_STATUS AcpiOsTableOverride(ACPI_TABLE_HEADER* ExistingTable, ACPI_TABLE_HEADER** NewTable) {
    printf("AcpiOsTableOverride called: ExistingTable=%p, NewTable=%p\n", ExistingTable, NewTable);
    *NewTable = NULL;
    return AE_OK;
}

ACPI_STATUS AcpiOsPhysicalTableOverride(ACPI_TABLE_HEADER *ExistingTable, ACPI_PHYSICAL_ADDRESS *NewAddress, UINT32 *NewTableLength) {
    printf("AcpiOsPhysicalTableOverride called: ExistingTable=%p, NewAddress=%p, NewTableLength=%p\n", ExistingTable, NewAddress, NewTableLength);
    *NewAddress = 0;
    return AE_OK;
}

void* AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS PhysicalAddress, ACPI_SIZE Size) {
    printf("AcpiOsMapMemory called: PhysicalAddress=0x%llx, Size=%llu\n", (unsigned long long)PhysicalAddress, (unsigned long long)Size);
    return (void*)PhysicalAddress;
}

void AcpiOsUnmapMemory(void* VirtualAddress, ACPI_SIZE Size) {
    printf("AcpiOsUnmapMemory called: VirtualAddress=%p, Size=%llu\n", VirtualAddress, (unsigned long long)Size);
}

ACPI_STATUS AcpiOsGetPhysicalAddress(void* VirtualAddress, ACPI_PHYSICAL_ADDRESS* PhysicalAddress) {
    printf("AcpiOsGetPhysicalAddress called: VirtualAddress=%p, PhysicalAddress=%p\n", VirtualAddress, PhysicalAddress);
    *PhysicalAddress = (ACPI_PHYSICAL_ADDRESS)VirtualAddress;
    return AE_OK;
}

void* AcpiOsAllocate(ACPI_SIZE Size) {
    printf("AcpiOsAllocate called: Size=%llu\n", (unsigned long long)Size);
    return Kernel->Allocate(Size);
}

void AcpiOsFree(void* Memory) {
    printf("AcpiOsFree called: Memory=%p\n", Memory);
    Kernel->Free(Memory);
}

/* BOOLEAN AcpiOsReadable(void* Memory, ACPI_SIZE Length) {
    return Kernel->Readable(Memory, Length);
}

BOOLEAN AcpiOsWritable(void* Memory, ACPI_SIZE Length) {
    return Kernel->Writable(Memory, Length);
} */

ACPI_THREAD_ID AcpiOsGetThreadId(void) {
    printf("AcpiOsGetThreadId called\n");
    return Kernel->GetThreadID();
}

ACPI_STATUS AcpiOsExecute(ACPI_EXECUTE_TYPE Type, ACPI_OSD_EXEC_CALLBACK Function, void* Context) {
    printf("AcpiOsExecute called: Type=%d, Function=%p, Context=%p\n", Type, Function, Context);
    Kernel->CreateTask(Function, Context);

    return AE_OK;
}

void AcpiOsWaitEventsComplete(void) {
    printf("AcpiOsWaitEventsComplete called\n");
}

void AcpiOsSleep(UINT64 Milliseconds) {
    printf("AcpiOsSleep called: Milliseconds=%llu\n", (unsigned long long)Milliseconds);
    Kernel->Sleep(Milliseconds);
}

void AcpiOsStall(UINT32 Microseconds) {
    printf("AcpiOsStall called: Microseconds=%u\n", Microseconds);
    Kernel->Stall(Microseconds);
}

ACPI_STATUS AcpiOsInstallInterruptHandler(UINT32 InterruptNumber, ACPI_OSD_HANDLER ServiceRoutine, void* Context) {
    printf("AcpiOsInstallInterruptHandler called: InterruptNumber=%u, ServiceRoutine=%p, Context=%p\n", InterruptNumber, ServiceRoutine, Context);
    Kernel->InstallIDT(InterruptNumber, ServiceRoutine, Context);
    return AE_OK;
}

ACPI_STATUS AcpiOsRemoveInterruptHandler(UINT32 InterruptNumber, ACPI_OSD_HANDLER ServiceRoutine) {
    printf("AcpiOsRemoveInterruptHandler called: InterruptNumber=%u, ServiceRoutine=%p\n", InterruptNumber, ServiceRoutine);
    Kernel->UninstallIDT(InterruptNumber);
    return AE_OK;
}

ACPI_STATUS AcpiOsCreateSemaphore(UINT32 MaxUnits, UINT32 InitialUnits, ACPI_SEMAPHORE* OutHandle) {
    printf("AcpiOsCreateSemaphore called: MaxUnits=%u, InitialUnits=%u, OutHandle=%p\n", MaxUnits, InitialUnits, OutHandle);
    return AE_OK;
}

ACPI_STATUS AcpiOsDeleteSemaphore(ACPI_SEMAPHORE Handle) {
    printf("AcpiOsDeleteSemaphore called: Handle=%p\n", Handle);
    return AE_OK;
}

ACPI_STATUS AcpiOsWaitSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units, UINT16 Timeout) {
    printf("AcpiOsWaitSemaphore called: Handle=%p, Units=%u, Timeout=%u\n", Handle, Units, Timeout);
    return AE_OK;
}

ACPI_STATUS AcpiOsSignalSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units) {
    printf("AcpiOsSignalSemaphore called: Handle=%p, Units=%u\n", Handle, Units);
    return AE_OK;
}

ACPI_STATUS AcpiOsCreateLock(ACPI_SPINLOCK* OutHandle) {
    printf("AcpiOsCreateLock called: OutHandle=%p\n", OutHandle);
    return AE_OK;
}

void AcpiOsDeleteLock(ACPI_SPINLOCK Handle) {
    printf("AcpiOsDeleteLock called: Handle=%p\n", Handle);
}

ACPI_CPU_FLAGS AcpiOsAcquireLock(ACPI_SPINLOCK Handle) {
    printf("AcpiOsAcquireLock called: Handle=%p\n", Handle);
    return 0;
}

void AcpiOsReleaseLock(ACPI_SPINLOCK Handle, ACPI_CPU_FLAGS Flags) {
    printf("AcpiOsReleaseLock called: Handle=%p, Flags=%lu\n", Handle, (unsigned long)Flags);
}

UINT64 AcpiOsGetTimer(void) {
    printf("AcpiOsGetTimer called\n");
    return Kernel->GetTimeMS();
}

ACPI_STATUS AcpiOsSignal(UINT32 Function, void* Info) {
    printf("AcpiOsSignal called: Function=%u, Info=%p\n", Function, Info);
    switch (Function) {
        case ACPI_SIGNAL_FATAL:
            {
                ACPI_SIGNAL_FATAL_INFO* FatalInfo = (ACPI_SIGNAL_FATAL_INFO*)Info;
                printf("ACPI: Fatal signal: Type=%x, Code=%x, Argument=%x\n", FatalInfo->Type, FatalInfo->Code, FatalInfo->Argument);
            }
            break;
        case ACPI_SIGNAL_BREAKPOINT:
            printf("ACPI: Breakpoint signal received: %s\n", (char*)Info);
            break;
        default:
            printf("ACPI: Unknown signal function: %u\n", Function);
            return AE_BAD_PARAMETER;
    }
    return AE_OK;
}

void _putchar(char c) {
    char Buffer[2] = {c, '\0'};

    Kernel->Print(Buffer);
}
    
void AcpiOsVprintf(const char* Format, va_list Args) {
    vprintf(Format, Args);
}

void AcpiOsPrintf(const char* Format, ...) {
    va_list Args;
    va_start(Args, Format);

    vprintf(Format, Args);

    va_end(Args);
}

ACPI_STATUS AcpiOsWritePort(ACPI_IO_ADDRESS Address, UINT32 Value, UINT32 Width) {
    printf("AcpiOsWritePort called: Address=0x%llx, Value=0x%x, Width=%u\n", (unsigned long long)Address, Value, Width);
    switch (Width) {
        case 1:
            Kernel->OutB(Address, (char)Value);
            break;
        case 2:
            Kernel->OutW(Address, (short)Value);
            break;
        case 4:
            Kernel->OutD(Address, Value);
            break;
        default:
            return AE_BAD_PARAMETER;
    }

    return AE_OK;
}

ACPI_STATUS AcpiOsReadPort(ACPI_IO_ADDRESS Address, UINT32* Value, UINT32 Width) {
    printf("AcpiOsReadPort called: Address=0x%llx, Value=%p, Width=%u\n", (unsigned long long)Address, Value, Width);
    switch (Width) {
        case 1:
            *Value = Kernel->InB(Address);
            break;
        case 2:
            *Value = Kernel->InW(Address);
            break;
        case 4:
            *Value = Kernel->InD(Address);
            break;
        default:
            return AE_BAD_PARAMETER;
    }

    return AE_OK;
}

ACPI_STATUS AcpiOsWriteMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64 Value, UINT32 Width) {
    printf("AcpiOsWriteMemory called: Address=0x%llx, Value=0x%llx, Width=%u\n", (unsigned long long)Address, (unsigned long long)Value, Width);
    switch (Width) {
        case 1:
            *(volatile char*)Address = (char)Value;
            break;
        case 2:
            *(volatile short*)Address = (short)Value;
            break;
        case 4:
            *(volatile int*)Address = (int)Value;
            break;
        case 8:
            *(volatile UINT64*)Address = Value;
            break;
        default:
            return AE_BAD_PARAMETER;
    }

    return AE_OK;
}

ACPI_STATUS AcpiOsReadMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64* Value, UINT32 Width) {
    printf("AcpiOsReadMemory called: Address=0x%llx, Value=%p, Width=%u\n", (unsigned long long)Address, Value, Width);
    switch (Width) {
        case 1:
            *Value = *(volatile char*)Address;
            break;
        case 2:
            *Value = *(volatile short*)Address;
            break;
        case 4:
            *Value = *(volatile int*)Address;
            break;
        case 8:
            *Value = *(volatile UINT64*)Address;
            break;
        default:
            return AE_BAD_PARAMETER;
    }

    return AE_OK;
}

ACPI_STATUS AcpiOsReadPciConfiguration(ACPI_PCI_ID* PciId, UINT32 Register, UINT64* Value, UINT32 Width) {
    printf("AcpiOsReadPciConfiguration called: PciId=%p (Seg=%x, Bus=%x, Dev=%x, Fun=%x), Register=0x%x, Value=%p, Width=%u\n",
        PciId, PciId ? PciId->Segment : 0, PciId ? PciId->Bus : 0, PciId ? PciId->Device : 0, PciId ? PciId->Function : 0, Register, Value, Width);
    switch (Width) {
        case 1:
            *Value = Kernel->PCIeInB(PciId->Segment, PciId->Bus, PciId->Device, PciId->Function, Register);
            break;
        case 2:
            *Value = Kernel->PCIeInW(PciId->Segment, PciId->Bus, PciId->Device, PciId->Function, Register);
            break;
        case 4:
            *Value = Kernel->PCIeInD(PciId->Segment, PciId->Bus, PciId->Device, PciId->Function, Register);
            break;
        case 8:
            *Value = Kernel->PCIeInQ(PciId->Segment, PciId->Bus, PciId->Device, PciId->Function, Register);
            break;
        default:
            return AE_BAD_PARAMETER;
    }

    return AE_OK;
}

ACPI_STATUS AcpiOsWritePciConfiguration(ACPI_PCI_ID* PciId, UINT32 Register, UINT64 Value, UINT32 Width) {
    printf("AcpiOsWritePciConfiguration called: PciId=%p (Seg=%x, Bus=%x, Dev=%x, Fun=%x), Register=0x%x, Value=0x%llx, Width=%u\n",
        PciId, PciId ? PciId->Segment : 0, PciId ? PciId->Bus : 0, PciId ? PciId->Device : 0, PciId ? PciId->Function : 0, Register, (unsigned long long)Value, Width);
    switch (Width) {
        case 1:
            Kernel->PCIeOutB(PciId->Segment, PciId->Bus, PciId->Device, PciId->Function, Register, (char)Value);
            break;
        case 2:
            Kernel->PCIeOutW(PciId->Segment, PciId->Bus, PciId->Device, PciId->Function, Register, (short)Value);
            break;
        case 4:
            Kernel->PCIeOutD(PciId->Segment, PciId->Bus, PciId->Device, PciId->Function, Register, (int)Value);
            break;
        case 8:
            Kernel->PCIeOutQ(PciId->Segment, PciId->Bus, PciId->Device, PciId->Function, Register, Value);
            break;
        default:
            return AE_BAD_PARAMETER;
    }

    return AE_OK;
}

ACPI_STATUS AcpiOsEnterSleep(UINT8 SleepState, UINT32 RegaValue, UINT32 RegbValue) {
    printf("AcpiOsEnterSleep called: SleepState=%u, RegaValue=0x%x, RegbValue=0x%x\n", SleepState, RegaValue, RegbValue);
    return AE_OK;
}

void _start(void* KernelPtr) {
    Kernel = (UHOSKernel*)KernelPtr;

    printf("Hello from ACPI! %d\n", 42);

    AcpiInitializeSubsystem();
    AcpiEnableSubsystem(ACPI_FULL_INITIALIZATION);

/* 
    ACPI_STATUS Status = AcpiOsInitialize();
    if (ACPI_FAILURE(Status)) {
        {
            struct {
                int Count;
                int Status;
            } Args = {
                .Count = 1,
                .Status = Status
            };

            printf("ACPI: Initialization failed with status %d\n", &Args);
        }

        return;
    } */

    printf("ACPI: Initialized successfully!\n");
}