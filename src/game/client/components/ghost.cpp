/* (c) Rajh, Redix and Sushi. */

#include <cstdio>

#include <engine/storage.h>
#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <engine/shared/compression.h>
#include <engine/shared/network.h>

#include <game/generated/client_data.h>
#include <game/client/animstate.h>

#include "skins.h"
#include "menus.h"
#include "ghost.h"

/*
Note:
Freezing fucks up the ghost
the ghost isnt really sync
don't really get the client tick system for prediction
can used PrevChar and PlayerChar and it would be fluent and accurate but won't be predicted
so it will be affected by lags
*/

static const unsigned char gs_aHeaderMarker[8] = {'T', 'W', 'G', 'H', 'O', 'S', 'T', 0};
static const unsigned char gs_ActVersion = 1;

CGhost::CGhost()
{
	m_lGhosts.clear();
	m_CurPath.clear();
	m_CurPos = 0;
	m_Recording = false;
	m_Rendering = false;
	m_RaceState = RACE_NONE;
	m_NewRecord = false;
	m_BestTime = -1;
	m_StartRenderTick = -1;
	m_StartRecordTick = -1;
}

void CGhost::AddInfos(CNetObj_Character Player)
{
	if(!m_Recording)
		return;
	
	
	// Just to be sure it doesnt eat too much memory, the first test should be enough anyway
	if((Client()->GameTick()-m_StartRecordTick) > Client()->GameTickSpeed()*60*13 || m_CurPath.size() > 50*15*60)
	{
		dbg_msg("ghost", "13 minutes elapsed. Stopping ghost record");
		StopRecord();
		m_CurPath.clear();
		return;
	}
	
	
	m_CurPath.add(Player);
}

void CGhost::OnRender()
{
	// only for race
	if(!m_pClient->m_IsRace || !g_Config.m_ClRaceGhost)
		return;
	
	// Check if the race line is crossed then start the render of the ghost if one
	if(m_RaceState != RACE_STARTED) {
		bool start = false;
		
		std::list < int > Indices = m_pClient->Collision()->GetMapIndices(m_pClient->m_PredictedPrevChar.m_Pos, m_pClient->m_LocalCharacterPos);
		if(!Indices.empty()) {
			for(std::list < int >::iterator i = Indices.begin(); i != Indices.end(); i++) {
				if(m_pClient->Collision()->GetTileIndex(*i) == TILE_BEGIN) start = true;
			}
		} else {
			int CurrentIndex = m_pClient->Collision()->GetPureMapIndex(m_pClient->m_LocalCharacterPos);
			if(m_pClient->Collision()->GetTileIndex(CurrentIndex) == TILE_BEGIN) start = true;
		}
		
		if(start) {
			dbg_msg("ghost", "race started");
			m_RaceState = RACE_STARTED;
			StartRender();
			StartRecord();
		}
		
	}

	if(m_RaceState == RACE_FINISHED)
	{
		int OwnIndex = -1;
		for(int i = 0; i < m_lGhosts.size(); i++)
			if(m_lGhosts[i].m_ID == -1)
			{
				OwnIndex = i;
				break;
			}
					
		if(m_NewRecord)
		{
			dbg_msg("ghost", "new path saved"); 
			m_NewRecord = false;
			CGhostList Ghost;
			Ghost.m_ID = -1;
			Ghost.m_GhostInfo = m_CurInfo;
			Ghost.m_BestPath.clear();
			Ghost.m_BestPath = m_CurPath;
			if(OwnIndex < 0)
				m_lGhosts.add(Ghost);
			else
				m_lGhosts[OwnIndex] = Ghost;

			Save();
		}
		StopRecord();
		StopRender();
		m_RaceState = RACE_NONE;
	}
	
	CNetObj_Character Player = m_pClient->m_Snap.m_aCharacters[m_pClient->m_Snap.m_LocalCid].m_Cur;
	m_pClient->m_PredictedChar.Write(&Player);
	
	if(m_pClient->m_NewPredictedTick)
		AddInfos(Player);

	// Play the ghost
	if(!m_Rendering || !g_Config.m_ClRaceShowGhost)
		return;
	
	m_CurPos = Client()->PredGameTick()-m_StartRenderTick;

	if(m_lGhosts.size() == 0 || m_CurPos < 0)
	{
		//dbg_msg("ghost", "Ghost path done");
		m_Rendering = false;
		return;
	}
	
	for(int i = 0; i < m_lGhosts.size(); i++)
	{
		RenderGhostHook(&m_lGhosts[i]);
		RenderGhost(&m_lGhosts[i]);
	}
	
}

void CGhost::RenderGhost(CGhostList *pGhost)
{
	if(m_CurPos >= pGhost->m_BestPath.size())
		return;
	
	CNetObj_Character Player = pGhost->m_BestPath[m_CurPos];
	CNetObj_Character Prev = pGhost->m_BestPath[m_CurPos];
	
	if(m_CurPos > 0)
		Prev = pGhost->m_BestPath[m_CurPos-1];
	
	char aSkinName[64];
	IntsToStr(&pGhost->m_GhostInfo.m_Skin0, 6, aSkinName);
	int SkinId = m_pClient->m_pSkins->Find(aSkinName);
	if(SkinId < 0)
	{
		SkinId = m_pClient->m_pSkins->Find("default");
		if(SkinId < 0)
			SkinId = 0;
	}
	
	CTeeRenderInfo RenderInfo;
	RenderInfo.m_ColorBody = m_pClient->m_pSkins->GetColorV4(pGhost->m_GhostInfo.m_ColorBody);
	RenderInfo.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(pGhost->m_GhostInfo.m_ColorFeet);
	
	if(pGhost->m_GhostInfo.m_UseCustomColor)
		RenderInfo.m_Texture = m_pClient->m_pSkins->Get(SkinId)->m_ColorTexture;
	else
	{
		RenderInfo.m_Texture = m_pClient->m_pSkins->Get(SkinId)->m_OrgTexture;
		RenderInfo.m_ColorBody = vec4(1,1,1,1);
		RenderInfo.m_ColorFeet = vec4(1,1,1,1);
	}
	
	RenderInfo.m_ColorBody.a = 0.5f;
	RenderInfo.m_ColorFeet.a = 0.5f;
	RenderInfo.m_Size = 64;
	
	float Angle = mix((float)Prev.m_Angle, (float)Player.m_Angle, Client()->IntraGameTick())/256.0f;
	vec2 Direction = GetDirection((int)(Angle*256.0f));
	vec2 Position = mix(vec2(Prev.m_X, Prev.m_Y), vec2(Player.m_X, Player.m_Y), Client()->PredIntraGameTick());
	vec2 Vel = mix(vec2(Prev.m_VelX/256.0f, Prev.m_VelY/256.0f), vec2(Player.m_VelX/256.0f, Player.m_VelY/256.0f), Client()->PredIntraGameTick());
	
	bool Stationary = Player.m_VelX <= 1 && Player.m_VelX >= -1;
	bool InAir = !Collision()->CheckPoint(Player.m_X, Player.m_Y+16);
	bool WantOtherDir = (Player.m_Direction == -1 && Vel.x > 0) || (Player.m_Direction == 1 && Vel.x < 0);

	float WalkTime = fmod(absolute(Position.x), 100.0f)/100.0f;
	CAnimState State;
	State.Set(&g_pData->m_aAnimations[ANIM_BASE], 0);

	if(InAir)
		State.Add(&g_pData->m_aAnimations[ANIM_INAIR], 0, 1.0f);
	else if(Stationary)
		State.Add(&g_pData->m_aAnimations[ANIM_IDLE], 0, 1.0f);
	else if(!WantOtherDir)
		State.Add(&g_pData->m_aAnimations[ANIM_WALK], WalkTime, 1.0f);
	
	if (Player.m_Weapon == WEAPON_GRENADE)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
		Graphics()->QuadsBegin();
		Graphics()->QuadsSetRotation(State.GetAttach()->m_Angle*pi*2+Angle);
		Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.5f);

		// normal weapons
		int iw = clamp(Player.m_Weapon, 0, NUM_WEAPONS-1);
		RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[iw].m_pSpriteBody, Direction.x < 0 ? SPRITE_FLAG_FLIP_Y : 0);
		
		vec2 Dir = Direction;
		float Recoil = 0.0f;
		// TODO: is this correct?
		float a = (Client()->PredGameTick()-Player.m_AttackTick+Client()->PredIntraGameTick())/5.0f;
		if(a < 1)
			Recoil = sinf(a*pi);
		
		vec2 p = Position + Dir * g_pData->m_Weapons.m_aId[iw].m_Offsetx - Direction*Recoil*10.0f;
		p.y += g_pData->m_Weapons.m_aId[iw].m_Offsety;
		RenderTools()->DrawSprite(p.x, p.y, g_pData->m_Weapons.m_aId[iw].m_VisualSize);
		Graphics()->QuadsEnd();
	}

	// Render ghost
	RenderTools()->RenderTee(&State, &RenderInfo, 0, Direction, Position, true);
}

void CGhost::RenderGhostHook(CGhostList *pGhost)
{
	if(m_CurPos >= pGhost->m_BestPath.size())
		return;
		
	CNetObj_Character Player = pGhost->m_BestPath[m_CurPos];
	CNetObj_Character Prev = pGhost->m_BestPath[m_CurPos];
	
	if(m_CurPos > 0)
		Prev = pGhost->m_BestPath[m_CurPos-1];
		
	if (Prev.m_HookState<=0 || Player.m_HookState<=0)
		return;

	float Angle = mix((float)Prev.m_Angle, (float)Player.m_Angle, Client()->IntraGameTick())/256.0f;
	vec2 Direction = GetDirection((int)(Angle*256.0f));
	vec2 Pos = mix(vec2(Prev.m_X, Prev.m_Y), vec2(Player.m_X, Player.m_Y), Client()->PredIntraGameTick());

	vec2 HookPos = mix(vec2(Prev.m_HookX, Prev.m_HookY), vec2(Player.m_HookX, Player.m_HookY), Client()->PredIntraGameTick());
	float d = distance(Pos, HookPos);
	vec2 Dir = normalize(Pos-HookPos);

	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->QuadsSetRotation(GetAngle(Dir)+pi);
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.5f);

	// render head
	RenderTools()->SelectSprite(SPRITE_HOOK_HEAD);
	IGraphics::CQuadItem QuadItem(HookPos.x, HookPos.y, 24, 16);
	Graphics()->QuadsDraw(&QuadItem, 1);

	// render chain
	RenderTools()->SelectSprite(SPRITE_HOOK_CHAIN);
	IGraphics::CQuadItem Array[1024];
	int j = 0;
	for(float f = 24; f < d && j < 1024; f += 24, j++)
	{
		vec2 p = HookPos + Dir*f;
		Array[j] = IGraphics::CQuadItem(p.x, p.y, 24, 16);
	}

	Graphics()->QuadsDraw(Array, j);
	Graphics()->QuadsSetRotation(0);
	Graphics()->QuadsEnd();
}

void CGhost::StartRecord()
{
	m_Recording = true;
	m_CurPath.clear();
	CNetObj_ClientInfo *pInfo = (CNetObj_ClientInfo *) Client()->SnapFindItem(IClient::SNAP_CURRENT, NETOBJTYPE_CLIENTINFO, m_pClient->m_Snap.m_LocalCid);
	m_CurInfo = *pInfo;
	m_StartRecordTick = Client()->GameTick();
}

void CGhost::StopRecord()
{
	m_Recording = false;
}

void CGhost::StartRender()
{
	m_CurPos = 0;
	m_Rendering = true;
	m_StartRenderTick = Client()->PredGameTick();
}

void CGhost::StopRender()
{
	m_Rendering = false;
}

void CGhost::Save()
{
	if(!g_Config.m_ClRaceSaveGhost)
		return;
	
	CGhostHeader Header;
	
	// check the player name
	char aName[MAX_NAME_LENGTH];
	str_copy(aName, g_Config.m_PlayerName, sizeof(aName));
	for(int i = 0; i < MAX_NAME_LENGTH; i++)
	{
		if(!aName[i])
			break;
		
		if(aName[i] == '\\' || aName[i] == '/' || aName[i] == '|' || aName[i] == ':' || aName[i] == '*' || aName[i] == '?' || aName[i] == '<' || aName[i] == '>' || aName[i] == '"')
			aName[i] = '%';
	}
	
	char aFilename[256];
	char aBuf[256];
	str_format(aFilename, sizeof(aFilename), "%s_%s_%.3f_%08x.gho", Client()->GetCurrentMap(), aName, m_BestTime, Client()->GetCurrentMapCrc());
	str_format(aBuf, sizeof(aBuf), "ghosts/%s", aFilename);
	IOHANDLE File = Storage()->OpenFile(aBuf, IOFLAG_WRITE, IStorage::TYPE_SAVE);
	if(!File)
		return;
		
	// write header
	int Crc = Client()->GetCurrentMapCrc();
	mem_zero(&Header, sizeof(Header));
	mem_copy(Header.m_aMarker, gs_aHeaderMarker, sizeof(Header.m_aMarker));
	Header.m_Version = gs_ActVersion;
	IntsToStr(&m_CurInfo.m_Name0, 6, Header.m_aOwner);
	str_copy(Header.m_aMap, Client()->GetCurrentMap(), sizeof(Header.m_aMap));
	Header.m_aCrc[0] = (Crc>>24)&0xff;
	Header.m_aCrc[1] = (Crc>>16)&0xff;
	Header.m_aCrc[2] = (Crc>>8)&0xff;
	Header.m_aCrc[3] = (Crc)&0xff;
	Header.m_Time = m_BestTime;
	io_write(File, &Header, sizeof(Header));
	
	// write client info
	io_write(File, &m_CurInfo, sizeof(m_CurInfo));
	
	// write data
	int ItemsPerPackage = 500; // 500 ticks per package
	int Num = m_CurPath.size();
	CNetObj_Character *Data = &m_CurPath[0];
	
	while(Num)
	{
		int Items = min(Num, ItemsPerPackage);
		Num -= Items;
		
		char aBuffer[100*500];
		char aBuffer2[100*500];
		unsigned char aSize[4];
		
		int Size = sizeof(CNetObj_Character)*Items;
		mem_copy(aBuffer2, Data, Size);
		Data += Items;
		
		Size = CVariableInt::Compress(aBuffer2, Size, aBuffer);
		Size = CNetBase::Compress(aBuffer, Size, aBuffer2, sizeof(aBuffer2));
		
		aSize[0] = (Size>>24)&0xff;
		aSize[1] = (Size>>16)&0xff;
		aSize[2] = (Size>>8)&0xff;
		aSize[3] = (Size)&0xff;
		
		io_write(File, aSize, sizeof(aSize));
		io_write(File, aBuffer2, Size);
	}
	
	io_close(File);
	
	// remove old ghost from list
	for(int i = 0; i < m_pClient->m_pMenus->m_lGhosts.size(); i++)
	{
		CMenus::CGhostItem TmpItem = m_pClient->m_pMenus->m_lGhosts[i];
		if(TmpItem.m_ID == -1)
		{
			char aFile[256];
			str_format(aFile, sizeof(aFile), "ghosts/%s", TmpItem.m_aFilename);
			Storage()->RemoveFile(aFile, IStorage::TYPE_SAVE);
			m_pClient->m_pMenus->m_lGhosts.remove_index(i);
			break; // TODO: remove other ghosts?
		}
	}
	
	// add new ghost to ghost list
	CMenus::CGhostItem Item;
	str_copy(Item.m_aFilename, aFilename, sizeof(Item.m_aFilename));
	str_copy(Item.m_aPlayer, Header.m_aOwner, sizeof(Item.m_aPlayer));
	Item.m_Time = m_BestTime;
	Item.m_Active = true;
	Item.m_ID = -1;
	m_pClient->m_pMenus->m_lGhosts.add(Item);
}

bool CGhost::GetHeader(IOHANDLE *pFile, CGhostHeader *pHeader)
{
	if(!*pFile)
		return 0;
	
	CGhostHeader Header;
	io_read(*pFile, &Header, sizeof(Header));
	
	*pHeader = Header;
	
	if(mem_comp(Header.m_aMarker, gs_aHeaderMarker, sizeof(gs_aHeaderMarker)) != 0)
		return 0;
	
	if(Header.m_Version > gs_ActVersion)
		return 0;
	
	int Crc = (Header.m_aCrc[0]<<24) | (Header.m_aCrc[1]<<16) | (Header.m_aCrc[2]<<8) | (Header.m_aCrc[3]);
	if(str_comp(Header.m_aMap, Client()->GetCurrentMap()) != 0 || Crc != Client()->GetCurrentMapCrc())
		return 0;
	
	return 1;
}

bool CGhost::GetInfo(const char* pFilename, CGhostHeader *pHeader)
{
	char aFilename[256];
	str_format(aFilename, sizeof(aFilename), "ghosts/%s", pFilename);
	IOHANDLE File = Storage()->OpenFile(aFilename, IOFLAG_READ, IStorage::TYPE_SAVE);
	if(!File)
		return 0;
	
	bool Check = GetHeader(&File, pHeader);
	io_close(File);
	
	return Check;
}

void CGhost::Load(const char* pFilename, int ID)
{
	char aFilename[256];
	str_format(aFilename, sizeof(aFilename), "ghosts/%s", pFilename);
	IOHANDLE File = Storage()->OpenFile(aFilename, IOFLAG_READ, IStorage::TYPE_SAVE);
	if(!File)
		return;

	// read header
	CGhostHeader Header;
	if(!GetHeader(&File, &Header))
	{
		io_close(File);
		return;
	}
	
	if(ID == -1)
		m_BestTime = Header.m_Time;
	
	// create ghost
	CGhostList Ghost;
	Ghost.m_ID = ID;
	
	// read client info
	io_read(File, &Ghost.m_GhostInfo, sizeof(Ghost.m_GhostInfo));
	
	// read data
	Ghost.m_BestPath.clear();
	
	while(1)
	{
		static char aCompresseddata[100*500];
		static char aDecompressed[100*500];
		static char aData[100*500];
		
		unsigned char aSize[4];
		if(io_read(File, aSize, sizeof(aSize)) != sizeof(aSize))
			break;
		int Size = (aSize[0]<<24) | (aSize[1]<<16) | (aSize[2]<<8) | aSize[3];
		
		if(io_read(File, aCompresseddata, Size) != (unsigned)Size)
		{
			Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "ghost", "error reading chunk");
			break;
		}
		
		Size = CNetBase::Decompress(aCompresseddata, Size, aDecompressed, sizeof(aDecompressed));
		if(Size < 0)
		{
			Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "ghost", "error during network decompression");
			break;
		}
		
		Size = CVariableInt::Decompress(aDecompressed, Size, aData);
		if(Size < 0)
		{
			Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "ghost", "error during intpack decompression");
			break;
		}
		
		CNetObj_Character *Tmp = (CNetObj_Character*)aData;
		for(int i = 0; i < (signed)(Size/sizeof(CNetObj_Character)); i++)
		{
			Ghost.m_BestPath.add(*Tmp);
			Tmp++;
		}
	}
	
	io_close(File);
	
	m_lGhosts.add(Ghost);
}

void CGhost::Unload(int ID)
{
	for(int i = 0; i < m_lGhosts.size(); i++)
	{
		if(m_lGhosts[i].m_ID == ID)
		{
			m_lGhosts.remove_index_fast(i);
			break;
		}
	}
}

void CGhost::ConGPlay(IConsole::IResult *pResult, void *pUserData, int ClientId)
{
	((CGhost *)pUserData)->StartRender();
}

void CGhost::OnConsoleInit()
{
	Console()->Register("gplay","", CFGFLAG_CLIENT, ConGPlay, this, "", -1);
}

void CGhost::OnMessage(int MsgType, void *pRawMsg)
{
	if(!g_Config.m_ClRaceGhost || m_pClient->m_Snap.m_Spectate)
		return;
	
	// only for race
	if(!m_pClient->m_IsRace)
		return;
		
	// check for messages from server
	if(MsgType == NETMSGTYPE_SV_KILLMSG)
	{
		CNetMsg_Sv_KillMsg *pMsg = (CNetMsg_Sv_KillMsg *)pRawMsg;
		if(pMsg->m_Victim == m_pClient->m_Snap.m_LocalCid)
		{
			OnReset();
		}
	}
	else if(MsgType == NETMSGTYPE_SV_CHAT)
	{
		CNetMsg_Sv_Chat *pMsg = (CNetMsg_Sv_Chat *)pRawMsg;
		if(pMsg->m_Cid == -1 && m_RaceState == RACE_STARTED)
		{
			const char* pMessage = pMsg->m_pMessage;
			
			int Num = 0;
			while(str_comp_num(pMessage, " finished in: ", 14))
			{
				pMessage++;
				Num++;
				if(!pMessage[0])
					return;
			}
			
			// store the name
			char aName[64];
			str_copy(aName, pMsg->m_pMessage, Num+1);
			
			// prepare values and state for saving
			int Minutes;
			float Seconds;
			if(!str_comp(aName, m_pClient->m_aClients[m_pClient->m_Snap.m_LocalCid].m_aName) && sscanf(pMessage, " finished in: %d minute(s) %f", &Minutes, &Seconds) == 2)
			{
				m_RaceState = RACE_FINISHED;
				if(m_Recording)
				{
					float CurTime = Minutes*60 + Seconds;
					if(CurTime < m_BestTime || m_BestTime == -1)
					{
						m_NewRecord = true;
						m_BestTime = CurTime;
					}
				}
			}
		}
	}
}

void CGhost::OnReset()
{
	StopRecord();
	StopRender();
	m_RaceState = RACE_NONE;
	m_NewRecord = false;
	m_CurPath.clear();
	m_StartRenderTick = -1;
}

void CGhost::OnMapLoad()
{
	OnReset();
	m_BestTime = -1;
	m_lGhosts.clear();
	m_pClient->m_pMenus->GhostlistPopulate();
}
