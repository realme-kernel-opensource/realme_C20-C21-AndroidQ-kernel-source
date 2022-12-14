/**
 * Copyright 2008-2013 OPPO Mobile Comm Corp., Ltd, All rights reserved.
 * VENDOR_EDIT:
 * FileName:devinfo.c
 * ModuleName:devinfo
 * Author: wangjc
 * Create Date: 2013-10-23
 * Description:add interface to get device information.
 * History:
   <version >  <time>  <author>  <desc>
   1.0		2013-10-23	wangjc	init
   2.0      2015-04-13  hantong modify as platform device  to support diffrent configure in dts
*/

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <soc/oppo/device_info.h>
#include <soc/oppo/oppo_project.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include "../../../../fs/proc/internal.h"
#include <linux/gpio.h>
#include <asm/uaccess.h>
#include <linux/delay.h>

#define DEVINFO_NAME "devinfo"
#define INFO_BUF_LEN 64
/**for definfo log**/
#define log_fmt(fmt) "[line:%d][module:%s][%s] " fmt

#define DEVINFO_ERR(a,arg...) \
 do { \
		printk(KERN_NOTICE log_fmt(a),__LINE__,DEVINFO_NAME,__func__,##arg); \
  } while (0)

#define DEVINFO_MSG(a, arg...) \
 do { \
		printk(KERN_INFO log_fmt(a),__LINE__,DEVINFO_NAME,__func__,##arg); \
  } while (0)

/**definfo log end**/
#ifdef ODM_HQ_EDIT
/*wangtaotao@ODM.BSP.kernel, 2019/11/19, Add for sdcard  detect*/
int sdcard_gpio_value = -1;
int kboard_gpio_value = -1;
#endif

extern pid_t fork_pid_child;
extern pid_t fork_pid_father;
extern int happend_times;

static struct of_device_id devinfo_id[] = {
	{.compatible = "oppo, device_info",},
	{},
};

struct devinfo_data {
	struct platform_device *devinfo;
	struct pinctrl *pinctrl;
	int hw_id1_gpio;
	int hw_id2_gpio;
	int hw_id3_gpio;
	int sub_hw_id1;
	int sub_hw_id2;
	int main_hw_id5;
	int main_hw_id6;
#ifdef ODM_HQ_EDIT
	int sub_board_id;
	int sdcard_gpio;
#endif
	struct pinctrl_state *hw_sub_id_sleep;
	struct pinctrl_state *hw_sub_id_active;
	struct pinctrl_state *hw_main_id5_active;
	struct pinctrl_state *hw_main_id6_active;
	int ant_select_gpio;
	struct manufacture_info sub_mainboard_info;
};

static struct devinfo_data *dev_info;
static struct proc_dir_entry *parent = NULL;

static void *device_seq_start(struct seq_file *s, loff_t *pos)
{
	static unsigned long counter = 0;
	if ( *pos == 0 ) {
		return &counter;
	}else{
		*pos = 0;
		return NULL;
	}
}

static void *device_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	return NULL;
}

static void device_seq_stop(struct seq_file *s, void *v)
{
	return;
}

static int device_seq_show(struct seq_file *s, void *v)
{
        struct proc_dir_entry *pde = s->private;
        struct manufacture_info *info = pde->data;
        if (info) {
                seq_printf(s, "Device version:\t\t%s\nDevice manufacture:\t\t%s\n",
                         info->version,        info->manufacture);
                if(info->fw_path)
                        seq_printf(s, "Device fw_path:\t\t%s\n",
                                info->fw_path);
        }
        return 0;
}

static struct seq_operations device_seq_ops = {
	.start = device_seq_start,
	.next = device_seq_next,
	.stop = device_seq_stop,
	.show = device_seq_show
};

static int device_proc_open(struct inode *inode,struct file *file)
{
	int ret = seq_open(file,&device_seq_ops);
	pr_err("%s is called\n",__func__);

	if(!ret){
		struct seq_file *sf = file->private_data;
		sf->private = PDE(inode);
	}

	return ret;
}
static const struct file_operations device_node_fops = {
	.read =  seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
	.open = device_proc_open,
	.owner = THIS_MODULE,
};

int register_device_proc(char *name, char *version, char *manufacture)
{
	struct proc_dir_entry *d_entry;
	struct manufacture_info *info;

	if(!parent) {
		parent =  proc_mkdir ("devinfo", NULL);
		if(!parent) {
			pr_err("can't create devinfo proc\n");
			return -ENOENT;
		}
	}

	info = kzalloc(sizeof *info, GFP_KERNEL);
	info->version = version;
	info->manufacture = manufacture;
	d_entry = proc_create_data (name, S_IRUGO, parent, &device_node_fops, info);
	if(!d_entry) {
		DEVINFO_ERR("create %s proc failed.\n", name);
		kfree(info);
		return -ENOENT;
	}
	return 0;
}

int register_devinfo(char *name, struct manufacture_info *info)
{
	struct proc_dir_entry *d_entry;
	if(!parent) {
	parent =  proc_mkdir ("devinfo", NULL);
	if(!parent) {
			pr_err("can't create devinfo proc\n");
			return -ENOENT;
		}
	}

	d_entry = proc_create_data (name, S_IRUGO, parent, &device_node_fops, info);
	if(!d_entry) {
		pr_err("create %s proc failed.\n", name);
		return -ENOENT;
	}
	return 0;
}

static int get_hw_opreator_version(struct devinfo_data *const devinfo_data)
{
	DEVINFO_MSG("%d\n", get_Operator_Version());
	return get_Operator_Version();
}

static int mainboard_verify(struct devinfo_data *const devinfo_data)
{
	struct manufacture_info mainboard_info;
	int hw_opreator_version = 0;
	static char temp[INFO_BUF_LEN] = {0}; //just alloc a memory

	mainboard_info.manufacture = temp;

	if (!devinfo_data) {
		DEVINFO_ERR("devinfo_data is NULL\n");
		return -1;
	}

	hw_opreator_version = get_hw_opreator_version(devinfo_data);
	switch(get_PCB_Version()) {
#if !defined(ODM_HQ_EDIT) || defined(CONFIG_MACH_MT6771)
		case HW_VERSION__10:
			mainboard_info.version ="10";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d-SA", hw_opreator_version);
			break;
		case HW_VERSION__11:
			mainboard_info.version = "11";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d-SB", hw_opreator_version);
			break;
		case HW_VERSION__12:
			mainboard_info.version = "12";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d-SC", hw_opreator_version);
			break;
		case HW_VERSION__13:
			mainboard_info.version = "13";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d-SD", hw_opreator_version);
			break;
		case HW_VERSION__14:
			mainboard_info.version = "14";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d-SE", hw_opreator_version);
			break;
		case HW_VERSION__15:
			mainboard_info.version = "15";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d-(T3-T4)", hw_opreator_version);
			break;
#else
		case HW_VERSION__EVB1:
			mainboard_info.version ="EVB1";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d", hw_opreator_version);
			break;
		case HW_VERSION__T0:
			mainboard_info.version = "T0";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d", hw_opreator_version);
			break;
		case HW_VERSION__T1:
			mainboard_info.version = "T1";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d", hw_opreator_version);
			break;
		case HW_VERSION__EVT1:
			mainboard_info.version = "EVT1";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d", hw_opreator_version);
			break;
		case HW_VERSION__EVT2:
			mainboard_info.version = "EVT2";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d", hw_opreator_version);
			break;
		case HW_VERSION__EVT3:
			mainboard_info.version = "EVT3";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d", hw_opreator_version);
			break;
		case HW_VERSION__DVT1:
			mainboard_info.version = "DVT1";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d", hw_opreator_version);
			break;
		case HW_VERSION__DVT2:
			mainboard_info.version = "DVT2";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d", hw_opreator_version);
			break;
		case HW_VERSION__DVT3:
			mainboard_info.version = "DVT3";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d", hw_opreator_version);
			break;
		case HW_VERSION__PVT:
			mainboard_info.version = "PVT";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d", hw_opreator_version);
			break;
		case HW_VERSION__MP1:
			mainboard_info.version = "MP1";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d", hw_opreator_version);
			break;
		case HW_VERSION__MP2:
			mainboard_info.version = "MP2";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d", hw_opreator_version);
			break;
		case HW_VERSION__MP3:
			mainboard_info.version = "MP3";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d", hw_opreator_version);
			break;
#endif
		default:
			mainboard_info.version = "UNKOWN";
			snprintf(mainboard_info.manufacture, INFO_BUF_LEN, "%d-UNKOWN", hw_opreator_version);
	}

	return register_device_proc("mainboard", mainboard_info.version, mainboard_info.manufacture);
}

#ifdef ODM_HQ_EDIT
static ssize_t kboard_read_proc(struct file *file, char __user *buf,
                size_t count, loff_t *off)
{
	char page[256] = {0};
    int len = 0;
	int value = -1;
	value = gpio_get_value(kboard_gpio_value);
	if(value < 0)
		return 0;
	len = sprintf(page,"%d", value);
        if (len > *off) {
                len -= *off;
        }
        else{
                len = 0;
        }

        if (copy_to_user(buf, page, (len < count ? len : count))) {
                return -EFAULT;
        }
        *off += len < count ? len : count;
        return (len < count ? len : count);
}
struct file_operations kboard_proc_fops = {
        .read = kboard_read_proc,
};
#endif
static int subboard_init(struct devinfo_data *const devinfo_data) {
	int ret = 0;
	struct device_node *np = NULL;
#ifdef ODM_HQ_EDIT
	struct proc_dir_entry *pentry;
#endif
	np = devinfo_data->devinfo->dev.of_node;

#if !defined(ODM_HQ_EDIT) || defined(CONFIG_MACH_MT6771)
	devinfo_data->sub_hw_id1 = of_get_named_gpio(np, "Hw,sub_hwid_1", 0);
	if(devinfo_data->sub_hw_id1 < 0 ) {
		DEVINFO_ERR("devinfo_data->sub_hw_id1 not specified\n");
		return -1;
	}

	devinfo_data->sub_hw_id2 = of_get_named_gpio(np, "Hw,sub_hwid_2", 0);
	if(devinfo_data->sub_hw_id2 < 0 ) {
		DEVINFO_ERR("devinfo_data->sub_hw_id2 not specified\n");
		return -1;
	}

	devinfo_data->pinctrl = devm_pinctrl_get(&devinfo_data->devinfo->dev);
	if (IS_ERR_OR_NULL(devinfo_data->pinctrl)) {
		 DEVINFO_ERR("Getting pinctrl handle failed\n");
		 return -1;
	}

	devinfo_data->hw_sub_id_sleep = pinctrl_lookup_state(devinfo_data->pinctrl, "hw_sub_id_sleep");
	if (IS_ERR_OR_NULL(devinfo_data->hw_sub_id_sleep)) {
		 DEVINFO_ERR("Failed to get the suspend state pinctrl handle\n");
		 return -1;
	}

	devinfo_data->hw_sub_id_active = pinctrl_lookup_state(devinfo_data->pinctrl, "hw_sub_id_active");
	if (IS_ERR_OR_NULL(devinfo_data->hw_sub_id_active)) {
		DEVINFO_ERR("Failed to get the suspend state pinctrl handle\n");
		return -1;
	}
#else
	devinfo_data->sub_board_id = of_get_named_gpio(np, "sub_board-gpio", 0);
	if(devinfo_data->sub_board_id < 0 ) {
		DEVINFO_ERR("devinfo_data->sub_board_id not specified\n");
		return -1;
	}
	kboard_gpio_value = devinfo_data->sub_board_id;
	pentry = proc_create("kboard", 0664, NULL, &kboard_proc_fops);
#endif
	devinfo_data->sub_mainboard_info.version = kzalloc(INFO_BUF_LEN, GFP_KERNEL);
	if (devinfo_data->sub_mainboard_info.version == NULL) {
		return -1;
	}

	devinfo_data->sub_mainboard_info.manufacture = kzalloc(INFO_BUF_LEN, GFP_KERNEL);
	if (devinfo_data->sub_mainboard_info.manufacture == NULL) {
		kfree(devinfo_data->sub_mainboard_info.version);
		return -1;
	}

	snprintf(devinfo_data->sub_mainboard_info.version, INFO_BUF_LEN, "MTK");
	ret = register_device_proc("audio_mainboard", 
		devinfo_data->sub_mainboard_info.version,
		devinfo_data->sub_mainboard_info.manufacture);

#ifdef ODM_HQ_EDIT
	ret = register_device_proc("sub_mainboard",
		devinfo_data->sub_mainboard_info.version,
		devinfo_data->sub_mainboard_info.manufacture);
#endif
	return ret;
}

/**
* @ sub_mainboard_verify
* normally we use one or two pins to distinguish sub mainboard(small board)
* after get the state of the pins, set to pin state to pull down.
*/
static int subboard_verify(struct devinfo_data *const devinfo_data)
{
	int ret = 0;
	int id1 = -1;
  	int id2 = -1;
#if !defined(ODM_HQ_EDIT) || defined(CONFIG_MACH_MT6771)
	int a = -1;
	int b = -1;
#endif
#ifdef ODM_HQ_EDIT
/*sunjingtao@ODM.BSP.bootloader, 2019/10/17, Add for oppo project*/
	int id = -1;
#endif
	static int operator = OPERATOR_UNKOWN ;

	DEVINFO_MSG("Enter\n");
	printk("subboard_verify Enter\n");
	if(IS_ERR_OR_NULL(devinfo_data)){
		DEVINFO_ERR("devinfo_data is NULL\n");
		return -1;
	}
printk("subboard_verify Enter111\n");
#if !defined(ODM_HQ_EDIT) || defined(CONFIG_MACH_MT6771)
	//ret = pinctrl_select_state(devinfo_data->pinctrl,devinfo_data->hw_sub_id_active);

	if(devinfo_data->sub_hw_id1 >= 0 ) {
		ret = gpio_request(devinfo_data->sub_hw_id1, "SUB_HW_ID1");
		if(ret){
			DEVINFO_ERR("unable to request gpio [%d]\n",devinfo_data->sub_hw_id1);
			return ret;
		}else{
			ret = pinctrl_select_state(devinfo_data->pinctrl,devinfo_data->hw_sub_id_sleep);
			msleep(2);
			a = gpio_get_value(devinfo_data->sub_hw_id1);
			ret = pinctrl_select_state(devinfo_data->pinctrl,devinfo_data->hw_sub_id_active);
			msleep(2);
			b = gpio_get_value(devinfo_data->sub_hw_id1);

			if ((a == 0) && (b == 0)){
				id1 = 0;
			} else if ((a == 1) && (b == 1)){
				id1 = 2;
			} else if ((a == 0) && (b == 1)){
				id1 = 1;
			}
			gpio_free(devinfo_data->sub_hw_id1);
		}
 	}

 	if(devinfo_data->sub_hw_id2 >= 0 ) {
		ret = gpio_request(devinfo_data->sub_hw_id2, "SUB_HW_ID2");
		if(ret){
			DEVINFO_ERR("unable to request gpio [%d]\n",devinfo_data->sub_hw_id2);
			return ret;
		}else{
			ret = pinctrl_select_state(devinfo_data->pinctrl,devinfo_data->hw_sub_id_sleep);
			msleep(2);
			a = gpio_get_value(devinfo_data->sub_hw_id2);
			msleep(2);
			ret = pinctrl_select_state(devinfo_data->pinctrl,devinfo_data->hw_sub_id_active);
			msleep(2);
			b = gpio_get_value(devinfo_data->sub_hw_id2);

			if ((a == 0) && (b == 0)){
				id2 = 0;
			} else if ((a == 1) && (b == 1)){
				id2 = 2;
			} else if ((a == 0) && (b == 1)){
				id2 = 1;
			}
			gpio_free(devinfo_data->sub_hw_id2);
		}
 	}

#else
	if(devinfo_data->sub_board_id >= 0 ) {
			id = gpio_get_value(devinfo_data->sub_board_id);
	}
#endif
	operator = get_Operator_Version();
#if !defined(ODM_HQ_EDIT) || defined(CONFIG_MACH_MT6771)
	DEVINFO_ERR("id1(%d) = %d id2(%d) = %d Operator = %d", devinfo_data->sub_hw_id1, id1,
		devinfo_data->sub_hw_id2, id2, operator);
#else
	DEVINFO_ERR("id(%d) = %d  Operator = %d", devinfo_data->sub_board_id, id, operator);
#endif
	switch(get_project()) {
		case OPPO_18151:
			if ((id1 == 0) && (id2 == 1) && (operator == OPERATOR_18151_MOBILE || operator == OPERATOR_18151_All_BAND)) {
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "rf-match");
			} else if ((id1 == 0) && (id2 == 0) && (operator == OPERATOR_18151_CARRIER)) {
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "rf-match");
			} else {
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "rf-unmatch");
			}
			break;
		case OPPO_18073:
		case OPPO_18593:
			if ((id1 == 1) && (id2 == 1) && (operator == OPERATOR_18073_MOBILE || operator == OPERATOR_18073_All_BAND)) {
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "rf-match");
			} else if ((id1 == 1) && (id2 == 0) && (operator == OPERATOR_18593_CARRIER)) {
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "rf-match");
			}else {
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "rf-unmatch");
			}
			break;
		case OPPO_19011:
		case OPPO_19301:
			if ((id1 == 1) && (id2 == 1) && (operator == OPERATOR_19011_All_BAND || operator == OPERATOR_19011_MOBILE)) {
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "rf-match");
			} else if ((id1 == 0) && (id2 == 1) && (operator == OPERATOR_19301_CARRIER || operator == OPERATOR_19305_CARRIER)) {
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "rf-match");
			}else {
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "rf-unmatch");
			}
			break;
		case OPPO_19551:
			if ((id1 == 1) && (id2 == 1)) {
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "rf-match");
			}else {
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "rf-unmatch");
			}
			break;
		case OPPO_19553:
		case OPPO_19556:
			if ((id1 == 0) && (id2 == 1)) {
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "rf-match");
			}else {
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "rf-unmatch");
			}
			break;
		case OPPO_19550:
			if ((id1 == 1) && (id2 == 0)) {
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "rf-match");
			}else {
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "rf-unmatch");
			}
			break;
		case OPPO_19597:
			if ((id1 == 1) && (id2 == 1)) {
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "rf-match");
			} else if ((id1 == 1) && (id2 == 0)) {
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "rf-match");
			} else if ((id1 == 1) && (id2 == 2)) {
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "rf-match");
			}else {
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "rf-unmatch");
			}
			break;
		case OPPO_19358:
		case OPPO_19357:
			if ((id1 == 1) && (id2 == 1)) {
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "rf-match");
			}else {
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "rf-unmatch");
			}
			break;
		case OPPO_19354:
		case OPPO_19359:
			if ((id1 == 1) && (id2 == 0)) {
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "rf-match");
			}else {
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "rf-unmatch");
			}
			break;
		case OPPO_20701:
			if ((0 == id || 1 == id)) {
			printk("subboard_verify 2222\n");
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "HQsub-match");
			}else {
			printk("subboard_verify 3333333\n");
				snprintf(devinfo_data->sub_mainboard_info.manufacture, INFO_BUF_LEN, "HQsub-unmatch");
			}
			break;
		default:
			DEVINFO_ERR("illegal project\n");
			break;
	}
#ifndef ODM_HQ_EDIT
/*sunjingtao@ODM.BSP.bootloader, 2019/10/17, Add for oppo project*/
	ret = pinctrl_select_state(devinfo_data->pinctrl,devinfo_data->hw_sub_id_sleep);
	if (ret < 0) {
		DEVINFO_ERR("set sub id1 failed\n");
		return -1;
	}
#endif

	return ret;
}
#ifdef ODM_HQ_EDIT
/*wangtaotao@ODM.BSP.kernel, 2019/11/19, Add for sdcard detect*/
static ssize_t sdcard_read_proc(struct file *file, char __user *buf,
                size_t count, loff_t *off)
{
	char page[256] = {0};
    int len = 0;
	int value = -1;
	value = gpio_get_value(sdcard_gpio_value);
	if(value < 0)
		return 0;
	len = sprintf(page,"%d", value);
        if (len > *off) {
                len -= *off;
        }
        else{
                len = 0;
        }

        if (copy_to_user(buf, page, (len < count ? len : count))) {
                return -EFAULT;
        }

        *off += len < count ? len : count;
        return (len < count ? len : count);
}

struct file_operations sdcard_proc_fops = {
        .read = sdcard_read_proc,
};
static int sdcard_init(struct devinfo_data *const devinfo_data) {
	struct device_node *np = NULL;
	struct proc_dir_entry *pentry;

	np = devinfo_data->devinfo->dev.of_node;
	devinfo_data->sdcard_gpio= of_get_named_gpio(np, "sdcard-gpio", 0);
	if(devinfo_data->sdcard_gpio < 0 ) {
		DEVINFO_ERR("devinfo_data->sdcard_gpio not specified\n");
		return -1;
	}

	sdcard_gpio_value = devinfo_data->sdcard_gpio;
	pentry = proc_create("sdcard_detect", 0664, NULL, &sdcard_proc_fops);
	if(!pentry) {
		pr_err("create sdcard_detect proc failed.\n");
		return -ENOENT;
	}
	return 0;
}
#endif
static ssize_t devinfo_modify_write(struct file *file, const char __user *buff, size_t count, loff_t *ppos){
	char proc_devinfo_modify_data[28];
	DEVINFO_ERR("call\n");
	if (copy_from_user(&proc_devinfo_modify_data, buff, count)){
		DEVINFO_ERR("error.\n");
		return count;
	}
	if (strncmp(proc_devinfo_modify_data, "1", 1) == 0){
		DEVINFO_ERR("subboard need to check again.\n");
		subboard_verify(dev_info);
	} else {
		DEVINFO_ERR("not support \n");
		return count;
	}

	return count;
}

static const struct file_operations devinfo_modify_fops = {
	.write = devinfo_modify_write,
	.llseek = noop_llseek,
};

static ssize_t fork_para_monitor_read_proc(struct file *file, char __user *buf,
                size_t count, loff_t *off)
{
        char page[256] = {0};
        int ret = 0;
        ret = snprintf(page, 255, " times:%d\n father pid:%d\n child pid:%d\n", happend_times, fork_pid_father, fork_pid_child);

        ret = simple_read_from_buffer(buf, count, off, page, strlen(page));
        return ret;
}

struct file_operations fork_para_monitor_proc_fops = {
        .read = fork_para_monitor_read_proc,
        .write = NULL,
};

static void recursive_fork_para_monitor(void)
{
		struct proc_dir_entry *pentry;

		pentry = proc_create("fork_monitor", S_IRUGO, parent, &fork_para_monitor_proc_fops);
        if (!pentry) {
                pr_err("create /devinfo/fork_monitor proc failed.\n");
        }
}

static void init_proc_devinfo_modify(void){
	struct proc_dir_entry *p = NULL;
	p = proc_create("devinfo_modify", S_IWUSR | S_IWGRP | S_IWOTH, NULL, &devinfo_modify_fops);
	if(!p)
		DEVINFO_ERR("proc_create devinfo_modify_fops fail!\n");
}

static int devinfo_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct devinfo_data * const devinfo_data = devm_kzalloc(&pdev->dev,sizeof(struct devinfo_data), GFP_KERNEL);

	if( IS_ERR_OR_NULL(devinfo_data) ) {
		DEVINFO_ERR("devinfo_data kzalloc failed\n");
		ret = -ENOMEM;
		return ret;
	}

	devinfo_data->devinfo = pdev;
	dev_info = devinfo_data;

	if(!parent) {
		parent =  proc_mkdir ("devinfo", NULL);
		if(!parent) {
			DEVINFO_ERR("can't create devinfo proc\n");
			ret = -ENOENT;
		}
	}

	ret = subboard_init(devinfo_data);
	if (ret < 0) {
		DEVINFO_ERR("register audio mainboard failed\n");
	} else {
		subboard_verify(devinfo_data);
	}

	ret = mainboard_verify(devinfo_data);
	if (ret < 0) {
		DEVINFO_ERR("register mainboard failed\n");
	}
	recursive_fork_para_monitor();
#ifdef ODM_HQ_EDIT
/*wangtaotao@ODM.BSP.kernel, 2019/11/19, Add for sdcard detect*/
	ret = sdcard_init(devinfo_data);
	if (ret < 0) {
		DEVINFO_ERR("register sdcard failed\n");
	}
#endif
	init_proc_devinfo_modify();
	return ret;
}

static int devinfo_remove(struct platform_device *dev)
{
	remove_proc_entry(DEVINFO_NAME, NULL);
	return 0;
}

static struct platform_driver devinfo_platform_driver = {
	.probe = devinfo_probe,
	.remove = devinfo_remove,
	.driver = {
		.name = DEVINFO_NAME,
		.of_match_table = devinfo_id,
	},
};

module_platform_driver(devinfo_platform_driver);

MODULE_DESCRIPTION("OPPO device info");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Wangjc <wjc@oppo.com>");
