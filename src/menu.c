/*   video_menu.c   */
/*
 * a program that outputs colors into the screen.
 */
#include <efi.h>
#include <efilib.h>

EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;

typedef struct {
    CHAR16          *title;                 /* menu title */
    CHAR16          **entries;              /* menu entries */
    size_t          size;                   /* number of entries */
    size_t          height;                 /* height of the menu */
    void            (*print_entry)(int);
} menu_t;

UINT16 read_key(void) {
    EFI_STATUS Status;
    EFI_INPUT_KEY Key;
    do {
        Status = uefi_call_wrapper(
            ST->ConIn->ReadKeyStroke, 2,
            ST->ConIn,
            &Key
        );
    } while (Status != EFI_SUCCESS);
    return Key.ScanCode;
}

UINTN menu_select(menu_t *menu) {
    size_t selected_entry = 0;
    size_t entry = 0;
    uint16_t pressed_key = 0;
    do {
        /* process input */
        switch (pressed_key) {
            case 0x01: 
                if (selected_entry > 0) selected_entry--; 
                break;
            case 0x02: 
                if (selected_entry < menu->size - 1)
                    selected_entry++; 
                break;
            default: 
                break;
        }

        /* clear screen */
        uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
        uefi_call_wrapper(
            ST->ConOut->ClearScreen, 1,
            ST->ConOut
        );

        /* print title */
        uefi_call_wrapper(
            ST->ConOut->SetAttribute, 2, 
            ST->ConOut, 
            EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY
        );
        Print(L"%s\n", menu->title);

        /* render entries */
        for (size_t i = 0; i < menu->height && i < menu->size; i++) {
            /* select entry to print */
            if (menu->height > menu->size)
                entry = (i < menu->size) ? i : 0; 
            else if (selected_entry <= menu->height/2)
                entry = i;
            else if (selected_entry >= menu->size - menu->height/2)
                entry = menu->size - menu->height + i;
            else
                entry = selected_entry - menu->height/2 + i;

            /* print entry */
            uefi_call_wrapper(
                ST->ConOut->SetAttribute, 2, 
                ST->ConOut, 
                (entry == selected_entry)
                ? EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY
                : EFI_WHITE | EFI_BACKGROUND_BLUE
            );
            menu->print_entry(entry);
        }

        /* get next key (blocking) */
        pressed_key = read_key();
    } while (pressed_key != 0x17);

    return selected_entry;
}

void print_videomem_entry(int i) {
    UINTN SizeOfInfo;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
    EFI_STATUS Status;
    Status = uefi_call_wrapper(
        Gop->QueryMode, 4,                          /* service */
        Gop,                                        /* 'this' pointer */
        (UINT32) i,                                 /* mode number */
        &SizeOfInfo,                                /* size of info buffer */
        &Info                                       /* info buffer */
    );
    if (EFI_ERROR(Status)) return;

    /* print mode info */
    Print(L"mode #%03u: %ux%u\n", 
          i,
          Info->HorizontalResolution,
          Info->VerticalResolution
    );
}

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    InitializeLib(ImageHandle, SystemTable);
    ST = SystemTable;
    EFI_STATUS Status;

    /* search GOP protocol */
    EFI_GUID GopGUID = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    Status = uefi_call_wrapper(
        ST->BootServices->LocateProtocol, 3,        /* service */
        &GopGUID,                                   /* GOP's GUID */
        NULL,                                       /* ignorable */
        &Gop                                        /* Gop interface pointer */
    );
    if (EFI_ERROR(Status)) return Status;

    /* create a menu with video modes */
    menu_t menu = {
        .title = (CHAR16 *) L"select a video mode",
        .size = Gop->Mode->MaxMode,
        .height = 20,
        .print_entry = &print_videomem_entry,
    };

    /* display menu and select video mode */
    while (1) {
        Status = uefi_call_wrapper(
            Gop->SetMode, 2,
            Gop,                                    /* GOP interface */
            menu_select(&menu)                      /* video mode index */
        );
        if (EFI_ERROR(Status)) return Status;
    }

    while (1);
    return EFI_SUCCESS;
}
