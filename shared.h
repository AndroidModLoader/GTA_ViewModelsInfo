#ifdef AML32
    #include "GTASA_STRUCTS.h"
#else
    #include "GTASA_STRUCTS_210.h"
#endif
#include <list>
#include <string>

#ifndef DO_SHARED_INIT
    #define EXTERNAL extern
#else
    #define EXTERNAL
#endif

enum eModStatus
{
    MODSTAT_DISABLED = 0,
    MODSTAT_SHOWBOTH,
    MODSTAT_SHOWENTS,
    MODSTAT_SHOWAIM,

    MODSTAT_MAX
};

static const char* pModStatusSwitches[MODSTAT_MAX] = 
{
    "Disabled",
    "Show All",
    "Show Entities",
    "Show Aimed ID",
};

EXTERNAL bool (*CalcScreenCoors)(const CVector *In, CVector *pResult, float *pScaleX, float *pScaleY, bool ClipFarPlane, bool ClipNearPlane);
EXTERNAL void (*Font_SetScale)(float, float);
EXTERNAL void (*Font_SetFontStyle)(uint8_t);
EXTERNAL void (*Font_SetProportional)(uint8_t);
EXTERNAL void (*Font_SetJustify)(uint8_t);
EXTERNAL void (*Font_SetOrientation)(eFontAlignment);
EXTERNAL void (*Font_SetEdge)(int8_t);
EXTERNAL void (*Font_SetDropColor)(CRGBA&);
EXTERNAL void (*Font_SetBackground)(uint8_t, uint8_t);
EXTERNAL void (*Font_SetColor)(CRGBA&);
EXTERNAL void (*Font_SetCentreSize)(float);
EXTERNAL void (*Font_PrintString)(float, float, uint16_t*);
EXTERNAL void (*AsciiToGxtChar)(const char*, uint16_t*);
EXTERNAL CPlayerPed* (*FindPlayerPed)(int);
EXTERNAL void* (*GetTaskUseGun)(CPedIntelligence*);
EXTERNAL bool (*Find3rdPersonCamTargetVector)(CCamera *self, float fRange, CVector vecGunMuzzle, CVector *vecSource, CVector *vecTarget);
EXTERNAL bool (*ProcessLineOfSight)(const CVector *vecStart, const CVector *vecEnd, CColPoint *colPoint, CEntity **refEntityPtr, bool bCheckBuildings, bool bCheckVehicles, bool bCheckPeds, bool bCheckObjects, bool bCheckDummies, bool bSeeThroughStuff, bool bIgnoreSomeObjectsForCamera, bool bShootThroughStuff);
EXTERNAL void (*RegisterCorona)(uintptr_t ID, CEntity *pAttachedToEntity, UInt8 R, UInt8 G, UInt8 B, UInt8 Intensity, const CVector *Coors, float Size, float Range, UInt8 CoronaType, UInt8 FlareType, UInt8 ReflType, UInt8 LOSCheck, UInt8 UsesTrails, float fNormalAngle, bool bNeonFade, float ArgPullTowardsCam, bool bFullBrightAtStart, float FadeSpeed, bool bOnlyFromBelow, bool bWhiteCore);

EXTERNAL RsGlobalType *RsGlobal;
EXTERNAL CCamera *TheCamera;
EXTERNAL CEntity **pIgnoreEntity;
EXTERNAL CBaseModelInfo **ms_modelInfoPtrs;

EXTERNAL std::list<CEntity*> entityList;
EXTERNAL int nModStatus;
EXTERNAL int bShowPeds;
EXTERNAL int bShowVehicles;