//---------------------------------------------------------------//
//  packet.h  - ���� � ������������� ��������� �������           //
//                                                               //
//  ������������ ��� �������� �������� ������� ������ �������    //
//  ����� ������������ ������������ ���������� � ������������    //
//  ��������-��������������������� ������ ���-1�.                //
//  ����� �� ������� �������������� ����� RS-232.                //
//                                                               //
//---------------------------------------------------------------//
#ifndef PACKET_H_
#define PACKET_H_

vu32 packet_stat;   //����� ��������� ��������� ���������
#define F_PACKET_FF			   0x0001
#define F_REPLACE_PACKET	  	0x0002
#define F_TX_ACK			      0x0004
#define F_START_BYTE		    	0x0008
#define F_PACKET2_FF		    	0x0010
#define F_PACKET2_COMPLETE		0x0020
#define F_REPEAT_LAST_PACKET  0x0080
#define F_REPLACE_PACKET2 		0x0100
#define F_START2_BYTE			0x0200
#define F_DATA_READY				0x0400
#define F_STA_REQUEST         0x0800
#define F_STA_REQUEST_IN_PROGRESS 0x1000
#define F_DATA_IPD_RECEIVED 		 0x2000

#define SET_PACKET_FF         packet_stat |=  F_PACKET_FF /* ���������� ���� */
#define RESET_PACKET_FF       packet_stat &= ~F_PACKET_FF /* �������� ���� */
#define PACKET_FF             (packet_stat & F_PACKET_FF) /* ��������� ���� */

#define SET_REPLACE_PACKET    packet_stat |=  F_REPLACE_PACKET /* ���������� ���� */
#define RESET_REPLACE_PACKET  packet_stat &= ~F_REPLACE_PACKET /* �������� ���� */
#define REPLACE_PACKET        (packet_stat & F_REPLACE_PACKET) /* ��������� ���� */

#define SET_TX_ACK            packet_stat |=  F_TX_ACK /* ���������� ���� */
#define RESET_TX_ACK          packet_stat &= ~F_TX_ACK /* �������� ���� */
#define TX_ACK                (packet_stat & F_TX_ACK) /* ��������� ���� */

#define SET_START_BYTE        packet_stat |=  F_START_BYTE /* ���������� ���� */
#define RESET_START_BYTE      packet_stat &= ~F_START_BYTE /* �������� ���� */
#define START_BYTE            (packet_stat & F_START_BYTE) /* ��������� ���� */

#define SET_PACKET2_FF        packet_stat |=  F_PACKET2_FF /* ���������� ���� */
#define RESET_PACKET2_FF      packet_stat &= ~F_PACKET2_FF /* �������� ���� */
#define PACKET2_FF            (packet_stat & F_PACKET2_FF) /* ��������� ���� */

#define SET_PACKET2_COMPLETE        packet_stat |=  F_PACKET2_COMPLETE /* ���������� ���� */
#define RESET_PACKET2_COMPLETE      packet_stat &= ~F_PACKET2_COMPLETE /* �������� ���� */
#define PACKET2_COMPLETE            (packet_stat & F_PACKET2_COMPLETE) /* ��������� ���� */

#define SET_REPEAT_LAST_PACKET   packet_stat |= F_REPEAT_LAST_PACKET    /* ���������� ���� */
#define RESET_REPEAT_LAST_PACKET packet_stat &= ~F_REPEAT_LAST_PACKET /* �������� ���� */
#define REPEAT_LAST_PACKET       (packet_stat & F_REPEAT_LAST_PACKET)       /* ��������� ���� */

#define SET_REPLACE_PACKET2 	packet_stat |= F_REPLACE_PACKET2        /* ���������� ���� */
#define RESET_REPLACE_PACKET2 packet_stat &= ~F_REPLACE_PACKET2       /* �������� ���� */
#define REPLACE_PACKET2 		(packet_stat & F_REPLACE_PACKET2)       /* ��������� ���� */

#define SET_START2_BYTE 		packet_stat |= F_START2_BYTE    	/* ���������� ���� */
#define RESET_START2_BYTE 		packet_stat &= ~F_START2_BYTE 		/* �������� ���� */
#define START2_BYTE 				(packet_stat & F_START2_BYTE)       /* ��������� ���� */

#define SET_DATA_READY 		packet_stat |= F_DATA_READY    		/* ���������� ���� */
#define RESET_DATA_READY 	packet_stat &= ~F_DATA_READY 			/* �������� ���� */
#define DATA_READY 			(packet_stat & F_DATA_READY)       	/* ��������� ���� */

#define SET_STA_REQUEST		packet_stat |= F_STA_REQUEST    		/* ���������� ���� */
#define RESET_STA_REQUEST 	packet_stat &= ~F_STA_REQUEST 			/* �������� ���� */
#define STA_REQUEST 			(packet_stat & F_STA_REQUEST)       	/* ��������� ���� */

#define SET_STA_REQUEST_IN_PORGRESS		packet_stat |= F_STA_REQUEST_IN_PROGRESS    		/* ���������� ���� */
#define RESET_STA_REQUEST_IN_PORGRESS 	packet_stat &= ~F_STA_REQUEST_IN_PROGRESS			/* �������� ���� */
#define STA_REQUEST_IN_PORGRESS 			(packet_stat & F_STA_REQUEST_IN_PROGRESS)       	/* ��������� ���� */

#define SET_DATA_IPD_RECEIVED 		packet_stat |= F_DATA_IPD_RECEIVED
#define RESET_DATA_IPD_RECEIVED 		packet_stat &= ~F_DATA_IPD_RECEIVED
#define DATA_IPD_RECEIVED 				(packet_stat & F_DATA_IPD_RECEIVED)


#define MAX_DATA_SIZE     125	//150	//146	//38
#define USART_Boards 	USART2

struct template_packets /* �������� �������� �������� ������� */
{
   u8 marker;
   u8 device_num;  /* ����� ���������� (������� �����) */
   u8 command_num; /* ����� ������� */
   u8 data_size;   /* ������ ����� ������ */

   union /* �������� �������� ������ ������� */
   {
      u8 data[MAX_DATA_SIZE]; /* ���� ������ */
      u16 idata[MAX_DATA_SIZE / 2];

      struct dt_1
      {
         u8 device_id;      /* ������������� ���������� */
         u8 major_firmware; /* ������� ���� ������ �� */
         u8 minor_firmware; /* ������� ���� ������ �� */
         u8 version_day;
         u8 version_month;
         u8 version_year;
         u8 data[MAX_DATA_SIZE - 6]; /* ������ ���� ������ */
      } command_1;                   /* ������� ������ ������ �� ���� */

      struct dt_2
      {
         u8 cur_mode;                /* ����� ������ ���� */
         u8 data[MAX_DATA_SIZE - 1]; /* ������ ���� ������ */
      } command_2;                   /* ������� ������� "������ ������ ����" */

      struct dt_3
      {
         u16 temperature;            /* ����������� � ������ */
         u8 vacuum;                  /* ���������� � ������ */
         u8 status_devices;          /* ������ ��������� */
         u8 status_zone;             /* ������ ������� */
         u8 data[MAX_DATA_SIZE - 5]; /* ������ ���� ������ */
      } command_3;

      struct dt_4
      {
         u16 temp_support;           /* ����������� ����������� � ������ */
         u8 data[MAX_DATA_SIZE - 2]; /* ������ ���� ������ */
      } command_4;                   /* Motor */

      struct dt_5
      {
         u16 temp_support;           /* ����������� ����������� � ������ */
         u8 data[MAX_DATA_SIZE - 2]; /* ������ ���� ������ */
      } command_5;                   /* ������� ������� ����������� ����������� */

      /*------------------------------------------------------------------*/
      struct dt_99
      {
         u8 cur_mode;
         u8 num_error;
         u8 pause_flags;
         u8 data[MAX_DATA_SIZE - 3]; /* ������ ���� ������ */
      } command_99;

      struct dt_117
      {
         u16 pressure[10];
         u16 temperature[10];
         u16 timeHold[10];
         u16 timePress[10];
         u16 flags[10];
         u8 data[MAX_DATA_SIZE - 100];
      } command_117; // send all programms parameters

      struct dt_124
      {
         u32 ser_num;
         u16 temperature;
         u16 time_process;
         u8 vacuum;
         u8 status_device;
         u8 dev_mode;
         u8 dev_id;
         u8 num_error;
         u8 dummy[7];
         u8 data[MAX_DATA_SIZE - 20];
      } command_124;

      struct dt_125
      {
         u8 mode;
         u8 ssid[33];
         u8 pswd[64];
         u8 data[MAX_DATA_SIZE - 98];
      } command_125;
      /*------------------------------------------------------------------*/
   } packet_data;
   /*------------------------------------------------------------------*/
};

enum commands
{
	e_VERSION = 1,
	e_RESET,
	e_STATUS,
	e_VENTING,
	e_FLOW_STATUS,
	e_VALVE1,
	e_VALVE2,
   e_GET_SERIAL_NUMBER,
	e_PRESSURE_ADJUST = 11,
	e_NEXT_ZONE = 13,
	e_TEMPERATURE_ADJUST,
	e_VENTING_OFF,
	e_VENTING_ON,
	e_TSTART_TX,
	e_TSTART_RX,
	e_FACTORS_RX = 20,
	e_FACTORS_TX,
   e_ADC_DATA,
	e_BIAS_FACTOR_UP,
	e_BIAS_FACTOR_DN,
	e_KFC_FACTOR_UP,
	e_KFC_FACTOR_DN,
   e_STATISTICS,
	e_RUN_PROG = 30,
	e_PROG,
	e_CONTINUE_PROG,
	e_CARTRIDGE_INSTALLED,
   e_SEND_TEST_CYLINDER = 42,
   e_SEND_MAINTAIN_TEMPERATURE = 45,
   e_RECEIVE_MAINTAIN_TEMPERATURE,
	e_SYSTEM_RESET = 50,
	e_RESTORE_FIRMWARE,
	e_TOKEN40_RX = 97,
	e_TOKEN40_TX,
	e_GET_ALL_PARAMS = 112,
	e_TRANSMIT_ALL_PARAMS = 117,
	e_SEND_CURRENT_STA,
	e_SEND_AVAILABLE_STA,
	e_TABLET_CONNECTED = 121,
	e_TABLET_DISCONNECTED,
	e_SYS_RESET,
	e_SEND_DEVICE_PARAMS,
	e_GET_WIFI_PARAMS
};

extern volatile struct template_packets packet, packet2rx, packet2tx;

extern vu8 *ptr_byte;
extern vu8 *ptr2_byte;
extern vu8 trans_buff[];
extern vu8 trans_buff2[];
extern vu8 end_trans_buff;
extern vu32 end_trans_buff2;
extern vu8 packet_count;
extern vu32 packet2rx_count;
extern vu32 packet2tx_count;
extern vu32 RX_recount, rx2_timeout;
extern vu8 packet_check;
extern vu8 packet2_check;
extern vu32 rx_wifi_timeout;

extern u8 STA_to_send; // to send available stas

void ReplacePacket(void);
void ReplacePacket2(void);
void FWTransmitPacket2(void);
void TransmitTermopress(u8 NoCom);
void TransmitPacket2(void);

#endif
