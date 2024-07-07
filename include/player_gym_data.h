#ifndef GUARD_PLAYER_GYM_DATA
#define GUARD_PLAYER_GYM_DATA

struct GymChallengerTrainerClass
{
    u8 trainerClass;
    u8 trainerPic;
};

const struct GymChallengerTrainerClass classes_male[] = 
{
    { TRAINER_CLASS_HIKER, TRAINER_PIC_HIKER },
    { TRAINER_CLASS_TEAM_AQUA, TRAINER_PIC_AQUA_GRUNT_M },
    { TRAINER_CLASS_PKMN_BREEDER, TRAINER_PIC_POKEMON_BREEDER_M },
    { TRAINER_CLASS_COOLTRAINER, TRAINER_PIC_COOLTRAINER_M },
    { TRAINER_CLASS_COOLTRAINER_2, TRAINER_PIC_COOLTRAINER_M },
    { TRAINER_CLASS_BIRD_KEEPER, TRAINER_PIC_BIRD_KEEPER },
    { TRAINER_CLASS_COLLECTOR, TRAINER_PIC_COLLECTOR },
    { TRAINER_CLASS_SWIMMER_M, TRAINER_PIC_SWIMMER_M },
    { TRAINER_CLASS_TEAM_MAGMA, TRAINER_PIC_MAGMA_GRUNT_M },
    { TRAINER_CLASS_EXPERT, TRAINER_PIC_EXPERT_M },
    { TRAINER_CLASS_BLACK_BELT, TRAINER_PIC_BLACK_BELT },
    { TRAINER_CLASS_RUIN_MANIAC, TRAINER_PIC_RUIN_MANIAC },
    { TRAINER_CLASS_TUBER_M, TRAINER_PIC_TUBER_M },
    { TRAINER_CLASS_RICH_BOY, TRAINER_PIC_RICH_BOY },
    { TRAINER_CLASS_POKEMANIAC, TRAINER_PIC_POKEMANIAC },
    { TRAINER_CLASS_GUITARIST, TRAINER_PIC_GUITARIST },
    { TRAINER_CLASS_KINDLER, TRAINER_PIC_KINDLER },
    { TRAINER_CLASS_CAMPER, TRAINER_PIC_CAMPER },
    { TRAINER_CLASS_BUG_MANIAC, TRAINER_PIC_BUG_MANIAC },
    { TRAINER_CLASS_PSYCHIC, TRAINER_PIC_PSYCHIC_M },
    { TRAINER_CLASS_GENTLEMAN, TRAINER_PIC_GENTLEMAN },
    { TRAINER_CLASS_SCHOOL_KID, TRAINER_PIC_SCHOOL_KID_M },
    { TRAINER_CLASS_POKEFAN, TRAINER_PIC_POKEFAN_M },
    { TRAINER_CLASS_YOUNGSTER, TRAINER_PIC_YOUNGSTER },
    { TRAINER_CLASS_FISHERMAN, TRAINER_PIC_FISHERMAN },
    { TRAINER_CLASS_TRIATHLETE, TRAINER_PIC_CYCLING_TRIATHLETE_M },
    { TRAINER_CLASS_TRIATHLETE, TRAINER_PIC_RUNNING_TRIATHLETE_M },
    { TRAINER_CLASS_TRIATHLETE, TRAINER_PIC_SWIMMING_TRIATHLETE_M },
    { TRAINER_CLASS_DRAGON_TAMER, TRAINER_PIC_DRAGON_TAMER },
    { TRAINER_CLASS_NINJA_BOY, TRAINER_PIC_NINJA_BOY },
    { TRAINER_CLASS_SAILOR, TRAINER_PIC_SAILOR },
    { TRAINER_CLASS_BUG_CATCHER, TRAINER_PIC_BUG_CATCHER },
    { TRAINER_CLASS_PKMN_RANGER, TRAINER_PIC_POKEMON_RANGER_M },
};
#define NUM_TRAINER_CLASSES_MALE ARRAY_COUNT(classes_male)

const struct GymChallengerTrainerClass classes_female[] = 
{
    { TRAINER_CLASS_TEAM_AQUA, TRAINER_PIC_AQUA_GRUNT_F },
    { TRAINER_CLASS_PKMN_BREEDER, TRAINER_PIC_POKEMON_BREEDER_F },
    { TRAINER_CLASS_COOLTRAINER, TRAINER_PIC_COOLTRAINER_F },
    { TRAINER_CLASS_COOLTRAINER_2, TRAINER_PIC_COOLTRAINER_F },
    { TRAINER_CLASS_SWIMMER_F, TRAINER_PIC_SWIMMER_F },
    { TRAINER_CLASS_TEAM_MAGMA, TRAINER_PIC_MAGMA_GRUNT_F },
    { TRAINER_CLASS_EXPERT, TRAINER_PIC_EXPERT_F },
    { TRAINER_CLASS_HEX_MANIAC, TRAINER_PIC_HEX_MANIAC },
    { TRAINER_CLASS_AROMA_LADY, TRAINER_PIC_AROMA_LADY },
    { TRAINER_CLASS_INTERVIEWER, TRAINER_PIC_INTERVIEWER },
    { TRAINER_CLASS_TUBER_F, TRAINER_PIC_TUBER_F },
    { TRAINER_CLASS_LADY, TRAINER_PIC_LADY },
    { TRAINER_CLASS_BEAUTY, TRAINER_PIC_BEAUTY },
    { TRAINER_CLASS_PICNICKER, TRAINER_PIC_PICNICKER },
    { TRAINER_CLASS_PSYCHIC, TRAINER_PIC_PSYCHIC_F },
    { TRAINER_CLASS_SCHOOL_KID, TRAINER_PIC_SCHOOL_KID_F },
    { TRAINER_CLASS_POKEFAN, TRAINER_PIC_POKEFAN_F },
    { TRAINER_CLASS_TRIATHLETE, TRAINER_PIC_CYCLING_TRIATHLETE_F },
    { TRAINER_CLASS_TRIATHLETE, TRAINER_PIC_RUNNING_TRIATHLETE_F },
    { TRAINER_CLASS_TRIATHLETE, TRAINER_PIC_SWIMMING_TRIATHLETE_F },
    { TRAINER_CLASS_BATTLE_GIRL, TRAINER_CLASS_BATTLE_GIRL },
    { TRAINER_CLASS_PARASOL_LADY, TRAINER_PIC_PARASOL_LADY },
    { TRAINER_CLASS_SWIMMER_F, TRAINER_PIC_SWIMMER_F },
    { TRAINER_CLASS_PKMN_RANGER, TRAINER_PIC_POKEMON_RANGER_F },
    { TRAINER_CLASS_LASS, TRAINER_PIC_LASS },
};
#define NUM_TRAINER_CLASSES_FEMALE ARRAY_COUNT(classes_female)

#define NUM_NAMES_MALE 50

static const u8 name_sven[] = _("SVEN");
static const u8 name_ivan[] = _("IVAN");
static const u8 name_omar[] = _("OMAR");
static const u8 name_hiroshi[] = _("HIROSHI");
static const u8 name_akio[] = _("AKIO");
static const u8 name_mohammed[] = _("MOHAMMED");
static const u8 name_ali[] = _("ALI");
static const u8 name_sanjay[] = _("SANJAY");
static const u8 name_rajesh[] = _("RAJESH");
static const u8 name_juan[] = _("JUAN");
static const u8 name_luis[] = _("LUIS");
static const u8 name_carlos[] = _("CARLOS");
static const u8 name_jorge[] = _("JORGE");
static const u8 name_pablo[] = _("PABLO");
static const u8 name_angelo[] = _("ANGELO");
static const u8 name_francesco[] = _("FRANCESCO");
static const u8 name_luca[] = _("LUCA");
static const u8 name_ryan[] = _("RYAN");
static const u8 name_shawn[] = _("SHAWN");
static const u8 name_aaron[] = _("AARON");
static const u8 name_vladimir[] = _("VLADIMIR");
static const u8 name_nikita[] = _("NIKITA");
static const u8 name_dmitry[] = _("DMITRY");
static const u8 name_sergei[] = _("SERGEI");
static const u8 name_leonid[] = _("LEONID");
static const u8 name_nelson[] = _("NELSON");
static const u8 name_kofi[] = _("KOFI");
static const u8 name_kwame[] = _("KWAME");
static const u8 name_olaf[] = _("OLAF");
static const u8 name_finn[] = _("FINN");
static const u8 name_lars[] = _("LARS");
static const u8 name_gunnar[] = _("GUNNAR");
static const u8 name_marcus[] = _("MARCUS");
static const u8 name_simon[] = _("SIMON");
static const u8 name_lucas[] = _("LUCAS");
static const u8 name_martin[] = _("MARTIN");
static const u8 name_oliver[] = _("OLIVER");
static const u8 name_emil[] = _("EMIL");
static const u8 name_kai[] = _("KAI");
static const u8 name_samuel[] = _("SAMUEL");
static const u8 name_leo[] = _("LEO");
static const u8 name_alexander[] = _("ALEXANDER");
static const u8 name_arthur[] = _("ARTHUR");
static const u8 name_mateo[] = _("MATEO");
static const u8 name_eduardo[] = _("EDUARDO");
static const u8 name_gabriel[] = _("GABRIEL");
static const u8 name_yannick[] = _("YANNICK");
static const u8 name_ben[] = _("BEN");
static const u8 name_alex[] = _("ALEX");
static const u8 name_sam[] = _("SAM");

const u8* const names_male[] = 
{
    name_sven,
    name_ivan,
    name_omar,
    name_hiroshi,
    name_akio,
    name_mohammed,
    name_ali,
    name_sanjay,
    name_rajesh,
    name_juan,
    name_luis,
    name_carlos,
    name_jorge,
    name_pablo,
    name_angelo,
    name_francesco,
    name_luca,
    name_ryan,
    name_shawn,
    name_aaron,
    name_vladimir,
    name_nikita,
    name_dmitry,
    name_sergei,
    name_leonid,
    name_nelson,
    name_kofi,
    name_kwame,
    name_olaf,
    name_finn,
    name_lars,
    name_gunnar,
    name_marcus,
    name_simon,
    name_lucas,
    name_martin,
    name_oliver,
    name_emil,
    name_kai,
    name_samuel,
    name_leo,
    name_alexander,
    name_arthur,
    name_mateo,
    name_eduardo,
    name_gabriel,
    name_yannick,
    name_ben,
    name_alex,
    name_sam,
};

#define NUM_NAMES_FEMALE 50
static const u8 name_aisha[] = _("AISHA");
static const u8 name_ana[] = _("ANA");
static const u8 name_ayesha[] = _("AYESHA");
static const u8 name_bella[] = _("BELLA");
static const u8 name_carmen[] = _("CARMEN");
static const u8 name_daniela[] = _("DANIELA");
static const u8 name_elena[] = _("ELENA");
static const u8 name_farida[] = _("FARIDA");
static const u8 name_fatima[] = _("FATIMA");
static const u8 name_gabriela[] = _("GABRIELA");
static const u8 name_hana[] = _("HANA");
static const u8 name_ines[] = _("INES");
static const u8 name_isla[] = _("ISLA");
static const u8 name_jana[] = _("JANA");
static const u8 name_kai_female[] = _("KAI");
static const u8 name_laila[] = _("LAILA");
static const u8 name_linh[] = _("LINH");
static const u8 name_luciana[] = _("LUCIANA");
static const u8 name_malika[] = _("MALIKA");
static const u8 name_mariam[] = _("MARIAM");
static const u8 name_mei[] = _("MEI");
static const u8 name_mina[] = _("MINA");
static const u8 name_nadia[] = _("NADIA");
static const u8 name_nina[] = _("NINA");
static const u8 name_noor[] = _("NOOR");
static const u8 name_olga[] = _("OLGA");
static const u8 name_parvati[] = _("PARVATI");
static const u8 name_queenie[] = _("QUEENIE");
static const u8 name_rajni[] = _("RAJNI");
static const u8 name_rania[] = _("RANIA");
static const u8 name_sam_female[] = _("SAM");
static const u8 name_sasha[] = _("SASHA");
static const u8 name_sayuri[] = _("SAYURI");
static const u8 name_silvia[] = _("SILVIA");
static const u8 name_sofia[] = _("SOFIA");
static const u8 name_sonali[] = _("SONALI");
static const u8 name_sun[] = _("SUN");
static const u8 name_tala[] = _("TALA");
static const u8 name_tania[] = _("TANIA");
static const u8 name_tatiana[] = _("TATIANA");
static const u8 name_thuy[] = _("THUY");
static const u8 name_ulrika[] = _("ULRIKA");
static const u8 name_valentina[] = _("VALENTINA");
static const u8 name_van[] = _("VAN");
static const u8 name_wendy[] = _("WENDY");
static const u8 name_ximena[] = _("XIMENA");
static const u8 name_yara[] = _("YARA");
static const u8 name_yasmin[] = _("YASMIN");
static const u8 name_zara[] = _("ZARA");
static const u8 name_zuri[] = _("ZURI");
 
const u8* const names_female[] = {
    name_aisha,
    name_ana,
    name_ayesha,
    name_bella,
    name_carmen,
    name_daniela,
    name_elena,
    name_farida,
    name_fatima,
    name_gabriela,
    name_hana,
    name_ines,
    name_isla,
    name_jana,
    name_kai_female,
    name_laila,
    name_linh,
    name_luciana,
    name_malika,
    name_mariam,
    name_mei,
    name_mina,
    name_nadia,
    name_nina,
    name_noor,
    name_olga,
    name_parvati,
    name_queenie,
    name_rajni,
    name_rania,
    name_sam_female,
    name_sasha,
    name_sayuri,
    name_silvia,
    name_sofia,
    name_sonali,
    name_sun,
    name_tala,
    name_tania,
    name_tatiana,
    name_thuy,
    name_ulrika,
    name_valentina,
    name_van,
    name_wendy,
    name_ximena,
    name_yara,
    name_yasmin,
    name_zara,
    name_zuri,
};

#endif