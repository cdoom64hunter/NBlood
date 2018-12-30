#include <stdlib.h>
#include <string.h>
#include "compat.h"
#include "file_lib.h"
#include "common_game.h"

#include "asound.h"
#include "blood.h"
#include "config.h"
#include "credits.h"
#include "endgame.h"
#include "inifile.h"
#include "levels.h"
#include "loadsave.h"
#include "messages.h"
#include "network.h"
#include "screen.h"
#include "seq.h"
#include "sound.h"
#include "sfx.h"
#include "view.h"

GAMEOPTIONS gGameOptions;

GAMEOPTIONS gSingleGameOptions = {
    0, 2, 0, 0, "", "", 2, "", "", 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 3600, 1800, 1800, 7200
};

EPISODEINFO gEpisodeInfo[7];

int gSkill;
int gEpisodeCount;
int gNextLevel;
bool gGameStarted;

char BloodIniFile[128] = "BLOOD.INI";
IniFile *BloodINI;


void sub_26988(void)
{
    if (!SafeFileExists(BloodIniFile))
        ThrowError("Initialization: %s does not exist\n           Please reinstall\n", BloodIniFile);
    BloodINI = new IniFile(BloodIniFile);
}


void sub_269D8(const char *pzIni)
{
    strcpy(BloodIniFile, pzIni);
}

void levelPlayIntroScene(int nEpisode)
{
    sndStopSong();
    sndKillAllSounds();
    sfxKillAllSounds();
    ambKillAll();
    seqKillAll();
    EPISODEINFO *pEpisode = &gEpisodeInfo[nEpisode];
    // NUKE-TODO
    /*
    if (pEpisode->at9028)
        credPlaySmk(pEpisode->at8f08, pEpisode->at9028);
    else
        credPlaySmk(pEpisode->at8f08, pEpisode->at9030);
    */
    scrSetDac();
    viewResizeView(gViewSize);
    credReset();
    scrSetDac();
}

void levelPlayEndScene(int nEpisode)
{
    sndStopSong();
    sndKillAllSounds();
    sfxKillAllSounds();
    ambKillAll();
    seqKillAll();
    EPISODEINFO *pEpisode = &gEpisodeInfo[nEpisode];
    if (pEpisode->at902c)
        credPlaySmk(pEpisode->at8f98, pEpisode->at902c);
    else
        credPlaySmk(pEpisode->at8f98, pEpisode->at90c0);
    scrSetDac();
    viewResizeView(gViewSize);
    credReset();
    scrSetDac();
}

void levelClearSecrets(void)
{
    gSecretMgr.Clear();
}

void levelSetupSecret(int nCount)
{
    gSecretMgr.SetCount(nCount);
}

void levelTriggerSecret(int nSecret)
{
    gSecretMgr.Found(nSecret);
}

void CheckSectionAbend(const char *pzSection)
{
    if (!pzSection || !BloodINI->SectionExists(pzSection))
        ThrowError("Section [%s] expected in BLOOD.INI", pzSection);
}

void CheckKeyAbend(const char *pzSection, const char *pzKey)
{
    dassert(pzSection != NULL);

    if (!pzKey || !BloodINI->KeyExists(pzSection, pzKey))
        ThrowError("Key %s expected in section [%s] of BLOOD.INI", pzKey, pzSection);
}

LEVELINFO * levelGetInfoPtr(int nEpisode, int nLevel)
{
    dassert(nEpisode >= 0 && nEpisode < gEpisodeCount);
    EPISODEINFO *pEpisodeInfo = &gEpisodeInfo[nEpisode];
    dassert(nLevel >= 0 && nLevel < pEpisodeInfo->nLevels);
    return &pEpisodeInfo->at28[nLevel];
}

char * levelGetFilename(int nEpisode, int nLevel)
{
    dassert(nEpisode >= 0 && nEpisode < gEpisodeCount);
    EPISODEINFO *pEpisodeInfo = &gEpisodeInfo[nEpisode];
    dassert(nLevel >= 0 && nLevel < pEpisodeInfo->nLevels);
    return pEpisodeInfo->at28[nLevel].at0;
}

char * levelGetMessage(int nMessage)
{
    int nEpisode = gGameOptions.nEpisode;
    int nLevel = gGameOptions.nLevel;
    dassert(nMessage < kMaxMessages);
    char *pMessage = gEpisodeInfo[nEpisode].at28[nLevel].atec[nMessage];
    if (*pMessage == 0)
        return NULL;
    return pMessage;
}

char * levelGetTitle(void)
{
    int nEpisode = gGameOptions.nEpisode;
    int nLevel = gGameOptions.nLevel;
    char *pTitle = gEpisodeInfo[nEpisode].at28[nLevel].at90;
    if (*pTitle == 0)
        return NULL;
    return pTitle;
}

char * levelGetAuthor(void)
{
    int nEpisode = gGameOptions.nEpisode;
    int nLevel = gGameOptions.nLevel;
    char *pAuthor = gEpisodeInfo[nEpisode].at28[nLevel].atb0;
    if (*pAuthor == 0)
        return NULL;
    return pAuthor;
}

void levelSetupOptions(int nEpisode, int nLevel)
{
    gGameOptions.nEpisode = nEpisode;
    gGameOptions.nLevel = nLevel;
    strcpy(gGameOptions.zLevelName, gEpisodeInfo[nEpisode].at28[nLevel].at0);
    gGameOptions.uMapCRC = dbReadMapCRC(gGameOptions.zLevelName);
    strcpy(gGameOptions.zLevelSong, gEpisodeInfo[nEpisode].at28[nLevel].atd0);
    gGameOptions.nTrackNumber = gEpisodeInfo[nEpisode].at28[nLevel].ate0;
}

void levelLoadMapInfo(IniFile *pIni, LEVELINFO *pLevelInfo, const char *pzSection)
{
    char buffer[16];
    strncpy(pLevelInfo->at90, pIni->GetKeyString(pzSection, "Title", pLevelInfo->at0), 31);
    strncpy(pLevelInfo->atb0, pIni->GetKeyString(pzSection, "Author", ""), 31);
    strncpy(pLevelInfo->atd0, pIni->GetKeyString(pzSection, "Song", ""), 15);
    pLevelInfo->ate0 = pIni->GetKeyInt(pzSection, "Track", -1);
    pLevelInfo->ate4 = pIni->GetKeyInt(pzSection, "EndingA", -1);
    pLevelInfo->ate8 = pIni->GetKeyInt(pzSection, "EndingB", -1);
    pLevelInfo->at8ec = pIni->GetKeyInt(pzSection, "Fog", -0);
    pLevelInfo->at8ed = pIni->GetKeyInt(pzSection, "Weather", -0);
    for (int i = 0; i < kMaxMessages; i++)
    {
        sprintf(buffer, "Message%d", i+1);
        strncpy(pLevelInfo->atec[i], pIni->GetKeyString(pzSection, buffer, ""), 63);
    }
}

extern void MenuSetupEpisodeInfo(void);

void levelLoadDefaults(void)
{
    char buffer[64];
    char buffer2[16];
    memset(gEpisodeInfo, 0, sizeof(gEpisodeInfo));
    int i;
    for (i = 0; i < 6; i++)
    {
        sprintf(buffer, "Episode%ld", i+1);
        if (!BloodINI->SectionExists(buffer))
            break;
        EPISODEINFO *pEpisodeInfo = &gEpisodeInfo[i];
        strncpy(pEpisodeInfo->at0, BloodINI->GetKeyString(buffer, "Title", buffer), 31);
        strncpy(pEpisodeInfo->at8f08, BloodINI->GetKeyString(buffer, "CutSceneA", ""), 144);
        pEpisodeInfo->at9028 = BloodINI->GetKeyInt(buffer, "CurWavA", -1);
        if (pEpisodeInfo->at9028 == 0)
            strncpy(pEpisodeInfo->at9030, BloodINI->GetKeyString(buffer, "CutWavA", ""), 144);
        else
            pEpisodeInfo->at9030[0] = 0;
        strncpy(pEpisodeInfo->at8f98, BloodINI->GetKeyString(buffer, "CutSceneB", ""), 144);
        pEpisodeInfo->at902c = BloodINI->GetKeyInt(buffer, "CurWavB", -1);
        if (pEpisodeInfo->at902c == 0)
            strncpy(pEpisodeInfo->at90c0, BloodINI->GetKeyString(buffer, "CutWavB", ""), 144);
        else
            pEpisodeInfo->at90c0[0] = 0;

        pEpisodeInfo->bloodbath = BloodINI->GetKeyInt(buffer, "BloodBathOnly", 0);
        pEpisodeInfo->cutALevel = BloodINI->GetKeyInt(buffer, "CutSceneALevel", 0);
        if (pEpisodeInfo->cutALevel > 0)
            pEpisodeInfo->cutALevel--;
        int j;
        for (j = 0; j < 16; j++)
        {
            LEVELINFO *pLevelInfo = &pEpisodeInfo->at28[j];
            sprintf(buffer2, "Map%ld", j+1);
            if (!BloodINI->KeyExists(buffer, buffer2))
                break;
            const char *pMap = BloodINI->GetKeyString(buffer, buffer2, NULL);
            CheckSectionAbend(pMap);
            strncpy(pLevelInfo->at0, pMap, 143);
            levelLoadMapInfo(BloodINI, pLevelInfo, pMap);
        }
        pEpisodeInfo->nLevels = j;
    }
    gEpisodeCount = i;
    MenuSetupEpisodeInfo();
}

void levelAddUserMap(const char *pzMap)
{
    char buffer[144];
    strcpy(buffer, UserPath);
    strcat(buffer, pzMap);
    ChangeExtension(buffer, ".DEF");

    IniFile UserINI(buffer);
    int nEpisode = ClipRange(UserINI.GetKeyInt(NULL, "Episode", 0), 0, 5);
    EPISODEINFO *pEpisodeInfo = &gEpisodeInfo[nEpisode];
    int nLevel = ClipRange(UserINI.GetKeyInt(NULL, "Level", pEpisodeInfo->nLevels), 0, 15);
    if (nLevel >= pEpisodeInfo->nLevels)
    {
        if (pEpisodeInfo->nLevels == 0)
        {
            gEpisodeCount++;
            sprintf(pEpisodeInfo->at0, "Episode %d", nEpisode);
        }
        nLevel = pEpisodeInfo->nLevels++;
    }
    LEVELINFO *pLevelInfo = &pEpisodeInfo->at28[nLevel];
    ChangeExtension(buffer, ".MAP");
    strncpy(pLevelInfo->at0, buffer, 143);
    levelLoadMapInfo(&UserINI, pLevelInfo, NULL);
    gGameOptions.nEpisode = nEpisode;
    gGameOptions.nLevel = nLevel;
    gGameOptions.uMapCRC = dbReadMapCRC(pLevelInfo->at0);
    strcpy(gGameOptions.zLevelName, pLevelInfo->at0);
    MenuSetupEpisodeInfo();
}

void levelGetNextLevels(int nEpisode, int nLevel, int *pnEndingA, int *pnEndingB)
{
    dassert(pnEndingA != NULL && pnEndingB != NULL);
    LEVELINFO *pLevelInfo = &gEpisodeInfo[nEpisode].at28[nLevel];
    int nEndingA = pLevelInfo->ate4;
    if (nEndingA >= 0)
        nEndingA--;
    int nEndingB = pLevelInfo->ate4;
    if (nEndingB >= 0)
        nEndingB--;
    *pnEndingA = nEndingA;
    *pnEndingB = nEndingB;
}

void levelEndLevel(int arg)
{
    int nEndingA, nEndingB;
    EPISODEINFO *pEpisodeInfo = &gEpisodeInfo[gGameOptions.nEpisode];
    gGameOptions.uGameFlags |= 1;
    levelGetNextLevels(gGameOptions.nEpisode, gGameOptions.nLevel, &nEndingA, &nEndingB);
    switch (arg)
    {
    case 0:
        if (nEndingA == -1)
        {
            if (pEpisodeInfo->at8f98[0])
                gGameOptions.uGameFlags |= 8;
            gGameOptions.nLevel = 0;
            gGameOptions.uGameFlags |= 2;
        }
        else
            gNextLevel = nEndingA;
        break;
    case 1:
        if (nEndingB == -1)
        {
            if (gGameOptions.nEpisode + 1 < gEpisodeCount)
            {
                if (pEpisodeInfo->at8f98[0])
                    gGameOptions.uGameFlags |= 8;
                gGameOptions.nLevel = 0;
                gGameOptions.uGameFlags |= 2;
            }
            else
            {
                gGameOptions.nLevel = 0;
                gGameOptions.uGameFlags |= 1;
            }
        }
        else
            gNextLevel = nEndingB;
        break;
    }
}

void levelRestart(void)
{
    levelSetupOptions(gGameOptions.nEpisode, gGameOptions.nLevel);
    gStartNewGame = true;
}

class LevelsLoadSave : public LoadSave
{
    virtual void Load(void);
    virtual void Save(void);
};


static LevelsLoadSave myLoadSave;

void LevelsLoadSave::Load(void)
{
    Read(&gNextLevel, sizeof(gNextLevel));
    Read(&gGameOptions, sizeof(gGameOptions));
    Read(&gGameStarted, sizeof(gGameStarted));
}

void LevelsLoadSave::Save(void)
{
    Write(&gNextLevel, sizeof(gNextLevel));
    Write(&gGameOptions, sizeof(gGameOptions));
    Write(&gGameStarted, sizeof(gGameStarted));
}