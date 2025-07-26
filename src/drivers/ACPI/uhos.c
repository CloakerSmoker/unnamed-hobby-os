 
#include "acpi.h"
#include "platform/acuhos.h"

#include "printf.h"


typedef struct {
    int Segment;
    int Bus;
    int Device;
    int Function;
    int PinsToInterruptNumbers[4];
} FlatInterruptRoutingEntry;

typedef struct {
    UINT64(*GetRSDP)();
    
    void*(*Allocate)(INT64);
    void(*Free)(void*);

    void*(*MapMemory)(ACPI_PHYSICAL_ADDRESS, ACPI_SIZE);
    void(*UnmapMemory)(void*, ACPI_SIZE);

    INT64(*GetThreadID)();
    void(*CreateTask)(void*, void*);
    void(*Sleep)(INT64);
    void(*Stall)(INT64);

    void*(*CreateSemaphore)(INT32);
    void(*DeleteSemaphore)(void*);
    char(*WaitSemaphore)(void*, INT32, INT32);
    void(*SignalSemaphore)(void*, INT32);

    void(*InstallIDT)(UINT64, void*, void*);
    void(*UninstallIDT)(UINT64);

    UINT64(*GetTimeMS)();

    void(*Print)(const char*);

    void(*OutB)(int, UINT32);
    void(*OutW)(int, UINT32);
    void(*OutD)(int, UINT32);

    char(*InB)(int);
    short(*InW)(int);
    int(*InD)(int);

    void(*PCIeOutB)(int, int, int, int, int, UINT64);
    void(*PCIeOutW)(int, int, int, int, int, UINT64);
    void(*PCIeOutD)(int, int, int, int, int, UINT64);
    void(*PCIeOutQ)(int, int, int, int, int, UINT64);

    char(*PCIeInB)(int, int, int, int, int);
    short(*PCIeInW)(int, int, int, int, int);
    int(*PCIeInD)(int, int, int, int, int);
    long long(*PCIeInQ)(int, int, int, int, int);

    void(*DefineInterruptRoute)(FlatInterruptRoutingEntry*);
} UHOSKernel;

UHOSKernel* Kernel = NULL;

ACPI_STATUS AcpiOsInitialize() {
    return AE_OK;
}

ACPI_STATUS AcpiOsTerminate() {
    return AE_OK;
}

//#define PRINT_CALL(format, ...) printf(format __VA_OPT__(,) __VA_ARGS__)
#define PRINT_CALL(format, ...)

ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer() {
    PRINT_CALL("AcpiOsGetRootPointer called\n");
    return Kernel->GetRSDP();
}

ACPI_STATUS AcpiOsPredefinedOverride(const ACPI_PREDEFINED_NAMES* PredefinedObject, ACPI_STRING* NewValue) {
    PRINT_CALL("AcpiOsPredefinedOverride called: PredefinedObject=%p, NewValue=%p\n", PredefinedObject, NewValue);
    *NewValue = NULL;
    return AE_OK;
}

ACPI_STATUS AcpiOsTableOverride(ACPI_TABLE_HEADER* ExistingTable, ACPI_TABLE_HEADER** NewTable) {
    PRINT_CALL("AcpiOsTableOverride called: ExistingTable=%p, NewTable=%p\n", ExistingTable, NewTable);
    *NewTable = NULL;
    return AE_OK;
}

ACPI_STATUS AcpiOsPhysicalTableOverride(ACPI_TABLE_HEADER *ExistingTable, ACPI_PHYSICAL_ADDRESS *NewAddress, UINT32 *NewTableLength) {
    PRINT_CALL("AcpiOsPhysicalTableOverride called: ExistingTable=%p, NewAddress=%p, NewTableLength=%p\n", ExistingTable, NewAddress, NewTableLength);
    *NewAddress = 0;
    return AE_OK;
}

void* AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS PhysicalAddress, ACPI_SIZE Size) {
    PRINT_CALL("AcpiOsMapMemory called: PhysicalAddress=0x%llx, Size=%llu\n", (unsigned long long)PhysicalAddress, (unsigned long long)Size);
    return Kernel->MapMemory(PhysicalAddress, Size);
}

void AcpiOsUnmapMemory(void* VirtualAddress, ACPI_SIZE Size) {
    PRINT_CALL("AcpiOsUnmapMemory called: VirtualAddress=%p, Size=%llu\n", VirtualAddress, (unsigned long long)Size);
    Kernel->UnmapMemory(VirtualAddress, Size);
}

ACPI_STATUS AcpiOsGetPhysicalAddress(void* VirtualAddress, ACPI_PHYSICAL_ADDRESS* PhysicalAddress) {
    PRINT_CALL("AcpiOsGetPhysicalAddress called: VirtualAddress=%p, PhysicalAddress=%p\n", VirtualAddress, PhysicalAddress);
    *PhysicalAddress = (ACPI_PHYSICAL_ADDRESS)VirtualAddress;
    return AE_OK;
}

void* AcpiOsAllocate(ACPI_SIZE Size) {
    PRINT_CALL("AcpiOsAllocate called: Size=%llu\n", (unsigned long long)Size);
    return Kernel->Allocate(Size);
}

void AcpiOsFree(void* Memory) {
    PRINT_CALL("AcpiOsFree called: Memory=%p\n", Memory);
    Kernel->Free(Memory);
}

/* BOOLEAN AcpiOsReadable(void* Memory, ACPI_SIZE Length) {
    return Kernel->Readable(Memory, Length);
}

BOOLEAN AcpiOsWritable(void* Memory, ACPI_SIZE Length) {
    return Kernel->Writable(Memory, Length);
} */

ACPI_THREAD_ID AcpiOsGetThreadId(void) {
    PRINT_CALL("AcpiOsGetThreadId called\n");
    return Kernel->GetThreadID();
}

ACPI_STATUS AcpiOsExecute(ACPI_EXECUTE_TYPE Type, ACPI_OSD_EXEC_CALLBACK Function, void* Context) {
    PRINT_CALL("AcpiOsExecute called: Type=%d, Function=%p, Context=%p\n", Type, Function, Context);
    Kernel->CreateTask(Function, Context);

    return AE_OK;
}

void AcpiOsWaitEventsComplete(void) {
    PRINT_CALL("AcpiOsWaitEventsComplete called\n");
}

void AcpiOsSleep(UINT64 Milliseconds) {
    PRINT_CALL("AcpiOsSleep called: Milliseconds=%llu\n", (unsigned long long)Milliseconds);
    Kernel->Sleep(Milliseconds);
}

void AcpiOsStall(UINT32 Microseconds) {
    PRINT_CALL("AcpiOsStall called: Microseconds=%u\n", Microseconds);
    Kernel->Stall(Microseconds);
}

ACPI_STATUS AcpiOsInstallInterruptHandler(UINT32 InterruptNumber, ACPI_OSD_HANDLER ServiceRoutine, void* Context) {
    PRINT_CALL("AcpiOsInstallInterruptHandler called: InterruptNumber=%u, ServiceRoutine=%p, Context=%p\n", InterruptNumber, ServiceRoutine, Context);
    Kernel->InstallIDT(InterruptNumber, ServiceRoutine, Context);
    return AE_OK;
}

ACPI_STATUS AcpiOsRemoveInterruptHandler(UINT32 InterruptNumber, ACPI_OSD_HANDLER ServiceRoutine) {
    PRINT_CALL("AcpiOsRemoveInterruptHandler called: InterruptNumber=%u, ServiceRoutine=%p\n", InterruptNumber, ServiceRoutine);
    Kernel->UninstallIDT(InterruptNumber);
    return AE_OK;
}

ACPI_STATUS AcpiOsCreateSemaphore(UINT32 MaxUnits, UINT32 InitialUnits, ACPI_SEMAPHORE* OutHandle) {
    PRINT_CALL("AcpiOsCreateSemaphore called: MaxUnits=%u, InitialUnits=%u, OutHandle=%p\n", MaxUnits, InitialUnits, OutHandle);

    *OutHandle = Kernel->CreateSemaphore(InitialUnits);

    return AE_OK;
}

ACPI_STATUS AcpiOsDeleteSemaphore(ACPI_SEMAPHORE Handle) {
    PRINT_CALL("AcpiOsDeleteSemaphore called: Handle=%p\n", Handle);

    Kernel->DeleteSemaphore(Handle);

    return AE_OK;
}

ACPI_STATUS AcpiOsWaitSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units, UINT16 Timeout) {
    PRINT_CALL("AcpiOsWaitSemaphore called: Handle=%p, Units=%u, Timeout=%u\n", Handle, Units, Timeout);

    if (Kernel->WaitSemaphore(Handle, Units, Timeout)) {
        return AE_OK;
    }
    else {
        return AE_TIME;
    }
}

ACPI_STATUS AcpiOsSignalSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units) {
    PRINT_CALL("AcpiOsSignalSemaphore called: Handle=%p, Units=%u\n", Handle, Units);

    Kernel->SignalSemaphore(Handle, Units);

    return AE_OK;
}

ACPI_STATUS AcpiOsCreateLock(ACPI_SPINLOCK* OutHandle) {
    PRINT_CALL("AcpiOsCreateLock called: OutHandle=%p\n", OutHandle);

    long long int* Lock = (long long int*)Kernel->Allocate(sizeof(long long int));
    *Lock = 0;
    *OutHandle = (ACPI_SPINLOCK)Lock;

    return AE_OK;
}

void AcpiOsDeleteLock(ACPI_SPINLOCK Handle) {
    PRINT_CALL("AcpiOsDeleteLock called: Handle=%p\n", Handle);

    if (Handle) {
        Kernel->Free((void*)Handle);
    }
}

ACPI_CPU_FLAGS AcpiOsAcquireLock(ACPI_SPINLOCK Handle) {
    PRINT_CALL("AcpiOsAcquireLock called: Handle=%p\n", Handle);

    while (__atomic_test_and_set(Handle, __ATOMIC_ACQUIRE)) {
        // Spin until the lock is acquired
    }

    return 0;
}

void AcpiOsReleaseLock(ACPI_SPINLOCK Handle, ACPI_CPU_FLAGS Flags) {
    PRINT_CALL("AcpiOsReleaseLock called: Handle=%p, Flags=%lu\n", Handle, (unsigned long)Flags);

    __atomic_clear(Handle, __ATOMIC_RELEASE);
}

UINT64 AcpiOsGetTimer(void) {
    PRINT_CALL("AcpiOsGetTimer called\n");
    return Kernel->GetTimeMS();
}

ACPI_STATUS AcpiOsSignal(UINT32 Function, void* Info) {
    PRINT_CALL("AcpiOsSignal called: Function=%u, Info=%p\n", Function, Info);
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
    PRINT_CALL("AcpiOsWritePort called: Address=0x%llx, Value=0x%x, Width=%u\n", (unsigned long long)Address, Value, Width);
    switch (Width) {
        case 8:
            Kernel->OutB(Address, Value);
            break;
        case 16:
            Kernel->OutW(Address, Value);
            break;
        case 32:
            Kernel->OutD(Address, Value);
            break;
        default:
            return AE_BAD_PARAMETER;
    }

    return AE_OK;
}

ACPI_STATUS AcpiOsReadPort(ACPI_IO_ADDRESS Address, UINT32* Value, UINT32 Width) {
    PRINT_CALL("AcpiOsReadPort called: Address=0x%llx, Value=%p, Width=%u\n", (unsigned long long)Address, Value, Width);
    switch (Width) {
        case 8:
            *Value = Kernel->InB(Address);
            break;
        case 16:
            *Value = Kernel->InW(Address);
            break;
        case 32:
            *Value = Kernel->InD(Address);
            break;
        default:
            return AE_BAD_PARAMETER;
    }

    return AE_OK;
}

ACPI_STATUS AcpiOsWriteMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64 Value, UINT32 Width) {
    PRINT_CALL("AcpiOsWriteMemory called: Address=0x%llx, Value=0x%llx, Width=%u\n", (unsigned long long)Address, (unsigned long long)Value, Width);
    switch (Width) {
        case 8:
            *(volatile char*)Address = (char)Value;
            break;
        case 16:
            *(volatile short*)Address = (short)Value;
            break;
        case 32:
            *(volatile int*)Address = (int)Value;
            break;
        case 64:
            *(volatile UINT64*)Address = Value;
            break;
        default:
            return AE_BAD_PARAMETER;
    }

    return AE_OK;
}

ACPI_STATUS AcpiOsReadMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64* Value, UINT32 Width) {
    PRINT_CALL("AcpiOsReadMemory called: Address=0x%llx, Value=%p, Width=%u\n", (unsigned long long)Address, Value, Width);
    switch (Width) {
        case 8:
            *Value = *(volatile char*)Address;
            break;
        case 16:
            *Value = *(volatile short*)Address;
            break;
        case 32:
            *Value = *(volatile int*)Address;
            break;
        case 64:
            *Value = *(volatile UINT64*)Address;
            break;
        default:
            return AE_BAD_PARAMETER;
    }

    return AE_OK;
}

ACPI_STATUS AcpiOsReadPciConfiguration(ACPI_PCI_ID* PciId, UINT32 Register, UINT64* Value, UINT32 Width) {
    PRINT_CALL("AcpiOsReadPciConfiguration called: PciId=%p (Seg=%x, Bus=%x, Dev=%x, Fun=%x), Register=0x%x, Value=%p, Width=%u\n",
        PciId, PciId ? PciId->Segment : 0, PciId ? PciId->Bus : 0, PciId ? PciId->Device : 0, PciId ? PciId->Function : 0, Register, Value, Width);
    switch (Width) {
        case 8:
            *Value = Kernel->PCIeInB(PciId->Segment, PciId->Bus, PciId->Device, PciId->Function, Register);
            break;
        case 16:
            *Value = Kernel->PCIeInW(PciId->Segment, PciId->Bus, PciId->Device, PciId->Function, Register);
            break;
        case 32:
            *Value = Kernel->PCIeInD(PciId->Segment, PciId->Bus, PciId->Device, PciId->Function, Register);
            break;
        case 64:
            *Value = Kernel->PCIeInQ(PciId->Segment, PciId->Bus, PciId->Device, PciId->Function, Register);
            break;
        default:
            return AE_BAD_PARAMETER;
    }

    return AE_OK;
}

ACPI_STATUS AcpiOsWritePciConfiguration(ACPI_PCI_ID* PciId, UINT32 Register, UINT64 Value, UINT32 Width) {
    PRINT_CALL("AcpiOsWritePciConfiguration called: PciId=%p (Seg=%x, Bus=%x, Dev=%x, Fun=%x), Register=0x%x, Value=0x%llx, Width=%u\n",
        PciId, PciId ? PciId->Segment : 0, PciId ? PciId->Bus : 0, PciId ? PciId->Device : 0, PciId ? PciId->Function : 0, Register, (unsigned long long)Value, Width);
    switch (Width) {
        case 8:
            Kernel->PCIeOutB(PciId->Segment, PciId->Bus, PciId->Device, PciId->Function, Register, Value);
            break;
        case 16:
            Kernel->PCIeOutW(PciId->Segment, PciId->Bus, PciId->Device, PciId->Function, Register, Value);
            break;
        case 32:
            Kernel->PCIeOutD(PciId->Segment, PciId->Bus, PciId->Device, PciId->Function, Register, Value);
            break;
        case 64:
            Kernel->PCIeOutQ(PciId->Segment, PciId->Bus, PciId->Device, PciId->Function, Register, Value);
            break;
        default:
            return AE_BAD_PARAMETER;
    }

    return AE_OK;
}

ACPI_STATUS AcpiOsEnterSleep(UINT8 SleepState, UINT32 RegaValue, UINT32 RegbValue) {
    PRINT_CALL("AcpiOsEnterSleep called: SleepState=%u, RegaValue=0x%x, RegbValue=0x%x\n", SleepState, RegaValue, RegbValue);
    return AE_OK;
}

#define INTERRUPT_MODEL_PIC 0
#define INTERRUPT_MODEL_APIC 1
#define INTERRUPT_MODEL_SAPIC 2

#define CHECK_RETURN_STATUS(call) \
    do { \
        ACPI_STATUS status = call; \
        if (ACPI_FAILURE(status)) { \
            printf("ACPI: %s failed: %s\n", #call, AcpiFormatException(status)); \
        } \
        return status;\
    } while (0)

#define CHECK(call) \
    do { \
        ACPI_STATUS status = call; \
        if (ACPI_FAILURE(status)) { \
            printf("ACPI: %s failed: %s\n", #call, AcpiFormatException(status)); \
            return; \
        } \
    } while (0)

ACPI_STATUS SetInterruptModel(int InterruptModel) {
    ACPI_OBJECT Arguments[1] = {
        {
            .Integer = {
                .Type = ACPI_TYPE_INTEGER,
                .Value = InterruptModel
            }
        }
    };

    ACPI_OBJECT_LIST ArgumentList = {
        .Count = 1,
        .Pointer = Arguments
    };

    ACPI_BUFFER ReturnValue = {
        .Length = ACPI_ALLOCATE_BUFFER,
        .Pointer = NULL
    };

    CHECK_RETURN_STATUS(AcpiEvaluateObject(NULL, "\\_PIC", &ArgumentList, &ReturnValue));
}

int GetBridgeBusNumber(ACPI_HANDLE Handle) {
    int BusNumber = 0;

    ACPI_BUFFER ReturnValue = {
        .Length = 4,
        .Pointer = &BusNumber
    };

    ACPI_OBJECT_LIST Arguments = {
        .Count = 0,
        .Pointer = NULL
    };

    ACPI_STATUS Status = AcpiEvaluateObject(Handle, "_BBN", &Arguments, &ReturnValue);

    if (ACPI_FAILURE(Status)) {
        printf("ACPI: _BBN evaluation failed: %s\n", AcpiFormatException(Status));
        return 0;
    }

    return BusNumber & 0xFF;
}

int GetBridgeSegmentGroup(ACPI_HANDLE Handle) {
    int SegmentGroup = 0;

    ACPI_BUFFER ReturnValue = {
        .Length = 4,
        .Pointer = &SegmentGroup
    };

    ACPI_OBJECT_LIST Arguments = {
        .Count = 0,
        .Pointer = NULL
    };

    ACPI_STATUS Status = AcpiEvaluateObject(Handle, "_SEG", &Arguments, &ReturnValue);

    if (ACPI_FAILURE(Status)) {
        printf("ACPI: _SEG evaluation failed: %s\n", AcpiFormatException(Status));
        return 0;
    }

    return SegmentGroup & 0xFFFF;
}

typedef struct {
    FlatInterruptRoutingEntry* ID;
    int Pin;
    int SourceIndex;
} IndirectInterruptInfo;

unsigned int OnCRSEntry(ACPI_RESOURCE* Resource, void* Context) {
    IndirectInterruptInfo* Info = Context;

    /* printf("ACPI: Found resource for device %i.%i.%i.%i with type %i\n",
        Info->ID.Segment, Info->ID.Bus, Info->ID.Device, Info->ID.Function, Resource->Type); */

    switch (Resource->Type)
    {
    case ACPI_RESOURCE_TYPE_IRQ:
        {
            ACPI_RESOURCE_IRQ* Irq = &Resource->Data.Irq;

            /* printf("ACPI: Device %i.%i.%i.%i Pin%c is using interrupt %i\n",
                Info->ID.Segment, Info->ID.Bus, Info->ID.Device, Info->ID.Function,
                'A' + Info->Pin,
                Irq->Interrupts[Info->SourceIndex]); */
            
            Info->ID->PinsToInterruptNumbers[Info->Pin] = Irq->Interrupts[Info->SourceIndex];
        }
        break;
    case ACPI_RESOURCE_TYPE_EXTENDED_IRQ:
        {
            ACPI_RESOURCE_EXTENDED_IRQ* ExtIrq = &Resource->Data.ExtendedIrq;

            /* printf("ACPI: Device %i.%i.%i.%i Pin%c is using extended interrupt %i\n",
                Info->ID.Segment, Info->ID.Bus, Info->ID.Device, Info->ID.Function,
                'A' + Info->Pin,
                ExtIrq->Interrupts[Info->SourceIndex]); */

            Info->ID->PinsToInterruptNumbers[Info->Pin] = ExtIrq->Interrupts[Info->SourceIndex];
        }
    default:
        break;
    }

    return 0;
}

unsigned int OnPCIeBridge(ACPI_HANDLE Bridge, unsigned int NestingLevel, void* Context, void** ReturnValue) {
    ACPI_BUFFER NameBuffer = {
        .Length = ACPI_ALLOCATE_BUFFER,
        .Pointer = NULL
    };

    AcpiGetName(Bridge, ACPI_FULL_PATHNAME, &NameBuffer);

    int SegmentGroup = GetBridgeSegmentGroup(Bridge);
    int Bus = GetBridgeBusNumber(Bridge); 

    printf("ACPI: Found PCIe bridge: %s, Segment Group: %i, Bus Number: %i\n", (char*)NameBuffer.Pointer, SegmentGroup, Bus);

    ACPI_BUFFER RoutingTableBuffer = {
        .Length = ACPI_ALLOCATE_BUFFER,
        .Pointer = NULL
    };

    if (ACPI_FAILURE(AcpiGetIrqRoutingTable(Bridge, &RoutingTableBuffer))) {
        printf("ACPI: Failed to get IRQ routing table for bridge %s\n", (char*)NameBuffer.Pointer);
        return 0;
    }

    ACPI_PCI_ROUTING_TABLE* RoutingTable = RoutingTableBuffer.Pointer;

    FlatInterruptRoutingEntry Entry = {
        .Segment = SegmentGroup,
        .Bus = Bus,
        .Device = 0,
        .Function = 0,
        .PinsToInterruptNumbers = {0, 0, 0, 0}
    };

    while (RoutingTable->Length > 0) {
        int Device = (RoutingTable->Address >> 16) & 0xFFFF;
        int Function = RoutingTable->Address & 0xFFFF;

        if (Entry.Device != Device || Entry.Function != Function) {
            if (Entry.Device != 0 || Entry.Function != 0) {
                Kernel->DefineInterruptRoute(&Entry);
            }

            Entry.Device = Device;
            Entry.Function = Function;
        }

        if (RoutingTable->Source[0] == '\0') {
            /* printf("ACPI: Device %i.%i.%i.%i is using pin %i, aka global interrupt %i\n",
                SegmentGroup, Bus, Device, Function,
                RoutingTable->Pin, RoutingTable->SourceIndex); */
            
            Entry.PinsToInterruptNumbers[RoutingTable->Pin] = RoutingTable->SourceIndex;
        }
        else {
            ACPI_HANDLE Link;

            if (AcpiGetHandle(Bridge, RoutingTable->Source, &Link) != AE_OK) {
                printf("ACPI: Failed to get handle for link route %s\n", RoutingTable->Source);
                return 0;
            }

            IndirectInterruptInfo Info = {
                .ID = &Entry,
                .Pin = RoutingTable->Pin,
                .SourceIndex = RoutingTable->SourceIndex
            };

            AcpiWalkResources(Link, "_CRS", &OnCRSEntry, &Info);
        }

        RoutingTable = (ACPI_PCI_ROUTING_TABLE*)((char*)RoutingTable + RoutingTable->Length);
    }

    return 0;
}

void Acpi() {
    AcpiGetDevices("PNP0A03", &OnPCIeBridge, NULL, NULL);
}

int strlen(const char*);

void _start(void* KernelPtr) {
    Kernel = (UHOSKernel*)KernelPtr;

    printf("Hello from ACPI! %d\n", sizeof(void*));

    //AcpiDbgLevel = ACPI_DEBUG_ALL;
    //AcpiDbgLayer = ACPI_ALL_COMPONENTS;

    CHECK(AcpiInitializeSubsystem());
    CHECK(AcpiInitializeTables(NULL, 0, FALSE));
    CHECK(AcpiLoadTables());
    CHECK(AcpiEnableSubsystem(ACPI_FULL_INITIALIZATION));
    CHECK(AcpiInitializeObjects(ACPI_FULL_INITIALIZATION));

    ACPI_OBJECT Argument[1] = {
        {
            .String = {
                .Type = ACPI_TYPE_STRING,
                .Pointer = "Windows 2001",
                .Length = 12,
            }
        }
    };

    ACPI_OBJECT_LIST Arguments = {
        .Count = 1,
        .Pointer = Argument
    };

    ACPI_BUFFER ReturnValue = {
        .Length = ACPI_ALLOCATE_BUFFER,
        .Pointer = NULL
    };

    CHECK(AcpiEvaluateObject(NULL, "\\_OSI", &Arguments, &ReturnValue));

    if (ReturnValue.Length < sizeof(ACPI_OBJECT)) {
        printf("ACPI: _OSI sux\n");
        return;
    }

    ACPI_OBJECT* Result = (ACPI_OBJECT*)ReturnValue.Pointer;

    if (Result->Type != ACPI_TYPE_INTEGER) {
        printf("ACPI: _OSI sux 2");
        return;
    }

    printf("ACPI: _OSI returned %llu\n", Result->Integer.Value);

    AcpiOsFree(Result);

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

    AcpiGetDevices("PNP0A03", &OnPCIeBridge, NULL, NULL);

    printf("ACPI: Initialized successfully!\n");
}