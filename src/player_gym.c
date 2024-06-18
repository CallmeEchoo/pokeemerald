#include "global.h"
#include "player_gym.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "script.h"
#include "constants/event_objects.h"

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