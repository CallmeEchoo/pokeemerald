#ifndef PLAYER_GYM_H
#define PLAYER_GYM_H

enum GymChallengerExpertise
{
    ROOKIE,
    NOVICE,
    BEGINNER,
    COMPETENT,
    PROFICIENT,
    ADVANCED,
    EXPERT,
    ELITE,
    LEADER,
    CHAMPION
};

enum GymChallengerStage
{
    // estimates   lvl    bst   stage
    LOW,        // 15   < 350   first
    MID,        // 30   < 450   second, weak first
    HIGH,       // 50   < 520   weak third, strong second, strong first
    VERY_HIGH,  // 70   < 560   string third, weak legendaries
    TOP,        // 100  < 800   legendaries, megas, etc.
};

enum GymChallengerTier
{
    UBER,
    OU,
    UU,
    RU,
    NU,
    PU,
    ZU,
};

struct GymChallengerTrainerData
{
    u32 aiFlags;
    struct TrainerMon party[PARTY_SIZE];
    u16 items[MAX_TRAINER_ITEMS];
    u8 trainerClass;
    u8 encounterMusic_gender; // last bit is gender
    u8 trainerPic;
    u8 trainerName[TRAINER_NAME_LENGTH + 1];
    bool8 doubleBattle:1;
    bool8 mugshotEnabled:1;
    u8 startingStatus:6;    // this trainer starts a battle with a given status. see include/constants/battle.h for values
    u8 mugshotColor;
    u8 partySize;

    enum GymChallengerExpertise expertise;
    enum GymChallengerStage stage;
    enum GymChallengerTier tier;
};

extern struct GymChallengerTrainerData* const challengerPtr;

u8 CreateNPCGymChallengerParty(struct Pokemon* party, bool32 firstTrainer, u32 battleTypeFlags);
u32 GetGymChallengerTrainerPic(void);
void GetGymChallengerTrainerName(u8* dst);
u8 GetGymChallengerTrainerClass(void);

#endif // PLAYER_GYM_H