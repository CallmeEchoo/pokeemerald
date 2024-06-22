#include "global.h"
#include "player_gym.h"
#include "battle_main.h"
#include "data.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "malloc.h"
#include "pokemon_sprite_visualizer.h"
#include "script.h"
#include "string_util.h"
#include "constants/abilities.h"
#include "constants/battle_ai.h"
#include "constants/event_objects.h"

u8 CreateNPCGymChallengerParty(struct Pokemon* party, bool32 firstTrainer, u32 battleTypeFlags)
{
    struct Trainer trainer = {
        .aiFlags = AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT,
        .trainerClass = TRAINER_CLASS_RIVAL,
        .encounterMusic_gender = TRAINER_ENCOUNTER_MUSIC_INTENSE,
        .trainerPic = TRAINER_PIC_RED,
        .trainerName = _("TEST"),
        .partySize = 2,
        .party = (const struct TrainerMon[])
        {
            {
            .species = SPECIES_BULBASAUR,
            .gender = TRAINER_MON_RANDOM_GENDER,
            .iv = TRAINER_PARTY_IVS(0, 0, 0, 0, 0, 0),
            .lvl = 15,
            .nature = NATURE_HARDY,
            .dynamaxLevel = MAX_DYNAMAX_LEVEL,
            },
            {
            .species = SPECIES_CHARMANDER,
            .gender = TRAINER_MON_RANDOM_GENDER,
            .iv = TRAINER_PARTY_IVS(0, 0, 0, 0, 0, 0),
            .lvl = 15,
            .nature = NATURE_HARDY,
            .dynamaxLevel = MAX_DYNAMAX_LEVEL,
            },
        },
    };

    return CreateNPCTrainerPartyFromTrainer(party, &trainer, firstTrainer, battleTypeFlags);
}


u16 GetVarObjLocalEventId(u16 id)
{
    return id - OBJ_EVENT_GFX_VARS;
}

void DoDebugPrintf(struct ScriptContext* ctx)
{
    u32 val = ScriptReadWord(ctx);

    DebugPrintfLevel(MGBA_LOG_DEBUG, "scriptprint: %d", val);
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