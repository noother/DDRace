/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
#include "gamecontext.h"
#include <engine/shared/config.h>
#include <engine/server/server.h>
#include <game/server/teams.h>
#include <game/server/gamemodes/DDRace.h>
#include <game/version.h>

void CGameContext::ConGoLeft(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->MoveCharacter(ClientId, pResult->GetVictim(), -1, 0);
}

void CGameContext::ConGoRight(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->MoveCharacter(ClientId, pResult->GetVictim(), 1, 0);
}

void CGameContext::ConGoDown(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->MoveCharacter(ClientId, pResult->GetVictim(), 0, 1);
}

void CGameContext::ConGoUp(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->MoveCharacter(ClientId, pResult->GetVictim(), 0, -1);
}

void CGameContext::ConMove(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->MoveCharacter(ClientId, pResult->GetVictim(), pResult->GetInteger(0), pResult->GetInteger(1));
}

void CGameContext::ConMoveRaw(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->MoveCharacter(ClientId, pResult->GetVictim(), pResult->GetInteger(0), pResult->GetInteger(1), true);
}

void CGameContext::MoveCharacter(int ClientId, int Victim, int X, int Y, bool Raw)
{
	CCharacter* pChr = GetPlayerChar(ClientId);
	
	if(!pChr)
		return;

	pChr->Core()->m_Pos.x += ((Raw) ? 1 : 32) * X;
	pChr->Core()->m_Pos.y += ((Raw) ? 1 : 32) * Y;

	if(!g_Config.m_SvCheatTime)
		pChr->m_DDRaceState = DDRACE_CHEAT;
}

void CGameContext::ConMute(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	int Seconds = pResult->GetInteger(0);
	char aBuf[512];
	if(Seconds < 10)
		Seconds = 10;

	if(Victim == ClientId)
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "You can\'t mute yourself");
	else
	{
		/*pSelf->m_apPlayers[Victim]->m_Muted = Seconds * pSelf->Server()->TickSpeed();
		str_format(aBuf, sizeof(aBuf), "You have been muted by for %d seconds", pSelf->Server()->ClientName(Victim), Seconds);
		pSelf->SendChatTarget(Victim, aBuf);*/

		pSelf->m_apPlayers[Victim]->m_Muted = Seconds * pSelf->Server()->TickSpeed();
		str_format(aBuf, sizeof(aBuf), "%s muted for %d seconds", pSelf->Server()->ClientName(Victim), Seconds);
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
	}
}

void CGameContext::ConUnmute(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	char aBuf[512];

	if(Victim == ClientId)
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "You can\'t unmute yourself");
	else
	{
		if(pSelf->m_apPlayers[Victim]->m_Muted > 0)
		{
			pSelf->m_apPlayers[Victim]->m_Muted = 0;
			str_format(aBuf, sizeof(aBuf), "You have been unmuted", pSelf->Server()->ClientName(Victim));
			pSelf->SendChatTarget(Victim, aBuf);
		}

		/*if(pSelf->m_apPlayers[Victim]->m_Muted > 0)
		{
			pSelf->m_apPlayers[Victim]->m_Muted = 0;
			str_format(aBuf, sizeof(aBuf), "%s has been unmuted", pSelf->Server()->ClientName(Victim));
			pSelf->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
		}*/
	}
}

void CGameContext::ConSetlvl3(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	CServer* pServ = (CServer*)pSelf->Server();
	if(pSelf->m_apPlayers[Victim])
	{
		pSelf->m_apPlayers[Victim]->m_Authed = 3;
		pServ->SetRconLevel(Victim, 3);
	}
}

void CGameContext::ConSetlvl2(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	CServer* pServ = (CServer*)pSelf->Server();
	if(pSelf->m_apPlayers[Victim])
	{
		pSelf->m_apPlayers[Victim]->m_Authed = 2;
		pServ->SetRconLevel(Victim, 2);
	}
}

void CGameContext::ConSetlvl1(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	CServer* pServ = (CServer*)pSelf->Server();
	if(pSelf->m_apPlayers[Victim])
	{
		pSelf->m_apPlayers[Victim]->m_Authed = 1;
		pServ->SetRconLevel(Victim, 1);
	}
}

void CGameContext::ConLogOut(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();

	CServer* pServ = (CServer*)pSelf->Server();
	if(pSelf->m_apPlayers[Victim])
	{
		pSelf->m_apPlayers[Victim]->m_Authed = -1;
		pServ->SetRconLevel(Victim, -1);
	}
}

void CGameContext::ConKillPlayer(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();

	if(pSelf->m_apPlayers[Victim])
	{
		pSelf->m_apPlayers[Victim]->KillCharacter(WEAPON_GAME);
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "%s was killed by admin", pSelf->Server()->ClientName(Victim));
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
	}
}

void CGameContext::ConNinja(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->ModifyWeapons(ClientId, pResult->GetVictim(), WEAPON_NINJA, false);
}


void CGameContext::ConHammer(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();

	char aBuf[128];
	int Type = pResult->GetInteger(0);

	CCharacter* pChr = pSelf->GetPlayerChar(Victim);
	
	if(!pChr)
		return;
	
	CServer* pServ = (CServer*)pSelf->Server();
	if(Type>3 || Type<0)
	{
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Select hammer between 0 and 3");
	}
	else
	{
		pChr->m_HammerType = Type;
		if(!g_Config.m_SvCheatTime)
			pChr->m_DDRaceState = DDRACE_CHEAT;
		str_format(aBuf, sizeof(aBuf), "Hammer of '%s' ClientId=%d setted to %d", pServ->ClientName(ClientId), Victim, Type);
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
	}
}

void CGameContext::ConSuper(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	CCharacter* pChr = pSelf->GetPlayerChar(Victim);
	if(pChr && !pChr->m_Super)
	{
		pChr->m_Super = true;
		pChr->UnFreeze();
		pChr->m_TeamBeforeSuper = pChr->Team();
		pChr->Teams()->SetCharacterTeam(Victim, TEAM_SUPER);
		if(!g_Config.m_SvCheatTime)
			pChr->m_DDRaceState = DDRACE_CHEAT;
	}
}

void CGameContext::ConUnSuper(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	CCharacter* pChr = pSelf->GetPlayerChar(Victim);
	if(pChr && pChr->m_Super)
	{
		pChr->m_Super = false;
		pChr->Teams()->SetForceCharacterTeam(Victim, pChr->m_TeamBeforeSuper);
	}
}

void CGameContext::ConShotgun(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->ModifyWeapons(ClientId, pResult->GetVictim(), WEAPON_SHOTGUN, false);
}

void CGameContext::ConGrenade(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->ModifyWeapons(ClientId, pResult->GetVictim(), WEAPON_GRENADE, false);
}

void CGameContext::ConRifle(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->ModifyWeapons(ClientId, pResult->GetVictim(), WEAPON_RIFLE, false);
}

void CGameContext::ConWeapons(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->ModifyWeapons(ClientId, pResult->GetVictim(), -1, false);
}

void CGameContext::ConUnShotgun(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->ModifyWeapons(ClientId, pResult->GetVictim(), WEAPON_SHOTGUN, true);
}

void CGameContext::ConUnGrenade(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->ModifyWeapons(ClientId, pResult->GetVictim(), WEAPON_GRENADE, true);
}

void CGameContext::ConUnRifle(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->ModifyWeapons(ClientId, pResult->GetVictim(), WEAPON_RIFLE, true);
}

void CGameContext::ConUnWeapons(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->ModifyWeapons(ClientId, pResult->GetVictim(), -1, true);
}

void CGameContext::ConAddWeapon(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->ModifyWeapons(ClientId, pResult->GetVictim(), pResult->GetInteger(0), false);
}

void CGameContext::ConRemoveWeapon(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->ModifyWeapons(ClientId, pResult->GetVictim(), pResult->GetInteger(0), true);
}

void CGameContext::ModifyWeapons(int ClientId, int Victim, int Weapon, bool Remove)
{
	if(clamp(Weapon, -1, NUM_WEAPONS - 1) != Weapon)
	{
		Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "invalid weapon id");
		return;
	}
	
	CCharacter* pChr = GetPlayerChar(Victim);
	if(!pChr)
		return;
	
	if(Weapon == -1)
	{
		if(Remove && (pChr->GetActiveWeapon() == WEAPON_SHOTGUN || pChr->GetActiveWeapon() == WEAPON_GRENADE || pChr->GetActiveWeapon() == WEAPON_RIFLE))
			pChr->SetActiveWeapon(WEAPON_GUN);
		
		if(Remove)
		{
			pChr->SetWeaponGot(WEAPON_SHOTGUN, false);
			pChr->SetWeaponGot(WEAPON_GRENADE, false);
			pChr->SetWeaponGot(WEAPON_RIFLE, false);
		}
		else
			pChr->GiveAllWeapons();	
	}
	else if(Weapon != WEAPON_NINJA)
	{
		if(Remove && pChr->GetActiveWeapon() == Weapon)
			pChr->SetActiveWeapon(WEAPON_GUN);
		
		if(Remove)
			pChr->SetWeaponGot(Weapon, false);
		else
			pChr->GiveWeapon(Weapon, -1);
	}
	else
	{
		if(Remove)
		{
			Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "you can't remove ninja");
			return;
		}
		
		pChr->GiveNinja();
	}

	if(!Remove && !g_Config.m_SvCheatTime)
		pChr->m_DDRaceState =	DDRACE_CHEAT;
}

void CGameContext::ConTeleport(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	int TeleTo = clamp(pResult->GetInteger(0), 0, (int)MAX_CLIENTS-1);
	if(pSelf->m_apPlayers[TeleTo])
	{
		{
			CCharacter* pChr = pSelf->GetPlayerChar(Victim);
			if(pChr)
			{
				pChr->Core()->m_Pos = pSelf->m_apPlayers[TeleTo]->m_ViewPos;
				if(!g_Config.m_SvCheatTime)
					pChr->m_DDRaceState = DDRACE_CHEAT;
			}
		}
	}
}

void CGameContext::ConTimerStop(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	CServer* pServ = (CServer*)pSelf->Server();
	int Victim = pResult->GetVictim();

	char aBuf[128];
	CCharacter* pChr = pSelf->GetPlayerChar(Victim);
	if(!pChr)
		return;
	if(pSelf->m_apPlayers[Victim])
	{
		pChr->m_DDRaceState=DDRACE_CHEAT;
		str_format(aBuf, sizeof(aBuf), "'%s' ClientId=%d Hasn't time now (Timer Stopped)", pServ->ClientName(ClientId), Victim);
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
	}
}

void CGameContext::ConTimerStart(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	CServer* pServ = (CServer*)pSelf->Server();
	int Victim = pResult->GetVictim();

	char aBuf[128];
	CCharacter* pChr = pSelf->GetPlayerChar(Victim);
	if(!pChr)
		return;
	if(pSelf->m_apPlayers[Victim])
	{
		pChr->m_DDRaceState = DDRACE_STARTED;
		str_format(aBuf, sizeof(aBuf), "'%s' ClientId=%d Has time now (Timer Started)", pServ->ClientName(ClientId), Victim);
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
	}
}

void CGameContext::ConTimerZero(IConsole::IResult *pResult, void *pUserData, int ClientId)
{

	CGameContext *pSelf = (CGameContext *)pUserData;
	CServer* pServ = (CServer*)pSelf->Server();
	int Victim = pResult->GetVictim();

	char aBuf[128];
	CCharacter* pChr = pSelf->GetPlayerChar(Victim);
	if(!pChr)
		return;
	if(pSelf->m_apPlayers[Victim])
	{
		pChr->m_StartTime = pSelf->Server()->Tick();
		pChr->m_RefreshTime = pSelf->Server()->Tick();
		pChr->m_DDRaceState=DDRACE_CHEAT;
		str_format(aBuf, sizeof(aBuf), "'%s' ClientId=%d time has been reset & stopped.", pServ->ClientName(ClientId), Victim);
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
	}
}

void CGameContext::ConTimerReStart(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	CServer* pServ = (CServer*)pSelf->Server();

	char aBuf[128];
	CCharacter* pChr = pSelf->GetPlayerChar(Victim);
	if(!pChr)
		return;
	if(pSelf->m_apPlayers[Victim])
	{
		pChr->m_StartTime = pSelf->Server()->Tick();
		pChr->m_RefreshTime = pSelf->Server()->Tick();
		pChr->m_DDRaceState=DDRACE_STARTED;
		str_format(aBuf, sizeof(aBuf), "'%s' ClientId=%d time has been reset & stopped.", pServ->ClientName(ClientId), Victim);
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
	}
}

void CGameContext::ConFreeze(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Seconds = -1;
	int Victim = pResult->GetVictim();

	char aBuf[128];

	if(pResult->NumArguments())
		Seconds = clamp(pResult->GetInteger(0), -2, 9999);
	
	CCharacter* pChr = pSelf->GetPlayerChar(Victim);
	if(!pChr)
		return;
	
	if(pSelf->m_apPlayers[Victim])
	{
		pChr->Freeze(Seconds);
		pChr->GetPlayer()->m_RconFreeze = Seconds != -2;
		CServer* pServ = (CServer*)pSelf->Server();
		if(Seconds >= 0)
			str_format(aBuf, sizeof(aBuf), "'%s' ClientId=%d has been Frozen for %d.", pServ->ClientName(ClientId), Victim, Seconds);
		else if(Seconds == -2)
		{
			pChr->m_DeepFreeze = true;
			str_format(aBuf, sizeof(aBuf), "'%s' ClientId=%d has been Deep Frozen.", pServ->ClientName(ClientId), Victim);
		}
		else
			str_format(aBuf, sizeof(aBuf), "'%s' ClientId=%d is Frozen until you unfreeze him.", pServ->ClientName(ClientId), Victim);
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
	}

}

void CGameContext::ConUnFreeze(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	static bool Warning = false;
	char aBuf[128];
	CCharacter* pChr = pSelf->GetPlayerChar(Victim);
	if(!pChr)
		return;
	if(pChr->m_DeepFreeze && !Warning)
	{
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "warning", "This client is deeply frozen, repeat the command to defrost him.");
		Warning = true;
		return;
	}
	if(pChr->m_DeepFreeze && Warning)
	{
		pChr->m_DeepFreeze = false;
		Warning = false;
	}
	pChr->m_FreezeTime = 2;
	pChr->GetPlayer()->m_RconFreeze = false;
	CServer* pServ = (CServer*)pSelf->Server();
	str_format(aBuf, sizeof(aBuf), "'%s' ClientId=%d has been defrosted.", pServ->ClientName(ClientId), Victim);
	pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
}

void CGameContext::ConInvis(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	char aBuf[128];
	int Victim = pResult->GetVictim();

	if(!pSelf->m_apPlayers[ClientId])
		return;

	if(pSelf->m_apPlayers[Victim])
	{
		pSelf->m_apPlayers[Victim]->m_Invisible = true;
		CServer* pServ = (CServer*)pSelf->Server();
		str_format(aBuf, sizeof(aBuf), "'%s' ClientId=%d is now invisible.", pServ->ClientName(ClientId), Victim);
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
	}
}

void CGameContext::ConVis(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();

	if(!pSelf->m_apPlayers[ClientId])
		return;
	char aBuf[128];
	if(pSelf->m_apPlayers[Victim])
	{
		pSelf->m_apPlayers[Victim]->m_Invisible = false;
		CServer* pServ = (CServer*)pSelf->Server();
		str_format(aBuf, sizeof(aBuf), "'%s' ClientId=%d is visible.", pServ->ClientName(ClientId), Victim);
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
	}
}

void CGameContext::ConCredits(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Teeworlds Team takes most of the credits also");
	pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "This mod was originally created by 3DA");
	pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Now it is maintained & re-coded by:");
	pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "[Egypt]GreYFoX@GTi and [BlackTee]den");
	pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Others Helping on the code: heinrich5991, noother & LemonFace");
	pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Documentation: Zeta-Hoernchen, Entities: Fisico");
	pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Code (in the past): 3da and Fluxid");
	pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Please check the changelog on DDRace.info.");
	pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Also the commit log on github.com/GreYFoXGTi/DDRace.");
}

void CGameContext::ConInfo(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "DDRace Mod. Version: " GAME_VERSION);
	pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Official site: DDRace.info");
	pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "For more Info /cmdlist");
	pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Or visit DDRace.info");
}

void CGameContext::ConHelp(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	if(pResult->NumArguments() == 0)
	{
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "/cmdlist will show a list of all chat commands");
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "/help + any command will show you the help for this command");
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Example /help settings will display the help about ");
	}
	else
	{
		const char *pArg = pResult->GetString(0);
		IConsole::CCommandInfo *pCmdInfo = pSelf->Console()->GetCommandInfo(pArg, CFGFLAG_SERVER);
		if(pCmdInfo && pCmdInfo->m_pHelp)
			pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", pCmdInfo->m_pHelp);
		else
				pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Command is either unknown or you have given a blank command without any parameters.");
	}
}

void CGameContext::ConSettings(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	if(pResult->NumArguments() == 0)
	{
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "to check a server setting say /settings and setting's name, setting names are:");
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "teams, cheats, collision, hooking, endlesshooking, me, ");
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "hitting, oldlaser, timeout, votes, pause and scores");
	}
	else
	{
		const char *pArg = pResult->GetString(0);
		char aBuf[256];
		float ColTemp;
		float HookTemp;
		pSelf->m_Tuning.Get("player_collision", &ColTemp);
		pSelf->m_Tuning.Get("player_hooking", &HookTemp);
		if(str_comp(pArg, "cheats") == 0)
		{
			str_format(aBuf, sizeof(aBuf), "%s%s",
					g_Config.m_SvCheats?"People can cheat":"People can't cheat",
					(g_Config.m_SvCheats)?(g_Config.m_SvCheatTime)?" with time":" without time":"");
			pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
			if(g_Config.m_SvCheats)
			{
				str_format(aBuf, sizeof(aBuf), "%s", g_Config.m_SvEndlessSuperHook?"super can hook you forever":"super can only hook you for limited time");
				pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
				str_format(aBuf, sizeof(aBuf), "%s", g_Config.m_SvTimer?"admins have the power to control your time":"admins have no power over your time");
				pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
			}
		}
		else if(str_comp(pArg, "teams") == 0)
		{
			str_format(aBuf, sizeof(aBuf), "%s %s", !g_Config.m_SvTeam?"Teams are available on this server":g_Config.m_SvTeam==-1?"Teams are not available on this server":"You have to be in a team to play on this server", !g_Config.m_SvTeamStrict?"and if you die in a team only you die":"and if you die in a team all of you die");
			pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
		}
		else if(str_comp(pArg, "collision") == 0)
		{
			pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", ColTemp?"Players can collide on this server":"Players Can't collide on this server");
		}
		else if(str_comp(pArg, "hooking") == 0)
		{
			pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", HookTemp?"Players can hook each other on this server":"Players Can't hook each other on this server");
		}
		else if(str_comp(pArg, "endlesshooking") == 0)
		{
			pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvEndlessDrag?"Players can hook time is unlimited":"Players can hook time is limited");
		}
		else if(str_comp(pArg, "hitting") == 0)
		{
			pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvHit?"Player's weapons affect each other":"Player's weapons has no affect on each other");
		}
		else if(str_comp(pArg, "oldlaser") == 0)
		{
			pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvOldLaser?"Lasers can hit you if you shot them and that they pull you towards the bounce origin (Like DDRace Beta)":"Lasers can't hit you if you shot them, and they pull others towards the shooter");
		}
		else if(str_comp(pArg, "me") == 0)
		{
			pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvSlashMe?"Players can use /me commands the famous IRC Command":"Players Can't use the /me command");
		}
		else if(str_comp(pArg, "timeout") == 0)
		{
			str_format(aBuf, sizeof(aBuf), "The Server Timeout is currently set to %d", g_Config.m_ConnTimeout);
			pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
		}
		else if(str_comp(pArg, "votes") == 0)
		{
			pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvVoteKick?"Players can use Callvote menu tab to kick offenders":"Players Can't use the Callvote menu tab to kick offenders");
			if(g_Config.m_SvVoteKick)
				str_format(aBuf, sizeof(aBuf), "Players are banned for %d second(s) if they get voted off", g_Config.m_SvVoteKickBantime);
			pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvVoteKickBantime?aBuf:"Players are just kicked and not banned if they get voted off");
		}
		else if(str_comp(pArg, "pause") == 0)
		{
			pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvPauseable?g_Config.m_SvPauseTime?"/pause is available on this server and it pauses your time too":"/pause is available on this server but it doesn't pause your time":"/pause is NOT available on this server");
		}
		else if(str_comp(pArg, "scores") == 0)
		{
			pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvHideScore?"Scores are private on this server":"Scores are public on this server");
		}
	}
}

void CGameContext::ConRules(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	bool printed=false;
	if(g_Config.m_SvDDRaceRules)
	{
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "No blocking.");
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "No insulting / spamming.");
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "No fun voting / vote spamming.");
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Breaking any of these rules will result in a penalty, decided by server admins.");
		printed=true;
	}
	if(g_Config.m_SvRulesLine1[0])
	{
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvRulesLine1);
		printed=true;
	}
	if(g_Config.m_SvRulesLine2[0])
	{
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvRulesLine2);
		printed=true;
	}
	if(g_Config.m_SvRulesLine3[0])
	{
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvRulesLine3);
		printed=true;
	}
	if(g_Config.m_SvRulesLine4[0])
	{
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvRulesLine4);
		printed=true;
	}
	if(g_Config.m_SvRulesLine5[0])
	{
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvRulesLine5);
		printed=true;
	}
	if(g_Config.m_SvRulesLine6[0])
	{
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvRulesLine6);
		printed=true;
	}
	if(g_Config.m_SvRulesLine7[0])
	{
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvRulesLine7);
		printed=true;
	}
	if(g_Config.m_SvRulesLine8[0])
	{
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvRulesLine8);
		printed=true;
	}
	if(g_Config.m_SvRulesLine9[0])
	{
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvRulesLine9);
		printed=true;
	}
	if(g_Config.m_SvRulesLine10[0])
	{
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvRulesLine10);
		printed=true;
	}
	if(!printed)
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "No Rules Defined, Kill em all!!");
}

void CGameContext::ConKill(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientId];

	if(pPlayer->m_Last_Kill && pPlayer->m_Last_Kill + pSelf->Server()->TickSpeed() * g_Config.m_SvKillDelay > pSelf->Server()->Tick())
		return;

	pPlayer->m_Last_Kill = pSelf->Server()->Tick();
	pPlayer->KillCharacter(WEAPON_SELF);
	pPlayer->m_RespawnTick = pSelf->Server()->Tick() + pSelf->Server()->TickSpeed() * g_Config.m_SvSuicidePenalty;
}

void CGameContext::ConTogglePause(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	CPlayer *pPlayer = pSelf->m_apPlayers[ClientId];

	if(g_Config.m_SvPauseable)
	{
		CCharacter* pChr = pPlayer->GetCharacter();
		if(!pPlayer->GetTeam() && pChr && (!pChr->GetWeaponGot(WEAPON_NINJA) || pChr->m_FreezeTime) && pChr->IsGrounded() && pChr->m_Pos==pChr->m_PrevPos && !pChr->Team() && !pPlayer->m_InfoSaved)
		{
			if(pPlayer->m_Last_Pause + pSelf->Server()->TickSpeed() * g_Config.m_SvPauseFrequency <= pSelf->Server()->Tick()) {
				pPlayer->SaveCharacter();
				pPlayer->SetTeam(TEAM_SPECTATORS);
				pPlayer->m_InfoSaved = true;
				pPlayer->m_Last_Pause = pSelf->Server()->Tick();
			}
			else
				pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "You can\'t pause that often.");
		}
		else if(pPlayer->GetTeam()==TEAM_SPECTATORS && pPlayer->m_InfoSaved)
		{
			pPlayer->m_InfoSaved = false;
			pPlayer->m_PauseInfo.m_Respawn = true;
			pPlayer->SetTeam(TEAM_RED);
			//pPlayer->LoadCharacter();//TODO:Check if this system Works
		}
		else if(pChr)
			pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", (pChr->Team())?"You can't pause while you are in a team":pChr->GetWeaponGot(WEAPON_NINJA)?"You can't use /pause while you are a ninja":(!pChr->IsGrounded())?"You can't use /pause while you are a in air":"You can't use /pause while you are moving");
		else
			pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "No pause data saved.");
	}
	else
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Pause isn't allowed on this server.");
}

void CGameContext::ConTop5(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	CPlayer *pPlayer = pSelf->m_apPlayers[ClientId];

	if(g_Config.m_SvHideScore)
	{
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Showing the top 5 is not allowed on this server.");
		return;
	}

	if(pResult->NumArguments() > 0)
		pSelf->Score()->ShowTop5(pPlayer->GetCID(), pResult->GetInteger(0));
	else
		pSelf->Score()->ShowTop5(pPlayer->GetCID());
}

void CGameContext::ConRank(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	CPlayer *pPlayer = pSelf->m_apPlayers[ClientId];

	if(/*g_Config.m_SvSpamprotection && */pPlayer->m_Last_Chat && pPlayer->m_Last_Chat + pSelf->Server()->TickSpeed() + g_Config.m_SvChatDelay > pSelf->Server()->Tick())
		return;
	
	pPlayer->m_Last_Chat = pSelf->Server()->Tick();

	if(pResult->NumArguments() > 0)
		if(!g_Config.m_SvHideScore)
			pSelf->Score()->ShowRank(pPlayer->GetCID(), pResult->GetString(0), true);
		else
			pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Showing the rank of other players is not allowed on this server.");
	else
		pSelf->Score()->ShowRank(pPlayer->GetCID(), pSelf->Server()->ClientName(ClientId));
}

void CGameContext::ConJoinTeam(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	
	CGameContext *pSelf = (CGameContext *)pUserData;
	CGameControllerDDRace* Controller = (CGameControllerDDRace*)pSelf->m_pController;
	if(g_Config.m_SvTeam == -1)
	{
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Admin disable teams");
		return;
	}
	else if (g_Config.m_SvTeam == 1)
	{
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "You must join to any team and play with anybody or you will not play");
	}
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientId];

	if(pResult->NumArguments() > 0)
	{
		int Team = pResult->GetInteger(0);
		if(pPlayer->GetCharacter() == 0)
		{
			pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "You can't change teams while you are dead/a spectator.");
		}
		else
		{
			if(pPlayer->m_Last_Team + pSelf->Server()->TickSpeed() * g_Config.m_SvTeamChangeDelay <= pSelf->Server()->Tick())
			{
				if(Controller->m_Teams.GetTeamLeader(Team) == -1)
					switch(Controller->m_Teams.SetCharacterTeam(pPlayer->GetCID(), Team))
					{
					case 0:
						char aBuf[512];
						pPlayer->m_Last_Team = pSelf->Server()->Tick();
						break;
					case CGameTeams::ERROR_ALREADY_THERE:
						str_format(aBuf, sizeof(aBuf), "You are already in team %d...!", Team);
						pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
						break;
					case CGameTeams::ERROR_CLOSED:
						str_format(aBuf, sizeof(aBuf), "Team %d is closed, they have to kill or finish for u to join.", Team);
						pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
						break;
					case CGameTeams::ERROR_NOT_SUPER:
						pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "You are trying to join team Super but you are not super.");
						break;
					case CGameTeams::ERROR_STARTED:
						pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "You already started, kill first.");
						break;
					case CGameTeams::ERROR_WRONG_PARAMS:
						pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "The wrong parameters were given.");
						break;
					default:
						pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "You cannot join this team at this time");
					}
				else
				{
					char aBuf[512];
					str_format(aBuf, sizeof(aBuf), "Team %d is led by \'%s\', please ask him to join his team.", Team, pSelf->Server()->ClientName(Controller->m_Teams.GetTeamLeader(Team)));
					pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
				}
			}
			else
			{
				pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "You can\'t change teams that fast!");
			}
		}
	}
	else
	{
		char aBuf[512];
		if(pPlayer->GetCharacter() == 0)
		{
			pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "You can't check your team while you are dead/a spectator.");
		}
		else
		{
			str_format(aBuf, sizeof(aBuf), "You are in team %d", pPlayer->GetCharacter()->Team());
			pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
		}
	}
}


void CGameContext::ConToggleFly(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	CCharacter* pChr = pSelf->m_apPlayers[ClientId]->GetCharacter();
	if(pChr && pChr->m_Super)
	{
		pChr->m_Fly = !pChr->m_Fly;
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", (pChr->m_Fly) ? "Fly enabled" : "Fly disabled");
	}
}

void CGameContext::ConMe(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	char aBuf[256 + 24];
	
	str_format(aBuf, 256 + 24, "'%s' %s", pSelf->Server()->ClientName(ClientId), pResult->GetString(0));
	if(g_Config.m_SvSlashMe)
		pSelf->SendChat(-2, CGameContext::CHAT_ALL, aBuf, ClientId);
	else
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "/me is disabled on this server, admin can enable it by using sv_slash_me");
}

void CGameContext::ConToggleEyeEmote(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	
	CCharacter *pChr = pSelf->m_apPlayers[ClientId]->GetCharacter();

	if(pChr)
	{
		pChr->m_EyeEmote = !pChr->m_EyeEmote;
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", (pChr->m_EyeEmote) ? "You can now use the preset eye emotes." : "You don't have any eye emotes, remember to bind some. (until you die)");
	}
}

void CGameContext::ConToggleBroadcast(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	
	CCharacter *pChr = pSelf->m_apPlayers[ClientId]->GetCharacter();

	if(pChr)
		pChr->m_BroadCast = !pChr->m_BroadCast;
}

void CGameContext::ConEyeEmote(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	
	CCharacter *pChr = pSelf->m_apPlayers[ClientId]->GetCharacter();
	
	if (pResult->NumArguments() == 0)
	{
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Emote commands are: /emote surprise /emote blink /emote close /emote angry /emote happy /emote pain");
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Example: /emote surprise 10 for 10 seconds or /emote surprise (default 1 second)");
	}
	else 
	{
	  if (pChr)
		{
			if (!str_comp(pResult->GetString(0), "angry"))
			  pChr->m_DefEmote = EMOTE_ANGRY;
			else if (!str_comp(pResult->GetString(0), "blink"))
			  pChr->m_DefEmote = EMOTE_BLINK;
			else if (!str_comp(pResult->GetString(0), "close"))
			  pChr->m_DefEmote = EMOTE_BLINK;
			else if (!str_comp(pResult->GetString(0), "happy"))
			  pChr->m_DefEmote = EMOTE_HAPPY;
			else if (!str_comp(pResult->GetString(0), "pain"))
			  pChr->m_DefEmote = EMOTE_PAIN;
			else if (!str_comp(pResult->GetString(0), "surprise"))
			  pChr->m_DefEmote = EMOTE_SURPRISE;
			else
			{
				pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Unkown emote... Say /emote");
			}
			
			int Duration = 1;
			if (pResult->NumArguments() > 1)
				Duration = pResult->GetInteger(1);
			  
			pChr->m_DefEmoteReset = pSelf->Server()->Tick() + Duration * pSelf->Server()->TickSpeed();
		}
	}
}

void CGameContext::ConShowOthers(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	if(pSelf->m_apPlayers[ClientId]->m_IsUsingDDRaceClient)
		pSelf->m_apPlayers[ClientId]->m_ShowOthers = !pSelf->m_apPlayers[ClientId]->m_ShowOthers;
	else
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Showing players from other teams is only available with DDRace Client, http://DDRace.info");
}

void CGameContext::ConAsk(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	char aBuf[512];
	CServer* pServ = (CServer*)pSelf->Server();
	const char *Name = pResult->GetString(0);
	int Matches = 0;
	int Victim = -1;
	CCharacter* pAsker = pSelf->m_apPlayers[ClientId]->GetCharacter();
	CCharacter* pVictim = 0;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(pSelf->m_apPlayers[i] && i != ClientId && !str_comp_nocase(pServ->ClientName(i), Name))
		{
			Victim = i;
			pVictim = pSelf->m_apPlayers[i]->GetCharacter();
			Matches = 1;
			pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Exact Match found");
			break;
		}
		else if(pSelf->m_apPlayers[i] && i != ClientId && str_find_nocase(pServ->ClientName(i), Name))
		{
			Victim = i;
			pVictim = pSelf->m_apPlayers[i]->GetCharacter();
			Matches++;
		}
	}
	if(!pAsker || !pAsker->IsAlive())
		str_format(aBuf, sizeof(aBuf), "You can\'t invite while dead.");
	else if(pAsker->Team())
		str_format(aBuf, sizeof(aBuf), "You already are in team %d, you can use /invite if you are the leader.", pAsker->Team());
	else if(Matches > 1)
		str_format(aBuf, sizeof(aBuf), "More Than one player matches the given string, maybe use auto complete (tab in any 0.5 trunk client)");
	else if(Matches == 0)
		str_format(aBuf, sizeof(aBuf), "No matches found.");
	else if(pAsker->m_DDRaceState != DDRACE_NONE)
		str_format(aBuf, sizeof(aBuf), "You can't start a team at this time, please kill.");
	else if(!pVictim || !pVictim->IsAlive())
		str_format(aBuf, sizeof(aBuf), "You can\'t invite him while he is dead.");
	else if(pVictim->m_DDRaceState != DDRACE_NONE)
	str_format(aBuf, sizeof(aBuf), "He can't change teams at this time, please tell him to kill.");
	else if(pSelf->m_apPlayers[Victim]->m_Asker != -1 && pSelf->m_apPlayers[Victim]->m_AskedTick > pSelf->Server()->Tick() - g_Config.m_SvTeamAskTime)
		str_format(aBuf, sizeof(aBuf), "\'%s\' is already being asked wait for %.1f seconds.", pServ->ClientName(Victim), (pSelf->m_apPlayers[Victim]->m_AskedTick + g_Config.m_SvTeamAskTime * pSelf->Server()->TickSpeed() - pSelf->Server()->Tick()) / (float)pSelf->Server()->TickSpeed());
	else if(pSelf->m_apPlayers[Victim]->m_Asked != -1 && pSelf->m_apPlayers[Victim]->m_AskerTick > pSelf->Server()->Tick() - g_Config.m_SvTeamAskTime)
		str_format(aBuf, sizeof(aBuf), "\'%s\' is already is asking someone wait for %.1f seconds.", pServ->ClientName(Victim), (pSelf->m_apPlayers[Victim]->m_AskedTick + g_Config.m_SvTeamAskTime * pSelf->Server()->TickSpeed() - pSelf->Server()->Tick()) / (float)pSelf->Server()->TickSpeed());
	else if(!pVictim->Team())
	{
		pSelf->m_apPlayers[Victim]->m_Asker = ClientId;
		pSelf->m_apPlayers[Victim]->m_AskedTick = pSelf->Server()->Tick();
		pSelf->m_apPlayers[ClientId]->m_Asked = Victim;
		pSelf->m_apPlayers[ClientId]->m_AskerTick = pSelf->Server()->Tick();
		char aTempBuf[512];
		str_format(aTempBuf, sizeof(aTempBuf), "Do you want to start a team with \'%s\' as leader ?", pServ->ClientName(ClientId));
		pSelf->SendChatTarget(Victim, aTempBuf);
		str_format(aTempBuf, sizeof(aTempBuf), "Please say /yes or /no within %d seconds.", g_Config.m_SvTeamAskTime);
		pSelf->SendChatTarget(Victim, aTempBuf);
		str_format(aBuf, sizeof(aBuf), "\'%s\' has been asked to start a team with you as leader.", pServ->ClientName(Victim));
	}
	else if(pVictim->Team() && pAsker->Teams()->GetTeamLeader(pVictim->Team()) != Victim)
	{
		str_format(aBuf, sizeof(aBuf), "You can't ask a team member you can only ask a team leader, Team %d is led by \'%s\'.", pVictim->Team(), pServ->ClientName(Victim));
	}
	else if(pVictim->Team() && pAsker->Teams()->GetTeamState(pVictim->Team()) == CGameTeams::TEAMSTATE_OPEN)
	{
		pSelf->m_apPlayers[Victim]->m_Asker = ClientId;
		pSelf->m_apPlayers[Victim]->m_AskedTick = pSelf->Server()->Tick();
		pSelf->m_apPlayers[ClientId]->m_Asked = Victim;
		pSelf->m_apPlayers[ClientId]->m_AskerTick = pSelf->Server()->Tick();
		char aTempBuf[512];
		str_format(aTempBuf, sizeof(aTempBuf), "%s wants to join your team ?", pServ->ClientName(ClientId));
		pSelf->SendChatTarget(Victim, aTempBuf);
		str_format(aTempBuf, sizeof(aTempBuf), "Please say /yes or /no within %d seconds.", g_Config.m_SvTeamAskTime);
		pSelf->SendChatTarget(Victim, aTempBuf);
		str_format(aBuf, sizeof(aBuf), "You asked to join %s\'s team %d.", pServ->ClientName(Victim), pVictim->Team());
	}
	else
		str_format(aBuf, sizeof(aBuf), "hmm, i don't know why but you are not allowed to ask this player");
	pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
	return;
}

void CGameContext::ConYes(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	CServer* pServ = (CServer*)pSelf->Server();
	CGameControllerDDRace* Controller = (CGameControllerDDRace*)pSelf->m_pController;
	char aBuf[512];
	if((pSelf->m_apPlayers[ClientId]->m_Asker == -1 || pSelf->m_apPlayers[ClientId]->m_Asker != -1) && pSelf->m_apPlayers[ClientId]->m_AskedTick + g_Config.m_SvTeamAskTime > pSelf->Server()->Tick() )
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "No valid questions, maybe they timed out.");
	else
	{
		str_format(aBuf, sizeof(aBuf), "\'%s\' has accepted your request.", pServ->ClientName(ClientId));
		pSelf->SendChatTarget(pSelf->m_apPlayers[ClientId]->m_Asker, aBuf);
		if(pSelf->m_apPlayers[pSelf->m_apPlayers[ClientId]->m_Asker]->GetCharacter()->Team() != 0)
		{
			Controller->m_Teams.SetCharacterTeam(ClientId, pSelf->m_apPlayers[pSelf->m_apPlayers[ClientId]->m_Asker]->GetCharacter()->Team());
		}
		else if(pSelf->m_apPlayers[ClientId]->GetCharacter()->Team() != 0)
		{
			Controller->m_Teams.SetCharacterTeam(pSelf->m_apPlayers[ClientId]->m_Asker, pSelf->m_apPlayers[ClientId]->GetCharacter()->Team());
			str_format(aBuf, sizeof(aBuf), "\'%s\' joined team %d.", pServ->ClientName(pSelf->m_apPlayers[ClientId]->m_Asker), pSelf->m_apPlayers[ClientId]->GetCharacter()->Team());
			pSelf->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
		}
		else
			for(int i = 1; i < MAX_CLIENTS; ++i)
			{
				if(Controller->m_Teams.GetTeamState(i) == CGameTeams::TEAMSTATE_EMPTY)
				{
					Controller->m_Teams.SetCharacterTeam(pSelf->m_apPlayers[ClientId]->m_Asker, i);
					Controller->m_Teams.SetCharacterTeam(ClientId, i);
					Controller->m_Teams.SetTeamLeader(i, pSelf->m_apPlayers[ClientId]->m_Asker);
					return;
				}
			}
	}
	/*
	if(pSelf->m_apPlayers[ClientId]->m_Asker != -1 && pSelf->m_apPlayers[pSelf->m_apPlayers[ClientId]->m_Asker]->m_Asked == ClientId)
	{
		pSelf->m_apPlayers[pSelf->m_apPlayers[ClientId]->m_Asker]->m_Asked = -1;
		pSelf->m_apPlayers[pSelf->m_apPlayers[ClientId]->m_Asker]->m_AskerTick = -g_Config.m_SvTeamAskTime;
	}
	pSelf->m_apPlayers[ClientId]->m_Asker = -1;
	pSelf->m_apPlayers[ClientId]->m_AskedTick = -g_Config.m_SvTeamAskTime;
	*/
}

void CGameContext::ConNo(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	CServer* pServ = (CServer*)pSelf->Server();
	char aBuf[512];
	if((pSelf->m_apPlayers[ClientId]->m_Asker == -1 || pSelf->m_apPlayers[ClientId]->m_Asker != -1) && pSelf->m_apPlayers[ClientId]->m_AskedTick + g_Config.m_SvTeamAskTime > pSelf->Server()->Tick() )
		pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "No valid question, maybe it timed out.");
	else
	{
		str_format(aBuf, sizeof(aBuf), "\'%s\' has rejected your request.", pServ->ClientName(ClientId));
		pSelf->SendChatTarget(pSelf->m_apPlayers[ClientId]->m_Asker, aBuf);
	}
	/*
	if(pSelf->m_apPlayers[ClientId]->m_Asker != -1 && pSelf->m_apPlayers[pSelf->m_apPlayers[ClientId]->m_Asker]->m_Asked == ClientId)
	{
		pSelf->m_apPlayers[pSelf->m_apPlayers[ClientId]->m_Asker]->m_Asked = -1;
		pSelf->m_apPlayers[pSelf->m_apPlayers[ClientId]->m_Asker]->m_AskerTick = -g_Config.m_SvTeamAskTime;
	}
	pSelf->m_apPlayers[ClientId]->m_Asker = -1;
	pSelf->m_apPlayers[ClientId]->m_AskedTick = -g_Config.m_SvTeamAskTime;
	*/
}

void CGameContext::ConInvite(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	CServer* pServ = (CServer*)pSelf->Server();
	CGameControllerDDRace* Controller = (CGameControllerDDRace*)pSelf->m_pController;
	char aBuf[512];
	const char *Name = pResult->GetString(0);
	int Matches = 0;
	int Victim = -1;
	CCharacter* pAsker = pSelf->m_apPlayers[ClientId]->GetCharacter();
	CCharacter* pVictim = 0;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(pSelf->m_apPlayers[i] && i != ClientId && !str_comp_nocase(pServ->ClientName(i), Name))
		{
			Victim = i;
			pVictim = pSelf->m_apPlayers[i]->GetCharacter();
			Matches = 1;
			pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Exact Match found");
			break;
		}
		else if(pSelf->m_apPlayers[i] && i != ClientId && str_find_nocase(pServ->ClientName(i), Name))
		{
			Victim = i;
			pVictim = pSelf->m_apPlayers[i]->GetCharacter();
			Matches++;
		}
	}
	if(!pAsker || !pAsker->IsAlive())
		str_format(aBuf, sizeof(aBuf), "You can\'t invite while dead.");
	else if(pAsker->Team() == 0)
		str_format(aBuf, sizeof(aBuf), "You are in team %d, use /leader or /ask.", pAsker->Team());
	else if(pAsker->Team() && ClientId != Controller->m_Teams.GetTeamLeader(pAsker->Team()))
		str_format(aBuf, sizeof(aBuf), "You already are in team %d, but not leader.", pAsker->Team());
	else if(Matches > 1)
		str_format(aBuf, sizeof(aBuf), "More Than one player matches the given string, maybe use auto complete (tab in any 0.5 trunk client)");
	else if(Matches == 0)
		str_format(aBuf, sizeof(aBuf), "No matches found.");
	else if(Controller->m_Teams.GetTeamState(pAsker->Team()) != CGameTeams::TEAMSTATE_OPEN)
		str_format(aBuf, sizeof(aBuf), "You can't invite anyone at this time, your team is closed.");
	else if(!pVictim || !pVictim->IsAlive())
		str_format(aBuf, sizeof(aBuf), "You can\'t invite him while he is dead.");
	else if(pVictim->m_DDRaceState != DDRACE_NONE)
	str_format(aBuf, sizeof(aBuf), "He can't change teams at this time, please tell him to kill.");
	else if(pSelf->m_apPlayers[Victim]->m_Asker != -1 && pSelf->m_apPlayers[Victim]->m_AskedTick > pSelf->Server()->Tick() - g_Config.m_SvTeamAskTime)
		str_format(aBuf, sizeof(aBuf), "\'%s\' is already being asked wait for %.1f seconds.", pServ->ClientName(Victim), (pSelf->m_apPlayers[Victim]->m_AskedTick + g_Config.m_SvTeamAskTime * pSelf->Server()->TickSpeed() - pSelf->Server()->Tick()) / (float)pSelf->Server()->TickSpeed());
	else if(pSelf->m_apPlayers[Victim]->m_Asked != -1 && pSelf->m_apPlayers[Victim]->m_AskerTick > pSelf->Server()->Tick() - g_Config.m_SvTeamAskTime)
		str_format(aBuf, sizeof(aBuf), "\'%s\' is already is asking someone wait for %.1f seconds.", pServ->ClientName(Victim), (pSelf->m_apPlayers[Victim]->m_AskedTick + g_Config.m_SvTeamAskTime * pSelf->Server()->TickSpeed() - pSelf->Server()->Tick()) / (float)pSelf->Server()->TickSpeed());
	else if(!pVictim->Team())
	{
		pSelf->m_apPlayers[Victim]->m_Asker = ClientId;
		pSelf->m_apPlayers[Victim]->m_AskedTick = pSelf->Server()->Tick();
		pSelf->m_apPlayers[ClientId]->m_Asked = Victim;
		pSelf->m_apPlayers[ClientId]->m_AskerTick = pSelf->Server()->Tick();
		char aTempBuf[512];
		str_format(aTempBuf, sizeof(aTempBuf), "Do you want to join \'%s\' \'s team?", pServ->ClientName(ClientId));
		pSelf->SendChatTarget(Victim, aTempBuf);
		str_format(aTempBuf, sizeof(aTempBuf), "Please say /yes or /no within %d seconds.", g_Config.m_SvTeamAskTime);
		pSelf->SendChatTarget(Victim, aTempBuf);
		str_format(aBuf, sizeof(aBuf), "\'%s\' has been asked to join your team.", pServ->ClientName(Victim));
	}
	else
		str_format(aBuf, sizeof(aBuf), "hmm, i don't know why but you are not allowed to ask this player");
	pSelf->Console()->PrintResponse(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
	return;
}
