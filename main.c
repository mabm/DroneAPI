/*
** main.c for APIDrone in /home/jobertomeu/Documents/APIDrone
** 
** Made by Joris Bertomeu
** Login   <jobertomeu@epitech.net>
** 
** Started on  Fri Jun 20 15:45:33 2014 Joris Bertomeu
** Last update Fri Jun 20 21:40:08 2014 Joris Bertomeu
*/

#include "drone_api.h"

int		iSeq=1;
int		isDroneStarted = 0;
float		fSpeed = (float) 0.2;
int		iStartBit = 0;
int		sockATCMD;
t_libclient	*slib;
int		flag = 0;
t_drone         g_glob;

void	print_error(char *msg)
{
  perror("API_ERROR ");
  exit (-1);
}

void	 SendATCmd(char *szCommand, int iLen)
{
  write(slib->to_server_socket, szCommand, strlen(szCommand));
  usleep(1);
}

void		SendPCMD_AT(int enable, float roll, float pitch, float gaz, float yaw)
{
  int		fiPitch;
  int		fiRoll;
  int		fiGaz;
  int		fiYaw;
  char		szSendBuffer[4096];

  fiPitch = pitch;
  fiRoll = roll;
  fiGaz = gaz;
  fiYaw = yaw;
  memset(szSendBuffer, 0, 4096);
  sprintf(szSendBuffer, "AT*PCMD=%d,%d,%d,%d,%d,%d\r", ++iSeq, enable,
	    fiRoll, fiPitch, fiGaz, fiYaw);
  SendATCmd(szSendBuffer, strlen(szSendBuffer));
}

void	*alive()
{
  float pitch = 0;
  float roll = 0;
  float gaz = 0;
  float yaw = 0;
  char	szSendBuffer[4096];

  while (1)
    {
      if (flag == 0)
	{
	  SendPCMD_AT(0, roll, pitch, gaz, yaw);
	  memset(szSendBuffer, 0, 4096);
	  sprintf(szSendBuffer, "AT*REF=%d,%d\r",++iSeq,iStartBit);
	  SendATCmd(szSendBuffer, strlen(szSendBuffer));
	  memset(szSendBuffer, 0, 4096);
	  sprintf(szSendBuffer, "AT*COMWDG=%d\r",++iSeq);
	  SendATCmd(szSendBuffer, strlen(szSendBuffer));
	  usleep(250000);
	}
    }
}

void		MessageHandler(char *str)
{
  float		pitch = 0;
  float		roll = 0;
  float		gaz = 0;
  float		yaw = 0;
  pthread_t	thread;
  char		szSendBuffer[4096];

  if (str[0] == KEY_LEFT_TURN) /* Gauche */
    {
      roll = -fSpeed;
      SendPCMD_AT(1, roll, pitch, gaz, yaw);
    }
  else if (str[0] == KEY_FORWARD) /* Avancer */
    {
      pitch = -fSpeed;
      SendPCMD_AT(1, roll, pitch, gaz, yaw);
    }
  else if (str[0] == KEY_RIGHT_TURN) /* Droite */
    {
      roll = fSpeed;
      SendPCMD_AT(1, roll, pitch, gaz, yaw);
    }
  else if (str[0] == KEY_BACKWARD) /* reculer */
    {
      pitch = fSpeed;
      SendPCMD_AT(1, roll, pitch, gaz, yaw);
    }
  else if (str[0] == KEY_TAKEON)
    {
      if (isDroneStarted == 0)
	{
	  flag = 0;
	  SendATCmd("AT*CONFIG=\"general:navdata_demo\",\"TRUE\"\r",
		    strlen("AT*CONFIG=\"general:navdata_demo\",\"TRUE\"\r"));
	  SendATCmd("AT*CTRL=5,0\r", strlen("AT*CTRL=5,0\r"));
	  iSeq = 1;
	  sprintf(szSendBuffer,
		    "AT*CONFIG=%d,\"CONTROL:altitude_max\",\"3000\"\r", iSeq);
	  SendATCmd(szSendBuffer, strlen(szSendBuffer));
	  memset(szSendBuffer, 0, 4096);
	  sprintf(szSendBuffer, "AT*FTRIM\r");
	  SendATCmd(szSendBuffer, strlen(szSendBuffer));
	  memset(szSendBuffer, 0, 4096);
	  sprintf(szSendBuffer, "AT*FTRIM=%d\r", ++iSeq);
	  SendATCmd(szSendBuffer, strlen(szSendBuffer));
	  usleep(50);
	  iStartBit = iStartBit | (1 << 9);
	  memset(szSendBuffer, 0, 4096);
	  sprintf(szSendBuffer, "AT*REF=%d,%d\r", ++iSeq, iStartBit);
	  SendATCmd(szSendBuffer, strlen(szSendBuffer));
	  memset(szSendBuffer, 0, 4096);
	  sprintf(szSendBuffer, "AT*REF=%d,%d\r", ++iSeq, iStartBit);
	  SendATCmd(szSendBuffer, strlen(szSendBuffer));
	  isDroneStarted = 1;
	  pthread_create(&thread, NULL, &alive, NULL);
	  /* THREAD Toute les 250ms */
	}
      else
	{
	  flag = 1;
	  /* Kill thread */
	  iStartBit = iStartBit &~ (1 << 9);
	  memset(szSendBuffer, 0, 4096);
	  sprintf(szSendBuffer, "AT*REF=%d,%d\r", ++iSeq, iStartBit);
	  SendATCmd(szSendBuffer, strlen(szSendBuffer));
	  memset(szSendBuffer, 0, 4096);
	  sprintf(szSendBuffer, "AT*REF=%d,%d\r", ++iSeq, iStartBit);
	  SendATCmd(szSendBuffer, strlen(szSendBuffer));
	  isDroneStarted = 0;
	}
    }
  else if (str[0] == KEY_ROT_LEFT) /* Rot gauche */
    {
      yaw = -fSpeed;
      SendPCMD_AT(1, roll, pitch, gaz, yaw);
    }
  else if (str[0] == KEY_ROT_RIGHT) /* rot droite */
    {
      yaw = fSpeed;
      SendPCMD_AT(1, roll, pitch, gaz, yaw);
    }
  else if (str[0] == KEY_GO_DOWN) /* descendre */
    {
      gaz = -fSpeed;
      SendPCMD_AT(1, roll, pitch, gaz, yaw);
    }
  else if (str[0] == KEY_GO_UP) /* monter */
    {
      gaz = fSpeed;
      SendPCMD_AT(1, roll, pitch, gaz, yaw);
    }
  else if (str[0] == KEY_EMERGENCY)
    {
      if (iStartBit & (1 << 8))
	{
	  iStartBit = iStartBit | (1 << 8);
	  memset(szSendBuffer, 0, 4096);
	  sprintf(szSendBuffer, "AT*REF=%d,%d\r", ++iSeq, iStartBit);
	  SendATCmd(szSendBuffer, strlen(szSendBuffer));
	  iStartBit = iStartBit &~ (1 << 8);
	  memset(szSendBuffer, 0, 4096);
	  sprintf(szSendBuffer, "AT*REF=%d,%d\r", ++iSeq, iStartBit);
	  SendATCmd(szSendBuffer, strlen(szSendBuffer));
	}
      else
	{
	  iStartBit = iStartBit | (1 << 8);
	  iStartBit = iStartBit &~ (1 << 9);
	  memset(szSendBuffer, 0, 4096);
	  sprintf(szSendBuffer, "AT*REF=%d,%d\r", ++iSeq, iStartBit);
	  SendATCmd(szSendBuffer, strlen(szSendBuffer));
	  flag = 1;
	  /* Kill thread */
	}
    }
  else
    {
      SendPCMD_AT(0, roll, pitch, gaz, yaw);
      memset(szSendBuffer, 0, 4096);
      sprintf(szSendBuffer, "AT*COMWDG=%d\r", ++iSeq);
      SendATCmd(szSendBuffer, strlen(szSendBuffer));
    }
}

void	aff_directives()
{
  printf("=============== ARDRONE2.0 API By\
 Joris Bertomeu ===============\n\n");
  printf("k : Décoller / Aterrir\n");
  /* printf("SPEED : \n"); */
  printf("z : Avancer - s : Reculer - q :\
 Tourner à gauche - d : Tourner à droite\n");
  printf("8 : Monter - 5 : Descendre - 4 :\
 Rotation gauche - 6 : Rotation droite\n");
  printf("r : Arrêt d'urgence\n\n");
  printf("================================================================\n");
}

void init_struct()
{
  drone.iSeq = 1;
  drone.isDroneStarted = 0;
  drone.flag = 0;
}

int		main(int ac, char **argv)
{
  char		buff[128];
  pthread_t	thread;

  if (ac == 3)
    {
      slib = malloc(sizeof(*slib));
      init_lib(drone.slib, argv[1], atoi(argv[2]));
      init_struct();
      slib->flag = 1;
      aff_directives();
      while (1)
	{
	  memset(buff, 0, 128);
	  write(1, "> ", 3);
	  read(0, &buff, 128);
	  MessageHandler(buff);
	  printf("> Succefully sent\n");
	}
    }
  else
    printf("Usage : %s <IP_addr> <Port>\n", argv[0]);
  return (0);
}
