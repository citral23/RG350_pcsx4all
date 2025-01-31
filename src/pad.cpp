/***************************************************************************
*   Copyright (C) 2010 PCSX4ALL Team                                      *
*   Copyright (C) 2010 Franxis                                            *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA 02111-1307 USA.           *
***************************************************************************/

#include "psxcommon.h"
#include "psxmem.h"
#include "r3000a.h"
#include "port.h"

/*
 * code ported from pcsx4all C version.
 *   original by Gameblabla, JamesOFarrel and code based on dfinput for Dual analog/shock.
*/

uint8_t CurPad = 0, CurCmd = 0;

typedef struct tagGlobalData
{
    uint8_t CurByte1;
    uint8_t CmdLen1;
    uint8_t CurByte2;
    uint8_t CmdLen2;
} GLOBALDATA;

static GLOBALDATA g={0,0,0,0};

enum {
	CMD_READ_DATA_AND_VIBRATE = 0x42,
	CMD_CONFIG_MODE = 0x43,
	CMD_SET_MODE_AND_LOCK = 0x44,
	CMD_QUERY_MODEL_AND_MODE = 0x45,
	CMD_QUERY_ACT = 0x46, // ??
	CMD_QUERY_COMB = 0x47, // ??
	CMD_QUERY_MODE = 0x4C, // QUERY_MODE ??
	CMD_VIBRATION_TOGGLE = 0x4D,
};

unsigned char PAD1_startPoll()
{
	g.CurByte1 = 0;
	return 0xFF;
}

unsigned char PAD2_startPoll()
{
	g.CurByte2 = 0;
	return 0xFF;
}

static const uint8_t unk46[8]    = {0xFF, 0x5A, 0x00, 0x00, 0x01, 0x02, 0x00, 0x0A};
static const uint8_t unk47[8]    = {0xFF, 0x5A, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00};
static const uint8_t unk4c[8]    = {0xFF, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t unk4d[8]    = {0xFF, 0x5A, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static const uint8_t stdmode[8]  = {0xFF, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t stdcfg[8]   = {0xFF, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t stdmodel[8] = {
		0xFF,
		0x5A,
		0x01, // 03 - dualshock2, 01 - dualshock
		0x02, // number of modes
		0x01, // current mode: 01 - analog, 00 - digital
		0x02,
		0x01,
		0x00
};

unsigned char PAD1_poll(unsigned char value) {
	static uint8_t buf[8] = {0xFF, 0x5A, 0xFF, 0xFF, 0x80, 0x80, 0x80, 0x80};
	if (g.CurByte1 == 0) {
		uint16_t n;

		n = pad_read(0);

		// Don't enable Analog/Vibration for a Digital or DualAnalog controller
		CurCmd = value;
		g.CurByte1++;
		if (player_controller[0].pad_controllertype != PSE_PAD_TYPE_ANALOGPAD) {
			CurCmd = CMD_READ_DATA_AND_VIBRATE;
		}
		g.CmdLen1 = 8;

		switch (CurCmd) {
		case CMD_SET_MODE_AND_LOCK:
			memcpy(buf, stdmode, sizeof(buf));
			return 0xF3;
		case CMD_QUERY_MODEL_AND_MODE:
			memcpy(buf, stdmodel, sizeof(buf));
			buf[4] = player_controller[0].pad_mode;
			return 0xF3;
		case CMD_QUERY_ACT:
			memcpy(buf, unk46, sizeof(buf));
			return 0xF3;
		case CMD_QUERY_COMB:
			memcpy(buf, unk47, sizeof(buf));
			return 0xF3;
		case CMD_QUERY_MODE:
			memcpy(buf, unk4c, sizeof(buf));
			return 0xF3;
		case CMD_VIBRATION_TOGGLE:
			memcpy(buf, unk4d, sizeof(buf));
			return 0xF3;
		case CMD_CONFIG_MODE:
			if (player_controller[0].configmode) {
				memcpy(buf, stdcfg, sizeof(buf));
				return 0xF3;
			}
			// else FALLTHROUGH
		case CMD_READ_DATA_AND_VIBRATE:
		default:

			buf[2] = n & 0xFF;
			buf[3] = n >> 8;

			/* Make sure that in Digital mode that lengh is 4, not 8 for the 2 analogs.
			 * This must be done here and not before otherwise, it might not enter the switch loop
			 * and Dualshock features won't work.
			 * */

			if (player_controller[0].pad_mode == 1) {
				buf[4] = player_controller[0].joy_right_ax0;
				buf[5] = player_controller[0].joy_right_ax1;
				buf[6] = player_controller[0].joy_left_ax0;
				buf[7] = player_controller[0].joy_left_ax1;
			}
			else
			{
				g.CmdLen1 = 4;
			}

			return player_controller[0].id;
		}
	}

	if (g.CurByte1 >= g.CmdLen1)
		return 0xFF;

	if (g.CurByte1 == 2) {
		switch (CurCmd) {
		case CMD_CONFIG_MODE: 
			player_controller[0].configmode = value;
			break;

		case CMD_SET_MODE_AND_LOCK: 
			player_controller[0].pad_mode = value;
			player_controller[0].id = value ? 0x73 : 0x41;
			break;

		case CMD_QUERY_ACT:
			switch (value) {
			case 0: // default
				buf[5] = 0x02;
				buf[6] = 0x00;
				buf[7] = 0x0A;
				break;

			case 1: // Param std conf change
				buf[5] = 0x01;
				buf[6] = 0x01;
				buf[7] = 0x14;
				break;
			}
			break;
		case CMD_QUERY_MODE:
			switch (value) {
			case 0: // mode 0 - digital mode
				buf[5] = PSE_PAD_TYPE_STANDARD;
				break;

			case 1: // mode 1 - analog mode
				buf[5] = PSE_PAD_TYPE_ANALOGPAD;
				break;
			}
			break;
		}
	}

	if (player_controller[0].pad_controllertype == PSE_PAD_TYPE_ANALOGPAD) 
	{
		switch (CurCmd) {
		case CMD_READ_DATA_AND_VIBRATE:
			{
				int changed = 0;

				if (g.CurByte1 == player_controller[0].Vib[0] &&
					 player_controller[0].VibF[0] != value) {
					player_controller[0].VibF[0] = value;
					changed = 1;
				}

				if (g.CurByte1 == player_controller[0].Vib[1] &&
					 player_controller[0].VibF[1] != value) {
					player_controller[0].VibF[1] = value;
					changed = 1;
				}

#ifdef RUMBLE
				if (changed) {
					trigger_rumble(player_controller[0].VibF[0],
							player_controller[0].VibF[1]);
				}
#endif
			}
			break;
		case CMD_VIBRATION_TOGGLE:
			for (uint8_t i = 0; i < 2; i++) {
				if (player_controller[0].Vib[i] == g.CurByte1)
					buf[g.CurByte1] = 0;
			}
			if (value < 2) {
				player_controller[0].Vib[value] = g.CurByte1;
				if ((player_controller[0].id & 0x0f) < (g.CurByte1 - 1) / 2) {
					player_controller[0].id = (player_controller[0].id & 0xf0) + (g.CurByte1 - 1) / 2;
				}
			}
			break;
		}
	}

	return buf[g.CurByte1++];
}

unsigned char PAD2_poll(unsigned char value) {
	static uint8_t buf[8] = {0xFF, 0x5A, 0xFF, 0xFF, 0x80, 0x80, 0x80, 0x80};

	if (g.CurByte2 == 0) {
		uint16_t n;
		g.CurByte2++;

		n = pad_read(1);

		buf[2] = n & 0xFF;
		buf[3] = n >> 8;

		g.CmdLen2 = 4;

		return 0x41;
	}

	if (g.CurByte2 >= g.CmdLen2) return 0xFF;
	return buf[g.CurByte2++];
}
