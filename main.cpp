// ---------------------------------------------------------------------------------------


#include <mod/amlmod.h>
#include <mod/logger.h>

#include "isautils.h"


// ---------------------------------------------------------------------------------------


MYMOD(net.juniordjjr.rusjj.vmi, ViewModelsInfo, 1.1, JuniorDjjr & RusJJ)
NEEDGAME(com.rockstargames.gtasa)
BEGIN_DEPLIST()
    ADD_DEPENDENCY_VER(net.rusjj.aml, 1.2.1)
    ADD_DEPENDENCY_VER(net.rusjj.gtasa.utils, 1.3.0)
END_DEPLIST()


// ---------------------------------------------------------------------------------------


uintptr_t pGTASA;
void* hGTASA;
ISAUtils* sautils;
const intptr_t g_nCoronaIdent = 13371334;


// ---------------------------------------------------------------------------------------


#define DO_SHARED_INIT
#include "shared.h"


// ---------------------------------------------------------------------------------------


void FixAspectRatio(float* x, float* y)
{
    float resX = (float)RsGlobal->maximumWidth;
    float resY = (float)RsGlobal->maximumHeight;
    resY *= 1.33333333f;
    resX /= resY;

    *x /= resX;
    *y /= 1.07142857f;
}

void DrawString(std::string text, float posX, float posY, float sizeX, float sizeY)
{
    if (sizeX < 0.01f) return; // size limit

    float alpha = 0.0f;
    if (sizeX < 0.1f)
    {
        alpha += (sizeX * 10.0f);
        sizeX = 0.1f;
    }
    else
    {
        alpha = 1.0f;
    }
    int finalAlpha = (int)(alpha * 255.0f);
    if (sizeY < 0.2f) sizeY = 0.2f;

    CRGBA fontColor = { 255, 255, 255, 255 };
    fontColor.a = finalAlpha;
    CRGBA edgeColor = { 0, 0, 0, 255 };
    edgeColor.a = finalAlpha;

    sizeX *= 1.2f;
    sizeY *= 1.2f;
    FixAspectRatio(&sizeX, &sizeY);

    float magicResolutionWidth = RsGlobal->maximumWidth * 0.0015625f;
    float magicResolutionHeight = RsGlobal->maximumHeight * 0.002232143f;
    float finalSizeX = sizeX * magicResolutionWidth;
    float finalSizeY = sizeY * magicResolutionHeight;

    Font_SetScale(finalSizeX, finalSizeY);
    Font_SetFontStyle(1);
    Font_SetProportional(true);
    Font_SetJustify(true);
    Font_SetOrientation(eFontAlignment::ALIGN_CENTER);

    if (finalAlpha > 128)
    {
        Font_SetEdge(1);
        Font_SetDropColor(edgeColor);
    }
    else
    {
        Font_SetEdge(0);
    }

    Font_SetBackground(false, false);
    Font_SetColor(fontColor);

    Font_SetCentreSize(640.0f * magicResolutionWidth);

    static uint16_t gxtArray[64];
    memset(gxtArray, 0, sizeof(gxtArray));
    AsciiToGxtChar(&text[0], &gxtArray[0]);
    Font_PrintString(posX * magicResolutionWidth, posY * magicResolutionHeight, gxtArray);
}


// ---------------------------------------------------------------------------------------


#define PreRenderPatch(__i) \
    DECL_HOOKv(EntityPreRender##__i, CEntity* self) { EntityPreRender##__i(self); if(nModStatus == MODSTAT_SHOWBOTH || nModStatus == MODSTAT_SHOWENTS) entityList.push_back(self); }

#define PedPreRenderPatch(__i) \
    DECL_HOOKv(PedPreRender##__i, CPed* self) { PedPreRender##__i(self); if((nModStatus == MODSTAT_SHOWBOTH || nModStatus == MODSTAT_SHOWENTS) && bShowPeds) entityList.push_back(self); }

#define PlayerPedPreRenderPatch(__i) \
    DECL_HOOKv(PlayerPedPreRender##__i, CPlayerPed* self) { PlayerPedPreRender##__i(self); if((nModStatus == MODSTAT_SHOWBOTH || nModStatus == MODSTAT_SHOWENTS) && bShowPeds) \
                                                            if(self->m_nModelIndex != 0 || FindPlayerPed(-1) == self) entityList.push_back(self); }

#define VehiclePreRenderPatch(__i) \
    DECL_HOOKv(VehiclePreRender##__i, CVehicle* self) { VehiclePreRender##__i(self); if((nModStatus == MODSTAT_SHOWBOTH || nModStatus == MODSTAT_SHOWENTS) && bShowVehicles) entityList.push_back(self); }

PreRenderPatch(1);
PreRenderPatch(2);
PreRenderPatch(3);
PreRenderPatch(4);
PreRenderPatch(5);
PreRenderPatch(6);
PreRenderPatch(7);

PedPreRenderPatch(1);
PedPreRenderPatch(2);
PedPreRenderPatch(3);
PedPreRenderPatch(4);

PlayerPedPreRenderPatch(1);

VehiclePreRenderPatch(1);
VehiclePreRenderPatch(2);

DECL_HOOKv(DrawingEvent)
{
    DrawingEvent();

    if(nModStatus == MODSTAT_SHOWBOTH || nModStatus == MODSTAT_SHOWENTS)
    {
        for (CEntity* entity : entityList)
        {
            CVector screenPos2D;
            float sizeX, sizeY;
            if (CalcScreenCoors(&entity->GetPosition(), &screenPos2D, &sizeX, &sizeY, true, true))
            {
                float x = (screenPos2D.x / (float)RsGlobal->maximumWidth) * 640.0f;
                float y = (screenPos2D.y / (float)RsGlobal->maximumHeight) * 448.0f;
                sizeX = (sizeX / (float)RsGlobal->maximumWidth) * 15.0f;
                sizeY = (sizeY / (float)RsGlobal->maximumHeight) * 15.0f;

                std::string text = std::to_string(entity->m_nModelIndex);
                DrawString(text, x, y, sizeX, sizeY);
            }
        }
    }
    if(entityList.size() > 0) entityList.clear();
}

DECL_HOOKv(GameIdleEvent)
{
    GameIdleEvent();

    if(nModStatus == MODSTAT_SHOWBOTH || nModStatus == MODSTAT_SHOWAIM)
    {
        CPlayerPed* player = FindPlayerPed(-1);
      #ifdef AML32
        if(player && GetTaskUseGun(player->m_pIntelligence) != NULL)
      #else
        if(player && GetTaskUseGun(player->m_pPedIntelligence) != NULL)
      #endif
        {
            CVector startPos = player->GetPosition(), outCamPos, outPointPos;
            Find3rdPersonCamTargetVector(TheCamera, 200.0f, startPos, &outCamPos, &outPointPos);

            CColPoint outColPoint;
            CEntity* outEntity = NULL;

            CEntity* backupIgnoreEnt = *pIgnoreEntity;
            *pIgnoreEntity = player;
            
            if(ProcessLineOfSight(&outCamPos, &outPointPos, &outColPoint, &outEntity, true, true, true, true, false, false, false, false) && outEntity)
            {
                RegisterCorona(g_nCoronaIdent, NULL, 255, 100, 50, 255, &outColPoint.m_vecPoint, 1.0f, 500.0f, eCoronaType::CORONATYPE_SHINYSTAR, eCoronaFlareType::FLARETYPE_NONE, false, false, 0, 0.0f, false, 1.5f, 0, 50.0f, false, false);

                float sizeX, sizeY;
                CVector screenPos2D;
                if (CalcScreenCoors(&outColPoint.m_vecPoint, &screenPos2D, &sizeX, &sizeY, true, true))
                {
                    float x = (screenPos2D.x / (float)RsGlobal->maximumWidth) * 640.0f;
                    float y = (screenPos2D.y / (float)RsGlobal->maximumHeight) * 448.0f;
                    sizeX = (sizeX / (float)RsGlobal->maximumWidth) * 15.0f;
                    sizeY = (sizeY / (float)RsGlobal->maximumHeight) * 15.0f;

                    std::string text = std::to_string(outEntity->m_nModelIndex);
                    CBaseModelInfo* mi = ms_modelInfoPtrs[outEntity->m_nModelIndex];
                    if(mi && (uintptr_t)mi != (uintptr_t)-1 && mi->m_szModelName[0] != 0)
                    {
                        text += " (";
                        text += mi->m_szModelName;
                        text += ")";
                    }
                    DrawString(text, x, y, sizeX, sizeY);
                }
            }
            *pIgnoreEntity = backupIgnoreEnt;
        }
    }
}


// ---------------------------------------------------------------------------------------


void OnSettingSwitch_Enabled(int oldVal, int newVal, void* data)
{
    clampint(0, MODSTAT_MAX-1, &newVal);
    nModStatus = newVal;
}
void OnSettingSwitch_Peds(int oldVal, int newVal, void* data)
{
    clampint(0, 1, &newVal);
    bShowPeds = (newVal!=0);
}
void OnSettingSwitch_Vehicles(int oldVal, int newVal, void* data)
{
    clampint(0, 1, &newVal);
    bShowVehicles = (newVal!=0);
}

extern "C" void OnModLoad()
{
    logger->SetTag("ViewModelsInfo");
    
    pGTASA = aml->GetLib("libGTASA.so");
    hGTASA = aml->GetLibHandle("libGTASA.so");
    if(!pGTASA || !hGTASA)
    {
        logger->Error("Get a real GTA:SA first");
        return;
    }

    sautils = (ISAUtils*)GetInterface("SAUtils");
    if(!sautils)
    {
        logger->Error("SAUtils is missing");
        return;
    }

    // SAUtils
    static const char* pYesNo[] = { "FEM_OFF", "FEM_ON" };
    nModStatus = MODSTAT_DISABLED; // default
    sautils->AddClickableItem(eTypeOfSettings::SetType_Mods, "ViewModelsInfo Status", nModStatus, 0, MODSTAT_MAX-1, pModStatusSwitches, OnSettingSwitch_Enabled, NULL);
    bShowPeds = false; // default
    sautils->AddClickableItem(eTypeOfSettings::SetType_Mods, "ViewModelsInfo: Show Peds ID", bShowPeds, 0, 1, pYesNo, OnSettingSwitch_Peds, NULL);
    bShowVehicles = false; // default
    sautils->AddClickableItem(eTypeOfSettings::SetType_Mods, "ViewModelsInfo: Show Vehicles ID", bShowVehicles, 0, 1, pYesNo, OnSettingSwitch_Vehicles, NULL);
}

extern "C" void OnAllModsLoaded()
{
    // Hooks
    HOOKPLT(EntityPreRender1,    pGTASA + BYBIT(0x662010, 0x824F40));
    HOOKPLT(EntityPreRender2,    pGTASA + BYBIT(0x667C2C, 0x82FEA8));
    HOOKPLT(EntityPreRender3,    pGTASA + BYBIT(0x667C90, 0x82FF70));
    HOOKPLT(EntityPreRender4,    pGTASA + BYBIT(0x667D04, 0x830058));
    HOOKPLT(EntityPreRender5,    pGTASA + BYBIT(0x667E24, 0x8302A8));
    HOOKPLT(EntityPreRender6,    pGTASA + BYBIT(0x668BD4, 0x831E08));
    HOOKPLT(EntityPreRender7,    pGTASA + BYBIT(0x671618, 0x842730));
    HOOKPLT(PedPreRender1,       pGTASA + BYBIT(0x668AEC, 0x831C38));
    HOOKPLT(PedPreRender2,       pGTASA + BYBIT(0x668B60, 0x831D20));
    HOOKPLT(PedPreRender3,       pGTASA + BYBIT(0x668C38, 0x831ED0));
    HOOKPLT(PedPreRender4,       pGTASA + BYBIT(0x668CB0, 0x831FC0));
    HOOKPLT(PlayerPedPreRender1, pGTASA + BYBIT(0x6692D0, 0x833188));
    HOOKPLT(VehiclePreRender1,   pGTASA + BYBIT(0x66E268, 0x83D310));
    HOOKPLT(VehiclePreRender2,   pGTASA + BYBIT(0x66F490, 0x83F1A8));
  #ifdef AML32
    HOOKBLX(DrawingEvent,        pGTASA + 0x3F6464 + 0x1);
    HOOKBLX(GameIdleEvent,       pGTASA + 0x3F4122 + 0x1);
  #else
    HOOKBL(DrawingEvent,         pGTASA + 0x4D89FC);
    HOOKBL(GameIdleEvent,        pGTASA + 0x4D6494);
  #endif

    // Game Funcs
    SET_TO(CalcScreenCoors,              aml->GetSym(hGTASA, "_ZN7CSprite15CalcScreenCoorsERK5RwV3dPS0_PfS4_bb"));
    SET_TO(Font_SetScale,                aml->GetSym(hGTASA, "_ZN5CFont8SetScaleEf"));
    SET_TO(Font_SetFontStyle,            aml->GetSym(hGTASA, "_ZN5CFont12SetFontStyleEh"));
    SET_TO(Font_SetProportional,         aml->GetSym(hGTASA, "_ZN5CFont15SetProportionalEh"));
    SET_TO(Font_SetJustify,              aml->GetSym(hGTASA, "_ZN5CFont10SetJustifyEh"));
    SET_TO(Font_SetOrientation,          aml->GetSym(hGTASA, "_ZN5CFont14SetOrientationEh"));
    SET_TO(Font_SetEdge,                 aml->GetSym(hGTASA, "_ZN5CFont7SetEdgeEa"));
    SET_TO(Font_SetDropColor,            aml->GetSym(hGTASA, "_ZN5CFont12SetDropColorE5CRGBA"));
    SET_TO(Font_SetBackground,           aml->GetSym(hGTASA, "_ZN5CFont13SetBackgroundEhh"));
    SET_TO(Font_SetColor,                aml->GetSym(hGTASA, "_ZN5CFont8SetColorE5CRGBA"));
    SET_TO(Font_SetCentreSize,           aml->GetSym(hGTASA, "_ZN5CFont13SetCentreSizeEf"));
    SET_TO(Font_PrintString,             aml->GetSym(hGTASA, "_ZN5CFont11PrintStringEffPt"));
    SET_TO(AsciiToGxtChar,               aml->GetSym(hGTASA, "_Z14AsciiToGxtCharPKcPt"));
    SET_TO(FindPlayerPed,                aml->GetSym(hGTASA, "_Z13FindPlayerPedi"));
    SET_TO(GetTaskUseGun,                aml->GetSym(hGTASA, "_ZNK16CPedIntelligence13GetTaskUseGunEv"));
    SET_TO(Find3rdPersonCamTargetVector, aml->GetSym(hGTASA, "_ZN7CCamera28Find3rdPersonCamTargetVectorEf7CVectorRS0_S1_"));
    SET_TO(ProcessLineOfSight,           aml->GetSym(hGTASA, "_ZN6CWorld18ProcessLineOfSightERK7CVectorS2_R9CColPointRP7CEntitybbbbbbbb"));
    SET_TO(RegisterCorona,               aml->GetSym(hGTASA, BYBIT("_ZN8CCoronas14RegisterCoronaEjP7CEntityhhhhRK7CVectorffhhhhhfbfbfbb", "_ZN8CCoronas14RegisterCoronaEyP7CEntityhhhhRK7CVectorffhhhhhfbfbfbb")));

    // Game Vars
    SET_TO(RsGlobal,                     aml->GetSym(hGTASA, "RsGlobal"));
    SET_TO(TheCamera,                    aml->GetSym(hGTASA, "TheCamera"));
    SET_TO(pIgnoreEntity,                aml->GetSym(hGTASA, "_ZN6CWorld13pIgnoreEntityE"));
    SET_TO(ms_modelInfoPtrs,             *(uintptr_t*)(pGTASA + BYBIT(0x6796D4, 0x850DB8)));

    // Final!
    logger->Info("The mod has been loaded");
}


// ---------------------------------------------------------------------------------------
