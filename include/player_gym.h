#ifndef PLAYER_GYM_H
#define PLAYER_GYM_H

struct PlayerGymChallengerTrainerData
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

extern struct PlayerGymChallengerTrainerData* const challengerPtr;

u8 CreateNPCGymChallengerParty(struct Pokemon* party, bool32 firstTrainer, u32 battleTypeFlags);
u32 GetGymChallengerTrainerPic(void);
void GetGymChallengerTrainerName(u8* dst);
u8 GetGymChallengerTrainerClass(void);

#endif // PLAYER_GYM_H