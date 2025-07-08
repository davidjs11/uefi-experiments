/*   framebuffer.c   */
/*
 * a program that outputs colors into the screen.
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

    /* search GOP protocol */
    EFI_GUID GopGUID = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
    Status = uefi_call_wrapper(
        ST->BootServices->LocateProtocol, 3,        /* service */
        &GopGUID,                                   /* GOP's GUID */
        NULL,                                       /* ignorable */
        &Gop                                        /* Gop interface pointer */
    );
    if (EFI_ERROR(Status)) return Status;

    /* obtain available video modes */
    for (UINTN i = 0; i < Gop->Mode->MaxMode; i++) {
        UINTN SizeOfInfo;
        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
        Status = uefi_call_wrapper(
            Gop->QueryMode, 4,                          /* service */
            Gop,                                        /* 'this' pointer */
            Gop->Mode == NULL ? 0 : Gop->Mode->Mode,    /* current mode */
            &SizeOfInfo,                                /* size of info buffer */
            &Info                                       /* info buffer */
        );
        if (EFI_ERROR(Status)) return Status;

        /* print mode info */
        Print(
            L"mode #%03u: %ux%u (format: %u)\n", 
            i,
            Info->HorizontalResolution,
            Info->VerticalResolution,
            Info->PixelFormat
        );
    }

    /* current mode's framebuffer info */
    Print(L"Framebuffer address 0x%016x size %d, width %d height %d pixelsperline %d\n",
      Gop->Mode->FrameBufferBase,
      Gop->Mode->FrameBufferSize,
      Gop->Mode->Info->HorizontalResolution,
      Gop->Mode->Info->VerticalResolution,
      Gop->Mode->Info->PixelsPerScanLine
    );

    /* print something */
    UINT32 color = 0x00000000;
    for (int i = 0; i < Gop->Mode->Info->HorizontalResolution*Gop->Mode->Info->VerticalResolution; i++) {
        if (i % Gop->Mode->Info->PixelsPerScanLine < Gop->Mode->Info->PixelsPerScanLine / 3)
            color = 0xFFFF0000;
        else if (i % Gop->Mode->Info->PixelsPerScanLine < 2*Gop->Mode->Info->PixelsPerScanLine / 3)
            color = 0xFF00FF00;
        else if (i % Gop->Mode->Info->PixelsPerScanLine < Gop->Mode->Info->PixelsPerScanLine)
            color = 0xFF0000FF;
        *((UINT32 *) Gop->Mode->FrameBufferBase+i) = color;
    }

    while (1);
    return EFI_SUCCESS;
}
