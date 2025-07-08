/*   files.c   */
/*
 * a program that reads a DATA.TXT file in the root of the EFI System Partition.
 */

#include <efi.h>
#include <efilib.h>

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    InitializeLib(ImageHandle, SystemTable);
    ST = SystemTable;
    EFI_STATUS Status;

    /* get the current image's protocol interface */
    EFI_GUID LipGUID = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_LOADED_IMAGE *LoadedImage = NULL;
    Status = uefi_call_wrapper(
        ST->BootServices->HandleProtocol, 3,
        ImageHandle,                                /* handle */
        &LipGUID,                                   /* protocol's GUID */
        &LoadedImage                                /* interface */
    );
    if (EFI_ERROR(Status)) return Status;

    /* get the image's filesystem */
    EFI_GUID SfsGUID = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_FILE_IO_INTERFACE *FileSystem = NULL;
    Status = uefi_call_wrapper(
        ST->BootServices->HandleProtocol, 3,
        LoadedImage->DeviceHandle,                  /* handle */
        &SfsGUID,                                   /* protocol's GUID */
        &FileSystem                                 /* interface */
    );
    if (EFI_ERROR(Status)) return Status;

    /* get the filesystem's protocol */
    EFI_FILE_PROTOCOL *Volume = NULL;
    Status = uefi_call_wrapper(
        FileSystem->OpenVolume, 2,
        FileSystem,                                 /* filesystem */
        &Volume                                     /* volume */
    );
    if (EFI_ERROR(Status)) return Status;

    /* open the file */
    CHAR16 *FileName = L"DATA.TXT";
    EFI_FILE_PROTOCOL *File;
    Status = uefi_call_wrapper(
        Volume->Open, 5, 
        Volume,                                     /* volume */
        &File,                                      /* file */
        FileName,                                   /* filename */
        EFI_FILE_MODE_READ,                         /* open mode */
        0                                           /* creation flags (ignore) */
    );
    if (EFI_ERROR(Status)) return Status;

    /* get file size */
    UINTN BufferSize = 0;
    EFI_FILE_INFO *InfoBuffer = NULL;
    EFI_GUID FinfoGUID = EFI_FILE_INFO_ID;
    Status = uefi_call_wrapper(
        File->GetInfo, 4, 
        File,                                       /* *this */
        &FinfoGUID,                                 /* file info GUID */
        &BufferSize,                                /* buffer size */
        InfoBuffer                                  /* result buffer */
    );
    /* this will happen, as InfoBuffer is NULL and size is initially 0 */
    if (Status == EFI_BUFFER_TOO_SMALL) {
        /* allocate memory */
        Status = uefi_call_wrapper(
            ST->BootServices->AllocatePool, 3,
            EfiReservedMemoryType,                  /* memory type */
            BufferSize,                             /* bytes */
            (void **) &InfoBuffer                   /* buffer */
        );
        if (EFI_ERROR(Status)) return Status;

        /* call again */
        Status = uefi_call_wrapper(
            File->GetInfo, 4, 
            File,                                   /* *this */
            &FinfoGUID,                             /* file info GUID */
            &BufferSize,                            /* buffer size */
            InfoBuffer                              /* result buffer */
        );
        if (EFI_ERROR(Status)) return Status;
    }

    /* allocate a buffer for reading the file */
    UINT8 *FileBuffer = NULL;
    UINT64 FileSize = InfoBuffer->FileSize;
    Status = uefi_call_wrapper(
        ST->BootServices->AllocatePool, 3,
        EfiReservedMemoryType,                  /* memory type */
        FileSize,                               /* bytes */
        (void **) &FileBuffer                   /* buffer */
    );
    if (EFI_ERROR(Status)) return Status;

    /* read from the file */
    Status = uefi_call_wrapper(
        File->Read, 3,
        File,                                   /* *this */
        &FileSize,                              /* buffer size */
        (void *) FileBuffer                     /* file buffer */
    );
    if (EFI_ERROR(Status)) return Status;

    /* output file */
    for (UINTN i = 0; i < FileSize; i++)
        Print(L"%c", FileBuffer[i]);

    /* close the file */
    Status = uefi_call_wrapper(
        File->Close, 1,
        File                                    /* *this */
    );
    if (EFI_ERROR(Status)) return Status;

    while (1);
    return EFI_SUCCESS;
}
