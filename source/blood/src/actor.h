#pragma once
#include "build.h"
#include "common_game.h"
#include "blood.h"
#include "db.h"
#include "fx.h"
#include "gameutil.h"

enum DAMAGE_TYPE {
    DAMAGE_TYPE_0 = 0,
    DAMAGE_TYPE_1, // Flame
    DAMAGE_TYPE_2,
    DAMAGE_TYPE_3,
    DAMAGE_TYPE_4,
    DAMAGE_TYPE_5,
    DAMAGE_TYPE_6, // Tesla
};

enum VECTOR_TYPE {
    VECTOR_TYPE_0 = 0,
    VECTOR_TYPE_1,
    VECTOR_TYPE_2,
    VECTOR_TYPE_3,
    VECTOR_TYPE_4,
    VECTOR_TYPE_5,
    VECTOR_TYPE_6,
    VECTOR_TYPE_7,
    VECTOR_TYPE_8,
    VECTOR_TYPE_9,
    VECTOR_TYPE_10,
    VECTOR_TYPE_11,
    VECTOR_TYPE_12,
    VECTOR_TYPE_13,
    VECTOR_TYPE_14,
    VECTOR_TYPE_15,
    VECTOR_TYPE_16,
    VECTOR_TYPE_17,
    VECTOR_TYPE_18,
    VECTOR_TYPE_19,
    VECTOR_TYPE_20,
    VECTOR_TYPE_21,
    kVectorMax,
};

struct THINGINFO
{
    short at0; // health
    short at2; // mass
    unsigned char at4; // clipdist
    short at5; // flags
    int at7; // elasticity
    int atb; // damage resistance
    short atf; // cstat
    short at11; // picnum
    char at13; // shade
    unsigned char at14; // pal
    unsigned char at15; // xrepeat
    unsigned char at16; // yrepeat
    int at17[7]; // damage
};

struct AMMOITEMDATA
{
    short at0;
    short picnum; // at2
    char shade; // at4
    char at5;
    unsigned char xrepeat; // at6
    unsigned char yrepeat; // at7
    short at8;
    unsigned char ata;
    unsigned char atb;
};

struct WEAPONITEMDATA
{
    short at0;
    short picnum; // at2
    char shade; // at4
    char at5;
    unsigned char xrepeat; // at6
    unsigned char yrepeat; // at7
    short at8;
    short ata;
    short atc;
};

struct ITEMDATA
{
    short at0; // unused?
    short picnum; // at2
    char shade; // at4
    char at5; // unused?
    unsigned char xrepeat; // at6
    unsigned char yrepeat; // at7
    short at8;
};

struct MissileType
{
    short picnum;
    int at2; // speed
    int at6; // angle
    unsigned char ata; // xrepeat
    unsigned char atb; // yrepeat
    char atc; // shade
    unsigned char atd; // clipdist
};

struct EXPLOSION
{
    unsigned char at0;
    char at1; // dmg
    char at2; // dmg rnd
    int at3; // radius
    int at7;
    int atb;
    int atf;
    int at13;
    int at17;
};

struct VECTORDATA_at1d {
    FX_ID at0;
    FX_ID at1;
    FX_ID at2;
    int at3;
};

struct VECTORDATA {
    DAMAGE_TYPE at0;
    int at1; // damage
    int at5;
    int at9; // range
    int atd;
    int at11; // burn
    int at15; // blood splats
    int at19; // blood splat chance
    VECTORDATA_at1d at1d[15];
};

struct SPRITEHIT {
    int hit, ceilhit, florhit;
};

extern AMMOITEMDATA gAmmoItemData[];
extern WEAPONITEMDATA gWeaponItemData[];
extern ITEMDATA gItemData[];
extern MissileType missileInfo[];
extern EXPLOSION explodeInfo[];
extern THINGINFO thingInfo[];
extern VECTORDATA gVectorData[22];

extern SPRITEHIT gSpriteHit[];

extern int gDudeDrag;
extern short gAffectedSectors[kMaxSectors];
extern short gAffectedXWalls[kMaxXWalls];

inline void GetSpriteExtents(SPRITE *pSprite, int *top, int *bottom)
{
    *top = *bottom = pSprite->z;
    if ((pSprite->cstat & 0x30) != 0x20)
    {
        int height = tilesiz[pSprite->picnum].y;
        int center = height / 2 + qpicanm[pSprite->picnum].yoffset;
        *top -= (pSprite->yrepeat << 2)*center;
        *bottom += (pSprite->yrepeat << 2)*(height - center);
    }
}


inline bool IsPlayerSprite(SPRITE *pSprite)
{
    if (pSprite->type >= kDudePlayer1 && pSprite->type <= kDudePlayer8)
        return 1;
    return 0;
}

inline bool IsDudeSprite(SPRITE *pSprite)
{
    if (pSprite->type >= kDudeBase && pSprite->type < kDudeMax)
        return 1;
    return 0;
}

inline void actBurnSprite(int nSource, XSPRITE *pXSprite, int nTime)
{
    pXSprite->at2c_0 = ClipHigh(pXSprite->at2c_0 + nTime, qsprite[pXSprite->reference].type == 6 ? 2400 : 1200);
    pXSprite->at2e_0 = nSource;
}

bool IsItemSprite(SPRITE *pSprite);
bool IsWeaponSprite(SPRITE *pSprite);
bool IsAmmoSprite(SPRITE *pSprite);
bool IsUnderwaterSector(int nSector);
int actSpriteOwnerToSpriteId(SPRITE *pSprite);
void actPropagateSpriteOwner(SPRITE *pTarget, SPRITE *pSource);
int actSpriteIdToOwnerId(int nSprite);
int actOwnerIdToSpriteId(int nSprite);
bool actTypeInSector(int nSector, int nType);
void actAllocateSpares(void);
void actInit(void);
void ConcussSprite(int a1, SPRITE *pSprite, int x, int y, int z, int a6);
int actWallBounceVector(long *x, long *y, int nWall, int a4);
int actFloorBounceVector(long *x, long *y, long *z, int nSector, int a5);
void sub_2A620(int nSprite, int x, int y, int z, int nSector, int nDist, int a7, int a8, DAMAGE_TYPE a9, int a10, int a11, int a12, int a13);
void sub_2AA94(SPRITE *pSprite, XSPRITE *pXSprite);
SPRITE *actSpawnFloor(SPRITE *pSprite);
SPRITE *actDropAmmo(SPRITE *pSprite, int nType);
SPRITE *actDropWeapon(SPRITE *pSprite, int nType);
SPRITE *actDropItem(SPRITE *pSprite, int nType);
SPRITE *actDropKey(SPRITE *pSprite, int nType);
SPRITE *actDropFlag(SPRITE *pSprite, int nType);
SPRITE *actDropObject(SPRITE *pSprite, int nType);
bool actHealDude(XSPRITE *pXDude, int a2, int a3);
void actKillDude(int a1, SPRITE *pSprite, DAMAGE_TYPE a3, int a4);
int actDamageSprite(int nSource, SPRITE *pSprite, DAMAGE_TYPE a3, int a4);
void actHitcodeToData(int a1, HITINFO *pHitInfo, int *a3, SPRITE **a4, XSPRITE **a5, int *a6, WALL **a7, XWALL **a8, int *a9, SECTOR **a10, XSECTOR **a11);
void actImpactMissile(SPRITE *pMissile, int a2);
void actKickObject(SPRITE *pSprite1, SPRITE *pSprite2);
void actTouchFloor(SPRITE *pSprite, int nSector);
void ProcessTouchObjects(SPRITE *pSprite, int nXSprite);
void actAirDrag(SPRITE *pSprite, int a2);
int MoveThing(SPRITE *pSprite);
void MoveDude(SPRITE *pSprite);
int MoveMissile(SPRITE *pSprite);
void actExplodeSprite(SPRITE *pSprite);
void actActivateGibObject(SPRITE *pSprite, XSPRITE *pXSprite);
bool IsUnderWater(SPRITE *pSprite);
void actProcessSprites(void);
SPRITE * actSpawnSprite(int nSector, int x, int y, int z, int nStat, char a6);
SPRITE *sub_36878(SPRITE *pSource, short nType, int a3, int a4);
SPRITE * actSpawnSprite(SPRITE *pSource, int nStat);
SPRITE * actSpawnThing(int nSector, int x, int y, int z, int nThingType);
SPRITE * actFireThing(SPRITE *pSprite, int a2, int a3, int a4, int thingType, int a6);
SPRITE* actFireMissile(SPRITE *pSprite, int a2, int a3, int a4, int a5, int a6, int nType);
int actGetRespawnTime(SPRITE *pSprite);
bool actCheckRespawn(SPRITE *pSprite);
bool actCanSplatWall(int nWall);
void actFireVector(SPRITE *pShooter, int a2, int a3, int a4, int a5, int a6, VECTOR_TYPE vectorType);
void actPostSprite(int nSprite, int nStatus);
void actPostProcess(void);
void MakeSplash(SPRITE *pSprite, XSPRITE *pXSprite);