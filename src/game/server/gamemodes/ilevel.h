#ifndef GAME_SERVER_GAMEMODES_ILEVEL_H
#define GAME_SERVER_GAMEMODES_ILEVEL_H
#include <game/server/gamecontroller.h>

class CGameControllerILevel : public IGameController
{
public:
	CGameControllerILevel(class CGameContext *pGameServer);
	virtual void Tick();
};
#endif
