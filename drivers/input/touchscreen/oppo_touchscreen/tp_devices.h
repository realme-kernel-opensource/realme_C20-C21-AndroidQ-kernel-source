/***************************************************
 * File:tp_devices.h
 * VENDOR_EDIT
 * Copyright (c)  2008- 2030  Oppo Mobile communication Corp.ltd.
 * Description:
 *             tp dev
 * Version:1.0:
 * Date created:2016/09/02
 * Author: Tong.han@Bsp.Driver
 * TAG: BSP.TP.Init
 *
 * -------------- Revision History: -----------------
 *  <author >  <data>  <version>  <desc>
 ***************************************************/
#ifndef OPPO_TP_DEVICES_H
#define OPPO_TP_DEVICES_H
//device list define
typedef enum tp_dev{
    TP_OFILM,
    TP_BIEL,
    TP_TRULY,
    TP_BOE,
    TP_G2Y,
    TP_TPK,
    TP_JDI,
    TP_TIANMA,
    TP_SAMSUNG,
    TP_DSJM,
    TP_BOE_B8,
    TP_INNOLUX,
    TP_HIMAX_DPT,
    TP_AUO,
    TP_DEPUTE,
    TP_BOE_90HZ_82n,
    TP_BOE_90HZ_06S,
	TP_BOE_90HZ_722,
	TP_TRULY_06S,
    TP_PANDA_90HZ_82h,
    TP_MN228_82n,
    TP_TRULY_82n,
    TP_PANDA_90HZ_102D,
    TP_TRULY_90HZ_102D,
	TP_PANDA_90HZ_112A,
    TP_NVT_90HZ_15b,
    TP_HUAXING,
    TP_HLT,
    TP_DJN,
    TP_UNKNOWN,
}tp_dev;

struct tp_dev_name {
    tp_dev type;
    char name[32];
};

#endif

