#include "global.h"
#include "ui_menu.h"
#include "strings.h"
#include "bg.h"
#include "data.h"
#include "decompress.h"
#include "dynamic_placeholder_text_util.h"
#include "event_data.h"
#include "field_weather.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "item.h"
#include "item_menu.h"
#include "item_menu_icons.h"
#include "list_menu.h"
#include "item_icon.h"
#include "item_use.h"
#include "international_string_util.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "menu_helpers.h"
#include "palette.h"
#include "party_menu.h"
#include "scanline_effect.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text_window.h"
#include "overworld.h"
#include "event_data.h"
#include "constants/abilities.h"
#include "constants/items.h"
#include "constants/field_weather.h"
#include "constants/songs.h"
#include "constants/rgb.h"

#include "pokemon_icon.h"
#include "wild_encounter.h"
#include "data/wild_encounters.h"
#include "constants/wild_encounter.h"
#include "data/text/abilities.h"

/*
 * 
 */
 
//==========DEFINES==========//
struct MenuResources
{
    MainCallback savedCallback;     // determines callback to run when we exit. e.g. where do we want to go after closing the menu
    u8 gfxLoadState;

    u16 headerId;

    u8 currPageIndex;
    u8 minPageIndex;
    u8 maxPageIndex;
    u8 currMonIndex;
    u8 minMonIndex;
    u8 maxMonIndex;
    u8 mode;
};

enum Pages
{
    EV_PAGE_LAND,
    EV_PAGE_FISHING,
    EV_PAGE_WATER_ROCK,
    EV_PAGE_COUNT,
};

enum SubMenu
{
    EV_SUBMENU_LEVEL_TOP,
    EV_SUBMENU_LEVEL_1,
    EV_SUBMENU_LEVEL_2,
    EV_SUBMENU_LEVEL_3,
};

enum Labels
{
    EV_LABEL_WINDOW_LAND_TITLE,
    EV_LABEL_WINDOW_FISHING_TITLE,
    EV_LABEL_WINDOW_WATER_ROCK_TITLE,

    // Direction Prompts
    EV_LABEL_WINDOW_PROMPT_DETAILS,
    EV_LABEL_WINDOW_PROMPT_CANCEL,

    //  Box
    EV_LABEL_WINDOW_BOX_OVERVIEW,
    EV_LABEL_WINDOW_BOX_DETAILS,

    // end
    EV_LABEL_END,
};

enum DetailBoxData
{
    EV_BOX_DATA_SPECIES,
    EV_BOX_DATA_LVL,
    EV_BOX_DATA_ENCRATE,
    EV_BOX_DATA_CATCHRATE,
    EV_BOX_DATA_EVYIELD,
    EV_BOX_DATA_ABILITIES,
};

enum EncViewerMode
{
    EV_MODE_DEFAULT,
    EV_MODE_SELECT_MON,
};

enum LandSlots
{
    SLOT_L_1,
    SLOT_L_2,
    SLOT_L_3,
    SLOT_L_4,
    SLOT_L_5,
    SLOT_L_6,
    SLOT_L_7,
    SLOT_L_8,
    SLOT_L_9,
    SLOT_L_10,
    SLOT_L_11,
    SLOT_L_12,
    MAX_LAND_SLOTS,
};

u16 encRateLand[] = 
{
    ENCOUNTER_CHANCE_LAND_MONS_SLOT_0, 
    ENCOUNTER_CHANCE_LAND_MONS_SLOT_1 - ENCOUNTER_CHANCE_LAND_MONS_SLOT_0,
    ENCOUNTER_CHANCE_LAND_MONS_SLOT_2 - ENCOUNTER_CHANCE_LAND_MONS_SLOT_1,
    ENCOUNTER_CHANCE_LAND_MONS_SLOT_3 - ENCOUNTER_CHANCE_LAND_MONS_SLOT_2,
    ENCOUNTER_CHANCE_LAND_MONS_SLOT_4 - ENCOUNTER_CHANCE_LAND_MONS_SLOT_3,
    ENCOUNTER_CHANCE_LAND_MONS_SLOT_5 - ENCOUNTER_CHANCE_LAND_MONS_SLOT_4,
    ENCOUNTER_CHANCE_LAND_MONS_SLOT_6 - ENCOUNTER_CHANCE_LAND_MONS_SLOT_5,
    ENCOUNTER_CHANCE_LAND_MONS_SLOT_7 - ENCOUNTER_CHANCE_LAND_MONS_SLOT_6,
    ENCOUNTER_CHANCE_LAND_MONS_SLOT_8 - ENCOUNTER_CHANCE_LAND_MONS_SLOT_7,
    ENCOUNTER_CHANCE_LAND_MONS_SLOT_9 - ENCOUNTER_CHANCE_LAND_MONS_SLOT_8,
    ENCOUNTER_CHANCE_LAND_MONS_SLOT_10 - ENCOUNTER_CHANCE_LAND_MONS_SLOT_9,
    ENCOUNTER_CHANCE_LAND_MONS_SLOT_11 - ENCOUNTER_CHANCE_LAND_MONS_SLOT_10,
};

#define Y_OFFSET 32
#define X_OFFSET 32

//==========EWRAM==========//
static EWRAM_DATA struct MenuResources *sMenuDataPtr = NULL;
static EWRAM_DATA u8 *sBg1TilemapBuffer = NULL;

//==========STATIC=DEFINES==========//
static void Menu_RunSetup(void);
static bool8 Menu_DoGfxSetup(void);
static bool8 Menu_InitBgs(void);
static void Menu_FadeAndBail(void);
static bool8 Menu_LoadGraphics(void);
static void Menu_InitWindows(void);
static void PrintToWindow(u8 windowId, u8 colorIdx);
static void Task_MenuWaitFadeIn(u8 taskId);
static void Task_MenuMain(u8 taskId);

static void PutPageWindowTilemap(u8 page);
static void ClearPageWindowTilemap(u8 page);
static void PutPageWindowText(u8 page);
static void PutPageWindowIcons(u8 page);
static void PrintTextOnWindow(u8 windowId, const u8 *string, u8 x, u8 y, u8 lineSpacing, u8 colorId);
static void PrintTextOnWindowSmall(u8 windowId, const u8 *string, u8 x, u8 y, u8 lineSpacing, u8 colorId);
static void DrawMonIcon(u16 species, u8 x, u8 y);
static u8 GetMaxMonIndex(u8 page);
static u16 SpeciesByIndex(u8 selection);
static u8 *LevelRangeByIndex(u8 selection);
static u8 *EncRateByIndex(u8 selection);
static u8 *CatchRateBySpecies(u8 species);
static u8 *AbilitiesBySpecies(u8 species);
static const struct WildPokemonInfo *GetWildMonInfo(void);

//==========CONST=DATA==========//
static const struct BgTemplate sMenuBgTemplates[] =
{
    {
        .bg = 0,    // windows, etc
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .priority = 1
    }, 
    {
        .bg = 1,    // this bg loads the UI tilemap
        .charBaseIndex = 3,
        .mapBaseIndex = 30,
        .priority = 2
    },
    {
        .bg = 2,    // this bg loads the UI tilemap
        .charBaseIndex = 0,
        .mapBaseIndex = 28,
        .priority = 0
    }
};

static const struct WindowTemplate sMenuWindowTemplates[] = 
{
    [EV_LABEL_WINDOW_LAND_TITLE] = 
    {
        .bg = 0,            // which bg to print text on
        .tilemapLeft = 0,   // position from left (per 8 pixels)
        .tilemapTop = 0,    // position from top (per 8 pixels)
        .width = 6,        // width (per 8 pixels)
        .height = 2,        // height (per 8 pixels)
        .paletteNum = 15,   // palette index to use for text
        .baseBlock = 1,     // tile start in VRAM
    },
    [EV_LABEL_WINDOW_FISHING_TITLE] = 
    {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 6,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 1 + 12,
    },
    [EV_LABEL_WINDOW_WATER_ROCK_TITLE] = 
    {
        .bg = 0,            
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 6,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 1 + 12 + 12,
    },
    [EV_LABEL_WINDOW_PROMPT_DETAILS] = 
    {
        .bg = 0,            
        .tilemapLeft = 22,
        .tilemapTop = 0,
        .width = 8,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 1 + 12 + 12 + 12,
    },
    [EV_LABEL_WINDOW_PROMPT_CANCEL] = 
    {
        .bg = 0,            
        .tilemapLeft = 18,
        .tilemapTop = 0,
        .width = 8,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 1 + 12 + 12 + 12 + 16,
    },
    [EV_LABEL_WINDOW_BOX_OVERVIEW] = 
    {
        .bg = 0,            
        .tilemapLeft = 1,
        .tilemapTop = 3,
        .width = 12,
        .height = 16,
        .paletteNum = 15,
        .baseBlock = 1 + 12 + 12 + 12 + 16 + 16,
    },
    [EV_LABEL_WINDOW_BOX_DETAILS] = 
    {
        .bg = 0,            
        .tilemapLeft = 13,
        .tilemapTop = 3,
        .width = 16,
        .height = 16,
        .paletteNum = 15,
        .baseBlock = 1 + 12 + 12 + 12 + 16 + 16 + 192,
    },
}; 
#define LABEL_END 1 + 12 + 12 + 12 + 16 + 16 + 192 + 256

static const struct WindowTemplate sDetailBoxTemplate[] =
{
    [EV_BOX_DATA_SPECIES] = 
    {
        .bg = 0,
        .tilemapLeft = 19,
        .tilemapTop = 3,
        .width = 8,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = LABEL_END,
    },
    [EV_BOX_DATA_LVL] = 
    {
        .bg = 0,
        .tilemapLeft = 19,
        .tilemapTop = 6,
        .width = 8,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = LABEL_END + 16,
    },
    [EV_BOX_DATA_ENCRATE] = 
    {
        .bg = 0,
        .tilemapLeft = 14,
        .tilemapTop = 9,
        .width = 6,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = LABEL_END + 16 + 12,
    },
    [EV_BOX_DATA_CATCHRATE] = 
    {
        .bg = 0,
        .tilemapLeft = 22,
        .tilemapTop = 8,
        .width = 6,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = LABEL_END + 16 + 12 + 12,
    },
    [EV_BOX_DATA_EVYIELD] = 
    {
        .bg = 0,
        .tilemapLeft = 14,
        .tilemapTop = 12,
        .width = 6,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = LABEL_END + 16 + 12 + 12 + 12 ,
    },
    [EV_BOX_DATA_ABILITIES] = 
    {
        .bg = 0,
        .tilemapLeft = 22,
        .tilemapTop = 12,
        .width = 6,
        .height = 6,
        .paletteNum = 15,
        .baseBlock = LABEL_END + 16 + 12 + 12 + 12 + 12,
    }, 
}; 
#define EV_BOX_DATA_END LABEL_END + 16 + 12 + 12 + 12 + 12 + 36

static const u32 sMenuTiles[] = INCBIN_U32("graphics/ui_menu/tiles.4bpp.lz");
static const u32 sMenuTilemap[] = INCBIN_U32("graphics/ui_menu/tilemap.bin.lz");
static const u16 sMenuPalette[] = INCBIN_U16("graphics/ui_menu/palette.gbapal");

enum Colors
{
    FONT_BLACK,
    FONT_WHITE,
    FONT_RED,
    FONT_BLUE,
};

static const u8 sMenuWindowFontColors[][3] = 
{
    [FONT_BLACK]  = {TEXT_COLOR_TRANSPARENT,  TEXT_COLOR_DARK_GRAY,  TEXT_COLOR_LIGHT_GRAY},
    [FONT_WHITE]  = {TEXT_COLOR_TRANSPARENT,  TEXT_COLOR_WHITE,  TEXT_COLOR_DARK_GRAY},
    [FONT_RED]   = {TEXT_COLOR_TRANSPARENT,  TEXT_COLOR_RED,        TEXT_COLOR_LIGHT_GRAY},
    [FONT_BLUE]  = {TEXT_COLOR_TRANSPARENT,  TEXT_COLOR_BLUE,       TEXT_COLOR_LIGHT_GRAY},
};

static const u8 dash[] = _(" - ");
static const u8 sText_AbilitiesDynamic[] = _("{DYNAMIC 0}\n{DYNAMIC 1}\n{DYNAMIC 2}");

//==========FUNCTIONS==========//
// UI loader template
void Task_OpenMenuFromStartMenu(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    if (!gPaletteFade.active)
    {
        CleanupOverworldWindowsAndTilemaps();
        Menu_Init(CB2_ReturnToFieldWithOpenMenu);
        DestroyTask(taskId);
    }
}

// This is our main initialization function if you want to call the menu from elsewhere
void Menu_Init(MainCallback callback)
{
    if ((sMenuDataPtr = AllocZeroed(sizeof(struct MenuResources))) == NULL)
    {
        SetMainCallback2(callback);
        return;
    }
    
    // initialize stuff
    sMenuDataPtr->gfxLoadState = 0;
    sMenuDataPtr->savedCallback = callback;
    sMenuDataPtr->mode = EV_MODE_DEFAULT;
    sMenuDataPtr->minPageIndex = EV_PAGE_LAND;
    sMenuDataPtr->maxPageIndex = EV_PAGE_WATER_ROCK;
    sMenuDataPtr->currPageIndex = EV_PAGE_LAND;
    sMenuDataPtr->minMonIndex = 0;
    sMenuDataPtr->maxMonIndex = 12; //GetMaxMonIndex(sMenuDataPtr->currPageIndex);
    sMenuDataPtr->currMonIndex = 0;
    sMenuDataPtr->headerId = GetCurrentMapWildMonHeaderId();
    
    SetMainCallback2(Menu_RunSetup);
}

static void Menu_RunSetup(void)
{
    while (1)
    {
        if (Menu_DoGfxSetup() == TRUE)
            break;
    }
}

static void Menu_MainCB(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static void Menu_VBlankCB(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static bool8 Menu_DoGfxSetup(void)
{
    u8 taskId;
    switch (gMain.state)
    {
    case 0:
        DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000)
        SetVBlankHBlankCallbacksToNull();
        ClearScheduledBgCopiesToVram();
        gMain.state++;
        break;
    case 1:
        ScanlineEffect_Stop();
        FreeAllSpritePalettes();
        ResetPaletteFade();
        ResetSpriteData();
        ResetTasks();
        gMain.state++;
        break;
    case 2:
        if (Menu_InitBgs())
        {
            sMenuDataPtr->gfxLoadState = 0;
            gMain.state++;
        }
        else
        {
            Menu_FadeAndBail();
            return TRUE;
        }
        break;
    case 3:
        if (Menu_LoadGraphics() == TRUE)
            gMain.state++;
        break;
    case 4:
        LoadMessageBoxAndBorderGfx();
        Menu_InitWindows();
        gMain.state++;
        break;
    case 5:
        PutPageWindowText(sMenuDataPtr->currPageIndex);
        gMain.state++;
        break;
    case 6:
        PutPageWindowIcons(sMenuDataPtr->currPageIndex);
        gMain.state++;
        break;
    case 7:
        //PrintToWindow(EV_LABEL_WINDOW_LAND_TITLE, FONT_WHITE);
        PutPageWindowTilemap(sMenuDataPtr->currPageIndex);
        taskId = CreateTask(Task_MenuWaitFadeIn, 0);
        BlendPalettes(0xFFFFFFFF, 16, RGB_BLACK);
        gMain.state++;
        break;
    case 8:
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
        gMain.state++;
        break;
    default:
        SetVBlankCallback(Menu_VBlankCB);
        SetMainCallback2(Menu_MainCB);
        return TRUE;
    }
    return FALSE;
}

#define try_free(ptr) ({        \
    void ** ptr__ = (void **)&(ptr);   \
    if (*ptr__ != NULL)                \
        Free(*ptr__);                  \
})

static void Menu_FreeResources(void)
{
    try_free(sMenuDataPtr);
    try_free(sBg1TilemapBuffer);
    FreeAllWindowBuffers();
}


static void Task_MenuWaitFadeAndBail(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        SetMainCallback2(sMenuDataPtr->savedCallback);
        Menu_FreeResources();
        DestroyTask(taskId);
    }
}

static void Menu_FadeAndBail(void)
{
    BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
    CreateTask(Task_MenuWaitFadeAndBail, 0);
    SetVBlankCallback(Menu_VBlankCB);
    SetMainCallback2(Menu_MainCB);
}

static bool8 Menu_InitBgs(void)
{
    ResetAllBgsCoordinates();
    sBg1TilemapBuffer = Alloc(0x800);
    if (sBg1TilemapBuffer == NULL)
        return FALSE;
    
    memset(sBg1TilemapBuffer, 0, 0x800);
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sMenuBgTemplates, NELEMS(sMenuBgTemplates));
    SetBgTilemapBuffer(1, sBg1TilemapBuffer);
    ScheduleBgCopyTilemapToVram(1);
    ShowBg(0);
    ShowBg(1);
    ShowBg(2);
    return TRUE;
}

static bool8 Menu_LoadGraphics(void)
{
    switch (sMenuDataPtr->gfxLoadState)
    {
    case 0:
        ResetTempTileDataBuffers();
        DecompressAndCopyTileDataToVram(1, sMenuTiles, 0, 0, 0);
        sMenuDataPtr->gfxLoadState++;
        break;
    case 1:
        if (FreeTempTileDataBuffersIfPossible() != TRUE)
        {
            LZDecompressWram(sMenuTilemap, sBg1TilemapBuffer);
            sMenuDataPtr->gfxLoadState++;
        }
        break;
    case 2:
        LoadPalette(sMenuPalette, 0, 32);
        sMenuDataPtr->gfxLoadState++;
        break;
    default:
        sMenuDataPtr->gfxLoadState = 0;
        return TRUE;
    }
    return FALSE;
}

static void Menu_InitWindows(void)
{
    u32 i;

    InitWindows(sMenuWindowTemplates);
    DeactivateAllTextPrinters();
    ScheduleBgCopyTilemapToVram(0);

    for (i = 0; i < EV_LABEL_END; i++)
    {
        FillWindowPixelBuffer(i, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));
        PutWindowTilemap(i);
        CopyWindowToVram(i, 3);
    }
    ScheduleBgCopyTilemapToVram(2);
}

const u8 pokemonNames[MAX_LAND_SLOTS][POKEMON_NAME_LENGTH + 1] = 
{
    [SLOT_L_1] = _("Charmander"),
    [SLOT_L_2] = _("Bulbasaur"),
    [SLOT_L_3] = _("Togepi"),
    [SLOT_L_4] = _("Voltorb"),
    [SLOT_L_5] = _("Bla"),
    [SLOT_L_6] = _("Floo"),
    [SLOT_L_7] = _("Fooo"),
    [SLOT_L_8] = _("Charizard"),
    [SLOT_L_9] = _("Venoshock"),
    [SLOT_L_10] = _("Chmander"),
    [SLOT_L_11] = _("Charnder"),
    [SLOT_L_12] = _("Charmar"),
};

static const u8 sText_Species[] = _("Species:");
static const u8 sText_Level[] = _("LVL:");
static const u8 sText_EncRate[] = _("Enc. Rate:");
static const u8 sText_CatchRate[] = _("Catch Rate:");
static const u8 sText_EVYield[] = _("EV Yield:");
static const u8 sText_Abilities[] = _("Abilities");

static void PutPageWindowText(u8 page)
{
    u8 i;

    switch(page)
    {
        case EV_PAGE_LAND:
            PrintTextOnWindowSmall(EV_LABEL_WINDOW_BOX_DETAILS, sText_Species,   6*8,    0*8 + 1,    0, FONT_WHITE);
            PrintTextOnWindowSmall(EV_LABEL_WINDOW_BOX_DETAILS, sText_Level,     6*8,    2*8,        0, FONT_WHITE);
            PrintTextOnWindowSmall(EV_LABEL_WINDOW_BOX_DETAILS, sText_EncRate,   1*8,    7*8,        0, FONT_WHITE);
            PrintTextOnWindowSmall(EV_LABEL_WINDOW_BOX_DETAILS, sText_CatchRate, 10*8,   7*8,        0, FONT_WHITE);
            PrintTextOnWindowSmall(EV_LABEL_WINDOW_BOX_DETAILS, sText_EVYield,   1*8,    10*8,       0, FONT_WHITE);
            PrintTextOnWindowSmall(EV_LABEL_WINDOW_BOX_DETAILS, sText_Abilities, 10*8,   10*8,       0, FONT_WHITE);
            break;
        case EV_PAGE_FISHING:
            PrintToWindow(page, FONT_WHITE);
            break;
        case EV_PAGE_WATER_ROCK:
            PrintToWindow(page, FONT_WHITE);
            break;
    }
}

static void PutPageMonDataText(u8 page)
{
    u8 selection = sMenuDataPtr->currMonIndex;
    u16 species = SpeciesByIndex(selection);

    switch (page)
    {
        case EV_PAGE_LAND:
            PrintTextOnWindowSmall(EV_BOX_DATA_SPECIES, gSpeciesNames[species], 0, 0, 0, FONT_WHITE);
            PrintTextOnWindowSmall(EV_BOX_DATA_LVL, LevelRangeByIndex(selection), 0, 0, 0, FONT_WHITE);
            PrintTextOnWindowSmall(EV_BOX_DATA_ENCRATE, EncRateByIndex(selection), 0, 0, 0, FONT_WHITE);
            PrintTextOnWindowSmall(EV_BOX_DATA_CATCHRATE, CatchRateBySpecies(species), 0, 0, 0, FONT_WHITE);
            PrintTextOnWindowSmall(EV_BOX_DATA_ABILITIES, gSpeciesNames[species], 0, 0, 0, FONT_WHITE);
        case EV_PAGE_FISHING:

        case EV_PAGE_WATER_ROCK:
    }
}

static void PutPageWindowTilemap(u8 page)
{
    ClearWindowTilemap(EV_LABEL_WINDOW_LAND_TITLE);
    ClearWindowTilemap(EV_LABEL_WINDOW_FISHING_TITLE);
    ClearWindowTilemap(EV_LABEL_WINDOW_WATER_ROCK_TITLE);

    switch (page)
    {
        case EV_PAGE_LAND:
            PutWindowTilemap(EV_LABEL_WINDOW_LAND_TITLE);
            CopyWindowToVram(EV_LABEL_WINDOW_LAND_TITLE, 3);
            break;
        case EV_PAGE_FISHING:
            PutWindowTilemap(EV_LABEL_WINDOW_FISHING_TITLE);
            CopyWindowToVram(EV_LABEL_WINDOW_FISHING_TITLE, 3);
            break;
        case EV_PAGE_WATER_ROCK:
            PutWindowTilemap(EV_LABEL_WINDOW_WATER_ROCK_TITLE);
            CopyWindowToVram(EV_LABEL_WINDOW_WATER_ROCK_TITLE, 3);
            break;
    }
    
}

#define OW_BOX_X 8 + 16
#define OW_BOX_Y 24 + 16
static void PutPageWindowIcons(u8 page)
{
    u8 i;
    switch (page)
    {
        case EV_PAGE_LAND:
            const struct WildPokemonInfo *LandMons =  gWildMonHeaders[sMenuDataPtr->headerId].landMonsInfo;
            for (i = 0; i < sMenuDataPtr->maxMonIndex; i++)
                DrawMonIcon(LandMons->wildPokemon[i].species, (i%3) * X_OFFSET + OW_BOX_X, (i/3) * Y_OFFSET + OW_BOX_Y);
                break;
        case EV_PAGE_FISHING:
            break;
        case EV_PAGE_WATER_ROCK:
            break;
    }
}

static void ClearPageWindowTilemap(u8 page)
{
    u8 i;

    switch (page)
    {
        case EV_PAGE_LAND:
            ClearWindowTilemap(EV_LABEL_WINDOW_LAND_TITLE);
            break;
        case EV_PAGE_FISHING:
            ClearWindowTilemap(EV_LABEL_WINDOW_FISHING_TITLE);
            break;
        case EV_PAGE_WATER_ROCK:
            ClearWindowTilemap(EV_LABEL_WINDOW_WATER_ROCK_TITLE);
            break;
    }
}

static void PrintTextOnWindow(u8 windowId, const u8 *string, u8 x, u8 y, u8 lineSpacing, u8 colorId)
{
    AddTextPrinterParameterized4(windowId, FONT_NORMAL, x, y, 0, lineSpacing, sMenuWindowFontColors[colorId], 0, string);
}

static void PrintTextOnWindowSmall(u8 windowId, const u8 *string, u8 x, u8 y, u8 lineSpacing, u8 colorId)
{
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, x, y, 0, lineSpacing, sMenuWindowFontColors[colorId], 0, string);
}

static const u8 sText_MyMenu[] = _("My Menu");
static void PrintToWindow(u8 windowId, u8 colorIdx)
{
    const u8 *str = sText_MyMenu;
    u8 x = 1;
    u8 y = 1;
    
    FillWindowPixelBuffer(windowId, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));
    AddTextPrinterParameterized4(windowId, 1, x, y, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, str);
    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, 3);
}

static void Task_MenuWaitFadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_MenuMain;
}

static void Task_MenuTurnOff(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (!gPaletteFade.active)
    {
        SetMainCallback2(sMenuDataPtr->savedCallback);
        Menu_FreeResources();
        DestroyTask(taskId);
    }
}


/* This is the meat of the UI. This is where you wait for player inputs and can branch to other tasks accordingly */
static void Task_MenuMain(u8 taskId)
{
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_PC_OFF);
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_MenuTurnOff;
    }
}

static void DrawMonIcon(u16 species, u8 x, u8 y)
{

    LoadMonIconPalette(species);
    CreateMonIconNoPersonality(species, SpriteCallbackDummy, x, y, 1, 0);
}

static u8 GetMaxMonIndex(u8 page)
{
    switch (page)
    {
        case EV_PAGE_LAND:
            return LAND_WILD_COUNT;
        case EV_PAGE_FISHING:
            return FISH_WILD_COUNT;
        case EV_PAGE_WATER_ROCK:
            return WATER_WILD_COUNT + ROCK_WILD_COUNT;
    }
}

static u16 SpeciesByIndex(u8 selection)
{
    return GetWildMonInfo()->wildPokemon[selection].species;
}

static u8* LevelRangeByIndex(u8 selection)
{
    u8 minLevel, maxLevel;
    
    minLevel = GetWildMonInfo()->wildPokemon[selection].minLevel;
    maxLevel = GetWildMonInfo()->wildPokemon[selection].maxLevel;
    ConvertIntToDecimalStringN(gStringVar1, minLevel, STR_CONV_MODE_RIGHT_ALIGN, 3);
    ConvertIntToDecimalStringN(gStringVar2, maxLevel, STR_CONV_MODE_RIGHT_ALIGN, 3);
    StringAppend(gStringVar1, dash);
    StringAppend(gStringVar1, gStringVar2);

    return gStringVar1;
}

static u8* EncRateByIndex(u8 selection)
{
    ConvertIntToDecimalStringN(gStringVar1, encRateLand[selection], STR_CONV_MODE_RIGHT_ALIGN, 2);
    return gStringVar1;
}

static u8* CatchRateBySpecies(u8 species)
{
    ConvertIntToDecimalStringN(gStringVar1, gSpeciesInfo[species].catchRate, STR_CONV_MODE_RIGHT_ALIGN, 3);
    return gStringVar1;
}

static u8* AbilitiesBySpecies(u8 species)
{
    StringCopy(gStringVar1, gAbilityNames[gSpeciesInfo[species].abilities[0]]);
    StringCopy(gStringVar2, gAbilityNames[gSpeciesInfo[species].abilities[1]]);
    StringCopy(gStringVar3, gAbilityNames[gSpeciesInfo[species].abilities[2]]);

    DynamicPlaceholderTextUtil_Reset();
    DynamicPlaceholderTextUtil_SetPlaceholderPtr(0, gStringVar1);
    DynamicPlaceholderTextUtil_SetPlaceholderPtr(1, gStringVar2);
    DynamicPlaceholderTextUtil_SetPlaceholderPtr(2, gStringVar3);
    DynamicPlaceholderTextUtil_ExpandPlaceholders(gStringVar4, sText_AbilitiesDynamic);

    return gStringVar4;
}

static const struct WildPokemonInfo* GetWildMonInfo(void)
{

    switch(sMenuDataPtr->currPageIndex)
    {
        case EV_PAGE_LAND:
            return gWildMonHeaders[sMenuDataPtr->headerId].landMonsInfo;
        case EV_PAGE_FISHING:
            return gWildMonHeaders[sMenuDataPtr->headerId].fishingMonsInfo;
        case EV_PAGE_WATER_ROCK:
            return gWildMonHeaders[sMenuDataPtr->headerId].rockSmashMonsInfo;
    }
}
