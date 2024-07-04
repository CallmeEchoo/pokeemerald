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

enum GymChallengerLevel
{
    LEVEL_0,
    LEVEL_1,
    LEVEL_2,
    LEVEL_3,
    LEVEL_4,
    LEVEL_5,
    LEVEL_6,
    LEVEL_7,
    LEVEL_8,
    LEVEL_9,
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
};

extern struct GymChallengerTrainerData* const challengerPtr;

u8 CreateNPCGymChallengerParty(struct Pokemon* party, bool32 firstTrainer, u32 battleTypeFlags);
u32 GetGymChallengerTrainerPic(void);
void GetGymChallengerTrainerName(u8* dst);
u8 GetGymChallengerTrainerClass(void);

#endif // PLAYER_GYM_H