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
#include "pokemon.h"
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
#define Y_OFFSET 32
#define X_OFFSET 32

#define COMMON 0
#define RARE 1

#define ABILITY_1 0
#define ABILITY_2 1
#define ABILITY_HIDDEN 2

#define TAG_MOVE_TYPES 5000
#define TAG_ITEM_ICON 5001 // 5002 for rare

#define MAX_MON_COUNT 12
#define TYPE_ICON_SPRITE_COUNT 2

enum SpriteArrIds
{
    SPRITE_ARR_ID_MON_ICONS = MAX_MON_COUNT, // for max 12 land mon icons
    SPRITE_ARR_ID_MON_FRONT,
    SPRITE_ARR_ID_ITEM_COMMON,
    SPRITE_ARR_ID_ITEM_RARE,
    SPRITE_ARR_ID_POKEBALL,
    SPRITE_ARR_ID_MON_TYPE_1,
    SPRITE_ARR_ID_MON_TYPE_2,
    SPRITE_ARR_ID_HAND_CURSOR,
    SPRITE_ARR_ID_HAND_SHADOW,
    SPRITE_ARR_ID_COUNT,
};

#define PALTAG_CURSOR_1 SPRITE_ARR_ID_HAND_CURSOR - 7
#define PALTAG_CURSOR_2 SPRITE_ARR_ID_HAND_SHADOW - 7

#define MAX_UNIQUE_POKEMON 12

enum HandCursorTags
{
    GFXTAG_CURSOR = 3000,
    GFXTAG_CURSOR_SHADOW,
};

enum HandCursorAnim
{
    CURSOR_ANIM_BOUNCE,
    CURSOR_ANIM_STILL,
    CURSOR_ANIM_OPEN,
    CURSOR_ANIM_FIST,
};

struct WildPokemonData
{
    u16 species;
    u8 types[2];
    u16 abilities[2];
    u8 baseHP;
    u8 baseAttack;
    u8 baseDefense;
    u8 baseSpAttack;
    u8 baseSpDefense;
    u8 baseSpeed;
    u16 baseBST;
    u8 evYield_HP;
    u8 evYield_Attack;
    u8 evYield_Defense;
    u8 evYield_SpAttack;
    u8 evYield_SpDefense;
    u8 evYield_Speed;
    u8 catchRate;
    u16 itemCommon;
    u16 itemRare;
};

struct WildPokemonUnique
{
    u8 minLevel;
    u8 maxLevel;
    struct WildPokemonData wildMonData;
    u8 encChance;
};

struct MenuResources
{
    MainCallback savedCallback;     // determines callback to run when we exit. e.g. where do we want to go after closing the menu
    u8 gfxLoadState;

    u8 spriteIds[SPRITE_ARR_ID_COUNT];
    u32 paletteTag[SPRITE_ARR_ID_COUNT];
    u32 tileTag[SPRITE_ARR_ID_COUNT];
    u16 headerId;
    u8 currPageIndex;
    u8 minPageIndex;
    u8 maxPageIndex;
    u8 currMonIndex;
    u8 minMonIndex;
    u8 maxMonIndex;
    u8 mode;
    struct Sprite *cursorSprite;
    struct Sprite *cursorShadowSprite;
    struct WildPokemonUnique uniquePokemon[MAX_UNIQUE_POKEMON];
    u8 uniquePokemonCount;
};

enum Pages
{
    EV_PAGE_LAND,
    EV_PAGE_FISH,
    EV_PAGE_WATER,
    EV_PAGE_ROCK,
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
    EV_LABEL_WINDOW_BOX_OVERVIEW,
    EV_LABEL_WINDOW_BOX_LEVEL,
    EV_LABEL_WINDOW_BOX_STATS,
    //EV_LABEL_WINDOW_BOX_ABILITIES,

    // Data
    EV_DATA_WINDOW_BOX_SPECIES,
    EV_DATA_WINDOW_BOX_LEVEL_CATCH,
    EV_DATA_WINDOW_BOX_BASE_STATS,
    EV_DATA_WINDOW_BOX_EV_YIELD,
    EV_DATA_WINDOW_BOX_ABILITIES,
    EV_DATA_WINDOW_BOX_ENCRATE,

    // end
    EV_WINDOW_END,
};

enum EncViewerMode
{
    EV_MODE_DEFAULT,
    EV_MODE_SELECT_MON,
};

enum RodType
{
    ROD_OLD,
    ROD_GOOD,
    ROD_SUPER,
    ROD_COUNT,
};

static const u8 encRateLand[LAND_WILD_COUNT]  = {20,20,10,10,10,10,5,5,4,4,1,1};
static const u8 encRateFish[FISH_WILD_COUNT] = {70,30,60,20,20,40,40,15,4,1};
static const u8 encRateWater[WATER_WILD_COUNT]  = {60,30,5,4,1};
static const u8 encRateRock[ROCK_WILD_COUNT]  = {60,30,5,4,1};

//==========EWRAM==========//
static EWRAM_DATA struct MenuResources *sMenuDataPtr = NULL;
static EWRAM_DATA u8 *sBg1TilemapBuffer = NULL;

//==========STATIC=DEFINES==========//
static void Menu_RunSetup(void);
static bool8 Menu_DoGfxSetup(void);
static bool8 Menu_InitBgs(void);
static void Menu_FadeAndBail(void);
static bool8 Menu_LoadGraphics(u8 page);
static void Menu_InitWindows(void);
static void Menu_BeginPageChange(u8 task);
static void PrintToWindow(u8 windowId, u8 colorIdx);
static void Task_MenuWaitFadeIn(u8 taskId);
static void Task_MenuMain(u8 taskId);
static void Task_MenuTurnOff(u8 taskId);
static void Task_BeginPageChange(u8 taskId);

static void ClearPageWindowTilemap(u8 page);
static void ClearSpriteData(u8 spriteId);
static void PutPageWindowTilemap(u8 page);
static void PutPageWindowText(u8 page);
static void PutPageMonDataText(u8 page);
static void PutPageWindowIcons(u8 page);
static void PutPageWindowSprites(u8 page);
static void ReloadPageData(void);
static void ReloadAllPageData(void);
static void PrintTextOnWindow(u8 windowId, const u8 *string, u8 x, u8 y, u8 lineSpacing, u8 colorId);
static void PrintTextOnWindowSmall(u8 windowId, const u8 *string, u8 x, u8 y, u8 lineSpacing, u8 colorId);
static void PrintTextOnWindowNarrow(u8 windowId, const u8 *string, u8 x, u8 y, u8 lineSpacing, u8 colorId);
static void CreateCursorSprites(void);
static void CursorUpdatePos(void);
static void DrawMonIcon(u16 species, u8 x, u8 y, u8 spriteArrId);
static void DrawMonSprite(u16 species, u8 x, u8 y);
static void DrawItemSprites(u16 itemId, u8 rarity, u8 x, u8 y, u8 subpriority);
static void DrawPokeballSprite(u8 x, u8 y, u8 subpriority);
static u8 GetMaxMonIndex(u8 page);
static u16 SpeciesByIndex(u8 selection);
static u8 *LevelRangeByIndex(u8 selection);
static u8 *EncRateByIndex(u8 selection);
static u8 *CatchRateByIndex(u8 selection);
static u8 *AbilitiesByIndex(u8 selection, u8 slot);
static u8 *BaseStatByIndex(u8 selection, u8 stat);
static u8 *EVYieldByIndex(u8 selection, u8 stat);
static u8 *BSTByIndex(u8 selection);
static const u8 *EncRateByPage(u8 page);
static const struct WildPokemonInfo *GetWildMonInfo(void);
static bool8 HasWildEncounter();
static u8 GetUniqueWildEncounter(const struct WildPokemonInfo *wildPokemonInfo);
static void InitWildEncounterData();
static void ResetWildEncounterData();
static void InitWildMonData(struct WildPokemonUnique *uniqueMons);
static void FreeResourcesForPageChange(void);

static void PrintDebug();

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
        .width = 2,        // width (per 8 pixels)
        .height = 2,        // height (per 8 pixels)
        .paletteNum = 15,   // palette index to use for text
        .baseBlock = 1,     // tile start in VRAM
    },
    [EV_LABEL_WINDOW_BOX_STATS] = 
    {
        .bg = 0,
        .tilemapLeft = 21,
        .tilemapTop = 10,
        .width = 3,
        .height = 9,
        .paletteNum = 15,
        .baseBlock = 1 + 4,
    },
    [EV_DATA_WINDOW_BOX_SPECIES] = 
    {
        .bg = 0,
        .tilemapLeft = 22,
        .tilemapTop = 3,
        .width = 7,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 1 + 4 + 27,
    },
    [EV_DATA_WINDOW_BOX_LEVEL_CATCH]
    {
        .bg = 0,
        .tilemapLeft = 24,
        .tilemapTop = 6,
        .width = 5,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 1 + 4 + 27 + 14,
    },
    [EV_DATA_WINDOW_BOX_BASE_STATS]
    {
        .bg = 0,
        .tilemapLeft = 24,
        .tilemapTop = 10,
        .width = 3,
        .height = 9,
        .paletteNum = 15,
        .baseBlock = 1 + 4 + 27 + 14 + 20,
    },
    [EV_DATA_WINDOW_BOX_EV_YIELD]
    {
        .bg = 0,
        .tilemapLeft = 27,
        .tilemapTop = 10,
        .width = 3,
        .height = 9,
        .paletteNum = 15,
        .baseBlock = 1 + 4 + 27 + 14 + 20 + 27,
    },
    [EV_DATA_WINDOW_BOX_ABILITIES]
    {
        .bg = 0,
        .tilemapLeft = 13,
        .tilemapTop = 14,
        .width = 8,
        .height = 5,
        .paletteNum = 15,
        .baseBlock = 1 + 4 + 27 + 14 + 20 + 27 + 27,
    },
    [EV_DATA_WINDOW_BOX_ENCRATE]
    {
        .bg = 0,
        .tilemapLeft = 18,
        .tilemapTop = 3,
        .width = 3,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 1 + 4 + 27 + 14 + 20 + 27 + 27 + 40,
    },
    [EV_LABEL_WINDOW_BOX_OVERVIEW]
    {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 3,
        .width = 12,
        .height = 16,
        .paletteNum = 15,
        .baseBlock = 1 + 4 + 27 + 14 + 20 + 27 + 27 + 40 + 6,
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
static const u32 sMenuTilemapLand[] = INCBIN_U32("graphics/ui_menu/tilemapLand.bin.lz");
static const u32 sMenuTilemapFish[] = INCBIN_U32("graphics/ui_menu/tilemapFish.bin.lz");
static const u32 sMenuTilemapWater[] = INCBIN_U32("graphics/ui_menu/tilemapWater.bin.lz");
static const u32 sMenuTilemapRock[] = INCBIN_U32("graphics/ui_menu/tilemapRock.bin.lz");
static const u16 sMenuPaletteRed[] = INCBIN_U16("graphics/ui_menu/paletteRed.gbapal");
static const u16 sMenuPaletteBlue[] = INCBIN_U16("graphics/ui_menu/paletteBlue.gbapal");
static const u16 sMenuPaletteLightBlue[] = INCBIN_U16("graphics/ui_menu/paletteLightBlue.gbapal");
static const u16 sMenuPaletteLime[] = INCBIN_U16("graphics/ui_menu/paletteLime.gbapal");
static const u16 sMenuPaletteOrange[] = INCBIN_U16("graphics/ui_menu/paletteOrange.gbapal");

static const u16 sHandCursor_Pal[] = INCBIN_U16("graphics/pokemon_storage/hand_cursor.gbapal");
static const u8 sHandCursor_Gfx[] = INCBIN_U8("graphics/pokemon_storage/hand_cursor.4bpp");
static const u8 sHandCursorShadow_Gfx[] = INCBIN_U8("graphics/pokemon_storage/hand_cursor_shadow.4bpp");

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

    sMenuDataPtr->headerId = GetCurrentMapWildMonHeaderId();
    sMenuDataPtr->mode = EV_MODE_DEFAULT;
    sMenuDataPtr->minPageIndex = EV_PAGE_LAND;
    sMenuDataPtr->maxPageIndex = EV_PAGE_ROCK;
    sMenuDataPtr->currPageIndex = EV_PAGE_LAND;
    sMenuDataPtr->minMonIndex = 0;
    InitWildEncounterData();
    InitWildMonData(sMenuDataPtr->uniquePokemon);
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
        if (Menu_LoadGraphics(sMenuDataPtr->currPageIndex) == TRUE)
            gMain.state++;
        break;
    case 4:
        LoadMessageBoxAndBorderGfx();
        Menu_InitWindows();
        gMain.state++;
        break;
    case 5:
        PrintDebug();
        gMain.state++;
        if (!HasWildEncounter)
            gMain.state = 12; 
        break;
    case 6:
        PutPageWindowTilemap(sMenuDataPtr->currPageIndex);
        gMain.state++;
        break;
    case 7:
        PutPageMonDataText(sMenuDataPtr->currPageIndex);
        gMain.state++;
        break;
    case 8:
        PutPageWindowIcons(sMenuDataPtr->currPageIndex);
        gMain.state++;
        break;
    case 9:
        PutPageWindowSprites(sMenuDataPtr->currPageIndex);
        gMain.state++;
        break;
    case 10:
        CreateCursorSprites();
        gMain.state++;
        break;
    case 11:
        PutPageWindowText(sMenuDataPtr->currPageIndex);
        gMain.state++;
        break;
    case 12:
        taskId = CreateTask(Task_MenuWaitFadeIn, 0);
        BlendPalettes(0xFFFFFFFF, 16, RGB_BLACK);
        gMain.state++;
        break;
    case 13:
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

static void FreeResourcesForPageChange(void)
{
    ResetSpriteData();
    FreeAllSpritePalettes();
    FreeAllWindowBuffers();
}

static bool8 Menu_CreateBgs()
{
    try_free(sBg1TilemapBuffer);
    return Menu_InitBgs();
}

static bool8 Menu_LoadGraphics(u8 page)
{
    switch (sMenuDataPtr->gfxLoadState)
    {
    case 0:
        ResetTempTileDataBuffers();
        switch (page)
            {
                case EV_PAGE_LAND:
                case EV_PAGE_FISH:
                case EV_PAGE_WATER:
                case EV_PAGE_ROCK:
                    DecompressAndCopyTileDataToVram(1, sMenuTiles, 0, 0, 0);
                    break;
            }
        sMenuDataPtr->gfxLoadState++;
        break;
    case 1:
        if (FreeTempTileDataBuffersIfPossible() != TRUE)
        {
            switch (page)
            {
                case EV_PAGE_LAND:
                    LZDecompressWram(sMenuTilemapLand, sBg1TilemapBuffer);
                    break;
                case EV_PAGE_FISH:
                    LZDecompressWram(sMenuTilemapFish, sBg1TilemapBuffer);
                    break;
                case EV_PAGE_WATER:
                    LZDecompressWram(sMenuTilemapWater, sBg1TilemapBuffer);
                    break;
                case EV_PAGE_ROCK:
                    LZDecompressWram(sMenuTilemapRock, sBg1TilemapBuffer);
                    break;
            }
            sMenuDataPtr->gfxLoadState++;
        }
        break;
    case 2:
        switch (page)
        {
            case EV_PAGE_LAND:
                LoadPalette(sMenuPaletteLime, 0, 32);
                break;
            case EV_PAGE_FISH:
                LoadPalette(sMenuPaletteLightBlue, 0, 32);
                break;
            case EV_PAGE_WATER:
                LoadPalette(sMenuPaletteBlue, 0, 32);
                break;
            case EV_PAGE_ROCK:
                LoadPalette(sMenuPaletteOrange, 0, 32);
                break;
        }
        FillPalBufferBlack();
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
}

static void Task_MenuWaitFadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_MenuMain;
}

/* This is the meat of the UI. This is where you wait for player inputs and can branch to other tasks accordingly */
static void Task_MenuMain(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_PC_OFF);
            BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
            gTasks[taskId].func = Task_MenuTurnOff;
        }
        if (JOY_NEW(DPAD_RIGHT))
        {
            if (sMenuDataPtr->currMonIndex < sMenuDataPtr->maxMonIndex)
            {
                PlaySE(SE_SELECT);
                sMenuDataPtr->currMonIndex++;
                CursorUpdatePos();
                ReloadPageData();
            }
        }
        if (JOY_NEW(DPAD_LEFT))
        {
            if (sMenuDataPtr->currMonIndex > sMenuDataPtr->minMonIndex)
            {
                PlaySE(SE_SELECT);
                sMenuDataPtr->currMonIndex--;
                CursorUpdatePos();
                ReloadPageData();
            }
        }
        if (JOY_NEW(DPAD_DOWN))
        {
            
            if (sMenuDataPtr->currMonIndex + 2 < sMenuDataPtr->maxMonIndex)
            {
                PlaySE(SE_SELECT);
                sMenuDataPtr->currMonIndex += 3;
                CursorUpdatePos();
                ReloadPageData();
            }
        }
        if (JOY_NEW(DPAD_UP))
        {
            if (sMenuDataPtr->currMonIndex - 2 > sMenuDataPtr->minMonIndex)
            {
                PlaySE(SE_SELECT);
                sMenuDataPtr->currMonIndex -= 3;
                CursorUpdatePos();
                ReloadPageData();
            }
        }
        if (JOY_NEW(R_BUTTON))
        {
            if (sMenuDataPtr->currPageIndex < sMenuDataPtr->maxPageIndex)
            {
                PlaySE(SE_SELECT);
                sMenuDataPtr->currPageIndex++;
                BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
                gTasks[taskId].func = Task_BeginPageChange;
            }
        }
        if (JOY_NEW(L_BUTTON))
        {
            if (sMenuDataPtr->currPageIndex > sMenuDataPtr->minPageIndex)
            {
                PlaySE(SE_SELECT);
                sMenuDataPtr->currPageIndex--;
                BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
                gTasks[taskId].func = Task_BeginPageChange;
            }
        }
    }
}

static void Task_BeginPageChange(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        Menu_BeginPageChange(taskId);
    }
}

static void Menu_BeginPageChange(u8 taskId)
{
    if (Menu_CreateBgs())
    {
        
        sMenuDataPtr->gfxLoadState = 0;
        while(1)
        {
            if (Menu_LoadGraphics(sMenuDataPtr->currPageIndex))
                break;
        }
        ResetWildEncounterData();
        DebugPrintfLevel(MGBA_LOG_ERROR, "print 1");
        PrintDebug();
        InitWildEncounterData();
        InitWildMonData(sMenuDataPtr->uniquePokemon);
        ReloadAllPageData();
        DebugPrintfLevel(MGBA_LOG_ERROR, "print 2");
        PrintDebug();
        FillPalBufferBlack();
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
        gTasks[taskId].func = Task_MenuMain;
    }
    else
    {
        PlaySE(SE_PC_OFF);
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_MenuTurnOff;
    }
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

static void ReloadPageData(void)
{

    ClearSpriteData(SPRITE_ARR_ID_MON_FRONT);
    ClearSpriteData(SPRITE_ARR_ID_ITEM_COMMON);
    ClearSpriteData(SPRITE_ARR_ID_ITEM_RARE);

    DestroySpriteAndFreeResources(&gSprites[sMenuDataPtr->spriteIds[SPRITE_ARR_ID_POKEBALL]]);
    //DestroySpriteAndFreeResources(&gSprites[sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_TYPE_1]]);
    //DestroySpriteAndFreeResources(&gSprites[sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_TYPE_2]]);

    ClearPageWindowTilemap(sMenuDataPtr->currPageIndex);
    if (!HasWildEncounter)
        return;

    PutPageWindowTilemap(sMenuDataPtr->currPageIndex);
    PutPageWindowText(sMenuDataPtr->currPageIndex);
    PutPageMonDataText(sMenuDataPtr->currPageIndex);
    PutPageWindowSprites(sMenuDataPtr->currPageIndex);
}

static void ReloadAllPageData(void)
{
    FreeResourcesForPageChange();
    Menu_InitWindows();
    
    if (!HasWildEncounter)
        return;

    PutPageWindowTilemap(sMenuDataPtr->currPageIndex);
    PutPageWindowText(sMenuDataPtr->currPageIndex);
    PutPageMonDataText(sMenuDataPtr->currPageIndex);
    PutPageWindowSprites(sMenuDataPtr->currPageIndex);
    PutPageWindowIcons(sMenuDataPtr->currPageIndex);
    CreateCursorSprites();
}

static const u8 sText_Level[] = _("{LV}.");
static const u8 sText_Abilities[] = _("Abilities");

static const u8 sText_Land[] = _("Land");
static const u8 sText_WaterRock[] = _("Water\nRock");
static const u8 sText_Fish[] = _("Fishing");

static const u8 sText_HP[]   = _("HP");
static const u8 sText_ATK[]  = _("ATK");
static const u8 sText_DEF[]  = _("DEF");
static const u8 sText_SATK[] = _("SPA");
static const u8 sText_SDEF[] = _("SPD");
static const u8 sText_SPD[]  = _("SPE");
static const u8 sText_BST[] = _("BST");

static const u8 sText_Base[] = _("Base");
static const u8 sText_EVs[] = _("EVs");

static void PutPageWindowText(u8 page)
{
    PrintTextOnWindowSmall(EV_LABEL_WINDOW_BOX_LEVEL, sText_Level, 0, 0, 0, FONT_BLACK);
    PrintTextOnWindowSmall(EV_LABEL_WINDOW_BOX_STATS, sText_HP, 3, 5, 0, FONT_BLACK);
    PrintTextOnWindowSmall(EV_LABEL_WINDOW_BOX_STATS, sText_ATK, 3, 14, 0, FONT_BLACK);
    PrintTextOnWindowSmall(EV_LABEL_WINDOW_BOX_STATS, sText_DEF, 3, 23, 0, FONT_BLACK);
    PrintTextOnWindowSmall(EV_LABEL_WINDOW_BOX_STATS, sText_SATK, 3, 32, 0, FONT_BLACK);
    PrintTextOnWindowSmall(EV_LABEL_WINDOW_BOX_STATS, sText_SDEF, 3, 41, 0, FONT_BLACK);
    PrintTextOnWindowSmall(EV_LABEL_WINDOW_BOX_STATS, sText_SPD, 3, 50, 0, FONT_BLACK);
    PrintTextOnWindowSmall(EV_LABEL_WINDOW_BOX_STATS, sText_BST, 3, 59, 0, FONT_BLACK);
}

#define OW_BOX_X 8 + 16
#define OW_BOX_Y 24 + 16
static void PutPageMonDataText(u8 page)
{
    u8 selection = sMenuDataPtr->currMonIndex;
    u16 species = SpeciesByIndex(selection);

    PrintTextOnWindowNarrow(EV_DATA_WINDOW_BOX_SPECIES, gSpeciesNames[species], 0, 2, 0, FONT_BLACK);
    PrintTextOnWindowSmall(EV_DATA_WINDOW_BOX_LEVEL_CATCH, LevelRangeByIndex(selection), 0, 0, 0, FONT_BLACK);
    PrintTextOnWindowSmall(EV_DATA_WINDOW_BOX_LEVEL_CATCH, CatchRateByIndex(selection), 0, 12, 0, FONT_BLACK);
    // base stats
    PrintTextOnWindowSmall(EV_DATA_WINDOW_BOX_BASE_STATS, BaseStatByIndex(selection, STAT_HP), 3, 5, 0, FONT_BLACK);
    PrintTextOnWindowSmall(EV_DATA_WINDOW_BOX_BASE_STATS, BaseStatByIndex(selection, STAT_ATK), 3, 14, 0, FONT_BLACK);
    PrintTextOnWindowSmall(EV_DATA_WINDOW_BOX_BASE_STATS, BaseStatByIndex(selection, STAT_DEF), 3, 23, 0, FONT_BLACK);
    PrintTextOnWindowSmall(EV_DATA_WINDOW_BOX_BASE_STATS, BaseStatByIndex(selection, STAT_SPATK), 3, 32, 0, FONT_BLACK);
    PrintTextOnWindowSmall(EV_DATA_WINDOW_BOX_BASE_STATS, BaseStatByIndex(selection, STAT_SPDEF), 3, 41, 0, FONT_BLACK);
    PrintTextOnWindowSmall(EV_DATA_WINDOW_BOX_BASE_STATS, BaseStatByIndex(selection, STAT_SPEED), 3, 50, 0, FONT_BLACK);
    PrintTextOnWindowSmall(EV_DATA_WINDOW_BOX_BASE_STATS, BSTByIndex(selection), 3, 59, 0, FONT_BLACK);
    // ev yields
    PrintTextOnWindowSmall(EV_DATA_WINDOW_BOX_EV_YIELD, EVYieldByIndex(selection, STAT_HP), 5, 5, 0, FONT_BLACK);
    PrintTextOnWindowSmall(EV_DATA_WINDOW_BOX_EV_YIELD, EVYieldByIndex(selection, STAT_ATK), 5, 14, 0, FONT_BLACK);
    PrintTextOnWindowSmall(EV_DATA_WINDOW_BOX_EV_YIELD, EVYieldByIndex(selection, STAT_DEF), 5, 23, 0, FONT_BLACK);
    PrintTextOnWindowSmall(EV_DATA_WINDOW_BOX_EV_YIELD, EVYieldByIndex(selection, STAT_SPATK), 5, 32, 0, FONT_BLACK);
    PrintTextOnWindowSmall(EV_DATA_WINDOW_BOX_EV_YIELD, EVYieldByIndex(selection, STAT_SPDEF), 5, 41, 0, FONT_BLACK);
    PrintTextOnWindowSmall(EV_DATA_WINDOW_BOX_EV_YIELD, EVYieldByIndex(selection, STAT_SPEED), 5, 50, 0, FONT_BLACK);
    // abilities
    PrintTextOnWindowSmall(EV_DATA_WINDOW_BOX_ABILITIES, EncRateByIndex(selection), 3, 12, 0, FONT_BLACK);
    //PrintTextOnWindowSmall(EV_DATA_WINDOW_BOX_ABILITIES, AbilitiesByIndex(selection, ABILITY_1), 3, 12, 0, FONT_BLACK);
    PrintTextOnWindowSmall(EV_DATA_WINDOW_BOX_ABILITIES, AbilitiesByIndex(selection, ABILITY_2), 3, 21, 0, FONT_BLACK);
    PrintTextOnWindowSmall(EV_DATA_WINDOW_BOX_ABILITIES, AbilitiesByIndex(selection, ABILITY_2), 3, 30, 0, FONT_BLACK);
}

static void PutPageWindowTilemap(u8 page)
{
    u8 i;
    for (i = 0; i < EV_WINDOW_END; i++)
    {
        FillWindowPixelBuffer(i, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));
        PutWindowTilemap(i);
        CopyWindowToVram(i, 3);
    }
    
}

static void PutPageWindowIcons(u8 page)
{
    u8 i;
    for (i = 0; i < sMenuDataPtr->uniquePokemonCount; i++)
        DrawMonIcon(sMenuDataPtr->uniquePokemon[i].wildMonData.species, (i%3) * X_OFFSET + OW_BOX_X, (i/3) * Y_OFFSET + OW_BOX_Y, i);
}

static void PutPageWindowSprites(u8 page)
{
    u8 spriteId;
    u8 selection = sMenuDataPtr->currMonIndex;
    u16 species = SpeciesByIndex(selection);
    u16 itemCommon = sMenuDataPtr->uniquePokemon[selection].wildMonData.itemCommon;
    u16 itemRare = sMenuDataPtr->uniquePokemon[selection].wildMonData.itemRare;

    DrawMonSprite(species, 138, 57);
    DrawItemSprites(itemCommon, COMMON, 124, 112, 0);
    DrawItemSprites(itemRare, RARE, 156, 112, 0);
    DrawPokeballSprite(182, 68, 0);
}

static void ClearPageWindowTilemap(u8 page)
{
    u8 i;
    for (i = 0; i < EV_WINDOW_END; i++)
        ClearWindowTilemap(i);
}

static void ClearSpriteData(u8 spriteId)
{
    DestroySprite(&gSprites[sMenuDataPtr->spriteIds[spriteId]]);
    FreeSpriteTilesByTag(sMenuDataPtr->paletteTag[spriteId]);
    FreeSpritePaletteByTag(sMenuDataPtr->tileTag[spriteId]);
}

static void PrintTextOnWindow(u8 windowId, const u8 *string, u8 x, u8 y, u8 lineSpacing, u8 colorId)
{
    AddTextPrinterParameterized4(windowId, FONT_NORMAL, x, y, 0, lineSpacing, sMenuWindowFontColors[colorId], 0, string);
}

static void PrintTextOnWindowSmall(u8 windowId, const u8 *string, u8 x, u8 y, u8 lineSpacing, u8 colorId)
{
    AddTextPrinterParameterized4(windowId, FONT_SMALL, x, y, 0, lineSpacing, sMenuWindowFontColors[colorId], 0, string);
}

static void PrintTextOnWindowNarrow(u8 windowId, const u8 *string, u8 x, u8 y, u8 lineSpacing, u8 colorId)
{
    AddTextPrinterParameterized4(windowId, FONT_NARROW, x, y, 0, lineSpacing, sMenuWindowFontColors[colorId], 0, string);
}

static void PrintTextOnWindowTiny(u8 windowId, const u8 *string, u8 x, u8 y, u8 lineSpacing, u8 colorId)
{
    AddTextPrinterParameterized4(windowId, FONT_TINY, x, y, 0, lineSpacing, sMenuWindowFontColors[colorId], 0, string);
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

const static struct CompressedSpritePalette *GetMonSpritePal(u16 species)
{
    return &gMonPaletteTable[species];
}

const static struct CompressedSpriteSheet *GetMonSpriteSheet(u16 species)
{
    return &gMonFrontPicTable[species];
}

static void DrawMonIcon(u16 species, u8 x, u8 y, u8 spriteArrId)
{
    LoadMonIconPalette(species);
    sMenuDataPtr->spriteIds[spriteArrId] = CreateMonIconNoPersonality(species, SpriteCallbackDummy, x, y, 1, 0);
}

static void SpriteCB_CursorShadow(struct Sprite *sprite)
{
    sprite->x = sMenuDataPtr->cursorSprite->x;
    sprite->y = sMenuDataPtr->cursorSprite->y + 20;
}

static void CreateCursorSprites(void)
{
    struct SpriteSheet spriteSheets[] =
    {
        {sHandCursor_Gfx, 0x800, GFXTAG_CURSOR},
        {sHandCursorShadow_Gfx, 0x80, GFXTAG_CURSOR_SHADOW},
        {}
    };

    struct SpritePalette spritePalettes[] =
    {
        {sHandCursor_Pal, PALTAG_CURSOR_1},
        {}
    };

    static const struct OamData sOamData_Cursor =
    {
        .shape = SPRITE_SHAPE(32x32),
        .size = SPRITE_SIZE(32x32),
        .priority = 1,
    };
    static const struct OamData sOamData_CursorShadow =
    {
        .shape = SPRITE_SHAPE(16x16),
        .size = SPRITE_SIZE(16x16),
        .priority = 1,
    };

    static const union AnimCmd sAnim_Cursor_Bouncing[] =
    {
        ANIMCMD_FRAME(0, 30),
        ANIMCMD_FRAME(16, 30),
        ANIMCMD_JUMP(0)
    };
    static const union AnimCmd sAnim_Cursor_Still[] =
    {
        ANIMCMD_FRAME(0, 5),
        ANIMCMD_END
    };
    static const union AnimCmd sAnim_Cursor_Open[] =
    {
        ANIMCMD_FRAME(32, 5),
        ANIMCMD_END
    };
    static const union AnimCmd sAnim_Cursor_Fist[] =
    {
        ANIMCMD_FRAME(48, 5),
        ANIMCMD_END
    };

    static const union AnimCmd *const sAnims_Cursor[] =
    {
        [CURSOR_ANIM_BOUNCE] = sAnim_Cursor_Bouncing,
        [CURSOR_ANIM_STILL]  = sAnim_Cursor_Still,
        [CURSOR_ANIM_OPEN]   = sAnim_Cursor_Open,
        [CURSOR_ANIM_FIST]   = sAnim_Cursor_Fist
    };

    static const struct SpriteTemplate sSpriteTemplate_Cursor =
    {
        .tileTag = GFXTAG_CURSOR,
        .paletteTag = PALTAG_CURSOR_1,
        .oam = &sOamData_Cursor,
        .anims = sAnims_Cursor,
        .images = NULL,
        .affineAnims = gDummySpriteAffineAnimTable,
        .callback = SpriteCallbackDummy,
    };

    static const struct SpriteTemplate sSpriteTemplate_CursorShadow =
    {
        .tileTag = GFXTAG_CURSOR_SHADOW,
        .paletteTag = PALTAG_CURSOR_1,
        .oam = &sOamData_CursorShadow,
        .anims = gDummySpriteAnimTable,
        .images = NULL,
        .affineAnims = gDummySpriteAffineAnimTable,
        .callback = SpriteCB_CursorShadow,
    };

    LoadSpriteSheets(spriteSheets);
    LoadSpritePalettes(spritePalettes);
    u8 i = sMenuDataPtr->currMonIndex;
    int x = (i%3) * X_OFFSET + OW_BOX_X;
    int y = (i/3) * Y_OFFSET + OW_BOX_Y - 8;
    sMenuDataPtr->spriteIds[SPRITE_ARR_ID_HAND_CURSOR] = CreateSprite(&sSpriteTemplate_Cursor, x, y, 0);
    sMenuDataPtr->paletteTag[SPRITE_ARR_ID_HAND_CURSOR] = sSpriteTemplate_Cursor.paletteTag;
    sMenuDataPtr->tileTag[SPRITE_ARR_ID_HAND_CURSOR] = sSpriteTemplate_Cursor.tileTag;

    if (sMenuDataPtr->spriteIds[SPRITE_ARR_ID_HAND_CURSOR] != MAX_SPRITES)
    {
        sMenuDataPtr->cursorSprite = &gSprites[sMenuDataPtr->spriteIds[SPRITE_ARR_ID_HAND_CURSOR]];
        //sMenuDataPtr->cursorSprite->oam.paletteNum = PALTAG_CURSOR_1;
        sMenuDataPtr->cursorSprite->oam.priority = 1;
    }
    else
    {
        sMenuDataPtr->cursorSprite = NULL;
    }

    sMenuDataPtr->spriteIds[SPRITE_ARR_ID_HAND_SHADOW] = CreateSprite(&sSpriteTemplate_CursorShadow, x, y, 10);
    if (sMenuDataPtr->spriteIds[SPRITE_ARR_ID_HAND_SHADOW] != MAX_SPRITES)
    {
        sMenuDataPtr->cursorShadowSprite = &gSprites[sMenuDataPtr->spriteIds[SPRITE_ARR_ID_HAND_SHADOW]];
        sMenuDataPtr->cursorShadowSprite->oam.priority = 1;
    }
    else
    {
        sMenuDataPtr->cursorShadowSprite = NULL;
    }
}

static void CursorUpdatePos(void)
{
    int i = sMenuDataPtr->currMonIndex;
    sMenuDataPtr->cursorSprite->x = (i%3) * X_OFFSET + OW_BOX_X;
    sMenuDataPtr->cursorSprite->y = (i/3) * Y_OFFSET + OW_BOX_Y - 8;
}

static void DrawMonSprite(u16 species, u8 x, u8 y)
{
    const struct CompressedSpritePalette *pal = GetMonSpritePal(species);
    const struct CompressedSpriteSheet *sheet = GetMonSpriteSheet(species);
    struct SpriteTemplate spriteTemplate = sSpriteTemplate_FrontPic;
    LoadCompressedSpritePalette(pal);
    LoadCompressedSpriteSheet(sheet);
    spriteTemplate.paletteTag = pal->tag;
    spriteTemplate.tileTag = sheet->tag;
    sMenuDataPtr->paletteTag[SPRITE_ARR_ID_MON_FRONT] = pal->tag;
    sMenuDataPtr->tileTag[SPRITE_ARR_ID_MON_FRONT] = sheet->tag;
    sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_FRONT] = CreateSprite(&spriteTemplate, x, y, 0);
}

static void DrawItemSprites(u16 itemId, u8 rarity, u8 x, u8 y, u8 subpriority)
{
    sMenuDataPtr->spriteIds[SPRITE_ARR_ID_ITEM_COMMON + rarity] = AddItemIconSpriteAt(TAG_ITEM_ICON + rarity, TAG_ITEM_ICON + rarity, itemId, x, y, subpriority);
    if (!rarity)
    {
        sMenuDataPtr->paletteTag[SPRITE_ARR_ID_ITEM_COMMON] = TAG_ITEM_ICON + rarity;
        sMenuDataPtr->tileTag[SPRITE_ARR_ID_ITEM_COMMON] = TAG_ITEM_ICON + rarity;
    } else 
    {
        sMenuDataPtr->paletteTag[SPRITE_ARR_ID_ITEM_RARE] = TAG_ITEM_ICON + rarity;
        sMenuDataPtr->tileTag[SPRITE_ARR_ID_ITEM_RARE] = TAG_ITEM_ICON + rarity;
    }
}

static void DrawPokeballSprite(u8 x, u8 y, u8 subpriority)
{
    u8 ball = ItemIdToBallId(ITEM_POKE_BALL);

    LoadBallGfx(ball);
    sMenuDataPtr->spriteIds[SPRITE_ARR_ID_POKEBALL] = CreateSprite(&gBallSpriteTemplates[ball], x, y, subpriority);
    gSprites[sMenuDataPtr->spriteIds[SPRITE_ARR_ID_POKEBALL]].callback = SpriteCallbackDummy;
    gSprites[sMenuDataPtr->spriteIds[SPRITE_ARR_ID_POKEBALL]].oam.priority = 0;

}

static u8 GetMaxMonIndex(u8 page)
{
    switch (page)
    {
        case EV_PAGE_LAND:
            return LAND_WILD_COUNT;
        case EV_PAGE_FISH:
            return FISH_WILD_COUNT;
        case EV_PAGE_WATER:
            return WATER_WILD_COUNT;
        case EV_PAGE_ROCK:
            return ROCK_WILD_COUNT;
    }
}

static u16 SpeciesByIndex(u8 selection)
{
    return sMenuDataPtr->uniquePokemon[selection].wildMonData.species;
}

static u8* LevelRangeByIndex(u8 selection)
{
    u8 minLevel, maxLevel;

    minLevel = sMenuDataPtr->uniquePokemon[selection].minLevel;
    maxLevel = sMenuDataPtr->uniquePokemon[selection].maxLevel;


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
    struct WildPokemonUnique wildMon = sMenuDataPtr->uniquePokemon[selection];
    ConvertIntToDecimalStringN(gStringVar1, wildMon.encChance, STR_CONV_MODE_RIGHT_ALIGN, 3);
    StringExpandPlaceholders(gStringVar4, gText_Var1Percent);
    return gStringVar4;
}

static u8* CatchRateByIndex(u8 selection)
{
    struct WildPokemonUnique wildMon = sMenuDataPtr->uniquePokemon[selection];
    ConvertIntToDecimalStringN(gStringVar1, wildMon.wildMonData.catchRate, STR_CONV_MODE_RIGHT_ALIGN, 3);
    return gStringVar1;
}

static u8* AbilitiesByIndex(u8 selection, u8 slot)
{
    struct WildPokemonUnique wildMon = sMenuDataPtr->uniquePokemon[selection];
    StringCopy(gStringVar1, gAbilityNames[wildMon.wildMonData.abilities[slot]]);
    return gStringVar1;
}

static u8 *BaseStatByIndex(u8 selection, u8 stat)
{
    struct WildPokemonUnique wildMon = sMenuDataPtr->uniquePokemon[selection];
    switch (stat)
    {
        case STAT_HP:
            ConvertIntToDecimalStringN(gStringVar1, wildMon.wildMonData.baseHP, STR_CONV_MODE_RIGHT_ALIGN, 3);
            break;
        case STAT_ATK:
            ConvertIntToDecimalStringN(gStringVar1, wildMon.wildMonData.baseAttack, STR_CONV_MODE_RIGHT_ALIGN, 3);
            break;
        case STAT_DEF:
            ConvertIntToDecimalStringN(gStringVar1, wildMon.wildMonData.baseDefense, STR_CONV_MODE_RIGHT_ALIGN, 3);
            break;
        case STAT_SPATK:
            ConvertIntToDecimalStringN(gStringVar1, wildMon.wildMonData.baseSpAttack, STR_CONV_MODE_RIGHT_ALIGN, 3);
            break;
        case STAT_SPDEF:
            ConvertIntToDecimalStringN(gStringVar1, wildMon.wildMonData.baseSpDefense, STR_CONV_MODE_RIGHT_ALIGN, 3);
            break;
        case STAT_SPEED:
            ConvertIntToDecimalStringN(gStringVar1, wildMon.wildMonData.baseSpeed, STR_CONV_MODE_RIGHT_ALIGN, 3);
            break;
    }
    return gStringVar1;
}

static u8 *BSTByIndex(u8 selection)
{
    struct WildPokemonUnique wildMon = sMenuDataPtr->uniquePokemon[selection];
    u16 bst = wildMon.wildMonData.baseBST;
    ConvertIntToDecimalStringN(gStringVar1, bst, STR_CONV_MODE_RIGHT_ALIGN, 3);
    return gStringVar1;
}

static u8 *EVYieldByIndex(u8 selection, u8 stat)
{
    struct WildPokemonUnique wildMon = sMenuDataPtr->uniquePokemon[selection];
    switch (stat)
    {
        case STAT_HP:
            ConvertIntToDecimalStringN(gStringVar1, wildMon.wildMonData.evYield_HP, STR_CONV_MODE_RIGHT_ALIGN, 1);
            break;
        case STAT_ATK:
            ConvertIntToDecimalStringN(gStringVar1, wildMon.wildMonData.evYield_Attack, STR_CONV_MODE_RIGHT_ALIGN, 1);
            break;
        case STAT_DEF:
            ConvertIntToDecimalStringN(gStringVar1, wildMon.wildMonData.evYield_Defense, STR_CONV_MODE_RIGHT_ALIGN, 1);
            break;
        case STAT_SPATK:
            ConvertIntToDecimalStringN(gStringVar1, wildMon.wildMonData.evYield_SpAttack, STR_CONV_MODE_RIGHT_ALIGN, 1);
            break;
        case STAT_SPDEF:
            ConvertIntToDecimalStringN(gStringVar1, wildMon.wildMonData.evYield_SpDefense, STR_CONV_MODE_RIGHT_ALIGN, 1);
            break;
        case STAT_SPEED:
            ConvertIntToDecimalStringN(gStringVar1, wildMon.wildMonData.evYield_Speed, STR_CONV_MODE_RIGHT_ALIGN, 1);
            break;
    }
    return gStringVar1;
}

static const struct WildPokemonInfo* GetWildMonInfo(void)
{

    switch(sMenuDataPtr->currPageIndex)
    {
        case EV_PAGE_LAND:
            return gWildMonHeaders[sMenuDataPtr->headerId].landMonsInfo;
        case EV_PAGE_FISH:
            return gWildMonHeaders[sMenuDataPtr->headerId].fishingMonsInfo;
        case EV_PAGE_WATER:
            return gWildMonHeaders[sMenuDataPtr->headerId].waterMonsInfo;
        case EV_PAGE_ROCK:
            return gWildMonHeaders[sMenuDataPtr->headerId].rockSmashMonsInfo; 
    }
}

static const u8 *EncRateByPage(u8 page)
{
    switch (page)
    {
        case EV_PAGE_LAND:
            return encRateLand;
        case EV_PAGE_FISH:
            return encRateFish;
        case EV_PAGE_WATER:
            return encRateWater;
        case EV_PAGE_ROCK:
            return encRateRock;
    }
}

static bool8 HasWildEncounter()
{
    return (GetWildMonInfo != NULL); 
}

static void PrintDebug()
{
    DebugPrintfLevel(MGBA_LOG_ERROR, "num: %d", sMenuDataPtr->uniquePokemonCount);
    DebugPrintfLevel(MGBA_LOG_ERROR, "maxidx: %d", sMenuDataPtr->maxMonIndex);
    for (int i = 0; i <= sMenuDataPtr->uniquePokemonCount; i++)
    {
        DebugPrintfLevel(MGBA_LOG_ERROR, "mon: %S", gSpeciesNames[sMenuDataPtr->uniquePokemon[i].wildMonData.species]);
    }
}

static u8 GetUniqueWildEncounter(const struct WildPokemonInfo *wildPokemonInfo) 
{
    u8 i, j;
    u8 uniqueCount = 0;
    bool8 isDuplicate = FALSE;
    u8 maxMonIndex = GetMaxMonIndex(sMenuDataPtr->currPageIndex);
    const u8 *encRate = EncRateByPage(sMenuDataPtr->currPageIndex);
    for (i = 0; i < MAX_UNIQUE_POKEMON; i++)
    {
        DebugPrintfLevel(MGBA_LOG_ERROR, "encR:%d", encRate[i]);
    }
    const struct WildPokemon *wildMons = wildPokemonInfo->wildPokemon;
    struct WildPokemonUnique *uniqueMons = sMenuDataPtr->uniquePokemon;
    
    for (i = 0; i < MAX_UNIQUE_POKEMON; i++)
        sMenuDataPtr->uniquePokemon[i].encChance = 0;

    for (i = 0; i < maxMonIndex; i++)
    {
        DebugPrintfLevel(MGBA_LOG_ERROR, "b:%d", uniqueCount);
        for (j = 0; j < MAX_UNIQUE_POKEMON; j++)
        {
            if (uniqueMons[j].wildMonData.species == wildMons[i].species)
            {
                isDuplicate = TRUE;
                uniqueMons[j].encChance += encRate[i];
                if (wildMons[i].maxLevel > uniqueMons[j].maxLevel)
                    uniqueMons[j].maxLevel = wildMons[i].maxLevel;
                if (wildMons[i].minLevel < uniqueMons[j].minLevel)
                    uniqueMons[j].minLevel = wildMons[i].minLevel;
                DebugPrintfLevel(MGBA_LOG_ERROR, "isDupe:%S", gSpeciesNames[wildMons[i].species]);
                DebugPrintfLevel(MGBA_LOG_ERROR, "sum:%d, chance:%d", uniqueMons[j].encChance, encRate[i]);
            }

        }
        if (!isDuplicate)
        {
            uniqueMons[uniqueCount].wildMonData.species = wildMons[i].species;
            uniqueMons[uniqueCount].maxLevel = wildMons[i].maxLevel;
            uniqueMons[uniqueCount].minLevel = wildMons[i].minLevel;
            uniqueMons[uniqueCount].encChance = encRate[i];
            uniqueCount++;
            DebugPrintfLevel(MGBA_LOG_ERROR, "isNotDupe:%S", gSpeciesNames[wildMons[i].species]);
            DebugPrintfLevel(MGBA_LOG_ERROR, "sum:%d, chance:%d", wildMons[i], encRate[i]);
        }
        isDuplicate = FALSE;
    }
    return uniqueCount;
}

static void InitWildEncounterData()
{
    sMenuDataPtr->uniquePokemonCount = GetUniqueWildEncounter(GetWildMonInfo());
    sMenuDataPtr->maxMonIndex = sMenuDataPtr->uniquePokemonCount - 1;
    sMenuDataPtr->currMonIndex = 0;
}

static void InitWildMonData(struct WildPokemonUnique *uniqueMons)
{
    u8 i;
    
    for (i = 0; i < sMenuDataPtr->uniquePokemonCount; i++)
    {
        struct WildPokemonData *wildMonData = &uniqueMons[i].wildMonData;
        u16 species = wildMonData->species;
        const struct SpeciesInfo *info = &gSpeciesInfo[species];

        wildMonData->types[0] = info->types[0];
        wildMonData->types[1] = info->types[1];
        wildMonData->abilities[0] = info->abilities[0];
        wildMonData->abilities[1] = info->abilities[1];
        wildMonData->baseHP = info->baseHP;
        wildMonData->baseAttack = info->baseAttack;
        wildMonData->baseDefense = info->baseDefense;
        wildMonData->baseSpAttack = info->baseSpAttack;
        wildMonData->baseSpDefense = info->baseSpDefense;
        wildMonData->baseSpeed = info->baseSpeed;
        wildMonData->baseBST    = info->baseHP 
                                + info->baseAttack
                                + info->baseDefense
                                + info->baseSpAttack
                                + info->baseSpDefense
                                + info->baseSpeed;
        wildMonData->evYield_HP = info->evYield_HP;
        wildMonData->evYield_Attack = info->evYield_Attack;
        wildMonData->evYield_Defense = info->evYield_Defense;
        wildMonData->evYield_SpAttack = info->evYield_SpAttack;
        wildMonData->evYield_SpDefense = info->evYield_SpDefense;
        wildMonData->evYield_Speed = info->evYield_Speed;
        wildMonData->catchRate = info->catchRate;
        wildMonData->itemCommon = info->itemCommon;
        wildMonData->itemRare = info->itemRare;
    }
}

static void ResetWildEncounterData(void)
{
    memset(&sMenuDataPtr->uniquePokemon, 0, MAX_UNIQUE_POKEMON * sizeof(struct WildPokemonUnique));
    sMenuDataPtr->uniquePokemonCount = 0;
    sMenuDataPtr->maxMonIndex = 0;
    sMenuDataPtr->currMonIndex = 0;
}
