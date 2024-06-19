.set LOCALID_OPPONENT, 3
.set LOCALID_TEST, 5

PlayerGym_MapScripts::
    map_script MAP_SCRIPT_ON_TRANSITION PlayerGym_MapScripts_OnTransition
    .byte 0

PlayerGym_MapScripts_OnTransition::
    end

PlayerGym_EventScript_ChooseType::
    lock
    faceplayer
    dynmultichoice 0, 0, FALSE, 5, 0, DYN_MULTICHOICE_CB_DEBUG, gText_Type_Normal, gText_Type_Fighting, gText_Type_Flying, gText_Type_Poison, gText_Type_Ground, gText_Type_Rock, gText_Type_Bug, gText_Type_Ghost, gText_Type_Steel, gText_Type_Fire, gText_Type_Water, gText_Type_Grass, gText_Type_Electric, gText_Type_Psychic, gText_Type_Ice, gText_Type_Dragon, gText_Type_Dark, gText_Type_Fairy
    savegymtype VAR_RESULT
PlayerGym_EventScript_Return:
    release
    end

PlayerGym_EventScript_StartGymBattle::
    lockall
    faceplayer
    msgbox PlayerGym_Text_StartGymBattleYesNo, MSGBOX_YESNO
    switch VAR_RESULT
    case YES, PlayerGym_EventScript_WalkToGymBattle
    case NO, PlayerGym_EventScript_Return

PlayerGym_EventScript_WalkToGymBattle:
    closemessage
    setgymopponent LOCALID_TEST, OBJ_EVENT_GFX_VAR_1, OBJ_EVENT_GFX_LEAF
    setgymopponent LOCALID_OPPONENT, OBJ_EVENT_GFX_VAR_0, OBJ_EVENT_GFX_LEAF

    trainerbattle_gym_challenger TRAINER_LEAF, PlayerGym_Text_BattleIntro, PlayerGym_Text_BattleLoss, PlayerGym_Text_BattleVictory, PlayerGym_EventScript_AfterBattle
    releaseall
    end

PlayerGym_EventScript_AfterBattle:
    msgbox PlayerGym_Text_AfterBattle, MSGBOX_DEFAULT
    closemessage
    goto PlayerGym_EventScript_Return


PlayerGym_Movement_WalkToGymBattle:
    walk_left
    walk_left
    walk_left
    walk_down
    walk_down
    walk_down
    walk_down
    walk_down
    step_end

PlayerGym_Movement_WalkToGymBattle_Opponent:
    walk_up
    walk_up
    walk_up
    walk_up
    walk_up
    walk_up
    walk_up
    step_end

PlayerGym_Text_StartGymBattleYesNo:
    .string "Do you want to start a battle?$"

PlayerGym_Text_BattleIntro:
    .string "Let's fight!$"

PlayerGym_Text_BattleLoss:
    .string "Tough!$"

PlayerGym_Text_BattleVictory:
    .string "I won!$"

PlayerGym_Text_AfterBattle:
    .string "Good Battle!$"