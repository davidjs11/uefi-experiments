#include <efi.h>
#include <efilib.h>


EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    InitializeLib(ImageHandle, SystemTable);
    ST = SystemTable;
    EFI_STATUS Status;

    /* set background to blue and white text */
    uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);

    /* get memory map */
    UINTN MemoryMapSize = 0;
    EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;
    Status = uefi_call_wrapper(
        ST->BootServices->GetMemoryMap, 5,      /* service */
        &MemoryMapSize,                         /* pointer to memory map size */
        MemoryMap,                              /* memory map buffer */
        &MapKey,                                /* map key */
        &DescriptorSize,                        /* size of each descriptor */
        &DescriptorVersion                      /* version of each descriptor */
    );

    /* because MemoryMap is NULL, this should always happen */
    if (Status == EFI_BUFFER_TOO_SMALL) {
        /* allocate memory */
        Status = uefi_call_wrapper(
            ST->BootServices->AllocatePool, 3,      /* service */
            EfiReservedMemoryType,                  /* memory type */
            MemoryMapSize + (1 * DescriptorSize),   /* size in bytes (with some extra space) */
            (void **) &MemoryMap                    /* buffer */
        );

        /* call again */
        Status = uefi_call_wrapper(
            ST->BootServices->GetMemoryMap, 5,      /* service */
            &MemoryMapSize,                         /* pointer to memory map size */
            MemoryMap,                              /* memory map buffer */
            &MapKey,                                /* map key */
            &DescriptorSize,                        /* size of each descriptor */
            &DescriptorVersion                      /* version of each descriptor */
        );
    }

    /* print memory map */
    UINTN EntryNum = MemoryMapSize / DescriptorSize;
    EFI_MEMORY_DESCRIPTOR *Entry = MemoryMap;
    for (UINTN i = 0; i < EntryNum; i++) {
        Print(
            L"entry #%03u: 0x%016x - 0x%016x (%u pages)\n", 
            i,
            Entry->PhysicalStart,
            Entry->PhysicalStart + (Entry->NumberOfPages * 4096),
            Entry->NumberOfPages
        );

        Entry = (EFI_MEMORY_DESCRIPTOR *) ((UINT8 *) Entry + DescriptorSize);
    }

    while (1);
    return EFI_SUCCESS;
}
