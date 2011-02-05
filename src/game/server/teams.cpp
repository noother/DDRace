#include "teams.h"
#include <engine/shared/config.h>
#include <engine/server/server.h>

CGameTeams::CGameTeams(CGameContext *pGameContext) : m_pGameContext(pGameContext)
{
	Reset();
}

void CGameTeams::Reset()
{
	m_Core.Reset();
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		m_TeamState[i] = TEAMSTATE_EMPTY;
		m_TeeFinished[i] = false;
		m_MembersCount[i] = 0;
		m_LastChat[i] = 0;
		m_TeamLeader[i] = -1;
		m_TeeJoinTick[i] = -1;
	}
}

void CGameTeams::OnCharacterStart(int ClientID)
{
	int Tick = Server()->Tick();
	int Team = m_Core.Team(ClientID);
	CCharacter* StartingChar = Character(ClientID);
	if(!StartingChar)
		return;
	if(StartingChar->m_DDRaceState == DDRACE_FINISHED)
		StartingChar->m_DDRaceState = DDRACE_NONE;
	if(Team == TEAM_FLOCK || Team == TEAM_SUPER)
	{
		StartingChar->m_DDRaceState = DDRACE_STARTED;
		StartingChar->m_StartTime = Tick;
		StartingChar->m_RefreshTime = Tick;
	}
	else
	{
		bool Waiting = false;
		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
			if(Team == m_Core.Team(i))
			{
				CCharacter* Char = Character(i);
				if(Char->m_DDRaceState == DDRACE_FINISHED)
				{
					Waiting = true;
					if(m_LastChat[ClientID] + Server()->TickSpeed() + g_Config.m_SvChatDelay < Tick)
					{
						char aBuf[128];
						str_format(aBuf, sizeof(aBuf), "%s has finished and didn't go through start yet, wait for him or join another team.", Server()->ClientName(i));
						GameServer()->SendChatTarget(ClientID, aBuf);
						m_LastChat[ClientID] = Tick;
					}
					if(m_LastChat[i] + Server()->TickSpeed() + g_Config.m_SvChatDelay < Tick)
					{
						char aBuf[128];
						str_format(aBuf, sizeof(aBuf), "%s wants to start a new round, kill or walk to start.", Server()->ClientName(ClientID));
						GameServer()->SendChatTarget(i, aBuf);
						m_LastChat[i] = Tick;
					}
				}
			}
		}

		if(m_TeamState[Team] <= TEAMSTATE_CLOSED && !Waiting)
		{
			ChangeTeamState(Team, TEAMSTATE_STARTED);
			for(int i = 0; i < MAX_CLIENTS; ++i)
			{
				if(Team == m_Core.Team(i))
				{
					CCharacter* Char = Character(i);
					if(Char)
					{
						Char->m_DDRaceState = DDRACE_STARTED;
						Char->m_StartTime = Tick;
						Char->m_RefreshTime = Tick;
					}
				}
			}
		}
	}
}

void CGameTeams::OnCharacterFinish(int ClientID)
{
	int Team = m_Core.Team(ClientID);
	if(Team == TEAM_FLOCK || Team == TEAM_SUPER)
	{
		Character(ClientID)->OnFinish();
	}
	else
	{
		m_TeeFinished[ClientID] = true;
		if(TeamFinished(Team))
		{
			ChangeTeamState(Team, TEAMSTATE_OPEN);
			for(int i = 0; i < MAX_CLIENTS; ++i)
			{
				if(Team == m_Core.Team(i))
				{
					CCharacter * Char = Character(i);
					if(Char != 0)
					{
						Char->OnFinish();
						m_TeeFinished[i] = false;
					}
				}
			}
			
		}
	}
}

int CGameTeams::SetCharacterTeam(int ClientID, int Team)
{
	//Check on wrong parameters. +1 for TEAM_SUPER
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || Team < 0 || Team >= MAX_CLIENTS + 1)
		return ERROR_WRONG_PARAMS;
	//You can join to TEAM_SUPER at any time, but any other group you cannot if it started
	if(Team != TEAM_SUPER && m_TeamState[Team] >= TEAMSTATE_CLOSED)
		return ERROR_CLOSED;
	//No need to switch team if you there
	if(m_Core.Team(ClientID) == Team)
		return ERROR_ALREADY_THERE;
	//You cannot be in TEAM_SUPER if you not super
	if(Team == TEAM_SUPER && !Character(ClientID)->m_Super)
		return ERROR_NOT_SUPER;
	//if you begin race
	if(Character(ClientID)->m_DDRaceState != DDRACE_NONE && Team != TEAM_SUPER)
		return ERROR_STARTED;
	SetForceCharacterTeam(ClientID, Team);
	
	return 0;
}

void CGameTeams::SetForceCharacterTeam(int ClientID, int Team)
{
	char aBuf[64];
	CServer* pServ = (CServer*)Server();
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
	m_TeeFinished[ClientID] = false;
	int OldTeam = m_Core.Team(ClientID);
	if(OldTeam != TEAM_FLOCK && OldTeam != TEAM_SUPER && m_TeamState[OldTeam] != TEAMSTATE_EMPTY)
	{
		bool NoOneInOldTeam = true;
		for(int i = 0; i < MAX_CLIENTS; ++i)
			if(i != ClientID && OldTeam == m_Core.Team(i))
			{
				NoOneInOldTeam = false;//all good exists someone in old team
				break;
			} 
		if(NoOneInOldTeam)
			m_TeamState[OldTeam] = TEAMSTATE_EMPTY;
	}

	if(Count(OldTeam) > 0)
		m_MembersCount[OldTeam]--;

	m_Core.Team(ClientID, Team);
	
	if(Team)
	{
		str_format(aBuf, sizeof(aBuf), "\'%s\' joined team %d.", pServ->ClientName(pPlayer->GetCID()), Team);
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
	}
	if(Count(OldTeam) > 0)
		m_TeeJoinTick[ClientID] = Server()->Tick();

	if(Team != TEAM_SUPER)
		m_MembersCount[Team]++;

	if(Team != TEAM_SUPER && m_TeamState[Team] == TEAMSTATE_EMPTY)
		ChangeTeamState(Team, TEAMSTATE_OPEN);

	if(OldTeam != TEAM_FLOCK && OldTeam != TEAM_SUPER && ClientID == GetTeamLeader(OldTeam))
	{
		if(Count(OldTeam))
		{
			int FirstJoinedID = -1;
			int Tick = Server()->Tick();
			for (int LoopCID = 0; LoopCID < MAX_CLIENTS; ++LoopCID)
				if(m_Core.Team(LoopCID) == OldTeam)
					if(m_TeeJoinTick[LoopCID] < Tick)
						FirstJoinedID = LoopCID;
			SetTeamLeader(OldTeam, FirstJoinedID);
		}
		else
			SetTeamLeader(OldTeam, -1);
	}

	dbg_msg1("Teams", "Id = %d Team = %d", ClientID, Team);
	
	for (int LoopCID = 0; LoopCID < MAX_CLIENTS; ++LoopCID)
	{
		if(Character(LoopCID) && Character(LoopCID)->GetPlayer()->m_IsUsingDDRaceClient)
			SendTeamsState(LoopCID);
	}
}

int CGameTeams::Count(int Team) const
{
	if(Team == TEAM_SUPER)
		return -1;
	return m_MembersCount[Team];
}

bool CGameTeams::TeamFinished(int Team)
{
	for(int i = 0; i < MAX_CLIENTS; ++i)
		if(m_Core.Team(i) == Team && !m_TeeFinished[i])
			return false;
	return true;
}

int CGameTeams::TeamMask(int Team, int ExceptID)
{
	if(Team == TEAM_SUPER) return -1;
	int Mask = 0;
	for(int i = 0; i < MAX_CLIENTS; ++i)
		if(i != ExceptID)
			if((Character(i) && (m_Core.Team(i) == Team || m_Core.Team(i) == TEAM_SUPER))
				|| (GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetTeam() == -1))
				Mask |= 1 << i;
	return Mask;
}

void CGameTeams::SendTeamsState(int ClientID)
{
	CNetMsg_Cl_TeamsState Msg;
	Msg.m_Tee0 = m_Core.Team(0);
	Msg.m_Tee1 = m_Core.Team(1);
	Msg.m_Tee2 = m_Core.Team(2);
	Msg.m_Tee3 = m_Core.Team(3);
	Msg.m_Tee4 = m_Core.Team(4);
	Msg.m_Tee5 = m_Core.Team(5);
	Msg.m_Tee6 = m_Core.Team(6);
	Msg.m_Tee7 = m_Core.Team(7);
	Msg.m_Tee8 = m_Core.Team(8);
	Msg.m_Tee9 = m_Core.Team(9);
	Msg.m_Tee10 = m_Core.Team(10);
	Msg.m_Tee11 = m_Core.Team(11);
	Msg.m_Tee12 = m_Core.Team(12);
	Msg.m_Tee13 = m_Core.Team(13);
	Msg.m_Tee14 = m_Core.Team(14);
	Msg.m_Tee15 = m_Core.Team(15);
	
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
	
}

void CGameTeams::SetTeamLeader(int Team, int ClientID)
{
	if(Team == TEAM_FLOCK || Team == TEAM_SUPER)
		return;
	char aBuf[64];
	m_TeamLeader[Team] = ClientID;
	str_format(aBuf, sizeof(aBuf), "\'%s\' is now the team %d leader.", Server()->ClientName(ClientID), Team);
	if(Count(Team) > 1)
		for (int LoopCID = 0; LoopCID < MAX_CLIENTS; ++LoopCID)
			if(LoopCID != ClientID)
				if(m_Core.Team(LoopCID) == Team)
					GameServer()->SendChatTarget(LoopCID, aBuf);

	str_format(aBuf, sizeof(aBuf), "You are now the team %d leader.", Team);
	GameServer()->SendChatTarget(ClientID, aBuf);
}
