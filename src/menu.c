/*   video_menu.c   */
/*
 * a program that outputs colors into the screen.
 */
#include <efi.h>
#include <efilib.h>

CHAR16 *entries[] = {
    L"entry #00",
    L"entry #01",
    L"entry #02",
    L"entry #03",
    L"entry #04",
    L"entry #05",
    L"entry #06",
    L"entry #07",
    L"entry #08",
    L"entry #09",
    L"entry #10",
    L"entry #11",
    L"entry #12",
    L"entry #13",
    L"entry #14",
    L"entry #15",
    L"entry #16",
    L"entry #17",
    L"entry #18",
    L"entry #19",
    L"entry #20",
    L"entry #21",
    L"entry #22",
    L"entry #23",
    L"entry #24",
    L"entry #25",
    L"entry #26",
    L"entry #27",
    L"entry #28",
    L"entry #29",
    L"entry #30",
    L"entry #31",
    L"entry #32",
    L"entry #33",
};

typedef struct {
    CHAR16          *title;                 /* menu title */
    CHAR16          **entries;              /* menu entries */
    size_t          size;                   /* number of entries */
    size_t          height;                 /* height of the menu */
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
            Print(L"%s\n", menu->entries[entry]);
        }

        /* get next key (blocking) */
        pressed_key = read_key();
    } while (pressed_key != 0x17);

    return selected_entry;
}

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    InitializeLib(ImageHandle, SystemTable);
    ST = SystemTable;
    EFI_STATUS Status;

    menu_t menu = {
        .title = (CHAR16 *) L"menu title",
        .entries = entries,
        .size = 34,
        .height = 20,
    };
    Print(L"selected entry: %u", menu_select(&menu));
    while (1);

    return EFI_SUCCESS;
}
