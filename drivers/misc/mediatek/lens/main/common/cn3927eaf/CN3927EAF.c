/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

/*
 * CN3927EAF voice coil motor driver
 *
 *
 */

#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>

#include "lens_info.h"

#define AF_DRVNAME "CN3927EAF_DRV"
#define AF_I2C_SLAVE_ADDR 0x18

#define AF_DEBUG
#ifdef AF_DEBUG
#define LOG_INF(format, args...)                                               \
	pr_debug(AF_DRVNAME " [%s] " format, __func__, ##args)
#else
#define LOG_INF(format, args...)
#endif

#ifndef VENDOR_EDIT
/*Cong.Zhou@HQ 20190428 add for 1922753 cn3927e*/
#define VENDOR_EDIT
#endif

static struct i2c_client *g_pstAF_I2Cclient;
static int *g_pAF_Opened;
static spinlock_t *g_pAF_SpinLock;

static unsigned long g_u4AF_INF;
static unsigned long g_u4AF_MACRO = 1023;
static unsigned long g_u4CurrPosition;

#if 0
static int s4AF_ReadReg(unsigned short *a_pu2Result)
{
	int i4RetValue = 0;
	char pBuff[2];

	g_pstAF_I2Cclient->addr = AF_I2C_SLAVE_ADDR;

	g_pstAF_I2Cclient->addr = g_pstAF_I2Cclient->addr >> 1;

	i4RetValue = i2c_master_recv(g_pstAF_I2Cclient, pBuff, 2);

	if (i4RetValue < 0) {
		LOG_INF("I2C read failed!!\n");
		return -1;
	}

	*a_pu2Result = (((u16)pBuff[0]) << 4) + (pBuff[1] >> 4);

	return 0;
}
#endif

static int s4AF_WriteReg(u16 a_u2Data)
{
	int i4RetValue = 0;
    #ifdef VENDOR_EDIT
        /*Cong.Zhou@HQ 20190428 add for 1922753 cn3927e*/
	char puSendCmd[2] = {(char)(a_u2Data >> 4),
			     (char)(((a_u2Data & 0xF) << 4)|0x07)};
    #else
    char puSendCmd[2] = {(char)(a_u2Data >> 4),
	             (char)((a_u2Data & 0xF) << 4)};
    #endif
	g_pstAF_I2Cclient->addr = AF_I2C_SLAVE_ADDR;

	g_pstAF_I2Cclient->addr = g_pstAF_I2Cclient->addr >> 1;

	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 2);

	if (i4RetValue < 0) {
		LOG_INF("I2C send failed!!\n");
		return -1;
	}

	return 0;
}
#ifdef VENDOR_EDIT
/*Cong.Zhou@HQ 20190428 add for 1922753 cn3927e*/
static int s4AF_WriteReg_Directly(u16 a_u2Data)
{
    int i4RetValue = 0;

    char puSendCmd[2] = { (char)(a_u2Data >> 8), (char)(a_u2Data & 0xFF) };
    g_pstAF_I2Cclient->addr = AF_I2C_SLAVE_ADDR;
    g_pstAF_I2Cclient->addr = g_pstAF_I2Cclient->addr >> 1;
    i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 2);
    if (i4RetValue < 0) {
        LOG_INF("I2C send failed!!\n");
        return -1;
    }
    return 0;
}
static int s4AF_esc_mode(void)
{
    s4AF_WriteReg_Directly(0x8000);
    s4AF_WriteReg_Directly(0x0000);
    mdelay(12);
    s4AF_WriteReg_Directly(0xECA3);
    s4AF_WriteReg_Directly(0xA10D);
    s4AF_WriteReg_Directly(0xF220);
    s4AF_WriteReg_Directly(0xDC51);
    return 0;
}
#if 0
static int s4AF_esc_mode_power_off(void)
{
    s4AF_WriteReg_Directly(0xECA3);
    s4AF_WriteReg_Directly(0xA10D);
    s4AF_WriteReg_Directly(0xF220);
    s4AF_WriteReg_Directly(0xDC51);
    s4AF_WriteReg_Directly(0x1900);
    mdelay(15);
    return 0;
}
#endif
#endif
static inline int getAFInfo(__user struct stAF_MotorInfo *pstMotorInfo)
{
	struct stAF_MotorInfo stMotorInfo;

	stMotorInfo.u4MacroPosition = g_u4AF_MACRO;
	stMotorInfo.u4InfPosition = g_u4AF_INF;
	stMotorInfo.u4CurrentPosition = g_u4CurrPosition;
	stMotorInfo.bIsSupportSR = 1;

	stMotorInfo.bIsMotorMoving = 1;

	if (*g_pAF_Opened >= 1)
		stMotorInfo.bIsMotorOpen = 1;
	else
		stMotorInfo.bIsMotorOpen = 0;

	if (copy_to_user(pstMotorInfo, &stMotorInfo,
			 sizeof(struct stAF_MotorInfo)))
		LOG_INF("copy to user failed when getting motor information\n");

	return 0;
}

/* initAF include driver initialization and standby mode */
static int initAF(void)
{
	LOG_INF("+\n");

	if (*g_pAF_Opened == 1) {

	   spin_lock(g_pAF_SpinLock);
	   *g_pAF_Opened = 2;
	   spin_unlock(g_pAF_SpinLock);
#ifdef VENDOR_EDIT
        /*CongZhou@HQ 20190428 add for 1922753 cn3927e*/
          s4AF_esc_mode();
#endif
	}

	LOG_INF("-\n");

	return 0;
}

/* moveAF only use to control moving the motor */
static inline int moveAF(unsigned long a_u4Position)
{
	int ret = 0;

	if (s4AF_WriteReg((unsigned short)a_u4Position) == 0) {
		g_u4CurrPosition = a_u4Position;
		ret = 0;
	} else {
		LOG_INF("set I2C failed when moving the motor\n");
		ret = -1;
	}

	return ret;
}

static inline int setAFInf(unsigned long a_u4Position)
{
	spin_lock(g_pAF_SpinLock);
	g_u4AF_INF = a_u4Position;
	spin_unlock(g_pAF_SpinLock);
	return 0;
}

static inline int setAFMacro(unsigned long a_u4Position)
{
	spin_lock(g_pAF_SpinLock);
	g_u4AF_MACRO = a_u4Position;
	spin_unlock(g_pAF_SpinLock);
	return 0;
}

/* ////////////////////////////////////////////////////////////// */
long CN3927EAF_Ioctl(struct file *a_pstFile, unsigned int a_u4Command,
		    unsigned long a_u4Param)
{
	long i4RetValue = 0;

	switch (a_u4Command) {
	case AFIOC_G_MOTORINFO:
		i4RetValue =
			getAFInfo((__user struct stAF_MotorInfo *)(a_u4Param));
		break;

	case AFIOC_T_MOVETO:
		i4RetValue = moveAF(a_u4Param);
		break;

	case AFIOC_T_SETINFPOS:
		i4RetValue = setAFInf(a_u4Param);
		break;

	case AFIOC_T_SETMACROPOS:
		i4RetValue = setAFMacro(a_u4Param);
		break;

	default:
		LOG_INF("No CMD\n");
		i4RetValue = -EPERM;
		break;
	}

	return i4RetValue;
}

/* Main jobs: */
/* 1.Deallocate anything that "open" allocated in private_data. */
/* 2.Shut down the device on last close. */
/* 3.Only called once on last time. */
/* Q1 : Try release multiple times. */
int CN3927EAF_Release(struct inode *a_pstInode, struct file *a_pstFile)
{
	LOG_INF("Start\n");

	if (*g_pAF_Opened == 2) {
	  LOG_INF("Wait\n");
#ifdef VENDOR_EDIT
      /*CongZhou@HQ 20190428 modify for update cn3927e driver*/
      //s4AF_WriteReg(0x80); /* Power down mode */
      //s4AF_esc_mode_power_off();
      /*Cong.Zhou@ODM_HQ 20190830 for improve rear cam switch to front cam performance*/
      s4AF_WriteReg(400);
      mdelay(10);
      s4AF_WriteReg_Directly(0xECA3);
      s4AF_WriteReg_Directly(0xA105);
      s4AF_WriteReg_Directly(0xF270);
      s4AF_WriteReg_Directly(0xDC51);
      s4AF_WriteReg(360);
      mdelay(8);
      s4AF_WriteReg(320);
      mdelay(8);
      s4AF_WriteReg(300);
      mdelay(8);
      s4AF_WriteReg(280);
      mdelay(8);
      s4AF_WriteReg(260);
      mdelay(8);
      s4AF_WriteReg(240);
      mdelay(8);
      s4AF_WriteReg(220);
      mdelay(8);
      s4AF_WriteReg(200);
      mdelay(8);
      s4AF_WriteReg(180);
      mdelay(8);
      s4AF_WriteReg(160);
      mdelay(8);
      s4AF_WriteReg(140);
      mdelay(8);
      s4AF_WriteReg(100);
      mdelay(8);
      s4AF_WriteReg(60);
      mdelay(8);
      s4AF_WriteReg(20);
      mdelay(8);
#endif
	}

	if (*g_pAF_Opened) {
		LOG_INF("Free\n");

		spin_lock(g_pAF_SpinLock);
		*g_pAF_Opened = 0;
		spin_unlock(g_pAF_SpinLock);
	}

	LOG_INF("End\n");

	return 0;
}

int CN3927EAF_SetI2Cclient(struct i2c_client *pstAF_I2Cclient,
			  spinlock_t *pAF_SpinLock, int *pAF_Opened)
{
	g_pstAF_I2Cclient = pstAF_I2Cclient;
	g_pAF_SpinLock = pAF_SpinLock;
	g_pAF_Opened = pAF_Opened;

	initAF();

	return 1;
}

int CN3927EAF_GetFileName(unsigned char *pFileName)
{
	#if SUPPORT_GETTING_LENS_FOLDER_NAME
	char FilePath[256];
	char *FileString;

	sprintf(FilePath, "%s", __FILE__);
	FileString = strrchr(FilePath, '/');
	*FileString = '\0';
	FileString = (strrchr(FilePath, '/') + 1);
	strncpy(pFileName, FileString, AF_MOTOR_NAME);
	LOG_INF("FileName : %s\n", pFileName);
	#else
	pFileName[0] = '\0';
	#endif
	return 1;
}
