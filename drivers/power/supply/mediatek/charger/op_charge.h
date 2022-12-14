/**********************************************************************************
* Copyright (c)  2008-2019  Guangdong OPPO Mobile Comm Corp., Ltd
* VENDOR_EDIT
* Description: OPPO Charge Module Device Tree
* 
* Version: 1.0
* Date: 2019-06-10
* ------------------------------ Revision History: --------------------------------
* <version>           <date>                <author>                            <desc>
*                   2019-07-03            Yichun.Chen                     add common function
***********************************************************************************/
#ifndef __OP_CHARGE_H__
#define __OP_CHARGE_H__
#ifdef ODM_HQ_EDIT
/* zhangchao@ODM.HQ.Charger 2019/12/06 modified for USB_NTC */
int con_volt_100K[] = {
	1648,
	1638,
	1628,
	1617,
	1605,
	1593,
	1580,
	1567,
	1553,
	1538,
	1523,
	1507,
	1491,
	1474,
	1456,
	1438,
	1420,
	1400,
	1381,
	1360,
	1339,
	1318,
	1296,
	1274,
	1251,
	1227,
	1204,
	1180,
	1156,
	1132,
	1107,
	1082,
	1057,
	1032,
	1007,
	982,
	957,
	932,
	906,
	882,
	857,
	833,
	809,
	785,
	761,
	737,
	714,
	692,
	671,
	649,
	628,
	606,
	587,
	566,
	546,
	528,
	509,
	492,
	475,
	457,
	442,
	426,
	409,
	396,
	378,
	367,
	352,
	340,
	325,
	312,
	303,
	290,
	280,
	270,
	260,
	250,
	240,
	229,
	222,
	211,
	204,
	197,
	189,
	182,
	175,
	167,
	163,
	155,
	152,
	144,
	140,
	132,
	128,
	124,
	120,
	116,
	112,
/*hongzhenglong@ODM.HQ.Charger 2020/06/15 modified for usb burn-proof port*/
	71,
	70,
	70,
	70,
	70,
	70,
	70,
	70,
	70,
	70,
	70,
	68,
	66,
	64,
	62,
	60,
	58,
	55,
	53,
	51,
	49,
	47,
	45,
	43,
	41,
	40,
	38,
	37,
	36,
	35,
	34,
	33,
	31,
	30,
	29,
	28,
	27,
	26,
	25,
	24,
	23,
/*hongzhenglong@ODM.HQ.Charger 2020/04/11 modified for usb burn-proof port*/
	22,
	21,
	20,
	9,
	9,
	9,
	9,
	9,
	9,
	9,
	9,
	9,
	9,
	9,
	9,
	9,
	9,
	9,
	9,
	9,
	9,
	9,
	9,
	9,
	9,
	9,
	9,
	9,
 };
int con_temp_100K[] = {
	-40,
	-39,
	-38,
	-37,
	-36,
	-35,
	-34,
	-33,
	-32,
	-31,
	-30,
	-29,
	-28,
	-27,
	-26,
	-25,
	-24,
	-23,
	-22,
	-21,
	-20,
	-19,
	-18,
	-17,
	-16,
	-15,
	-14,
	-13,
	-12,
	-11,
	-10,
	-9,
	-8,
	-7,
	-6,
	-5,
	-4,
	-3,
	-2,
	-1,
	0,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	8,
	9,
	10,
	11,
	12,
	13,
	14,
	15,
	16,
	17,
	18,
	19,
	20,
	21,
	22,
	23,
	24,
	25,
	26,
	27,
	28,
	29,
	30,
	31,
	32,
	33,
	34,
	35,
	36,
	37,
	38,
	39,
	40,
	41,
	42,
	43,
	44,
	45,
	46,
	47,
	48,
	49,
	50,
	51,
	52,
	53,
	54,
	55,
	56,
	57,
	58,
	59,
	60,
	61,
	62,
	63,
	64,
	65,
	66,
	67,
	68,
	69,
	70,
	71,
	72,
	73,
	74,
	75,
	76,
	77,
	78,
	79,
	80,
	81,
	82,
	83,
	84,
	85,
	86,
	87,
	88,
	89,
	90,
	91,
	92,
	93,
	94,
	95,
	96,
	97,
	98,
	99,
	100,
	101,
	102,
	103,
	104,
	105,
	106,
	107,
	108,
	109,
	110,
	111,
	112,
	113,
	114,
	115,
	116,
	117,
	118,
	119,
	120,
	121,
	122,
	123,
	124,
	125,
};

int con_temp_20682[] = {
	-40,
	-39,
	-38,
	-37,
	-36,
	-35,
	-34,
	-33,
	-32,
	-31,
	-30,
	-29,
	-28,
	-27,
	-26,
	-25,
	-24,
	-23,
	-22,
	-21,
	-20,
	-19,
	-18,
	-17,
	-16,
	-15,
	-14,
	-13,
	-12,
	-11,
	-10,
	-9,
	-8,
	-7,
	-6,
	-5,
	-4,
	-3,
	-2,
	-1,
	0,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	8,
	9,
	10,
	11,
	12,
	13,
	14,
	15,
	16,
	17,
	18,
	19,
	20,
	21,
	22,
	23,
	24,
	25,
	26,
	27,
	28,
	29,
	30,
	31,
	32,
	33,
	34,
	35,
	36,
	37,
	38,
	39,
	40,
	41,
	42,
	43,
	44,
	45,
	46,
	47,
	48,
	49,
	50,
	51,
	52,
	53,
	54,
	55,
	56,
	57,
	58,
	59,
	60,
	61,
	62,
	63,
	64,
	65,
	66,
	67,
	68,
	69,
	70,
	71,
	72,
	73,
	74,
	75,
	76,
	77,
	78,
	79,
	80,
	81,
	82,
	83,
	84,
	85,
	86,
	87,
	88,
	89,
	90,
	91,
	92,
	93,
	94,
	95,
	96,
	97,
	98,
	99,
	100,
	101,
	102,
	103,
	104,
	105,
	106,
	107,
	108,
	109,
	110,
	111,
	112,
	113,
	114,
	115,
	116,
	117,
	118,
	119,
	120,
	121,
	122,
	123,
	124,
	125,
};
int con_volt_20682[] = {
	1649,
	1639,
	1628,
	1617,
	1605,
	1593,
	1580,
	1567,
	1553,
	1539,
	1524,
	1508,
	1491,
	1475,
	1457,
	1439,
	1420,
	1401,
	1381,
	1361,
	1340,
	1319,
	1296,
	1274,
	1251,
	1228,
	1205,
	1181,
	1157,
	1132,
	1108,
	1083,
	1058,
	1033,
	1008,
	983,
	958,
	933,
	908,
	883,
	858,
	834,
	810,
	786,
	763,
	739,
	716,
	694,
	672,
	650,
	629,
	608,
	588,
	568,
	548,
	529,
	511,
	493,
	476,
	459,
	442,
	426,
	411,
	396,
	381,
	367,
	354,
	341,
	328,
	316,
	304,
	293,
	282,
	271,
	261,
	251,
	241,
	232,
	224,
	215,
	207,
	199,
	192,
	184,
	178,
	171,
	164,
	158,
	152,
	147,
	141,
	136,
	131,
	126,
	121,
	117,
	113,
	108,
	105,
	101,
	97,
	94,
	90,
	87,
	84,
	81,
	78,
	75,
	72,
	70,
	67,
	65,
	63,
	61,
	59,
	57,
	55,
	53,
	51,
	49,
	48,
	46,
	44,
	43,
	41,
	40,
	39,
	38,
	36,
	35,
	34,
	33,
	32,
	31,
	30,
	29,
	28,
	27,
	26,
	25,
	25,
	24,
	23,
	22,
	22,
	21,
	20,
	20,
	19,
	19,
	18,
	18,
	17,
	17,
	16,
	16,
	15,
	15,
	14,
	14,
	14,
	13,
	13,
	12,
	12,
	12,
};
#endif
#endif /* __OP_CHARGE_H__ */
