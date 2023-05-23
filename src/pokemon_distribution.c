#include "global.h"
#include "data.h"
#include "pokemon_distribution.h"
#include "characters.h"
#include "event_data.h"
#include "pokemon.h"
#include "malloc.h"
#include "string_util.h"

struct TransferData
{
    u8 pokeball;
    u8 level;
    u16 species;
    u16 heldItem;
    u16 moves[MAX_MON_MOVES];
    u32 hpIV:5;
    u32 attackIV:5;
    u32 defenseIV:5;
    u32 spAttackIV:5;
    u32 spDefenseIV:5;
    u32 speedIV:5;
    u32 abilityNum:2;
    u32 personality;
};

#define TRANSFER_SIZE sizeof(struct TransferData)
#define STRING_SIZE (TRANSFER_SIZE + sizeof(u32))
#define BASE64_SIZE ((STRING_SIZE*8)/6 + ((STRING_SIZE % 8 > 0) ? 1 : 0))
#define PADDING_SIZE ((STRING_SIZE % 3 == 1) ? 2 : ((STRING_SIZE % 3 == 2) ? 1 : 0))
#define WHITESPACE 64
#define EQUALS     65
#define INVALID    66

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0')

#define INT_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c"
#define INT_TO_BINARY(num)  \
  ((num) & 0x80000000 ? '1' : '0'), \
  ((num) & 0x40000000 ? '1' : '0'), \
  ((num) & 0x20000000 ? '1' : '0'), \
  ((num) & 0x10000000 ? '1' : '0'), \
  ((num) & 0x8000000 ? '1' : '0'), \
  ((num) & 0x4000000 ? '1' : '0'), \
  ((num) & 0x2000000 ? '1' : '0'), \
  ((num) & 0x1000000 ? '1' : '0'), \
  ((num) & 0x800000 ? '1' : '0'), \
  ((num) & 0x400000 ? '1' : '0'), \
  ((num) & 0x200000 ? '1' : '0'), \
  ((num) & 0x100000 ? '1' : '0'), \
  ((num) & 0x80000 ? '1' : '0'), \
  ((num) & 0x40000 ? '1' : '0'), \
  ((num) & 0x20000 ? '1' : '0'), \
  ((num) & 0x10000 ? '1' : '0'), \
  ((num) & 0x8000 ? '1' : '0'), \
  ((num) & 0x4000 ? '1' : '0'), \
  ((num) & 0x2000 ? '1' : '0'), \
  ((num) & 0x1000 ? '1' : '0'), \
  ((num) & 0x800 ? '1' : '0'), \
  ((num) & 0x400 ? '1' : '0'), \
  ((num) & 0x200 ? '1' : '0'), \
  ((num) & 0x100 ? '1' : '0'), \
  ((num) & 0x80 ? '1' : '0'), \
  ((num) & 0x40 ? '1' : '0'), \
  ((num) & 0x20 ? '1' : '0'), \
  ((num) & 0x10 ? '1' : '0'), \
  ((num) & 0x08 ? '1' : '0'), \
  ((num) & 0x04 ? '1' : '0'), \
  ((num) & 0x02 ? '1' : '0'), \
  ((num) & 0x01 ? '1' : '0')

// lookup table
static const u8 d[] =
{
    64,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66, // 24
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,62,66,66,66, // 49
    66,66,66,65,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66, // 74
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66, // 99
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66, // 124
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66, // 149
    66,66,66,66,66,66,66,66,66,66,66,52,53,54,55,56,57,58,59,60,61,66,66,66,66, // 174
    66,66,66,66,66,66,66,66,66,66,66,63, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12, // 199
    13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37, // 224
    38,39,40,41,42,43,44,45,46,47,48,49,50,51,66,66,66,66,66,66,66,66,66,66,66, // 249
    66,66,66,66,66,66                                                           // 255
};

static void pokemonToBuffer(struct Pokemon *pokemon, u8 *dst, u32 len);
static void stringToBuffer(u8 *src, struct Pokemon *pokemonBuffer, u32 len);
static void cypher(u8 *enc, u8 *src, u8 key, u32 len);
static void djb33_hash(u8 *hash, u8 *src, u32 len);
static u32 base64encode(const void* data_buf, size_t dataLength, char* result, size_t resultSize);
static u32 base64decode (u8 *in, u32 inLen, u8 *out, u32 *outLen);

// fill the transfer data and copy the data to the char array
static void pokemonToBuffer(struct Pokemon *pokemon, u8* dst, u32 len)
{
    struct TransferData transferData =
    {
        .pokeball = GetMonData(pokemon, MON_DATA_POKEBALL),
        .level = GetMonData(pokemon, MON_DATA_LEVEL),
        .species = GetMonData(pokemon, MON_DATA_SPECIES),
        .heldItem = GetMonData(pokemon, MON_DATA_HELD_ITEM),
        .moves[0] = GetMonData(pokemon, MON_DATA_MOVE1),
        .moves[1] = GetMonData(pokemon, MON_DATA_MOVE2),
        .moves[2] = GetMonData(pokemon, MON_DATA_MOVE3),
        .moves[3] = GetMonData(pokemon, MON_DATA_MOVE4),
        .hpIV = GetMonData(pokemon, MON_DATA_HP_IV),
        .attackIV = GetMonData(pokemon, MON_DATA_ATK_IV),
        .defenseIV = GetMonData(pokemon, MON_DATA_DEF_IV),
        .spAttackIV = GetMonData(pokemon, MON_DATA_SPATK_IV),
        .spDefenseIV = GetMonData(pokemon, MON_DATA_SPDEF_IV),
        .speedIV = GetMonData(pokemon, MON_DATA_SPEED_IV),
        .abilityNum = GetMonData(pokemon, MON_DATA_ABILITY_NUM),
        .personality = GetMonData(pokemon, MON_DATA_PERSONALITY),
    };

    memcpy(dst, &transferData, len);
}

// create a mon with the base arguments and then set any addition argument
static void stringToBuffer(u8 *src, struct Pokemon *pokemonBuffer, u32 len)
{
    u8 i, temp;
    struct TransferData *transferData = Alloc(sizeof(struct TransferData));
    memcpy(transferData, src, len);

    CreateMon(pokemonBuffer, transferData->species, transferData->level, 0, 1, transferData->personality, OT_ID_PLAYER_ID, 0);

    temp = transferData->abilityNum;
    SetMonData(pokemonBuffer, MON_DATA_ABILITY_NUM, &temp);
    temp = transferData->speedIV;
    SetMonData(pokemonBuffer, MON_DATA_SPEED_IV, &temp);
    temp = transferData->spDefenseIV;
    SetMonData(pokemonBuffer, MON_DATA_SPDEF_IV, &temp);
    temp = transferData->spAttackIV;
    SetMonData(pokemonBuffer, MON_DATA_SPATK_IV, &temp);
    temp = transferData->defenseIV;
    SetMonData(pokemonBuffer, MON_DATA_DEF_IV, &temp);
    temp = transferData->attackIV;
    SetMonData(pokemonBuffer, MON_DATA_ATK_IV, &temp);
    temp = transferData->hpIV;
    SetMonData(pokemonBuffer, MON_DATA_HP_IV, &temp);

    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        SetMonData(pokemonBuffer, MON_DATA_MOVE1 + i, &transferData->moves[i]);
        SetMonData(pokemonBuffer, MON_DATA_PP1 + i, &gBattleMoves[transferData->moves[i]].pp);
    }

    SetMonData(pokemonBuffer, MON_DATA_HELD_ITEM, &transferData->heldItem);
    SetMonData(pokemonBuffer, MON_DATA_POKEBALL, &transferData->pokeball);
    CalculateMonStats(pokemonBuffer);

    Free(transferData);
}

static void cypher(u8 *enc, u8 *src, u8 key, u32 len)
{
    u8 i;
    for (i = 0; i < len; i++)
    {
        enc[i] = src[i] ^ key;
    }
}

static void djb33_hash(u8 *hash, u8 *src, u32 len)
{
    u32 h = 5381;

    while(len--)
    {
        /* h = 33 * h ^ s[i]; */
        h += (h << 5);
        h ^= *src++;
    }
    DebugPrintfLevel(MGBA_LOG_ERROR, "hash func: "INT_TO_BINARY_PATTERN, INT_TO_BINARY(h));
    hash[3] = (u8)(h >> 0);
    hash[2] = (u8)(h >> 8);
    hash[1] = (u8)(h >> 16);
    hash[0] = (u8)(h >> 24);
}

static u32 base64encode(const void* data_buf, size_t dataLength, char* result, size_t resultSize)
{
    const char base64chars[] = {
        CHAR_A, CHAR_B, CHAR_C, CHAR_D, CHAR_E, CHAR_F, CHAR_G, CHAR_H, // 7
        CHAR_I, CHAR_J, CHAR_K, CHAR_L, CHAR_M, CHAR_N, CHAR_O, CHAR_P, // 15
        CHAR_Q, CHAR_R, CHAR_S, CHAR_T, CHAR_U, CHAR_V, CHAR_W, CHAR_X, // 23
        CHAR_Y, CHAR_Z,                                                 // 25
        CHAR_a, CHAR_b, CHAR_c, CHAR_d, CHAR_e, CHAR_f, CHAR_g, CHAR_h, // 33
        CHAR_i, CHAR_j, CHAR_k, CHAR_l, CHAR_m, CHAR_n, CHAR_o, CHAR_p, // 41
        CHAR_q, CHAR_e, CHAR_s, CHAR_t, CHAR_u, CHAR_v, CHAR_w, CHAR_x, // 49
        CHAR_y, CHAR_z,                                                 // 51
        CHAR_0, CHAR_1, CHAR_2, CHAR_3, CHAR_4, CHAR_5, CHAR_6, CHAR_7, // 59
        CHAR_8, CHAR_9,                                                 // 61
        CHAR_PLUS, CHAR_SLASH,                                          // 64
    };

    const char *data = (const u8 *)data_buf;
    size_t resultIndex = 0;
    size_t x;
    u32 n = 0;
    int padCount = dataLength % 3;
    u8 n0, n1, n2, n3;

    /* increment over the length of the string, three characters at a time */
    for (x = 0; x < dataLength; x += 3)
    {
        /* these three 8-bit (ASCII) characters become one 24-bit number */
        n = ((u32)data[x]) << 16; //parenthesis needed, compiler depending on flags can do the shifting before conversion to uint32_t, resulting to 0
        if((x+1) < dataLength)
            n += ((u32)data[x+1]) << 8;//parenthesis needed, compiler depending on flags can do the shifting before conversion to uint32_t, resulting to 0
        if((x+2) < dataLength)
            n += data[x+2];
        /* this 24-bit number gets separated into four 6-bit numbers */
        n0 = (u8)(n >> 18) & 63;
        n1 = (u8)(n >> 12) & 63;
        n2 = (u8)(n >> 6) & 63;
        n3 = (u8)n & 63;
        //DebugPrintfLevel(MGBA_LOG_ERROR, "n0,1,2,3: %d, %d, %d, %d", n0,n1,n2,n3);

        /*
        * if we have one byte available, then its encoding is spread
        * out over two characters
        */
        if(resultIndex >= resultSize) return 1;   /* indicate failure: buffer too small */
        result[resultIndex++] = base64chars[n0];
        //DebugPrintfLevel(MGBA_LOG_ERROR, "result n0: %d, %c", result[resultIndex-1], result[resultIndex-1]);
        //DebugPrintfLevel(MGBA_LOG_ERROR, "return 2, %d",resultIndex);
        if(resultIndex >= resultSize) return 1;   /* indicate failure: buffer too small */
        result[resultIndex++] = base64chars[n1];

        /*
        * if we have only two bytes available, then their encoding is
        * spread out over three chars
        */
        if((x+1) < dataLength)
        {
            if(resultIndex >= resultSize) return 1;   /* indicate failure: buffer too small */
            result[resultIndex++] = base64chars[n2];
        }

        /*
        * if we have all three bytes available, then their encoding is spread
        * out over four characters
        */
        if((x+2) < dataLength)
        {
            if(resultIndex >= resultSize) return 1;   /* indicate failure: buffer too small */
            result[resultIndex++] = base64chars[n3];
        }
    }

    /*
    * create and add padding that is required if we did not have a multiple of 3
    * number of characters available
    */
    if (padCount > 0)
    {
        for (; padCount < 3; padCount++)
        {
            if(resultIndex >= resultSize) return 1;   /* indicate failure: buffer too small */
            result[resultIndex++] = CHAR_EQUALS;
        }
    }
    if(resultIndex >= resultSize) return 1;   /* indicate failure: buffer too small */

    result[resultIndex] = EOS;
    return 0;   /* indicate success */
}

static u32 base64decode (u8 *in, u32 inLen, u8 *out, u32 *outLen)
{
    u8 *end = in + inLen;
    u8 iter = 0;
    u32 buf = 0;
    u32 len = 0;

    while (in < end) {
        u8 c = d[*in++];

        switch (c) {
        case WHITESPACE: continue;   /* skip whitespace */

        case INVALID:   return 1;   /* invalid input, return error */
        case EQUALS:                 /* pad character, end of data */
            in = end;
            continue;
        default:
            buf = buf << 6 | c;
            iter++; // increment the number of iteration
            /* If the buffer is full, split it into bytes */
            if (iter == 4) {
                if ((len += 3) > *outLen) return 1; /* buffer overflow */
                *(out++) = (buf >> 16) & 255;
                *(out++) = (buf >> 8) & 255;
                *(out++) = buf & 255;
                buf = 0; iter = 0;
            }
        }
    }
    if (iter == 3) {
        if ((len += 2) > *outLen) return 1; /* buffer overflow */
        *(out++) = (buf >> 10) & 255;
        *(out++) = (buf >> 2) & 255;
    }
    else if (iter == 2) {
        if (++len > *outLen) return 1; /* buffer overflow */
        *(out++) = (buf >> 4) & 255;
    }

    *outLen = len; /* modify to reflect the actual output size */
    return 0;
}

void CreatePasswordFromPokemon()
{
    u8 i, endl = EOS;
    u8 p = PADDING_SIZE; // calculate needed padding

    u8 *stringBuffer = Alloc(TRANSFER_SIZE);  // does not have an EOS
    u8 *stringBuffer2 = Alloc(STRING_SIZE);   // does not have an EOS
    u8 *encodedString = Alloc(BASE64_SIZE+1+p); // +1 to append EOS, +p for needed padding
    u8 *hashBuffer = Alloc(sizeof(u32));

    pokemonToBuffer(&gPlayerParty[0], stringBuffer, TRANSFER_SIZE);
    djb33_hash(hashBuffer, stringBuffer, TRANSFER_SIZE);
    memcpy(stringBuffer2+TRANSFER_SIZE, hashBuffer, sizeof(u32));
    cypher(stringBuffer2, stringBuffer, key, TRANSFER_SIZE);

    // base64encode returns 0 on success so we invert
    if (!base64encode((const char*)stringBuffer2, STRING_SIZE, encodedString, BASE64_SIZE+1+p))
    {
        gSpecialVar_Result = TRUE;
        StringCopy(gStringVar1, encodedString);
    }

    Free(hashBuffer);
    Free(stringBuffer);
    Free(stringBuffer2);
    Free(encodedString);
}

void CreatePokemonFromPassword()
{
    u8 p = PADDING_SIZE; // calculate needed padding
    u32 outLen = STRING_SIZE;
    u32 transferedHash, calculatedHash;
    u8 *decodedString = Alloc(STRING_SIZE);
    u8 *stringBuffer = Alloc(TRANSFER_SIZE);
    u8 *hashBuffer = Alloc(sizeof(u32));
    struct Pokemon *pokemonBuffer = Alloc(sizeof(struct Pokemon));

    // returns 0 on success so we invert
    if (!base64decode(gStringVar1, (BASE64_SIZE+1+p), decodedString, &outLen))
    {
        cypher(stringBuffer, decodedString, key, TRANSFER_SIZE);
        djb33_hash(hashBuffer, stringBuffer, TRANSFER_SIZE);

        transferedHash = (decodedString[TRANSFER_SIZE] << 24 | decodedString[TRANSFER_SIZE+1] << 16 | decodedString[TRANSFER_SIZE+2] << 8 | decodedString[TRANSFER_SIZE+3]);
        calculatedHash = (hashBuffer[0] << 24 | hashBuffer[1] << 16 | hashBuffer[2] << 8 | hashBuffer[3]);

        if (calculatedHash == transferedHash)
        {
            gSpecialVar_Result = TRUE;
            stringToBuffer(stringBuffer, pokemonBuffer, TRANSFER_SIZE);
            GiveMonToPlayer(pokemonBuffer);
        }
    }

    Free(hashBuffer);
    Free(decodedString);
    Free(stringBuffer);
    Free(pokemonBuffer);
}