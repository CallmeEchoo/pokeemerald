#ifndef GUARD_BATTLE_SETUP_H
#define GUARD_BATTLE_SETUP_H

#include "gym_leader_rematch.h"

#define REMATCHES_COUNT 5
#define MAX_STACK_SIZE 20

struct RematchTrainer
{
    u16 trainerIds[REMATCHES_COUNT];
    u16 mapGroup;
    u16 mapNum;
};

/*
the layout of the first byte can be confusing here
isDoubleBattle is the least lsb. msb is in the padding.
*/
typedef union PACKED TrainerBattleParameterUnion
{
    struct PACKED Parameters
    {
        u8 isDoubleBattle:1;
        u8 isRematch:1;
        u8 isTrainerHill:1;
        u8 isTrainerPyramid:1;
        u8 playMusicA:1;
        u8 playMusicB:1;
        u8 multiChooseMons:1;
        u8 multiChooseCancel:1;
        u8 objEventLocalIdA;
        u16 battleOpponentA;
        u8* introTextA;
        u8* defeatTextA;
        u8* battleScriptRetAddrA;
        u8 objEventLocalIdB;
        u16 battleOpponentB;
        u8* introTextB;
        u8* defeatTextB;
        u8* battleScriptRetAddrB;
        u8* victoryText;
        u8* cannotBattleText;
        u16 battlePartner;
    } params;
    u8 data[sizeof(struct Parameters)];
} TrainerBattleParameterU;

#define DebugPrintTrainerParams(battleParameter) DebugPrintfLevel(MGBA_LOG_DEBUG, "\nisDouble: %d\nplayMusicA: %d\nplayMusicB: %d\nisRematch: %d\nisTrainerHill: %d\nisTrainerPyramid: %d\npadding: %d\nlocalIdA: %d\ntrainerA: %d\nintroA: %x\ndefeatA: %x\neventA: %x\nlocalIdB: %d\ntrainerB: %d\nintroB: %x\ndefeatB: %x\neventB: %x\nvictory: %x\nnotBattle:%x\npartner: %d\nchoose mons: %d\nallow cancel: %d\n", \
        battleParameter->params.isDoubleBattle, battleParameter->params.playMusicA, battleParameter->params.playMusicB, battleParameter->params.isRematch, battleParameter->params.isTrainerHill, battleParameter->params.isTrainerPyramid, \
        battleParameter->params.objEventLocalIdA, battleParameter->params.battleOpponentA, battleParameter->params.introTextA, battleParameter->params.defeatTextA, battleParameter->params.battleScriptRetAddrA, \
        battleParameter->params.objEventLocalIdB, battleParameter->params.battleOpponentB, battleParameter->params.introTextB, battleParameter->params.defeatTextB, battleParameter->params.battleScriptRetAddrB, \
        battleParameter->params.victoryText, battleParameter->params.cannotBattleText, battleParameter->params.battlePartner, battleParameter->params.multiChooseMons, battleParameter->params.multiChooseCancel)

typedef struct TrainerBattleScriptStack
{
    const u8* stack[MAX_STACK_SIZE];
    s8 stackPtr;
} TrainerBattleScriptStack;

#define TRAINER_BATTLE_PARAM gTrainerBattleParameter.params

extern const struct RematchTrainer gRematchTable[REMATCH_TABLE_ENTRIES];

extern TrainerBattleParameterU gTrainerBattleParameter;
extern bool16 gHideCancelOptionOnPartySelect;
extern u16 gTrainerBattleOpponent_A;
extern u16 gTrainerBattleOpponent_B;
extern u16 gPartnerTrainerId;

void BattleSetup_StartWildBattle(void);
void BattleSetup_StartDoubleWildBattle(void);
void BattleSetup_StartBattlePikeWildBattle(void);
void BattleSetup_StartRoamerBattle(void);
void StartWallyTutorialBattle(void);
void BattleSetup_StartScriptedWildBattle(void);
void BattleSetup_StartScriptedDoubleWildBattle(void);
void BattleSetup_StartLatiBattle(void);
void BattleSetup_StartLegendaryBattle(void);
void StartGroudonKyogreBattle(void);
void StartRegiBattle(void);
u8 BattleSetup_GetTerrainId(void);
u8 GetWildBattleTransition(void);
u8 GetTrainerBattleTransition(void);
u8 GetSpecialBattleTransition(s32 id);
void ChooseStarter(void);
void ResetTrainerOpponentIds(void);
void SetMapVarsToTrainerA(void);
const u8 *BattleSetup_ConfigureTrainerBattle(const u8 *data, PtrStack *scrStack, bool32 isApproaching);
void ConfigureAndSetUpOneTrainerBattle(u8 trainerObjEventId, const u8 *trainerScript);
void ConfigureTwoTrainersBattle(u8 trainerObjEventId, const u8 *trainerScript);
void SetUpTwoTrainersBattle(void);
bool32 GetTrainerFlagFromScriptPointer(const u8 *data);
void SetTrainerFacingDirection(void);
u8 GetTrainerBattleMode(void);
bool8 GetTrainerFlag(void);
bool8 HasTrainerBeenFought(u16 trainerId);
void SetTrainerFlag(u16 trainerId);
void ClearTrainerFlag(u16 trainerId);
void BattleSetup_StartTrainerBattle(void);
void BattleSetup_StartRematchBattle(void);
void ShowTrainerIntroSpeech(void);
const u8 *BattleSetup_GetScriptAddrAfterBattle(void);
const u8 *BattleSetup_GetTrainerPostBattleScript(void);
void ShowTrainerCantBattleSpeech(void);
void PlayTrainerEncounterMusic(void);
const u8 *GetTrainerALoseText(void);
const u8 *GetTrainerBLoseText(void);
const u8 *GetTrainerWonSpeech(void);
void UpdateRematchIfDefeated(s32 rematchTableId);
void IncrementRematchStepCounter(void);
void TryUpdateRandomTrainerRematches(u16 mapGroup, u16 mapNum);
bool32 DoesSomeoneWantRematchIn(u16 mapGroup, u16 mapNum);
bool32 IsRematchTrainerIn(u16 mapGroup, u16 mapNum);
u16 GetLastBeatenRematchTrainerId(u16 trainerId);
bool8 ShouldTryRematchBattle(void);
bool8 IsTrainerReadyForRematch(void);
void ShouldTryGetTrainerScript(void);
u16 CountBattledRematchTeams(u16 trainerId);
void TrainerBattleLoadArgs_2(const u8* data);
void TrainerBattleLoadArgsTrainerA(const u8* data);
void TrainerBattleLoadArgsTrainerB(const u8* data);
void TrainerBattleLoadArgsSecondTrainer(const u8* data);

void DoStandardWildBattle_Debug(void);
void BattleSetup_StartTrainerBattle_Debug(void);
s32 TrainerIdToRematchTableId(const struct RematchTrainer *table, u16 trainerId);
s32 FirstBattleTrainerIdToRematchTableId(const struct RematchTrainer *table, u16 trainerId);
u16 GetRematchTrainerIdFromTable(const struct RematchTrainer *table, u16 firstBattleTrainerId);
void CB2_EndTrainerBattle(void);
void CB2_EndScriptedWildBattle(void);
u16 GetBattlePartner(u16 partnerId);

#endif // GUARD_BATTLE_SETUP_H
