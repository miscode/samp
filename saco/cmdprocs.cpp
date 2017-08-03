//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: cmdprocs.cpp,v 1.51 2006/05/07 15:38:35 kyeman Exp $
//
//----------------------------------------------------------

#include "main.h"
#include "game/util.h"
#include "game/task.h"

extern CGame		 *pGame;
extern CChatWindow   *pChatWindow;
extern CCmdWindow	 *pCmdWindow;
extern CDeathWindow	 *pDeathWindow;
extern CNetGame		 *pNetGame;
extern IDirect3DDevice9 *pD3DDevice;
extern GAME_SETTINGS tSettings;

extern BYTE	*pbyteCameraMode;
extern bool bShowDebugLabels;

CRemotePlayer *pTestPlayer;

VEHICLE_TYPE *pTrain;

int iCurrentPlayerTest=1;

extern float fFarClip;

extern unsigned long _recvfromUncompressedTotal;
extern unsigned long _recvfromCompressedTotal;
extern unsigned long _sendtoUncompressedTotal;
extern unsigned long _sendtoCompressedTotal;
extern float fOnFootCorrectionMultiplier;
extern float fInCarCorrectionMultiplier;
extern float fInaccuracyFactorMultiplier;

//////////////////////////////////////////////////////
//
// -------R E L E A S E   C O M M A N D S--------
//
// (INCLUDES SCRIPTING UTILS)
//
//////////////////////////////////////////////////////

void cmdDefaultCmdProc(PCHAR szCmd)
{
	if(pNetGame) {
		CLocalPlayer *pLocalPlayer;
		pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
		pLocalPlayer->Say(szCmd);
	}
}

//----------------------------------------------------

void cmdMsg(PCHAR szCmd)
{
	if (!strlen(szCmd)) return;
	
	char * szDestination;
	char * szMsg;

	szDestination = strtok(szCmd, " ");
	if (!szDestination || !IsNumeric(szDestination)) {
		pChatWindow->AddInfoMessage("Enter a valid player ID!");
		return;
	}

	szMsg = strtok(NULL, "");
	if (szMsg == NULL)
	{
		pChatWindow->AddInfoMessage("Enter a message!");
		return;
	}
	
	if(pNetGame) {
		CLocalPlayer *pLocalPlayer;
		pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
		pLocalPlayer->Msg((BYTE)atoi(szDestination), szMsg);
	}
}

//----------------------------------------------------

void cmdTeamMsg(PCHAR szCmd)
{
	if (!strlen(szCmd)) return;
	if(pNetGame) {
		CLocalPlayer *pLocalPlayer;
		pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
		pLocalPlayer->TeamMsg(szCmd);
	}
}

//----------------------------------------------------

extern BOOL gDisableAllFog;

BOOL bDontProcessVehiclePool=FALSE;

void cmdDiscon(PCHAR szCmd)
{	
}

//----------------------------------------------------

void cmdOnfootCorrection(PCHAR szCmd)
{
	if(strlen(szCmd)) {
		fOnFootCorrectionMultiplier = atof(szCmd);
	}	
	pChatWindow->AddDebugMessage("OnFootCorrection: %.4f",fOnFootCorrectionMultiplier);
}

//----------------------------------------------------

void cmdInCarCorrection(PCHAR szCmd)
{
	if(strlen(szCmd)) {
		fInCarCorrectionMultiplier = atof(szCmd);
	}
	pChatWindow->AddDebugMessage("InCarCorrection: %.4f",fInCarCorrectionMultiplier);
}

//----------------------------------------------------

void cmdInAccMultiplier(PCHAR szCmd)
{
	if(strlen(szCmd)) {
		fInaccuracyFactorMultiplier = atof(szCmd);
	}
	pChatWindow->AddDebugMessage("InAccMultiplier: %.4f",fInaccuracyFactorMultiplier);
}

//----------------------------------------------------

void cmdSetFrameLimit(PCHAR szCmd)
{
	if(strlen(szCmd)) {
		DWORD dwFLAmount = atoi(szCmd);
		if(dwFLAmount >= 20 && dwFLAmount <= 100) {
			UnFuck(0xC1704C,4);
			*(PDWORD)0xC1704C = (DWORD)(dwFLAmount * 1.375f);
			pChatWindow->AddDebugMessage("FrameLimiter: %u",dwFLAmount);
		} else {
			pChatWindow->AddDebugMessage("FrameLimiter: valid amounts are 20-100",dwFLAmount);
		}
	}
}

//----------------------------------------------------

void cmdQuit(PCHAR szCmd)
{
	QuitGame();
}

//----------------------------------------------------

void cmdKill(PCHAR szCmd)
{
	pGame->FindPlayerPed()->ResetDamageEntity();
	pGame->FindPlayerPed()->SetDead();
}

//----------------------------------------------------

void cmdDance(PCHAR szCmd)
{
	int iStyle = atoi(szCmd);
	pGame->FindPlayerPed()->StartDancing(iStyle);
}

//----------------------------------------------------

void cmdHandsUp(PCHAR szCmd)
{
	pGame->FindPlayerPed()->HandsUp();
}

//----------------------------------------------------

void cmdCigar(PCHAR szCmd)
{
	pGame->FindPlayerPed()->HoldItem(0x5CD);
}

//----------------------------------------------------

void cmdRcon(PCHAR szCmd)
{
	if (!szCmd) return;

	BYTE bytePacketId = ID_RCON_COMMAND;
	RakNet::BitStream bsCommand;
	bsCommand.Write(bytePacketId);
	DWORD dwCmdLen = (DWORD)strlen(szCmd);
	bsCommand.Write(dwCmdLen);
	bsCommand.Write(szCmd, dwCmdLen);
	pNetGame->GetRakClient()->Send(&bsCommand, HIGH_PRIORITY, RELIABLE, 0);
}

//----------------------------------------------------

void cmdTimestamp(PCHAR szCmd)
{
	if (pChatWindow) pChatWindow->m_bTimestamp = !pChatWindow->m_bTimestamp;
}


//----------------------------------------------------

void cmdCmpStat(PCHAR szCmd)
{
	float upRatio = (float)_sendtoCompressedTotal / _sendtoUncompressedTotal;
	float downRatio = (float)_recvfromCompressedTotal / _recvfromUncompressedTotal;

	if(pChatWindow) {
		pChatWindow->AddDebugMessage("u: %.2f d: %.2f",upRatio*100.0f,downRatio*100.0f);
	}	
}

//----------------------------------------------------

void cmdSavePos(PCHAR szCmd)
{
	CPlayerPed *pPlayer = pGame->FindPlayerPed();
	FILE *fileOut;
	DWORD dwVehicleID;
	float fZAngle;

	//if(!tSettings.bDebug) return;

	fileOut = fopen("savedpositions.txt","a");
	if(!fileOut) {
		pChatWindow->AddDebugMessage("I can't open the savepositions.txt file for append.");
		return;
	}

	// incar savepos

	if(pPlayer->IsInVehicle()) {

		VEHICLE_TYPE *pVehicle = pPlayer->GetGtaVehicle();
	
		dwVehicleID = GamePool_Vehicle_GetIndex(pVehicle);
		ScriptCommand(&get_car_z_angle,dwVehicleID,&fZAngle);

		fprintf(fileOut,"AddStaticVehicle(%u,%.4f,%.4f,%.4f,%.4f,%u,%u); // %s\n",
			pVehicle->entity.nModelIndex,pVehicle->entity.mat->pos.X,pVehicle->entity.mat->pos.Y,pVehicle->entity.mat->pos.Z,
			fZAngle,pVehicle->byteColor1,pVehicle->byteColor2,szCmd);

		fclose(fileOut);

		pChatWindow->AddDebugMessage("-> Vehicle pos saved");

		return;
	}

	// onfoot savepos

	PED_TYPE *pActor = pPlayer->GetGtaActor();
	ScriptCommand(&get_actor_z_angle,pPlayer->m_dwGTAId,&fZAngle);

	fprintf(fileOut,"AddPlayerClass(%u,%.4f,%.4f,%.4f,%.4f,0,0,0,0,0,0); // %s\n",pPlayer->GetModelIndex(),
		pActor->entity.mat->pos.X,pActor->entity.mat->pos.Y,pActor->entity.mat->pos.Z,fZAngle,szCmd);

	pChatWindow->AddDebugMessage("-> Class pos saved");

	fclose(fileOut);
}

//----------------------------------------------------

void cmdPlayerSkin(PCHAR szCmd)
{
#ifndef _DEBUG
	if(!tSettings.bDebug) return;
#endif

	if(!strlen(szCmd)){	
		pChatWindow->AddDebugMessage("Usage: player_skin (skin number).");
		return;
	}
	int iPlayerSkin = atoi(szCmd);

	if(pGame->IsGameLoaded())
	{
		CPlayerPed *pPlayer = pGame->FindPlayerPed();

		if(pPlayer)
		{
			// Ok, now we can set the model.
			pPlayer->SetModelIndex(iPlayerSkin);
		}
		else
		{
			return;
		}		
	}
	else //! game loaded
	{
		return;
	}
}

//----------------------------------------------------

void cmdCreateVehicle(PCHAR szCmd)
{
	if(!tSettings.bDebug) return;

	if(!strlen(szCmd)){
		pChatWindow->AddDebugMessage("Usage: /v (vehicle id).");
		return;
	}	
	int iVehicleType = atoi(szCmd);

	if(pGame->IsGameLoaded())
	{
		pGame->RequestModel(iVehicleType);
		pGame->LoadRequestedModels();

		// place this actor near the player.
		CPlayerPed *pPlayer = pGame->FindPlayerPed();

		if(pPlayer) 
		{
			MATRIX4X4 matPlayer;
			pPlayer->GetMatrix(&matPlayer);
			CHAR blank[9] = "";
			sprintf(blank, "TYPE_%d", iVehicleType);
			CVehicle *pTestVehicle = pGame->NewVehicle(iVehicleType,
				(matPlayer.pos.X - 5.0f), (matPlayer.pos.Y - 5.0f),
				matPlayer.pos.Z+1.0f, 0.0f, (PCHAR)blank);

			pTestVehicle->Add();
			
			/*DWORD dwRet;
			ScriptCommand(&get_active_interior,&dwRet);
			if ((int)dwRet != 0) pTestVehicle->LinkToInterior((int)dwRet);*/

			return;
		}
		else {
			pChatWindow->AddDebugMessage("I couldn't find the player actor.");
			return;
		}
	}
	else {
		pChatWindow->AddDebugMessage("game not loaded.");
	}
}

//----------------------------------------------------

void cmdSelectVehicle(PCHAR szCmd)
{
	if(!tSettings.bDebug) return;

	GameDebugEntity(0,0,10);
}

//-----------------------------------------------------

void cmdShowInterior(PCHAR szCmd)
{
	DWORD dwRet;
	ScriptCommand(&get_active_interior,&dwRet);
	pChatWindow->AddDebugMessage("Current Interior: %u",dwRet);
}

//-----------------------------------------------------

void cmdDebugLabels(PCHAR szCmd)
{
	bShowDebugLabels = !bShowDebugLabels;
}

//-----------------------------------------------------

//////////////////////////////////////////////////////
//
// --------D E B U G   C O M M A N D S--------
//
// WILL BE #ifdef'd OUT BEFORE RELEASE!
// 
//////////////////////////////////////////////////////

#ifdef _DEBUG

//-----------------------------------------------------

void cmdSetInterior(PCHAR szCmd)
{
	if(!strlen(szCmd)){
		pChatWindow->AddDebugMessage("Usage: /setinterior (interior id).");
		return;
	}
	int iInterior = atoi(szCmd);
	ScriptCommand(&select_interior,iInterior);
}

//----------------------------------------------------
/*
void cmdDebugLabels(PCHAR szCmd)
{
	bShowDebugLabels = !bShowDebugLabels;
}
*/
//----------------------------------------------------

void cmdPlayerToVehicle(PCHAR szCmd)
{
	int iPlayer,iVehicle;
	sscanf(szCmd,"%u%u",&iPlayer,&iVehicle);

	if(pNetGame) {
		CRemotePlayer *pRemotePlayer = pNetGame->GetPlayerPool()->GetAt(iPlayer);
		CVehicle *pVehicle = pNetGame->GetVehiclePool()->GetAt(iVehicle);
		if(pRemotePlayer && pVehicle) {
			pRemotePlayer->GetPlayerPed()->PutDirectlyInVehicle(pVehicle->m_dwGTAId,0);
			//pRemotePlayer->GetPlayerPed()->EnterVehicle(pVehicle->m_dwGTAId,FALSE);
		}
	}
}

//----------------------------------------------------

void cmdCamMode(PCHAR szCmd)
{
	int iMode;
	sscanf(szCmd,"%d",&iMode);
	*pbyteCameraMode = (BYTE)iMode;	
}

//----------------------------------------------------

void cmdCreatePlayer(PCHAR szCmd)
{
	if(!strlen(szCmd)) {
	    pChatWindow->AddDebugMessage("Usage: /p (player_num) (skin).");
		return;
	}

	int iActorSkin=0,iPlayerNum=0;

	sscanf(szCmd, "%d%d", &iPlayerNum, &iActorSkin);

	if(pGame->IsGameLoaded()) {

		pGame->RequestModel(iActorSkin);
		pGame->LoadRequestedModels();

		// place this new player near the local player.
		CPlayerPed *pPlayer = pGame->FindPlayerPed();

		if(pPlayer)
		{
			MATRIX4X4 matPlayer;
			pPlayer->GetMatrix(&matPlayer);
			CPlayerPed *pTestPlayer;

			pTestPlayer = pGame->NewPlayer(iPlayerNum, 0,
					(matPlayer.pos.X - 5.0f), (matPlayer.pos.Y - 5.0f),
					matPlayer.pos.Z,0.0f);

			//pTestPlayer->SetSkin(iActorSkin);
			//pTestPlayer->ShowMarker(1);

			return;
		}
		else {
			pChatWindow->AddDebugMessage("I couldn't obtain the player actor.");
			return;
		}
	}
	else {
		pChatWindow->AddDebugMessage("Game not loaded.");
		return;
	}
}

//----------------------------------------------------

void cmdActorDebug(PCHAR szCmd)
{
	if(!strlen(szCmd)){
		pChatWindow->AddDebugMessage("Usage: /actor_debug (actor number).");
		return;
	}
	int iActor = atoi(szCmd);
	GameDebugEntity(iActor,0,1);
}

//----------------------------------------------------

void cmdActorTaskDebug(PCHAR szCmd)
{
	if(!strlen(szCmd)){
		pChatWindow->AddDebugMessage("Usage: /actor_task_debug (actor number).");
		return;
	}
	int iActor = atoi(szCmd);
	GameDebugEntity(iActor,0,8);
}

//----------------------------------------------------

void cmdObjectDebug(PCHAR szCmd)
{

}

//----------------------------------------------------

void cmdVehicleDebug(PCHAR szCmd)
{
	if(!strlen(szCmd)){
		pChatWindow->AddDebugMessage("Usage: /vehicle_debug (vehicle number).");
		return;
	}
	
	int iVehicle = atoi(szCmd);
	if(iVehicle < 3) pChatWindow->AddDebugMessage("Warning: vehicles below 3 are usually reserved.");
	GameDebugEntity(iVehicle,0,2);
}

//----------------------------------------------------

void cmdShowGameInterfaceDebug(PCHAR szCmd)
{
	GameDebugEntity(0,0,6);
}

//----------------------------------------------------

void cmdPlayerVehicleDebug(PCHAR szCmd)
{
	GameDebugEntity(0,0,3);
}

//----------------------------------------------------

void cmdDebugStop(PCHAR szCmd)
{
	GameDebugScreensOff();
}

//----------------------------------------------------

void cmdSetCameraPos(PCHAR szCmd)
{
	float fX,fY,fZ,fLookX,fLookY,fLookZ;

	if(!strlen(szCmd)){
		pChatWindow->AddDebugMessage("Usage: /set_camera_pos (x) (y) (x) (lookx) (looky) (lookz)");
		return;
	}

	sscanf(szCmd,"%f%f%f%f%f%f",&fX,&fY,&fZ,&fLookX,&fLookY,&fLookZ);
	pGame->GetCamera()->SetPosition(fX,fY,fZ,0.0f,0.0f,0.0f);
	pGame->GetCamera()->LookAtPoint(fLookX,fLookY,fLookZ,0);
}

//----------------------------------------------------

void cmdPutActorInVehicle(PCHAR szCmd) // kyeman
{

}

//----------------------------------------------------

void cmdGiveActorWeapon(PCHAR szCmd)
{
	if(!strlen(szCmd)){
		pChatWindow->AddDebugMessage("Usage: give_weapon (actor number) (weapon number).");
		return;
	}

	int	 iWeaponID=0;
	sscanf(szCmd,"%d",&iWeaponID);
	pGame->FindPlayerPed()->GiveWeapon(iWeaponID,1000);
}

//----------------------------------------------------

void cmdDisplayMemory(PCHAR szCmd)
{
	if(!strlen(szCmd)){
		pChatWindow->AddDebugMessage("Usage: disp_mem (debug address) (debug offset)");
		return;
	}	
	
	DWORD dwDebugAddr;
	DWORD dwDebugOffset;

	sscanf(szCmd,"%X%u",&dwDebugAddr,&dwDebugOffset);

	if(!dwDebugAddr) {
		pChatWindow->AddDebugMessage("usage: /disp_mem (Hex Address) (Decimal Offset)");
		return;
	}
	
	GameDebugEntity(dwDebugAddr,dwDebugOffset,5);
}

//----------------------------------------------------

void cmdDisplayMemoryAsc(PCHAR szCmd)
{
	if(!strlen(szCmd)){
		pChatWindow->AddDebugMessage("Usage: disp_mem_asc (debug address) (debug offset)");
		return;
	}	
	
	DWORD dwDebugAddr;
	DWORD dwDebugOffset;

	sscanf(szCmd,"%X%u",&dwDebugAddr,&dwDebugOffset);

	if(!dwDebugAddr) {
		pChatWindow->AddDebugMessage("usage: /disp_mem_asc (Hex Address) (Decimal Offset)");
		return;
	}
	
	GameDebugEntity(dwDebugAddr,dwDebugOffset,15);
}

//----------------------------------------------------

void cmdSetWeather(PCHAR szCmd)
{
	if(!strlen(szCmd)){
		pChatWindow->AddDebugMessage("Usage: set_weather (weather number)");
		return;
	}	
	int iWeatherID = atoi(szCmd);
	pGame->SetWorldWeather(iWeatherID);
}

//----------------------------------------------------

void cmdSetTime(PCHAR szCmd)
{
	if(!strlen(szCmd)){
		pChatWindow->AddDebugMessage("Usage: set_time (hour) (minute)");
		return;
	}
		
	int	iHour=0,iMinute=0;

	sscanf(szCmd,"%d%d",&iHour,&iMinute);

	if ((iHour >= 0 && iHour <= 23) && (iMinute >= 0 && iMinute <= 59)) {
		pGame->SetWorldTime(iHour,iMinute);
	}
	else {
		pChatWindow->AddDebugMessage("Invalid Time. Use /set_time (hour 0-23) (minute 0-59)");
	}
}

//----------------------------------------------------

void cmdRemotePlayer(PCHAR szCmd)
{
	if(pNetGame) {		
		CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
		CPlayerPed *pPlayer = pGame->FindPlayerPed();

		MATRIX4X4 matPlayer;
		pPlayer->GetMatrix(&matPlayer);

		VECTOR vecPos;
		vecPos.X = matPlayer.pos.X - 5.0f;
		vecPos.Y = matPlayer.pos.Y - 5.0f;
		vecPos.Z = matPlayer.pos.Z;

		while(iCurrentPlayerTest != 10) {
			pPlayerPool->New(iCurrentPlayerTest,"TestPlayer");
			CRemotePlayer *pRemotePlayer = pPlayerPool->GetAt((BYTE)iCurrentPlayerTest);
			pRemotePlayer->Spawn(255,50,&vecPos,0.0f,0);
			//pRemotePlayer->Say("kick me coz I'm lame");
			vecPos.X += 1.0f;
			pRemotePlayer->GetPlayerPed()->Remove();
			pTestPlayer = pRemotePlayer;
			pChatWindow->AddDebugMessage("Created Player: %u\n",iCurrentPlayerTest);
			iCurrentPlayerTest++;
		}
	}
}

//----------------------------------------------------

void cmdRemotePlayerRespawn(PCHAR szCmd)
{
	CPlayerPed *pPlayer = pGame->FindPlayerPed();

	if(pPlayer)
	{
		MATRIX4X4 matPlayer;
		pPlayer->GetMatrix(&matPlayer);

		VECTOR vecPos;
		vecPos.X = matPlayer.pos.X - 10.0f;
		vecPos.Y = matPlayer.pos.Y - 10.0f;
		vecPos.Z = matPlayer.pos.Z;

		//pTestPlayer->SpawnPlayer((BYTE)1+190,&vecPos,0.0f,0,25,100,-1,-1,-1,-1);
	}
}

//----------------------------------------------------

void cmdPlayerAddr(PCHAR szCmd)
{
	int iActor=0;
	sscanf(szCmd,"%d",&iActor);

	if(iActor==0) {
		pChatWindow->AddDebugMessage("0x%X",GamePool_FindPlayerPed());
	} else {
		pChatWindow->AddDebugMessage("0x%X",GamePool_Ped_GetAt(iActor));
	}
}

//----------------------------------------------------

void cmdVehicleSyncTest(PCHAR szCmd)
{
	GameDebugEntity(0,0,7);
}

//----------------------------------------------------

void cmdSay(PCHAR szCmd)
{
	int iSay = atoi(szCmd);
	BYTE * dwPlayerPed1 = (BYTE *)GamePool_FindPlayerPed();

	_asm push 0
	_asm push 0
	_asm push 0
	_asm push 0x3F800000
	_asm push 0
	_asm push iSay
	_asm mov ecx, dwPlayerPed1
	_asm mov ebx, 0x5EFFE0
	_asm call ebx
}

//----------------------------------------------------

DWORD MyCreateActor(int iModelID, float X, float Y, float Z, float fRotation)
{
	DWORD dwRetID;
	pGame->RequestModel(iModelID);
	pGame->LoadRequestedModels();
	while(!pGame->IsModelLoaded(iModelID)) Sleep(5);
	ScriptCommand(&create_actor,5,iModelID,X,Y,Z,&dwRetID);
	ScriptCommand(&set_actor_z_angle,dwRetID,fRotation);
	pChatWindow->AddDebugMessage("New Actor: 0x%X\n",GamePool_Ped_GetAt(dwRetID));
	return dwRetID;
}

//----------------------------------------------------

void cmdCreateActor(PCHAR szCmd)
{
	int iActor = atoi(szCmd);

	// place this new actor near the local player.
	CPlayerPed *pPlayer = pGame->FindPlayerPed();

	if(pPlayer)
	{
		MATRIX4X4 matPlayer;
		pPlayer->GetMatrix(&matPlayer);

		VECTOR vecPos;
		vecPos.X = matPlayer.pos.X - 5.0f;
		vecPos.Y = matPlayer.pos.Y - 5.0f;
		vecPos.Z = matPlayer.pos.Z;
		
		MyCreateActor(iActor,vecPos.X,vecPos.Y,vecPos.Z,0.0f);
	}
}

//----------------------------------------------------

void cmdFreeAim(PCHAR szCmd)
{ 
	CPlayerPed *pPlayer = pGame->FindPlayerPed();

	pPlayer->GiveWeapon(28,200);
	pPlayer->StartPassengerDriveByMode();	
}

//----------------------------------------------------

void cmdWorldVehicleRemove(PCHAR szCmd)
{ 	
	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	CVehicle *pThisVehicle;

	int iVehicle=0;
	sscanf(szCmd,"%d",&iVehicle);
	pThisVehicle = pVehiclePool->GetAt(iVehicle);
	if(pThisVehicle) pThisVehicle->Remove();
}

//----------------------------------------------------

void cmdWorldVehicleAdd(PCHAR szCmd)
{ 	
	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	CVehicle *pThisVehicle;

	int iVehicle=0;
	sscanf(szCmd,"%d",&iVehicle);
	pThisVehicle = pVehiclePool->GetAt(iVehicle);
	if(pThisVehicle) pThisVehicle->Add();
}

//----------------------------------------------------

void cmdWorldPlayerRemove(PCHAR szCmd)
{ 	
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	CRemotePlayer *pThisPlayer;

	int iPlayer=0;
	sscanf(szCmd,"%d",&iPlayer);
	pThisPlayer = pPlayerPool->GetAt(iPlayer);
	if(pThisPlayer) pThisPlayer->GetPlayerPed()->Remove();
}

//----------------------------------------------------

void cmdWorldPlayerAdd(PCHAR szCmd)
{ 	
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	CRemotePlayer *pThisPlayer;

	int iPlayer=0;
	sscanf(szCmd,"%d",&iPlayer);
	pThisPlayer = pPlayerPool->GetAt(iPlayer);
	if(pThisPlayer) pThisPlayer->GetPlayerPed()->Add();
}

//----------------------------------------------------

void cmdAnimSet(PCHAR szCmd)
{ 	
	CPlayerPed *pPlayer = pGame->FindPlayerPed();
	if(pPlayer) pPlayer->SetAnimationSet(szCmd);
}

//----------------------------------------------------

void cmdKillRemotePlayer(PCHAR szCmd)
{
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	CRemotePlayer *pThisPlayer;

	int iPlayer=0;
	sscanf(szCmd,"%d",&iPlayer);
	pThisPlayer = pPlayerPool->GetAt(iPlayer);
	if(pThisPlayer) pThisPlayer->GetPlayerPed()->SetDead();
}

//----------------------------------------------------

void cmdApplyAnimation(PCHAR szCmd)
{
	char szAnimFile[256];
	char szAnimName[256];
	sscanf(szCmd,"%s%s",szAnimName,szAnimFile);

	//CPlayerPed *pPlayer = pGame->FindPlayerPed();
	//pPlayer->ApplyAnimation(szAnimName,szAnimFile,4.0f,1,0,0,0,0);

	pChatWindow->AddDebugMessage("Applied Anim: %s Of Group: %s",szAnimName,szAnimFile);
}

//----------------------------------------------------

void cmdCheckpointTest(PCHAR szCmd)
{
	CPlayerPed *pPlayer = pGame->FindPlayerPed();

	if(pPlayer)
	{
		MATRIX4X4 matPlayer;
		pPlayer->GetMatrix(&matPlayer);

		VECTOR vecPos;
		vecPos.X = matPlayer.pos.X - 5.0f;
		vecPos.Y = matPlayer.pos.Y - 5.0f;
		vecPos.Z = matPlayer.pos.Z;

		VECTOR vecExtent = {2.0f,2.0f,2.0f};

		pGame->SetCheckpointInformation(&vecPos,&vecExtent);
		pGame->ToggleCheckpoints(TRUE);
	}
}

//----------------------------------------------------

void cmdCreateRadarMarkers(PCHAR szCmd)
{
	CPlayerPed *pPlayer = pGame->FindPlayerPed();

	if(pPlayer)
	{
		MATRIX4X4 matPlayer;
		pPlayer->GetMatrix(&matPlayer);

		VECTOR vecPos;
		vecPos.X = matPlayer.pos.X - 10.0f;
		vecPos.Y = matPlayer.pos.Y;
		vecPos.Z = matPlayer.pos.Z;

		for (int i = 0; i <= 100; i++) {
			pGame->CreateRadarMarkerIcon(i, vecPos.X, vecPos.Y, vecPos.Z);
			vecPos.X -= 40.0f;
		}
	}

}

//----------------------------------------------------

void cmdPlaySound(PCHAR szCmd)
{
	CPlayerPed *pPlayer = pGame->FindPlayerPed();

	if(pPlayer)
	{
		MATRIX4X4 matPlayer;
		pPlayer->GetMatrix(&matPlayer);

		int iSound = 0;
		sscanf(szCmd, "%d", &iSound);
		ScriptCommand(&play_sound, matPlayer.pos.X, matPlayer.pos.Y, matPlayer.pos.Z, iSound);
	}
}

//----------------------------------------------------

void cmdSaveLocation(PCHAR szCmd)
{
	FILE* fileOut = fopen("savedlocations.txt", "a");
	if (fileOut)
	{
		float fZAngle;
		ScriptCommand(&get_actor_z_angle, 0x1, &fZAngle);

		VECTOR* pVec = &pGame->FindPlayerPed()->GetGtaActor()->entity.mat->pos;
		fprintf(fileOut,"%.4f, %.4f, %.4f, %.4f   // %s\n", pVec->X, pVec->Y, pVec->Z, fZAngle, szCmd);

		fclose(fileOut);
	}
}

//----------------------------------------------------
// To test whether reseting the vehicle pool works

void cmdResetVehiclePool(PCHAR szCmd)
{
	if(pNetGame) {
		pNetGame->ResetVehiclePool();
	}
}

//----------------------------------------------------
// To test whether reseting the vehicle pool works

void cmdDeactivatePlayers(PCHAR szCmd)
{
	if(pNetGame) {
		pNetGame->GetPlayerPool()->DeactivateAll();
	}
}

//----------------------------------------------------

void cmdSetFarClip(PCHAR szCmd)
{
	float f;
	sscanf(szCmd,"%f",&f);
	fFarClip = f;
}

//----------------------------------------------------

void cmdStartJetpack(PCHAR szCmd)
{
	pGame->FindPlayerPed()->StartJetpack();
}

//----------------------------------------------------

void cmdStopJetpack(PCHAR szCmd)
{
	pGame->FindPlayerPed()->StopJetpack();	
}

//----------------------------------------------------

void cmdLotsOfV(PCHAR szCmd)
{
	//CVehicle *pVehicle;
	int i=210;
	int iNumAdded=0;

	VECTOR vecSpawnPos;
	vecSpawnPos.X=1482.7546f;
	vecSpawnPos.Y=3236.4551f;
	vecSpawnPos.Z=0.1f;
	CHAR plate[2] = "";

	while(i != MAX_VEHICLES) {
		//pVehicle = new CVehicle(473,x,y,z,8.7f,plate);
		pNetGame->GetVehiclePool()->New(i,473,&vecSpawnPos,0.0f,0,0,&vecSpawnPos,0.0f,0,plate);
		vecSpawnPos.X -= 7.0f;
		i++;
	}

	pChatWindow->AddDebugMessage("Done");
}

//----------------------------------------------------

void TestExit(PCHAR szCmd)
{
	pD3DDevice->ShowCursor(TRUE);
	ShowCursor(TRUE);
	SetCursor(LoadCursor(NULL,IDC_ARROW));
	ShowCursor(TRUE);

	pChatWindow->AddDebugMessage("Done");
}

//----------------------------------------------------

void DriveByTest(PCHAR szCmd)
{
	cmdRemotePlayer("");
	//cmdRemotePlayer("");
	//cmdRemotePlayer("");
	//cmdRemotePlayer("");

	CRemotePlayer *pRemote[4];

	pRemote[0] = pNetGame->GetPlayerPool()->GetAt(1);
	//pRemote[1] = pNetGame->GetPlayerPool()->GetAt(2);
	//pRemote[2] = pNetGame->GetPlayerPool()->GetAt(3);
	//pRemote[3] = pNetGame->GetPlayerPool()->GetAt(4);

	pRemote[0]->GetPlayerPed()->GiveWeapon(31,200);
	//pRemote[1]->GetPlayerPed()->GiveWeapon(31,200);
	//pRemote[2]->GetPlayerPed()->GiveWeapon(31,200);
	//pRemote[3]->GetPlayerPed()->GiveWeapon(31,200);

	int iVehicleID = pNetGame->GetVehiclePool()->FindGtaIDFromID(3);

	pGame->FindPlayerPed()->GiveWeapon(31,200);

	/*
	pRemote[0]->GetPlayerPed()->PutDirectlyInVehicle(iVehicleID,0);
	pRemote[1]->GetPlayerPed()->PutDirectlyInVehicle(iVehicleID,1);
	pRemote[2]->GetPlayerPed()->PutDirectlyInVehicle(iVehicleID,2);
	//pRemote[3]->GetPlayerPed()->PutDirectlyInVehicle(iVehicleID,3);

	//pRemote[0]->GetPlayerPed()->SetCameraMode(55);
	pRemote[1]->GetPlayerPed()->SetCameraMode(55);
	pRemote[2]->GetPlayerPed()->SetCameraMode(55);
	pRemote[3]->GetPlayerPed()->SetCameraMode(55);

	pRemote[1]->GetPlayerPed()->StartPassengerDriveByMode();
	pRemote[2]->GetPlayerPed()->StartPassengerDriveByMode();
	pRemote[3]->GetPlayerPed()->StartPassengerDriveByMode();

	pGame->FindPlayerPed()->GiveWeapon(28,200);
	pGame->FindPlayerPed()->PutDirectlyInVehicle(iVehicleID,3);*/
}

//-----------------------------------------------------

void cmdLotsOfPlayers(PCHAR c)
{


}

//-----------------------------------------------------

void cmdSpectatev(PCHAR szCmd)
{
	if (!pNetGame) return;
	int iPlayerID;
	int iSpectateMode = 0;
	CLocalPlayer *pPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
	if (strlen(szCmd))
	{
		sscanf(szCmd, "%d%d", &iPlayerID, &iSpectateMode);
		pPlayer->ToggleSpectating(TRUE);
		pPlayer->m_byteSpectateMode = iSpectateMode;
		pPlayer->SpectateVehicle(iPlayerID);
	} else {
		pPlayer->ToggleSpectating(FALSE);
	}
}
//-----------------------------------------------------

void cmdSpectatep(PCHAR szCmd)
{
	if (!pNetGame) return;
	int iPlayerID;
	int iSpectateMode = 0;
	CLocalPlayer *pPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
	if (strlen(szCmd))
	{
		sscanf(szCmd, "%d%d", &iPlayerID, &iSpectateMode);
		pPlayer->ToggleSpectating(TRUE);
		pPlayer->m_byteSpectateMode = iSpectateMode;
		pPlayer->SpectatePlayer(iPlayerID);
	} else {
		pPlayer->ToggleSpectating(FALSE);
	}
}

//-----------------------------------------------------

void cmdClothes(PCHAR szCmd)
{
	char szTexture[64];
	char szModel[64];
	int iType = 0;

	sscanf(szCmd, "%s %s %d", szTexture, szModel, &iType);
	if (strcmp(szTexture, "-")==0)
		strcpy(szTexture, "");
	if (strcmp(szModel, "-")==0)
		strcpy(szModel, "");

	__asm
	{
		push 0xFFFFFFFF;
		mov eax, 0x56E210; // Get CPlayerPed
		call eax;
		add esp, 4;

		// Set up stack for later.. CPlayerPed__UpdateAfterPhysicalChange
		push 0;
		push eax;

		// Get the clothes thing and process
		mov ecx, [eax+1152];	// get CPlayerInfo
		mov ecx, [ecx+4];		// get CPlayerClothes
		
		push iType;
		lea eax, szModel;
		push eax;
		lea eax, szTexture;
		push eax;
		mov eax, 0x5A8080;		// CPlayerClothes__SetClothes
		call eax;				// thiscall, callee cleans up stack

		// Stack is already set up at this point
		mov eax, 0x5A82C0;		// CPlayerPed__UpdateAfterPhysicalChange
		call eax;
		add esp, 8;
	}

}

//-----------------------------------------------------

void cmdNoFire(PCHAR szCmd)
{
	pGame->FindPlayerPed()->ExtinguishFire();
}

//-----------------------------------------------------

void cmdSit(PCHAR szCmd)
{
	ScriptCommand(&actor_task_sit,pGame->FindPlayerPed()->m_dwGTAId,-1);

	/*ApplyCommandTask( "TaskSitInChair",ent1,ent2(0),
	// (unk),-1,(unk),0x3E800000,500,3,0 );
	
	CTask *pTaskSitInChair = new CTask(52);
	DWORD taskPtr = (DWORD)pTaskSitInChair->m_pTaskType;
	DWORD p = (DWORD)pGame->FindPlayerPed()->m_pPed->Tasks;

	//_asm push 0
	//_asm push 0
	_asm push p
	_asm mov ecx, taskPtr
	_asm mov edx, 0x6753E0
	_asm call edx

	pTaskSitInChair->ApplyToPed(pGame->FindPlayerPed());*/	
}

//-----------------------------------------------------

void cmdAttractAddr(PCHAR szCmd)
{
	pChatWindow->AddDebugMessage("0x%X",*(DWORD *)0xB744C4);	
}

//-----------------------------------------------------

void cmdPlayerVtbl(PCHAR szCmd)
{
	//GamePool_FindPlayerPed()->entity.vtable = 0x86C0A8;
	CPlayerPed *pPed = pGame->FindPlayerPed();
	BYTE *b = (BYTE *)pPed->m_pPed+1156;
	BYTE sv = *b;

	pPed->m_pPed->dwPedType = 3;
	*b = 2;

	pPed->TeleportTo(pPed->m_pPed->entity.mat->pos.X,
			pPed->m_pPed->entity.mat->pos.Y,
			pPed->m_pPed->entity.mat->pos.Z+10.0f);

	*b = sv;
	pPed->m_pPed->dwPedType = 0;

}

//-----------------------------------------------------

void cmdDumpFreqTable(PCHAR szCmd)
{
	unsigned int freqTable[256];

	if(pNetGame) {
		pNetGame->GetRakClient()->GetSendFrequencyTable(freqTable);
		FILE *fout = fopen("freqclient.txt","w");
		if(!fout) return;
		int x=0;
		while(x!=256) {
			fprintf(fout,"%u,",freqTable[x]);
			x++;
		}
		fclose(fout);
	}
	pChatWindow->AddDebugMessage("Done.");
}

//-----------------------------------------------------

void cmdPopWheels(PCHAR szCmd)
{
	BYTE byteToggle = atoi(szCmd);
	VEHICLEID vehicleid;
	if (pNetGame) {
		if (pNetGame->GetPlayerPool() && pNetGame->GetVehiclePool()) {
			vehicleid = pNetGame->GetPlayerPool()->GetLocalPlayer()->m_CurrentVehicle;
			if (vehicleid != 0xFFFF) {
				pNetGame->GetVehiclePool()->GetAt(vehicleid)->SetWheelPopped(0, byteToggle);
				pNetGame->GetVehiclePool()->GetAt(vehicleid)->SetWheelPopped(1, byteToggle);
				pNetGame->GetVehiclePool()->GetAt(vehicleid)->SetWheelPopped(2, byteToggle);
				pNetGame->GetVehiclePool()->GetAt(vehicleid)->SetWheelPopped(3, byteToggle);
				pChatWindow->AddDebugMessage((byteToggle) ? "Wheels popped" : "Wheels not popped");
			} else {
				pChatWindow->AddDebugMessage("Get in a vehicle.");
			}
		}
	}
}

//-----------------------------------------------------

void cmdVModels(PCHAR szCmd)
{
    CVehiclePool *pVehiclePool;
	if(pNetGame) {
		pVehiclePool = pNetGame->GetVehiclePool();

		int x=0,iTotalVehicles=0;
        int iCount[211];
		int iVehicleType;

		memset(&iCount[0],0,sizeof(iCount));

		while(x!=MAX_VEHICLES) {
			if(pVehiclePool->GetSlotState(x)) {
				iVehicleType = pVehiclePool->m_SpawnInfo[x].iVehicleType;
				iCount[iVehicleType-400]++;
				iTotalVehicles++;
			}
			x++;
		}

        x=0;
		int iTotal=0;
		while(x!=211) {
			if(iCount[x]) {
				iTotal++;
			}
			x++;
		}
		pChatWindow->AddDebugMessage("Vehicle Types: %u/%u",iTotal,iTotalVehicles);
	}            
}

//-----------------------------------------------------

void cmdFollowPedSA(PCHAR szCmd)
{
	VECTOR LookAt;
    CAMERA_AIM *Aim = GameGetInternalAim();

	LookAt.X = Aim->pos1x + (Aim->f1x * 10.0f);
	LookAt.Y = Aim->pos1y + (Aim->f1y * 10.0f);
	LookAt.Z = Aim->pos1z + (Aim->f1z * 10.0f);

    pGame->FindPlayerPed()->ApplyCommandTask(
		"FollowPedSA",0,1500,-1,&LookAt,0,1.0f,500,3,0);
}



//-----------------------------------------------------

void cmdTaskSitIdle(PCHAR szCmd)
{
	VECTOR LookAt;
    CAMERA_AIM *Aim = GameGetInternalAim();

	LookAt.X = Aim->pos1x + (Aim->f1x * 10.0f);
	LookAt.Y = Aim->pos1y + (Aim->f1y * 10.0f);
	LookAt.Z = Aim->pos1z + (Aim->f1z * 10.0f);

    pGame->FindPlayerPed()->ApplyCommandTask(
		"TaskSitIdle",0,99999,-1,0,0,0.25f,750,3,0);

	//  pGame->FindPlayerPed()->ApplyCommandTask(
		//"TaskProzzy",0,99999,-1,0,0,0.25f,500,3,0);
}

//-----------------------------------------------------

extern int iVehiclesBench;
extern int iPlayersBench;
extern int iPicksupsBench;
extern int iMenuBench;
extern int iObjectBench;
extern int iTextDrawBench;

void cmdBench(PCHAR szCmd)
{
	pChatWindow->AddDebugMessage("Pl: %u V: %u Pk: %u Mn: %u O: %u T: %u",
		iPlayersBench,iVehiclesBench,iPicksupsBench,iMenuBench,iObjectBench,
		iTextDrawBench);
}

//-----------------------------------------------------

void cmdDisableEnEx(PCHAR szCmd)
{
	pGame->DisableEnterExits();
}

//-----------------------------------------------------

void cmdStartPiss(PCHAR szCmd)
{
	if(pNetGame) {
		pNetGame->GetPlayerPool()->
			GetLocalPlayer()->ApplySpecialAction(SPECIAL_ACTION_URINATE);
	}
}

//-----------------------------------------------------

void cmdShowMyVirtualWorld(PCHAR szCmd)
{
	if(pNetGame) {
		pChatWindow->AddDebugMessage("MyVW: %u",pNetGame->GetPlayerPool()->GetLocalPlayer()->GetVirtualWorld());
	}
}

//------------------------------------------------------

void cmdShowPlayerVirtualWorld(PCHAR szCmd)
{
	BYTE bytePlayerID = atoi(szCmd);

	if(pNetGame) {
		CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
		if(pPlayerPool->GetSlotState(bytePlayerID)) {
			pChatWindow->AddDebugMessage("VW: %u",pPlayerPool->GetAt(bytePlayerID)->m_byteVirtualWorld);
		}
	}
}

//------------------------------------------------------

void cmdRemoveBuilding(PCHAR szCmd) {
	CLocalPlayer *pPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
	MATRIX4X4 mat;
	pPlayer->GetPlayerPed()->GetMatrix(&mat);
	VECTOR pos = mat.pos;
	pGame->RemoveBuildingForPlayer(-1, pos, 100.0f);
}

//----------------------------------------------------

void cmdUnderMap(PCHAR szCmd) {
	CLocalPlayer *pPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
	MATRIX4X4 mat;
	pPlayer->GetPlayerPed()->GetMatrix(&mat);
	pPlayer->GetPlayerPed()->TeleportTo(mat.pos.X, mat.pos.Y, mat.pos.Z - 10.0f);
}

//----------------------------------------------------

void cmdColour(PCHAR szCmd) {
	CLocalPlayer *pPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
	CVehicle *pVehicle = pNetGame->GetVehiclePool()->GetAt(pPlayer->GetPlayerPed()->GetCurrentVehicleID());
	if (pVehicle) {
		pChatWindow->AddDebugMessage("Colour1: %i | Colour2: %i", pVehicle->m_pVehicle->byteColor1, pVehicle->m_pVehicle->byteColor2);
		pChatWindow->AddDebugMessage("Vehicle: 0x%X | Colour1_mem: 0x%X | Coour2 mem: 0x%X", &pVehicle, &pVehicle->m_pVehicle->byteColor1, &pVehicle->m_pVehicle->byteColor2);
	}
}

//----------------------------------------------------

#endif

//----------------------------------------------------

void SetupCommands()
{
	// RELEASE COMMANDS
	pCmdWindow->AddDefaultCmdProc(cmdDefaultCmdProc);

	pCmdWindow->AddCmdProc("quit",cmdQuit);
	pCmdWindow->AddCmdProc("q",cmdQuit);
	pCmdWindow->AddCmdProc("save",cmdSavePos);
	pCmdWindow->AddCmdProc("rcon",cmdRcon);

	pCmdWindow->AddCmdProc("dl", cmdDebugLabels);
	pCmdWindow->AddCmdProc("timestamp", cmdTimestamp);

#ifndef _DEBUG
	if (tSettings.bDebug)
	{
#endif
		pCmdWindow->AddCmdProc("vsel",cmdSelectVehicle);
		pCmdWindow->AddCmdProc("v",cmdCreateVehicle);
		pCmdWindow->AddCmdProc("vehicle",cmdCreateVehicle);
		pCmdWindow->AddCmdProc("player_skin",cmdPlayerSkin);
#ifndef _DEBUG
	}
#endif

	pCmdWindow->AddCmdProc("msg",cmdMsg);
	pCmdWindow->AddCmdProc("pm",cmdMsg);
	pCmdWindow->AddCmdProc("tmsg",cmdTeamMsg);
	pCmdWindow->AddCmdProc("tpm",cmdTeamMsg);

	pCmdWindow->AddCmdProc("interior",cmdShowInterior);
	pCmdWindow->AddCmdProc("cmpstat",cmdCmpStat);
	pCmdWindow->AddCmdProc("onfoot_correct",cmdOnfootCorrection);
	pCmdWindow->AddCmdProc("incar_correct",cmdInCarCorrection);
	pCmdWindow->AddCmdProc("inacc_multiplier",cmdInAccMultiplier);
	pCmdWindow->AddCmdProc("fpslimit",cmdSetFrameLimit);

	// DEBUG COMMANDS
#ifdef _DEBUG

	/*
	pCmdWindow->AddCmdProc("handsup",cmdHandsUp);
	pCmdWindow->AddCmdProc("cigar",cmdCigar);
	pCmdWindow->AddCmdProc("kill", cmdKill);
	pCmdWindow->AddCmdProc("cellin",cmdCellIn);
	pCmdWindow->AddCmdProc("cellout",cmdCellOut);*/

	pCmdWindow->AddCmdProc("setinterior",cmdSetInterior);
	pCmdWindow->AddCmdProc("markers", cmdCreateRadarMarkers);
	pCmdWindow->AddCmdProc("vstest",cmdVehicleSyncTest);
	pCmdWindow->AddCmdProc("paddr",cmdPlayerAddr);
	pCmdWindow->AddCmdProc("cammode",cmdCamMode);
	pCmdWindow->AddCmdProc("ptov",cmdPlayerToVehicle);
	pCmdWindow->AddCmdProc("p",cmdCreatePlayer);
	pCmdWindow->AddCmdProc("player",cmdCreatePlayer);
	pCmdWindow->AddCmdProc("a",cmdCreateActor);
	pCmdWindow->AddCmdProc("actor",cmdCreateActor);
	pCmdWindow->AddCmdProc("actor_debug",cmdActorDebug);
	pCmdWindow->AddCmdProc("actor_task_debug",cmdActorTaskDebug);
	pCmdWindow->AddCmdProc("vehicle_debug",cmdVehicleDebug);
	pCmdWindow->AddCmdProc("pv_debug",cmdPlayerVehicleDebug);
	pCmdWindow->AddCmdProc("debug_stop",cmdDebugStop);
	pCmdWindow->AddCmdProc("s",cmdDebugStop);
	pCmdWindow->AddCmdProc("set_cam_pos",cmdSetCameraPos);
	pCmdWindow->AddCmdProc("a2v",cmdPutActorInVehicle);
	pCmdWindow->AddCmdProc("give_weapon",cmdGiveActorWeapon);
	pCmdWindow->AddCmdProc("disp_mem",cmdDisplayMemory);
	pCmdWindow->AddCmdProc("disp_mem_asc",cmdDisplayMemoryAsc);
	pCmdWindow->AddCmdProc("set_weather",cmdSetWeather);
	pCmdWindow->AddCmdProc("set_time",cmdSetTime);
	pCmdWindow->AddCmdProc("rp",cmdRemotePlayer);
	pCmdWindow->AddCmdProc("rpr",cmdRemotePlayerRespawn);
//	pCmdWindow->AddCmdProc("dl", cmdDebugLabels);
	pCmdWindow->AddCmdProc("say",cmdSay);
	pCmdWindow->AddCmdProc("freeaim",cmdFreeAim);
	pCmdWindow->AddCmdProc("vehicle_remove",cmdWorldVehicleRemove);
	pCmdWindow->AddCmdProc("vehicle_add",cmdWorldVehicleAdd);
	pCmdWindow->AddCmdProc("player_remove",cmdWorldPlayerRemove);
	pCmdWindow->AddCmdProc("player_add",cmdWorldPlayerAdd);
	pCmdWindow->AddCmdProc("animset",cmdAnimSet);
	pCmdWindow->AddCmdProc("kill_player",cmdKillRemotePlayer);
	pCmdWindow->AddCmdProc("anim",cmdApplyAnimation);
	pCmdWindow->AddCmdProc("checktest",cmdCheckpointTest);
	pCmdWindow->AddCmdProc("ps",cmdPlaySound);
	pCmdWindow->AddCmdProc("sl",cmdSaveLocation);
	pCmdWindow->AddCmdProc("resetvp",cmdResetVehiclePool);
	pCmdWindow->AddCmdProc("deactivate",cmdDeactivatePlayers);
	pCmdWindow->AddCmdProc("setfarclip",cmdSetFarClip);
	pCmdWindow->AddCmdProc("start_jetpack",cmdStartJetpack);
	pCmdWindow->AddCmdProc("stop_jetpack",cmdStopJetpack);
	pCmdWindow->AddCmdProc("lotsofv",cmdLotsOfV);
	pCmdWindow->AddCmdProc("cursor",TestExit);
	pCmdWindow->AddCmdProc("drivebytest",DriveByTest);
	pCmdWindow->AddCmdProc("specp",cmdSpectatep);
	pCmdWindow->AddCmdProc("specv",cmdSpectatev);
	pCmdWindow->AddCmdProc("clothes",cmdClothes);
	pCmdWindow->AddCmdProc("nofire",cmdNoFire);
	pCmdWindow->AddCmdProc("sit",cmdSit);
	pCmdWindow->AddCmdProc("dumpfreq",cmdDumpFreqTable);
	pCmdWindow->AddCmdProc("attract",cmdAttractAddr);
	pCmdWindow->AddCmdProc("pop",cmdPopWheels);
	pCmdWindow->AddCmdProc("pvtbl",cmdPlayerVtbl);
	pCmdWindow->AddCmdProc("danc",cmdDance);
	pCmdWindow->AddCmdProc("vmodels",cmdVModels);
	pCmdWindow->AddCmdProc("followped",cmdFollowPedSA);
	pCmdWindow->AddCmdProc("sitidle",cmdTaskSitIdle);
	pCmdWindow->AddCmdProc("bench",cmdBench);
	pCmdWindow->AddCmdProc("disenex",cmdDisableEnEx);
	pCmdWindow->AddCmdProc("dbgpiss",cmdStartPiss);

	pCmdWindow->AddCmdProc("showmyvw",cmdShowMyVirtualWorld);
	pCmdWindow->AddCmdProc("showplayervw",cmdShowPlayerVirtualWorld);
	pCmdWindow->AddCmdProc("removebuilding", cmdRemoveBuilding);
	pCmdWindow->AddCmdProc("undermap", cmdUnderMap);
	pCmdWindow->AddCmdProc("getvehiclecolour", cmdColour);
#endif
	
}

//----------------------------------------------------
