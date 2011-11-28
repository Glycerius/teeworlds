#include "ilevel.h"

CGameControllerILevel::CGameControllerILevel(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	m_pGameType = "iLevel";
}

void CGameControllerILevel::Tick()
{
	IGameController::Tick();
}