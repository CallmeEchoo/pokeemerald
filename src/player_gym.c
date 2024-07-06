#include "global.h"
#include "data.h"
#include "player_gym.h"
#include "battle_main.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "malloc.h"
#include "pokemon_sprite_visualizer.h"
#include "random.h"
#include "script.h"
#include "string_util.h"
#include "constants/abilities.h"
#include "constants/battle_ai.h"
#include "constants/event_objects.h"

#define VAR_ARR_1(item, x) item[x]
#define VAR_ARR_2(item, x) item[x], VAR_ARR_1(item, x+1)
#define VAR_ARR_3(item, x) item[x], VAR_ARR_2(item, x+1)
#define VAR_ARR_4(item, x) item[x], VAR_ARR_3(item, x+1)
#define VAR_ARR_5(item, x) item[x], VAR_ARR_4(item, x+1)
#define VAR_ARR_6(item, x) item[x], VAR_ARR_5(item, x+1)
#define VAR_ARR_7(item, x) item[x], VAR_ARR_6(item, x+1)
#define VAR_ARR_8(item, x) item[x], VAR_ARR_7(item, x+1)

#define VAR_ARR(item, n) VAR_ARR_## n(item, 0)
#define ExpandArray(item, n) { VAR_ARR(item, n) }


static EWRAM_DATA struct GymChallengerTrainerData sGymChallenger = {0};
struct GymChallengerTrainerData* const challengerPtr = &sGymChallenger;

static struct TrainerMon GenerateTrainerMonc(void);
static u16 GenerateItem(void);
static void GenerateGymChallengerTrainerName(struct GymChallengerTrainerData* challenger);
static void GenerateGymChallengerTrainerClass(struct GymChallengerTrainerData* challenger);
static void GenerateGymChallengerTrainerPic(struct GymChallengerTrainerData* challenger);
static void GenerateGymChallengerEncounterMusic(struct GymChallengerTrainerData* challenger);
static void GenerateGymChallengerPartySize(struct GymChallengerTrainerData* challenger);
static void GenerateGymChallengerParty(struct GymChallengerTrainerData* challenger);
static void GenerateGymChallengerItems(struct GymChallengerTrainerData* challenger);
static void GenerateGymChallengerAiFlags(struct GymChallengerTrainerData* challenger);

static struct TrainerMon GenerateTrainerMon(void)
{
    struct TrainerMon mon =
    {
        .species = Random() % NUM_SPECIES,
        .lvl = 15,
    };

    return mon;
}

static u16 GenerateItem(void)
{
    return Random() % ITEMS_COUNT;
}

static void GenerateGymChallenger(void)
{
    struct GymChallengerTrainerData challenger;
    challenger.expertise = ROOKIE;
    challenger.stage = LOW;
    challenger.tier = OU;

    GenerateGymChallengerTrainerName(&challenger);
    challenger.trainerClass = GenerateGymChallengerTrainerClass();
    challenger.trainerPic = GenerateGymChallengerTrainerPic();
    challenger.encounterMusic_gender = GenerateGymChallengerEncounterMusic();
    
    challenger.partySize = GenerateGymChallengerPartySize();
    GenerateGymChallengerParty(&challenger);
    GenerateGymChallengerItems(&challenger);

    challenger.aiFlags = GenerateGymChallengerAiFlags();
    challenger.doubleBattle = 0;
    challenger.startingStatus = 0;

    challenger.mugshotColor = 0;
    challenger.mugshotEnabled = 0;
    
    *challengerPtr = challenger;
}

static struct Trainer GetTrainerDataFromChallenger(struct GymChallengerTrainerData *challenger)
{
    struct Trainer trainer =
    {
        .aiFlags = challenger->aiFlags,
        .party = (const struct TrainerMon*)challenger->party,
        .partySize = challenger->partySize,
        .doubleBattle = challenger->doubleBattle,
        .trainerClass = challenger->trainerClass,
        .trainerPic = challenger->trainerPic,
        .encounterMusic_gender = challenger->encounterMusic_gender,
        .items = ExpandArray(challenger->items, MAX_TRAINER_ITEMS),
        .startingStatus = challenger->startingStatus,
        .mugshotColor = challenger->mugshotColor,
        .mugshotEnabled = challenger->mugshotEnabled,
    };

    return trainer;
}

static void GenerateGymChallengerTrainerName(struct GymChallengerTrainerData* challenger)
{
    const u8 name[] = _("CHALLENGER");

    StringCopy(challenger->trainerName, name);
}

static void GenerateGymChallengerTrainerClass(struct GymChallengerTrainerData* challenger)
{
    challenger->trainerClass = Random() % TRAINER_CLASS_COUNT;
}

static void GenerateGymChallengerTrainerPic(struct GymChallengerTrainerData* challenger)
{
    challenger->trainerPic = Random() % TRAINER_PIC_COUNT;
}

static void GenerateGymChallengerEncounterMusic(struct GymChallengerTrainerData* challenger)
{
    challenger->encounterMusic =  Random() % TRAINER_ENCOUNTER_MUSIC_COUNT;
}

static void GenerateGymChallengerPartySize(struct GymChallengerTrainerData* challenger)
{
    challenger->partySize = Random() % PARTY_SIZE;
}

static void GenerateGymChallengerParty(struct GymChallengerTrainerData* challenger)
{
    int i;

    for (i = 0; i < challenger->partySize; i++)
    {
        challenger->party[i] = GenerateTrainerMon();
    }
}

static void GenerateGymChallengerItems(struct GymChallengerTrainerData* challenger)
{
    int i;

    for (i = 0; i < MAX_TRAINER_ITEMS; i++)
    {
        challenger->items[i] = GenerateItem();
    }
}

static void GenerateGymChallengerAiFlags(struct GymChallengerTrainerData* challenger)
{
    challenger->aiFlags = AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_BAD_MOVE; 
}

u32 GetGymChallengerTrainerPic()
{
    return challengerPtr->trainerPic;
}

void GetGymChallengerTrainerName(u8* dst)
{
    s32 i;

    for (i = 0; i < TRAINER_NAME_LENGTH + 1; i++)
        dst[i] = challengerPtr->trainerName[i];
}

u8 GetGymChallengerTrainerClass(void)
{
    return challengerPtr->trainerClass;
}

u8 CreateNPCGymChallengerParty(struct Pokemon* party, bool32 firstTrainer, u32 battleTypeFlags)
{
    GenerateGymChallenger();
    struct Trainer trainer = GetTrainerDataFromChallenger(challengerPtr);
    
    return CreateNPCTrainerPartyFromTrainer(party, &trainer, firstTrainer, battleTypeFlags);
}


u16 GetVarObjLocalEventId(u16 id)
{
    return id - OBJ_EVENT_GFX_VARS;
}

void ReloadEventObject(u16 objectId, u8 mapNum, u8 mapGroup)
{
    RemoveObjectEventByLocalIdAndMap(objectId, mapNum, mapGroup);
    TrySpawnObjectEvent(objectId, mapNum, mapGroup);
}

void SetVarObjEventGfx(u16 id, u16 gfx)
{
    u8 localId = GetVarObjLocalEventId(id);

    VarSet(VAR_OBJ_GFX_ID_0 + localId, gfx);
    ReloadEventObject(GetObjectEventIdByLocalIdAndMap(localId, 0, 0), 0, 0);
}

void SetGymOpponentGfx(u16 id, u16 gfx)
{
    VarSet(VAR_OBJ_GFX_ID_0 + id, gfx);
}

void SaveGymType(struct ScriptContext *ctx)
{
    u16 type = VarGet(ScriptReadWord(ctx)) + 1; // add one to align with types macros
    if (type >= TYPE_MYSTERY) type++;           // offset for mystery type
    gSaveBlock2Ptr->playerGym.type = type;
}

void SetGymOpponent(struct ScriptContext *ctx)
{
    u16 varObjId = ScriptReadHalfword(ctx);
    u16 gfxId = ScriptReadHalfword(ctx);

    DebugPrintfLevel(MGBA_LOG_DEBUG, "%d, %d\n", varObjId, gfxId);
    SetVarObjEventGfx(varObjId, gfxId);
}

void DoDebugPrintf(struct ScriptContext* ctx)
{
    u32 val = ScriptReadWord(ctx);

    DebugPrintfLevel(MGBA_LOG_DEBUG, "scriptprint: %d", val);
}