#include "zcommon.acs"
#library "base"
#include "definitions.acs"
#include "funcs.acs"

int tTime[MAXPLAYERS];
int PTeleport[MAXPLAYERS][3];
int PiniPos[MAXPLAYERS][3];
int PlayerPos[MAXPLAYERS][3]; //Posicion del jugador actual para usar en kamikaze death.
int Palive[MAXPLAYERS]; //estado de los jugadores, si estan vivos o no
int PlayerPAR[MAXPLAYERS]; //Parejas de cada jugador en caso de modo PAREJASMODE
int PlayerPWS[MAXPLAYERS][11]; //Poderes de cada jugador
int PlayersHT[MAXPLAYERS]; //La vida de los jugadores vieja, para el modo NOHEALTHMODE
int PlayerAngle[MAXPLAYERS]; //Angulo actual del jugador para usar en el kamikaze
int PactPos[MAXPLAYERS][3]; //Actual posición de cada jugador en "tiempo real"
int acTidKz[MAXPLAYERS];

int Theking = -1;
int gameMode = 0;
int allDeath = 0;
int started=0;
int nProjectiles = 1;

/*Que el escudo sea el Armor
*/

script 500 (int type) NET
{
	if (ActivatorTid() != 0)
		terminate;
	int tmpTid = NextTID();
	Thing_ChangeTID(ActivatorTid(), tmpTid);
	bool dead = false;
	int acX, acY, acZ;
	while (ThingCount(T_NONE, tmpTid))
	{
	int i;
	if (!dead)
	{
	acX = GetActorX(tmpTid);
	acY = GetActorY(tmpTid);
	acZ = GetActorZ(tmpTid);
	}
	for (i=0; i < MAXPLAYERS; i++)
	{
		if (!Palive[i] && !PlayerInGame(i))
			continue;
		if (!PlayerPWS[i][INVESCUDO])
			continue;
		if (dead)
			continue;
		int playerTid = i+1000;
		int armorType = FoundArmorType(playerTid);
		/*if (armorType == -1)
			continue;	*/
		int armorAmmount = CheckActorInventory(playerTid, "Armor");
		if (armorAmmount <= 0)
			continue;
		int dMin = distMax(PactPos[i][0], PactPos[i][1], PactPos[i][2], acX, acY, acZ);
		//Si el projectil se encuentra en el rango, proceden las acciones
		if (dMin <= 250)
		{
			int doDamage = typeDamage(type);
			if (doDamage > armorAmmount)
			{
				TakeActorInventory(playerTid, "Armor", armorAmmount);
				if (random(0, 1) == 1)
					break;
			} else {
				TakeActorInventory(playerTid, "Armor", doDamage);
			}
			SetActorState(tmpTid, "Death", true);
			SetActorVelocity(tmpTid, 0.0, 0.0, 0.0, FALSE, FALSE);
			dead = true;
			break;
		}
	}
	delay(4);
	}
}
script 665 OPEN
{
	int i;
	int rand = GetCVar("cv_gmtime") % 256; 
    for (i = 0; i < rand; i++) Random(0,255); 
	int j;
	for (i=0; i < MAXPLAYERS; i++)
	{
		Palive[i] = FALSE;
		tTime[i] = 0;
		PTeleport[i][0] = -1;
		PTeleport[i][1] = -1;
		PTeleport[i][2] = -1;
		PlayerPAR[i] = -1;
		PlayersHT[i] = -1;
		PlayerPos[i][0] = 0;
		PlayerPos[i][1] = 0;
		PlayerPos[i][2] = 0;
		PactPos[i][0] = 0;
		PactPos[i][1] = 0;
		PactPos[i][2] = 0;
		PlayerAngle[i] = 0;
		acTidKz[i] = 0;
		for (j=0; j < 11; j++)
			PlayerPWS[i][j] = false;
	}	
	while (PlayerCount() <= 0)
		delay(20);
		
	//Ajustando tiempos para Speedrun
	int Ptime = GetLevelInfo(LEVELINFO_PAR_TIME);
	if (Ptime >= 300 && Ptime < 400)
		Ptime -=100;
	if (Ptime >= 400)
			Ptime -= 150;

	 Ptime = Ptime*2 + 15;
	 if (Ptime < 180)
		Ptime = 180;
	//Eliminando los mensajes fijos de la pantalla
	HudMessageBold(s:" "; HUDMSG_PLAIN, MODEMSG, CR_BLUE, 1.8, 0.8, 1);
	HudMessageBold(s:" "; HUDMSG_PLAIN, TIMEMSG, CR_BLUE, 1.8, 0.8, 1);
	//Aca comienza la ronda con jugadores
	//Terminar el script de gameModes
	gameMode = random(0, 6); 
	//gameMode = NOHEALTHMODE;
	started = 1;
	str sGameMode = "";
	switch (gameMode)
	{
		case NIGHTMAREMODE:
			sGameMode = "Nightmare estilo Doom 3";
			printbold(s:"Modo Nightmare (Estilo Doom 3)");
		break;
		//Armar para que aparezca el reloj en la pantalla
		case SPEEDRUNMODE:
			sGameMode = "Speedrun";
			printbold(s:"Modo Speed Run\nCorre, Corre como Shanic! ");
			ACS_ExecuteAlways(TIMECOUNT, 0, Ptime);
		break;
		case THEKINGMODE:
			delay(20);
			sGameMode = "The king";
			HudMessageBold(s:".:Modo de juego:.\n", s:sGameMode; HUDMSG_PLAIN, MODEMSG, CR_WHITE, 1.05, 0.83, 0);
			do
			{
				Theking = random(0, MAXPLAYERS);
			}while(!PlayerInGame(Theking) && !Palive[Theking]);
			printbold(s:"Modo The king\nTodos tienen que evitar que ", n:Theking + 1, s:" se muera!");
			int kingPlayerNumer = Theking;
			Theking = Theking + 1000;
			while (1)
			{
			do
			{
				kingPlayerNumer = Theking - 1000;
				delay(10);
			}while(PlayerInGame(kingPlayerNumer) && Palive[kingPlayerNumer]);
			delay(5);
			kingPlayerNumer = Theking - 1000;
			if (PlayerInGame(kingPlayerNumer) && Palive[kingPlayerNumer])
				continue;
			else
				break;
			}
			printbold(s:"El rey ",  n:kingPlayerNumer + 1, s:" ha muerto\nAhora todos moriran en 5 segundos");
			delay(35*5);
			ACS_ExecuteAlways(KILLALL, 0);
		break;
		case PAREJASMODE:
			sGameMode = "Parejas";
			delay(20);
			printbold(s:"Modo Amigos\nTienen que evitar que se muera su pareja asignada");
			HudMessageBold(s:".:Modo de juego:.\n", s:sGameMode; HUDMSG_PLAIN, MODEMSG, CR_WHITE, 1.05, 0.83, 0);
			for (i=0; i < MAXPLAYERS; i++)
			{
				if (!PlayerInGame(i) && !Palive[i])
					continue;
				if (PlayerPAR[i] != -1)
					continue;
				for (j=0; j < MAXPLAYERS; j++)
				{
					if (!PlayerInGame(j) && !Palive[j])
						continue;
					if ((PlayerPAR[j] != -1) || j == i)
						continue;
					PlayerPAR[i] = j;
					PlayerPAR[j] = i;
					break;
				}
			}
			delay (20);
			do
			{
				for (i=0; i < MAXPLAYERS; i++)
				{
					if (PlayerPAR[i] == -1)
						continue;
					if (!Palive[PlayerPAR[i]])
						Thing_Damage(i+1000, 800, MOD_RAILGUN);
				}
				delay(10);
			} while (1);
		break;
		case VANILLAMODE:
			sGameMode = "Vanilla";
			printbold(s:"Modo Vanilla");
		break;
		case NOHEALTHMODE:
			sGameMode = "Pain Time";
			printBold(s:"Pain Mode: hora de sufrir");
		break;
		default:
			sGameMode = "Normal";
			printbold(s:"Modo Normal");
		break;
	}
	do
	{
		HudMessageBold(s:".:Modo de juego:.\n", s:sGameMode; HUDMSG_PLAIN, MODEMSG, CR_WHITE, 1.05, 0.83, 0);
		delay(10);
	} while (!allDeath);
	//Aca se mueren todos
}
script TIMECOUNT (int left)
{
	int minutos=0;
	int segundos=0;
	for (int i=60; i <= left; i+=60)
		minutos++;
	segundos = left-(minutos*60);

	if (minutos <=0 && segundos <= 30)
		SetFont("BIGFONT");
	if (segundos >= 10)
		HudMessageBold(s:"Tiempo restante: ", d:minutos, s:":", d:segundos; HUDMSG_PLAIN, TIMEMSG, CR_RED, 1.5, 0.15, 0);
	else
		HudMessageBold(s:"Tiempo restante: ", d:minutos, s:":0", d:segundos; HUDMSG_PLAIN, TIMEMSG, CR_RED, 1.5, 0.15, 0);

	if (minutos <=0 && segundos <= 30)	
		SetFont("SMALLFONT");
	delay(35);
	left--;
	if (minutos <=0 && segundos <=0)
	{
		do
		{
			HudMessageBold(s:"To fast 4 you"; HUDMSG_PLAIN, TIMEMSG, CR_RED, 1.5, 0.15, 0);
			ACS_ExecuteAlways(KILLALL, 0);
			delay(35);
		} while (1);
	} else {
		ACS_ExecuteAlways(TIMECOUNT, 0, left);
	}
}
script KILLALL (void)
{
	for (int x=0; x < MAXPLAYERS; x++)
	{
		if (!PlayerInGame(x+1000) && !Palive[x])
			continue;
		Thing_Damage(x+1000, 800, MOD_RAILGUN);
	}
}
script 666 ENTER
{
	do
	{
		delay(10);
	}while (!started);
	//Se ejecuta el script comenzada la ronda
	Thing_ChangeTID(0, 1000 + PlayerNumber());
	int acTid = 1000 + PlayerNumber();
	
	Palive[PlayerNumber()] = TRUE;
	ACS_ExecuteAlways(QUITARRUNAS, 0, acTid);
	//Obtener posicion inicial del jugador, para modo Resurreccion
	int x = GetActorX(acTid);
	int y = GetActorY(acTid);
	int z = GetActorZ(acTid);
	PiniPos[PlayerNumber()][0] = x;
	PiniPos[PlayerNumber()][1] = y;
	PiniPos[PlayerNumber()][2] = z;
	int PressButtons;	
	if (!CheckInventory("Pistol"))
		GiveInventory("Pistol", 1);
	if (CheckInventory("Clip") < 20)
		GiveInventory("Clip", 20);
		
	//Eliminando los mensajes fijos de la pantalla
	HudMessage(s:" "; HUDMSG_PLAIN, KINGMSG, CR_BLUE, 1.8, 0.8, 1);
	HudMessage(s:" "; HUDMSG_PLAIN, PAREJAMSG, CR_BLUE, 1.8, 0.8, 1);
	HudMessage(s:" "; HUDMSG_PLAIN, PODERMSG, CR_BLUE, 1.8, 0.8, 1);

	int nJump = getActorProperty(ActivatorTID(),  APROP_Jumpz);

	delay(70);
	if (gameMode == VANILLAMODE)
		terminate;

	ACS_ExecuteAlways(SETPOWERS, 0, acTid);
	ACS_ExecuteAlways(KAMIKAZEPW, 0, acTid);
	while(PlayerInGame(PlayerNumber())) {
		if (!Palive[PlayerNumber()])
		{
			delay(50);
			continue;
		}
		int Ang = (GetActorAngle(acTid) >> 8)-64;
		PlayerAngle[PlayerNumber()] = Ang;
		ACS_ExecuteAlways(PODERES, 0, acTid, nJump);
		int tmpLife = getActorProperty(acTid, APROP_Health);
		switch (gameMode)
		{
			case NIGHTMAREMODE:
				if (tmpLife > 25)
				{
					tmpLife -= 5;
					if (tmpLife < 25)
						tmpLife = 25;
					setActorProperty(acTid, APROP_Health, tmpLife);
				}
			break;
			case PAREJASMODE:
				if (PlayerPAR[PlayerNumber()] != -1)
				{
					HudMessage(s:"Tu pareja es ", n: PlayerPAR[PlayerNumber()]+1; HUDMSG_PLAIN, PAREJAMSG, CR_GOLD, 1.9, 0.75, 0);
				} else {
					HudMessage(s:"No tenes pareja"; HUDMSG_PLAIN, PAREJAMSG, CR_GOLD, 1.9, 0.75, 0);				
				}
			break;
			case THEKINGMODE:
					HudMessage(s:"El rey es ", n: Theking-999; HUDMSG_PLAIN, KINGMSG, CR_GOLD, 1.9, 0.75, 0);
			break;
			case NOHEALTHMODE:
				if (PlayersHT[PlayerNumber()] == -1)
					PlayersHT[PlayerNumber()] = tmpLife;
					
				if (tmpLife > PlayersHT[PlayerNumber()])
					setActorProperty(acTid, APROP_Health, PlayersHT[PlayerNumber()]);
				else
					PlayersHT[PlayerNumber()] = tmpLife;
			break;
		}
		int pk=0;
		int tDelay = 0;
		do
		{
			tDelay++;
			//En caso de que el jugador tenga poderes y no los haya usado todavia, se verfica si los va a usar
			PactPos[PlayerNumber()][0] = GetActorX(acTid);
			PactPos[PlayerNumber()][1] = GetActorY(acTid);
			PactPos[PlayerNumber()][2] = GetActorZ(acTid);
			if (PlayerPWS[PlayerNumber()][TELEPORTPOWER])
			{
				PressButtons = GetPlayerInput(-1, INPUT_BUTTONS);
				if (PressButtons & BT_ALTATTACK)
					ACS_ExecuteAlways(TELPOINT, 0);	
			}
			delay(1);
		}while (tDelay <  20);
		//Obteniendo la posicion de cada jugador en tiempo real
	}

}
script SETPOWERS(int Tid)
{
	int poder = random(0, 10);
	//int poder = RESURRECTPOWER;
	int nPlayer = Tid-1000;
	str sPoder = "";
	switch (poder)
	{
		case SPEEDPOWER:
			HudMessage(s:"Tenes Speed Power"; HUDMSG_TYPEON | HUDMSG_LOG, 0, CR_TAN, 1.5, 0.8, 5.0, 0.05, 2.0);
			sPoder = "Sanic Speed";
			PlayerPWS[nPlayer][SPEEDPOWER] = true;
		break;
		case JUMPPOWER:
			HudMessage(s:"Tenes Jump Power"; HUDMSG_TYPEON | HUDMSG_LOG, 0, CR_TAN, 1.5, 0.8, 5.0, 0.05, 2.0);
			sPoder = "Super Salto";
			PlayerPWS[nPlayer][JUMPPOWER] = true;
		break;
		case REGENERATEPOWER:
			HudMessage(s:"Tenes poder de regeneracion"; HUDMSG_TYPEON | HUDMSG_LOG, 0, CR_TAN, 1.5, 0.8, 5.0, 0.05, 2.0);
			sPoder = "Regeneracion de vida";
			PlayerPWS[nPlayer][REGENERATEPOWER] = true;
		break;
		case KAMIKAZEPOWER:
			HudMessage(s:"Tenes poder Kamikaze, se un heroe"; HUDMSG_TYPEON | HUDMSG_LOG, 0, CR_TAN, 1.5, 0.8, 5.0, 0.05, 2.0);
			sPoder = "Kamikaze";
			PlayerPWS[nPlayer][KAMIKAZEPOWER] = true;
		break;
		case REGENERATEAMMO:
			HudMessage(s:"Tenes poder de regeneracion de ammo"; HUDMSG_TYPEON | HUDMSG_LOG, 0, CR_TAN, 1.5, 0.8, 5.0, 0.05, 2.0);
			sPoder = "Regeneracion de municion";
			PlayerPWS[nPlayer][REGENERATEAMMO] = true;
		break;
		case SUPERWEAPONS:
			HudMessage(s:"Tenes poder de triple disparo"; HUDMSG_TYPEON | HUDMSG_LOG, 0, CR_TAN, 1.5, 0.8, 5.0, 0.05, 2.0);
			sPoder = "Triple disparo";
			PlayerPWS[nPlayer][SUPERWEAPONS] = true;
		break;
		case TELEPORTPOWER:
			HudMessage(s:"Tenes poder de teletransportacion\n(Utiliza el disparo secundario)"; HUDMSG_TYPEON | HUDMSG_LOG, 0, CR_TAN, 1.5, 0.8, 5.0, 0.05, 2.0);
			PlayerPWS[nPlayer][TELEPORTPOWER] = true;
			sPoder = "Teletransportacion";
			//HudMessage(s:"Este poder todavia no esta disponible"; HUDMSG_TYPEON | HUDMSG_LOG, 0, CR_TAN, 1.5, 0.8, 5.0, 0.05, 2.0);
		break;
		case DAMAGEPOWER:
			HudMessage(s:"Tenes poder de Damage"; HUDMSG_TYPEON | HUDMSG_LOG, 0, CR_TAN, 1.5, 0.8, 5.0, 0.05, 2.0);
			sPoder = "Double Damage";
			PlayerPWS[nPlayer][DAMAGEPOWER] = true;
		break;
		case RESURRECTPOWER:
			HudMessage(s:"Tenes poder de resureccion"; HUDMSG_TYPEON | HUDMSG_LOG, 0, CR_TAN, 1.5, 0.8, 5.0, 0.05, 2.0);
			sPoder = "Resurreccion";
			//HudMessage(s:"Este poder todavia no esta disponible"; HUDMSG_TYPEON | HUDMSG_LOG, 0, CR_TAN, 1.5, 0.8, 5.0, 0.05, 2.0);
			PlayerPWS[nPlayer][RESURRECTPOWER] = true;
		break;
		case SUPERMARINE:
			HudMessage(s:"Sos un marine supremo"; HUDMSG_TYPEON | HUDMSG_LOG, 0, CR_TAN, 1.5, 0.8, 5.0, 0.05, 2.0);
			sPoder = "Marine supremo";
			PlayerPWS[nPlayer][SUPERMARINE] = true;
		break;
		case INVESCUDO:
			HudMessage(s:"Tenes un campo protector"; HUDMSG_TYPEON | HUDMSG_LOG, 0, CR_TAN, 1.5, 0.8, 5.0, 0.05, 2.0);
			sPoder = "Campo protector invisible";
			PlayerPWS[nPlayer][INVESCUDO] = true;
			GiveInventory("ArmorBonus", 200);
		break;
	}
	HudMessage(s:"Poder: ", s: sPoder; HUDMSG_PLAIN, PODERMSG, CR_GRAY, 1.9, 0.83, 0);

}
script QUITARRUNAS(int Tid)
{
	if (!CheckInventory("StrengthRune"))
		TakeInventory("StrengthRune", 1);
	if (!CheckInventory("SpreadRune"))
		TakeInventory("SpreadRune", 1);
}
script KAMIKAZEPW (int Tid) NET
{
	delay(5);
	int nPlayer = Tid - 1000;
	if (!PlayerPWS[nPlayer][KAMIKAZEPOWER])
		terminate;
	if (!Palive[nPlayer])
	{
		int j, i;
		int Ang, IncrA;
		acTidKz[PlayerNumber()]++;
		int tmpTid = 500 + acTidKz[PlayerNumber()];
		Thing_ChangeTID(ActivatorTID(), tmpTid);
		for (i=0; i < KMZTIMES; i++)
		{
			if (Palive[nPlayer])
				continue;
		Ang = PlayerAngle[nPlayer];
		IncrA = 225/KMMISSILES;
		for (j=0; j < KMMISSILES; j++)
		{
			if (Palive[nPlayer])
				continue;
			SpawnProjectile(tmpTid, "Rocket", Ang, 120, random(-10, 20), 0, 0);
			Ang += IncrA;
			delay(4);
		}
			delay(5);
		}
		PlayerPWS[nPlayer][KAMIKAZEPOWER] = false;
	} else {
		ACS_ExecuteAlways(KAMIKAZEPW, 0, Tid);
	}
}
script PODERES (int Tid, int jumpTmp)
{
	//Regeneracion de vida
	int nPlayer = Tid - 1000;
	int Laif = getActorProperty(Tid, APROP_Health);
	for (int i=0; i < 10; i++)
	{
		if (!PlayerPWS[nPlayer][i])
			continue;
		switch (i)
		{
			case SPEEDPOWER:
				setActorProperty(Tid, APROP_Speed, 1.5);
			break;
			case JUMPPOWER:
				setActorProperty(ActivatorTID(), APROP_Jumpz, jumpTmp*2);		
			break;
			case REGENERATEPOWER:
				if (gameMode == NOHEALTHMODE)
					break;
				if (Laif < 100)
				{
					Laif += 3;
					if (Laif > 100)
						Laif = 100;
					setActorProperty(Tid, APROP_Health, Laif);
				}
			
			break;
			case KAMIKAZEPOWER:
				//no usar todavia
			break;
			case REGENERATEAMMO:
				if ((CheckInventory("Pistol") || CheckInventory("Chaingun")) && CheckInventory("Clip") < C_SLOT2)
				{
					GiveInventory("Clip", 3);
				}
				if ((CheckInventory("Shotgun") || CheckInventory("SuperShotgun")) && CheckInventory("Shell") < C_SLOT3)
				{
					GiveInventory("Shell", 1);
				}
				if (CheckInventory("RocketLauncher") && CheckInventory("RocketAmmo") < C_SLOT5)
				{
					GiveInventory("RocketAmmo", 1);
				}
				if ((CheckInventory("BFG9000") || CheckInventory("PlasmaRifle")) && CheckInventory("Cell") < C_SLOT6)
				{
					GiveInventory("Cell", 5);
				}
			break;
			case SUPERWEAPONS:
				if (!CheckInventory("SpreadRune"))
					GiveInventory("SpreadRune", 1);
			break;
			case TELEPORTPOWER:
				if (tTime[nPlayer] > 0)
					tTime[nPlayer]--;
			break;
			case DAMAGEPOWER:
				if (!CheckInventory("StrengthRune"))
					GiveInventory("StrengthRune", 1);
			break;
			case RESURRECTPOWER:
				if ((Laif < 20) && PlayerPWS[nPlayer][RESURRECTPOWER])
				{
					SetActorPosition(Tid, PiniPos[nPlayer][0], PiniPos[nPlayer][1], PiniPos[nPlayer][2], 0);
					PlayersHT[nPlayer] = 100;
					setActorProperty(Tid, APROP_Health, 100);
					PlayerPWS[nPlayer][RESURRECTPOWER] = false;
					HudMessage(s:"Te salvo tu vida extra, ahora ya no tenes mas"; HUDMSG_TYPEON | HUDMSG_LOG, 0, CR_TAN, 1.5, 0.8, 5.0, 0.05, 2.0);
				}
			break;
			case 9:
				PlayerPWS[nPlayer][SPEEDPOWER] = true;
				PlayerPWS[nPlayer][REGENERATEPOWER] = true;
				PlayerPWS[nPlayer][DAMAGEPOWER] = true;				
			break;			
		}
	}
}
script 667 DEATH
{
	Palive[PlayerNumber()] = FALSE;
	PlayersHT[PlayerNumber()] = -1;
	int Alives=0;
	for (int i=0; i < MAXPLAYERS; i++)
	{
		if (Palive[i])
			Alives++;
	}
	if (Alives <= 0)
	{
		allDeath = true;
		delay(20);
		//El ultimo jugador en morir es informado
	}
}
script 668 RESPAWN
{
	int j;
	Thing_ChangeTID(ActivatorTID(), 1000 + PlayerNumber());
	int acTid = 1000 + PlayerNumber();
	if (!CheckInventory("Pistol"))
		GiveInventory("Pistol", 1);
	if (CheckInventory("Clip") < 20)
		GiveInventory("Clip", 20);

	tTime[PlayerNumber()] = 0;
	PTeleport[PlayerNumber()][0] = -1;
	PTeleport[PlayerNumber()][1] = -1;
	PTeleport[PlayerNumber()][2] = -1;
	PlayerPAR[PlayerNumber()] = -1;
	PlayersHT[PlayerNumber()] = -1;
	for (j=0; j < 11; j++)
			PlayerPWS[PlayerNumber()][j] = false;

	if (gameMode != VANILLAMODE)
	{
		ACS_ExecuteAlways(QUITARRUNAS, 0, acTid);
		ACS_ExecuteAlways(SETPOWERS, 0, acTid);
	}
	delay(15);
	switch (gameMode)
	{
		case PAREJASMODE:
			for (j=0; j < MAXPLAYERS; j++)
			{
				if (!PlayerInGame(j) || !Palive[j])
					continue;
				if ((PlayerPAR[j] != -1) || j == PlayerNumber())
					continue;
				PlayerPAR[PlayerNumber()] = j;
				PlayerPAR[j] = PlayerNumber();
				break;
			}
		break;
	}
	Palive[PlayerNumber()] = TRUE;
	if (gameMode != VANILLAMODE)
		ACS_ExecuteAlways(KAMIKAZEPW, 0, acTid);
}

script 669 (int gone) DISCONNECT
{
	switch (gameMode)
	{
		case THEKINGMODE:
			int KingPlayer = Theking - 1000;
			if (KingPlayer == gone)
			{
				int Alives=0;
				for (int i=0; i < MAXPLAYERS; i++)
				{
					if (Palive[i])
						Alives++;
				}
				if ((PlayerCount() > 0) && (Alives > 0))
				{
					int tmpKing;
					do
					{
						tmpKing = random(0, MAXPLAYERS);
					}while(!PlayerInGame(tmpKing) && !Palive[tmpKing]);
					Theking = tmpKing+1000;
					printbold(s:"El nuevo Rey es: ", n:tmpKing+1);
				}
			}
		break;
	}
	Palive[gone] = FALSE;
	PlayersHT[gone] = -1;
}


script TELPOINT (void)
{
	int nPlayer = ActivatorTID() - 1000;
	if (nPlayer < 0 || nPlayer >= 16)
		terminate;
	if ((tTime[nPlayer] > 0) || !PlayerPWS[nPlayer][TELEPORTPOWER])
		terminate;
	if ((PTeleport[nPlayer][0] == -1) && (PTeleport[nPlayer][1] == -1) && (PTeleport[nPlayer][2] == -1))
	{
		int x = GetActorX(ActivatorTID());
		int y = GetActorY(ActivatorTID());
		int z = GetActorZ(ActivatorTID());
		PTeleport[nPlayer][0] = x; 
		PTeleport[nPlayer][1] = y; 
		PTeleport[nPlayer][2] = z; 
		HudMessage(s:"Seleccionaste un punto de teleport"; HUDMSG_TYPEON | HUDMSG_LOG, 0, CR_TAN, 1.5, 0.8, 5.0, 0.05, 2.0);
	} else {
		SetActorPosition(ActivatorTID(), PTeleport[nPlayer][0], PTeleport[nPlayer][1], PTeleport[nPlayer][2], 0);
		PTeleport[nPlayer][0] = -1; 
		PTeleport[nPlayer][1] = -1; 
		PTeleport[nPlayer][2] = -1; 
		HudMessage(s:"Te has teletransportado"; HUDMSG_TYPEON | HUDMSG_LOG, 0, CR_TAN, 1.5, 0.8, 5.0, 0.05, 2.0);
	
	}
	tTime[nPlayer] = 10;
}