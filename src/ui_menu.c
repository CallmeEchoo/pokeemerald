#include "global.h"
#include "ui_menu.h"
#include "strings.h"
#include "battle.h"
#include "battle_anim.h"
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
#include "pokeball.h"
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
#include "constants/wild_encounter.h"
//#include "data/text/abilities.h"
//#include "data/wild_encounters.h"

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

enum Windows
{
    //EV_LABEL_WINDOW_LAND_TITLE,
    //EV_LABEL_WINDOW_FISHING_TITLE,
    //EV_LABEL_WINDOW_WATER_ROCK_TITLE,

    // Direction Prompts
    //EV_LABEL_WINDOW_PROMPT_DETAILS,
    //EV_LABEL_WINDOW_PROMPT_CANCEL,

    // Box
    //EV_LABEL_WINDOW_BOX_OVERVIEW,
    EV_LABEL_WINDOW_BOX_LEVEL,
    EV_LABEL_WINDOW_BOX_CATCHRATE,
    EV_LABEL_WINDOW_BOX_STATS,
    EV_LABEL_WINDOW_BOX_STATS_ROW,
    //EV_LABEL_WINDOW_BOX_ABILITIES,

    // Data
    //EV_DATA_WINDOW_BOX_SPECIES,
    //EV_DATA_WINDOW_BOX_LEVEL,
    //EV_DATA_WINDOW_BOX_CATCHRATE,
    //EV_DATA_WINDOW_BOX_BASE_STATS,
    //EV_DATA_WINDOW_BOX_EV_YIELD,
    //EV_DATA_WINDOW_BOX_ABILITIES,
    //EV_DATA_WINDOW_BOX_ENCRATE,

    // end
    EV_WINDOW_END,
};

enum EncViewerMode
{
    EV_MODE_DEFAULT,
    EV_MODE_SELECT_MON,
};

static const u8 encRateLand[12] = {20,20,10,10,10,10,5,5,4,4,1,1};

#define Y_OFFSET 32
#define X_OFFSET 32

#define COMMON 0
#define RARE 1

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
static void PutPageMonDataText(u8 page);
static void PutPageWindowIcons(u8 page);
static void PutPageWindowSprites(u8 page);
static void PrintTextOnWindow(u8 windowId, const u8 *string, u8 x, u8 y, u8 lineSpacing, u8 colorId);
static void PrintTextOnWindowSmall(u8 windowId, const u8 *string, u8 x, u8 y, u8 lineSpacing, u8 colorId);
static void DrawMonIcon(u16 species, u8 x, u8 y);
static u8 DrawMonSprite(u16 species, u8 x, u8 y);
static void DrawItemSprites(u16 itemId, u8 rarity, u8 x, u8 y, u8 subpriority);
static void DrawPokeballSprite(u8 x, u8 y, u8 subpriority);
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
        .priority = 0,
    }, 
    {
        .bg = 1,    // this bg loads the UI tilemap
        .charBaseIndex = 3,
        .mapBaseIndex = 28,
        .priority = 1,
    }
};

static const struct WindowTemplate sMenuWindowTemplates[] = 
{
    [EV_LABEL_WINDOW_BOX_LEVEL] = 
    {
        .bg = 0,            // which bg to print text on
        .tilemapLeft = 22,   // position from left (per 8 pixels)
        .tilemapTop = 6,    // position from top (per 8 pixels)
        .width = 4,        // width (per 8 pixels)
        .height = 2,        // height (per 8 pixels)
        .paletteNum = 15,   // palette index to use for text
        .baseBlock = 1,     // tile start in VRAM
    },
    [EV_LABEL_WINDOW_BOX_CATCHRATE] = 
    {
        .bg = 0,
        .tilemapLeft = 22,
        .tilemapTop = 8,
        .width = 6,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 1 + 8,
    },
    [EV_LABEL_WINDOW_BOX_STATS] = 
    {
        .bg = 0,
        .tilemapLeft = 13,
        .tilemapTop = 11,
        .width = 3,
        .height = 8,
        .paletteNum = 15,
        .baseBlock = 1 + 8 + 12,
    },
    [EV_LABEL_WINDOW_BOX_STATS_ROW] = 
    {
        .bg = 0,
        .tilemapLeft = 13,
        .tilemapTop = 10,
        .width = 8,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 1 + 8 + 12 + 24,
    },
}; 

static const struct OamData sOamData_FrontPic = 
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = 0,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x64),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(64x64),
    .tileNum = 0,
    .priority = 1,
    .paletteNum = 0,
    .affineParam = 0,
};

static const struct SpriteTemplate sSpriteTemplate_FrontPic = 
{
    .tileTag = TAG_NONE,
    .paletteTag = TAG_NONE,
    .oam = &sOamData_FrontPic,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

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
        PutPageMonDataText(sMenuDataPtr->currPageIndex);
        gMain.state++;
        break;
    case 7:
        PutPageWindowIcons(sMenuDataPtr->currPageIndex);
        gMain.state++;
        break;
    case 8:
        PutPageWindowSprites(sMenuDataPtr->currPageIndex);
        gMain.state++;
        break;
    case 9:
        PutPageWindowTilemap(sMenuDataPtr->currPageIndex);
        taskId = CreateTask(Task_MenuWaitFadeIn, 0);
        BlendPalettes(0xFFFFFFFF, 16, RGB_BLACK);
        gMain.state++;
        break;
    case 10:
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

    for (i = 0; i < EV_WINDOW_END; i++)
    {
        FillWindowPixelBuffer(i, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));
        PutWindowTilemap(i);
        CopyWindowToVram(i, 3);
    }
    //ScheduleBgCopyTilemapToVram(2);
}

static const u8 sText_Level[] = _("LVL:");
static const u8 sText_CatchRate[] = _("CatchRate:");
static const u8 sText_Abilities[] = _("Abilities");

static const u8 sText_Land[] = _("Land");
static const u8 sText_WaterRock[] = _("Water\nRock");
static const u8 sText_Fish[] = _("Fishing");

static const u8 sText_HP[]   = _("HP");
static const u8 sText_ATK[]  = _("ATK");
static const u8 sText_DEF[]  = _("DEF");
static const u8 sText_SATK[] = _("SAT");
static const u8 sText_SDEF[] = _("SDF");
static const u8 sText_SPD[]  = _("SPD");

static const u8 sText_Base[] = _("Base");
static const u8 sText_EVs[] = _("EVs");

static void PutPageWindowText(u8 page)
{
    switch(page)
    {
        case EV_PAGE_LAND:
            PrintTextOnWindowSmall(EV_LABEL_WINDOW_BOX_LEVEL, sText_Level, 0, 0, 0, FONT_WHITE);
            PrintTextOnWindowSmall(EV_LABEL_WINDOW_BOX_CATCHRATE, sText_CatchRate, 0, 0, 0, FONT_WHITE);

            PrintTextOnWindowSmall(EV_LABEL_WINDOW_BOX_STATS, sText_HP, 3, 5, 0, FONT_WHITE);
            PrintTextOnWindowSmall(EV_LABEL_WINDOW_BOX_STATS, sText_ATK, 3, 14, 0, FONT_WHITE);
            PrintTextOnWindowSmall(EV_LABEL_WINDOW_BOX_STATS, sText_DEF, 3, 23, 0, FONT_WHITE);
            PrintTextOnWindowSmall(EV_LABEL_WINDOW_BOX_STATS, sText_SATK, 3, 32, 0, FONT_WHITE);
            PrintTextOnWindowSmall(EV_LABEL_WINDOW_BOX_STATS, sText_SDEF, 3, 41, 0, FONT_WHITE);
            PrintTextOnWindowSmall(EV_LABEL_WINDOW_BOX_STATS, sText_SPD, 3, 50, 0, FONT_WHITE);

            PrintTextOnWindowSmall(EV_LABEL_WINDOW_BOX_STATS_ROW, sText_Base, 20, 4, 0, FONT_WHITE);
            PrintTextOnWindowSmall(EV_LABEL_WINDOW_BOX_STATS_ROW, sText_EVs, 44, 4, 0, FONT_WHITE);
            break;
        case EV_PAGE_FISHING:
            //stub
            break;
        case EV_PAGE_WATER_ROCK:
            //stub
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
            //stub
            break; 
        case EV_PAGE_FISHING:
            //stub
            break;
        case EV_PAGE_WATER_ROCK:
            //stub
            break;
    }
}

static void PutPageWindowTilemap(u8 page)
{
    switch (page)
    {
        case EV_PAGE_LAND:
            //stub
            break;
        case EV_PAGE_FISHING:
            //stub
            break;
        case EV_PAGE_WATER_ROCK:
            //stub
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
            //stub
            break;
        case EV_PAGE_WATER_ROCK:
            //stub
            break;
    }
}

static void PutPageWindowSprites(u8 page)
{
    u8 spriteId;
    u8 selection = sMenuDataPtr->currMonIndex;
    u16 species = SpeciesByIndex(selection);
    u16 itemCommon = gSpeciesInfo[species].itemCommon;
    u16 itemRare = gSpeciesInfo[species].itemRare;

    switch(page)
    {
        case EV_PAGE_LAND:
            DrawMonSprite(species, 138, 57);
            DrawItemSprites(itemCommon, COMMON, 188, 92, 0);
            DrawItemSprites(itemRare, RARE, 212, 92, 0);
            DrawPokeballSprite(10, 10, 0);
            break;
        case EV_PAGE_FISHING:
            //stub
            break;
        case EV_PAGE_WATER_ROCK:
            //stub
            break;
    }
}

static void ClearPageWindowTilemap(u8 page)
{
    u8 i;

    switch (page)
    {
        case EV_PAGE_LAND:
            //stub
            break;
        case EV_PAGE_FISHING:
            //stub
            break;
        case EV_PAGE_WATER_ROCK:
            //stub
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

const static struct CompressedSpritePalette *GetMonSpritePal(u16 species)
{
    return &gMonPaletteTable[species];
}

const static struct CompressedSpriteSheet *GetMonSpriteSheet(u16 species)
{
    return &gMonFrontPicTable[species];
}

static void DrawMonIcon(u16 species, u8 x, u8 y)
{
    LoadMonIconPalette(species);
    CreateMonIconNoPersonality(species, SpriteCallbackDummy, x, y, 1, 0);
}

static u8 DrawMonSprite(u16 species, u8 x, u8 y)
{
    u8 spriteId;
    const struct CompressedSpritePalette *pal = GetMonSpritePal(species);
    const struct CompressedSpriteSheet *sheet = GetMonSpriteSheet(species);
    struct SpriteTemplate spriteTemplate = sSpriteTemplate_FrontPic;
    LoadCompressedSpritePalette(pal);
    LoadCompressedSpriteSheet(sheet);
    spriteTemplate.paletteTag = pal->tag;
    spriteTemplate.tileTag = sheet->tag;
    spriteId = CreateSprite(&spriteTemplate, x, y, 0);
    return spriteId;
}

#define TAG_ITEM_ICON 5000
static void DrawItemSprites(u16 itemId, u8 rarity, u8 x, u8 y, u8 subpriority)
{
    AddItemIconSpriteAt(TAG_ITEM_ICON + rarity, TAG_ITEM_ICON + rarity, itemId, x, y, subpriority);
}

static void DrawPokeballSprite(u8 x, u8 y, u8 subpriority)
{
    u8 ball = ItemIdToBallId(ITEM_POKE_BALL);

    LoadBallGfx(ball);
    u8 spriteId = CreateSprite(&gBallSpriteTemplates[ball], x, y, subpriority);
    gSprites[spriteId].callback = SpriteCallbackDummy;
    gSprites[spriteId].oam.priority = 0;
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

    if (minLevel <= 9)
        ConvertIntToDecimalStringN(gStringVar1, minLevel, STR_CONV_MODE_RIGHT_ALIGN, 1);
    else if (minLevel <= 99)
        ConvertIntToDecimalStringN(gStringVar1, minLevel, STR_CONV_MODE_RIGHT_ALIGN, 2);
    else
        ConvertIntToDecimalStringN(gStringVar1, minLevel, STR_CONV_MODE_RIGHT_ALIGN, 3);

    if (maxLevel <= 9)
        ConvertIntToDecimalStringN(gStringVar2, maxLevel, STR_CONV_MODE_RIGHT_ALIGN, 1);
    else if (maxLevel <= 99)
        ConvertIntToDecimalStringN(gStringVar2, maxLevel, STR_CONV_MODE_RIGHT_ALIGN, 2);
    else
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
