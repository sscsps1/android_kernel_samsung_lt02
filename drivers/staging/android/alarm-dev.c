/* drivers/rtc/alarm-dev.c
 *
 * Copyright (C) 2007-2009 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/time.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
<<<<<<< HEAD
#include <linux/alarmtimer.h>
#include <linux/wakelock.h>
#include <linux/rtc.h>
#include <linux/mfd/88pm80x.h>
#include "android_alarm.h"

=======
#include "android_alarm.h"

/* XXX - Hack out wakelocks, while they are out of tree */
struct wake_lock {
	int i;
};
#define wake_lock(x)
#define wake_lock_timeout(x, y)
#define wake_unlock(x)
#define WAKE_LOCK_SUSPEND 0
#define wake_lock_init(x, y, z) ((x)->i = 1)
#define wake_lock_destroy(x)

>>>>>>> v3.4.6
#define ANDROID_ALARM_PRINT_INFO (1U << 0)
#define ANDROID_ALARM_PRINT_IO (1U << 1)
#define ANDROID_ALARM_PRINT_INT (1U << 2)

<<<<<<< HEAD
=======

>>>>>>> v3.4.6
static int debug_mask = ANDROID_ALARM_PRINT_INFO;
module_param_named(debug_mask, debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);

#define pr_alarm(debug_level_mask, args...) \
	do { \
		if (debug_mask & ANDROID_ALARM_PRINT_##debug_level_mask) { \
			pr_info(args); \
		} \
	} while (0)

#define ANDROID_ALARM_WAKEUP_MASK ( \
	ANDROID_ALARM_RTC_WAKEUP_MASK | \
	ANDROID_ALARM_ELAPSED_REALTIME_WAKEUP_MASK)

/* support old usespace code */
#define ANDROID_ALARM_SET_OLD               _IOW('a', 2, time_t) /* set alarm */
#define ANDROID_ALARM_SET_AND_WAIT_OLD      _IOW('a', 3, time_t)

static int alarm_opened;
static DEFINE_SPINLOCK(alarm_slock);
static struct wake_lock alarm_wake_lock;
static DECLARE_WAIT_QUEUE_HEAD(alarm_wait_queue);
static uint32_t alarm_pending;
static uint32_t alarm_enabled;
static uint32_t wait_pending;
<<<<<<< HEAD
static struct rtc_device *pwr_rtc_dev;

struct devalarm {
	union {
		struct hrtimer hrt;
		struct alarm alrm;
	} u;
	enum android_alarm_type type;
};

static struct devalarm alarms[ANDROID_ALARM_TYPE_COUNT];

static inline int rtc_match(struct device *dev, void *data)
{
	return strcmp(dev_name(dev), "rtc0");
}

/**
 * alarmpwr_get_rtcdev - Return power up rtcdevice
 *
 * This function returns the rtc device to power up for wakealarms.
 * If one has not already been chosen, it checks to see if a
 * functional rtc device is available.
 */
struct rtc_device *alarmpwr_get_rtcdev(void)
{
	struct rtc_device *ret;
	struct device *dev;

	dev = class_find_device(rtc_class, NULL, NULL, rtc_match);
	if (dev == NULL)
		return NULL;

	ret = to_rtc_device(dev);

	return ret;
}

static int is_wakeup(enum android_alarm_type type)
{
	if (type == ANDROID_ALARM_RTC_WAKEUP ||
	    type == ANDROID_ALARM_ELAPSED_REALTIME_WAKEUP ||
	    type == ANDROID_ALARM_POWER_UP)
		return 1;
	return 0;
}

int alarm_irq_enable(unsigned int enabled)
{
	int err;
	if (pwr_rtc_dev != NULL) {
		err = rtc_alarm_irq_enable(pwr_rtc_dev, enabled);
		if (err < 0)
			return err;
	}
	return 0;
}

int alarm_set_rtc_ring(struct timespec alarm_time)
{
	struct rtc_wkalrm   rtc_alarm;
	unsigned long       rtc_alarm_time;

	if (pwr_rtc_dev != NULL) {
		rtc_alarm_time = alarm_time.tv_sec;
		pr_alarm(INT, "%s, alarm time: %lu\n",
				__func__, rtc_alarm_time);
		rtc_time_to_tm(rtc_alarm_time, &rtc_alarm.time);
		rtc_alarm.enabled = 1;
		rtc_set_alarm(pwr_rtc_dev, &rtc_alarm);
	}
	return 0;
}

int alarm_read_rtc_ring(int *flag, unsigned long *alarm_time)
{
	struct rtc_wkalrm rtc_alarm;
	int ret = 0;

	if (pwr_rtc_dev != NULL) {
		if (pwr_rtc_dev->dev.platform_data)
			*flag = *(int *)(pwr_rtc_dev->dev.platform_data);
		ret = rtc_read_alarm(pwr_rtc_dev, &rtc_alarm);
		if (ret < 0)
			goto out;
		rtc_tm_to_time(&rtc_alarm.time, alarm_time);
		pr_alarm(INT, "%s, flag: %d, alarm time: %lu\n",
				__func__, *flag, *alarm_time);
	}
out:
	return ret;
}

static void devalarm_start(struct devalarm *alrm, ktime_t exp)
{
	if (is_wakeup(alrm->type))
		alarm_start(&alrm->u.alrm, exp);
	else
		hrtimer_start(&alrm->u.hrt, exp, HRTIMER_MODE_ABS);
}


static int devalarm_try_to_cancel(struct devalarm *alrm)
{
	int ret;
	if (is_wakeup(alrm->type))
		ret = alarm_try_to_cancel(&alrm->u.alrm);
	else
		ret = hrtimer_try_to_cancel(&alrm->u.hrt);
	return ret;
}

static void devalarm_cancel(struct devalarm *alrm)
{
	if (is_wakeup(alrm->type))
		alarm_cancel(&alrm->u.alrm);
	else
		hrtimer_cancel(&alrm->u.hrt);
}

#if defined(CONFIG_RTC_CHN_ALARM_BOOT)
#define ANDROID_ALARM_SET_ALARM		_IOW('a', 7, struct timespec)

#define BOOTALM_BIT_EN       0
#define BOOTALM_BIT_YEAR     1
#define BOOTALM_BIT_MONTH    5
#define BOOTALM_BIT_DAY      7
#define BOOTALM_BIT_HOUR     9
#define BOOTALM_BIT_MIN     11
#define BOOTALM_BIT_TOTAL   13

void alarm_set_bootalarm(char *alarm_data)
{
	int ret;
	struct rtc_wkalrm alm;
	char buf_ptr[BOOTALM_BIT_TOTAL+1];

	printk("alarm_set_bootalarm\n");

	if (!pwr_rtc_dev) {
		pr_alarm(INFO,
			"alarm_set_alarm: no RTC, time will be lost on reboot\n");
		return;
	}

	pr_info("BSY STAR : alarm_set_alarm using AlarmManager!\n");
	strlcpy(buf_ptr, alarm_data, BOOTALM_BIT_TOTAL+1);

	alm.time.tm_sec = 0;
	alm.time.tm_min  =  (buf_ptr[BOOTALM_BIT_MIN] - '0') * 10
		+ (buf_ptr[BOOTALM_BIT_MIN+1] - '0');
	alm.time.tm_hour =  (buf_ptr[BOOTALM_BIT_HOUR] - '0') * 10
		+ (buf_ptr[BOOTALM_BIT_HOUR+1] - '0');
	alm.time.tm_mday =  (buf_ptr[BOOTALM_BIT_DAY] - '0') * 10
		+ (buf_ptr[BOOTALM_BIT_DAY+1] - '0');
	alm.time.tm_mon  =  (buf_ptr[BOOTALM_BIT_MONTH] - '0') * 10
		+ (buf_ptr[BOOTALM_BIT_MONTH+1] - '0');
	alm.time.tm_year =  (buf_ptr[BOOTALM_BIT_YEAR] - '0') * 1000
		+ (buf_ptr[BOOTALM_BIT_YEAR+1] - '0') * 100
		+ (buf_ptr[BOOTALM_BIT_YEAR+2] - '0') * 10
		+ (buf_ptr[BOOTALM_BIT_YEAR+3] - '0');
	alm.enabled = (*buf_ptr == '1');

	if (alm.enabled) {
		alm.time.tm_mon -= 1;
		alm.time.tm_year -= 1900;
	} else {
#if 0	
		alm.enabled = 1;
		alm.time.tm_year = 99;
		alm.time.tm_mon = 0;
		alm.time.tm_mday = 1;
		alm.time.tm_hour = 0;
		alm.time.tm_min = 0;
		alm.time.tm_sec = 0;
#else
                printk("alarm_set_bootalarm alarm disable\n");	
		/* set temp time to clear bootalarm  */
		alm.time.tm_year = 2000;
		alm.time.tm_mon	= 1;
		alm.time.tm_mday = 1;
		alm.time.tm_hour = 0;
		alm.time.tm_min	= 0;
		alm.time.tm_sec = 0;

		alm.time.tm_mon -= 1;
		alm.time.tm_year -= 1900;
#endif

	}


	rtc_set_alarm(pwr_rtc_dev, &alm);
}
#endif
=======

static struct android_alarm alarms[ANDROID_ALARM_TYPE_COUNT];
>>>>>>> v3.4.6

static long alarm_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int rv = 0;
	unsigned long flags;
	struct timespec new_alarm_time;
	struct timespec new_rtc_time;
	struct timespec tmp_time;
<<<<<<< HEAD
	struct rtc_time new_rtc_tm;
	struct rtc_device *rtc_dev;
	enum android_alarm_type alarm_type = ANDROID_ALARM_IOCTL_TO_TYPE(cmd);
	uint32_t alarm_type_mask = 1U << alarm_type;


=======
	enum android_alarm_type alarm_type = ANDROID_ALARM_IOCTL_TO_TYPE(cmd);
	uint32_t alarm_type_mask = 1U << alarm_type;

>>>>>>> v3.4.6
	if (alarm_type >= ANDROID_ALARM_TYPE_COUNT)
		return -EINVAL;

	if (ANDROID_ALARM_BASE_CMD(cmd) != ANDROID_ALARM_GET_TIME(0)) {
		if ((file->f_flags & O_ACCMODE) == O_RDONLY)
			return -EPERM;
		if (file->private_data == NULL &&
		    cmd != ANDROID_ALARM_SET_RTC) {
			spin_lock_irqsave(&alarm_slock, flags);
			if (alarm_opened) {
				spin_unlock_irqrestore(&alarm_slock, flags);
				return -EBUSY;
			}
			alarm_opened = 1;
			file->private_data = (void *)1;
			spin_unlock_irqrestore(&alarm_slock, flags);
		}
	}

	switch (ANDROID_ALARM_BASE_CMD(cmd)) {
	case ANDROID_ALARM_CLEAR(0):
<<<<<<< HEAD
		switch (alarm_type) {
		case ANDROID_ALARM_POWER_UP:
			/* disable power up alarm interrupt */
			rv = alarm_irq_enable(0);
			break;
		case ANDROID_ALARM_RTC_WAKEUP:
		case ANDROID_ALARM_RTC:
		case ANDROID_ALARM_ELAPSED_REALTIME_WAKEUP:
		case ANDROID_ALARM_ELAPSED_REALTIME:
		case ANDROID_ALARM_SYSTEMTIME:
			spin_lock_irqsave(&alarm_slock, flags);
			pr_alarm(IO, "alarm %d clear\n", alarm_type);
			devalarm_try_to_cancel(&alarms[alarm_type]);
			if (alarm_pending) {
				alarm_pending &= ~alarm_type_mask;
				if (!alarm_pending && !wait_pending)
					wake_unlock(&alarm_wake_lock);
			}
			alarm_enabled &= ~alarm_type_mask;
			spin_unlock_irqrestore(&alarm_slock, flags);
			break;
		default:
			break;
		}
=======
		spin_lock_irqsave(&alarm_slock, flags);
		pr_alarm(IO, "alarm %d clear\n", alarm_type);
		android_alarm_try_to_cancel(&alarms[alarm_type]);
		if (alarm_pending) {
			alarm_pending &= ~alarm_type_mask;
			if (!alarm_pending && !wait_pending)
				wake_unlock(&alarm_wake_lock);
		}
		alarm_enabled &= ~alarm_type_mask;
		spin_unlock_irqrestore(&alarm_slock, flags);
		break;
>>>>>>> v3.4.6

	case ANDROID_ALARM_SET_OLD:
	case ANDROID_ALARM_SET_AND_WAIT_OLD:
		if (get_user(new_alarm_time.tv_sec, (int __user *)arg)) {
			rv = -EFAULT;
			goto err1;
		}
		new_alarm_time.tv_nsec = 0;
		goto from_old_alarm_set;

<<<<<<< HEAD
#if defined(CONFIG_RTC_CHN_ALARM_BOOT)
	case ANDROID_ALARM_SET_ALARM:
	{
		char bootalarm_data[14];

		if (copy_from_user(bootalarm_data, (void __user *)arg, 14)) {
			rv = -EFAULT;
			goto err1;
		}

		alarm_set_bootalarm(bootalarm_data);	

		break;
	}
#endif
=======
>>>>>>> v3.4.6
	case ANDROID_ALARM_SET_AND_WAIT(0):
	case ANDROID_ALARM_SET(0):
		if (copy_from_user(&new_alarm_time, (void __user *)arg,
		    sizeof(new_alarm_time))) {
			rv = -EFAULT;
			goto err1;
		}
from_old_alarm_set:
		spin_lock_irqsave(&alarm_slock, flags);
		pr_alarm(IO, "alarm %d set %ld.%09ld\n", alarm_type,
			new_alarm_time.tv_sec, new_alarm_time.tv_nsec);
		alarm_enabled |= alarm_type_mask;
<<<<<<< HEAD
		devalarm_start(&alarms[alarm_type],
			timespec_to_ktime(new_alarm_time));
		spin_unlock_irqrestore(&alarm_slock, flags);
		if (alarm_type == ANDROID_ALARM_POWER_UP)
			alarm_set_rtc_ring(new_alarm_time);
=======
		android_alarm_start_range(&alarms[alarm_type],
			timespec_to_ktime(new_alarm_time),
			timespec_to_ktime(new_alarm_time));
		spin_unlock_irqrestore(&alarm_slock, flags);
>>>>>>> v3.4.6
		if (ANDROID_ALARM_BASE_CMD(cmd) != ANDROID_ALARM_SET_AND_WAIT(0)
		    && cmd != ANDROID_ALARM_SET_AND_WAIT_OLD)
			break;
		/* fall though */
	case ANDROID_ALARM_WAIT:
		spin_lock_irqsave(&alarm_slock, flags);
		pr_alarm(IO, "alarm wait\n");
		if (!alarm_pending && wait_pending) {
			wake_unlock(&alarm_wake_lock);
			wait_pending = 0;
		}
		spin_unlock_irqrestore(&alarm_slock, flags);
		rv = wait_event_interruptible(alarm_wait_queue, alarm_pending);
		if (rv)
			goto err1;
		spin_lock_irqsave(&alarm_slock, flags);
		rv = alarm_pending;
		wait_pending = 1;
		alarm_pending = 0;
		spin_unlock_irqrestore(&alarm_slock, flags);
		break;
	case ANDROID_ALARM_SET_RTC:
		if (copy_from_user(&new_rtc_time, (void __user *)arg,
		    sizeof(new_rtc_time))) {
			rv = -EFAULT;
			goto err1;
		}
<<<<<<< HEAD
		rtc_time_to_tm(new_rtc_time.tv_sec, &new_rtc_tm);
		rtc_dev = alarmtimer_get_rtcdev();
		rv = do_settimeofday(&new_rtc_time);
		if (rv < 0)
			goto err1;
		if (rtc_dev)
			rv = rtc_set_time(rtc_dev, &new_rtc_tm);

		if (pwr_rtc_dev)
			rv = rtc_set_time(pwr_rtc_dev, &new_rtc_tm);

=======
		rv = android_alarm_set_rtc(new_rtc_time);
>>>>>>> v3.4.6
		spin_lock_irqsave(&alarm_slock, flags);
		alarm_pending |= ANDROID_ALARM_TIME_CHANGE_MASK;
		wake_up(&alarm_wait_queue);
		spin_unlock_irqrestore(&alarm_slock, flags);
		if (rv < 0)
			goto err1;
		break;
	case ANDROID_ALARM_GET_TIME(0):
		switch (alarm_type) {
		case ANDROID_ALARM_RTC_WAKEUP:
<<<<<<< HEAD
		case ANDROID_ALARM_POWER_UP:
=======
>>>>>>> v3.4.6
		case ANDROID_ALARM_RTC:
			getnstimeofday(&tmp_time);
			break;
		case ANDROID_ALARM_ELAPSED_REALTIME_WAKEUP:
		case ANDROID_ALARM_ELAPSED_REALTIME:
<<<<<<< HEAD
			get_monotonic_boottime(&tmp_time);
=======
			tmp_time =
				ktime_to_timespec(alarm_get_elapsed_realtime());
>>>>>>> v3.4.6
			break;
		case ANDROID_ALARM_TYPE_COUNT:
		case ANDROID_ALARM_SYSTEMTIME:
			ktime_get_ts(&tmp_time);
			break;
		}
		if (copy_to_user((void __user *)arg, &tmp_time,
		    sizeof(tmp_time))) {
			rv = -EFAULT;
			goto err1;
		}
		break;

	default:
		rv = -EINVAL;
		goto err1;
	}
err1:
	return rv;
}

static int alarm_open(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}

static int alarm_release(struct inode *inode, struct file *file)
{
	int i;
	unsigned long flags;

	spin_lock_irqsave(&alarm_slock, flags);
	if (file->private_data != 0) {
		for (i = 0; i < ANDROID_ALARM_TYPE_COUNT; i++) {
			uint32_t alarm_type_mask = 1U << i;
			if (alarm_enabled & alarm_type_mask) {
				pr_alarm(INFO, "alarm_release: clear alarm, "
					"pending %d\n",
					!!(alarm_pending & alarm_type_mask));
				alarm_enabled &= ~alarm_type_mask;
			}
			spin_unlock_irqrestore(&alarm_slock, flags);
<<<<<<< HEAD
			devalarm_cancel(&alarms[i]);
=======
			android_alarm_cancel(&alarms[i]);
>>>>>>> v3.4.6
			spin_lock_irqsave(&alarm_slock, flags);
		}
		if (alarm_pending | wait_pending) {
			if (alarm_pending)
				pr_alarm(INFO, "alarm_release: clear "
					"pending alarms %x\n", alarm_pending);
			wake_unlock(&alarm_wake_lock);
			wait_pending = 0;
			alarm_pending = 0;
		}
		alarm_opened = 0;
	}
	spin_unlock_irqrestore(&alarm_slock, flags);
	return 0;
}

<<<<<<< HEAD
static void devalarm_triggered(struct devalarm *alarm)
=======
static void alarm_triggered(struct android_alarm *alarm)
>>>>>>> v3.4.6
{
	unsigned long flags;
	uint32_t alarm_type_mask = 1U << alarm->type;

<<<<<<< HEAD
	pr_alarm(INT, "devalarm_triggered type %d\n", alarm->type);
=======
	pr_alarm(INT, "alarm_triggered type %d\n", alarm->type);
>>>>>>> v3.4.6
	spin_lock_irqsave(&alarm_slock, flags);
	if (alarm_enabled & alarm_type_mask) {
		wake_lock_timeout(&alarm_wake_lock, 5 * HZ);
		alarm_enabled &= ~alarm_type_mask;
		alarm_pending |= alarm_type_mask;
		wake_up(&alarm_wait_queue);
	}
	spin_unlock_irqrestore(&alarm_slock, flags);
}

<<<<<<< HEAD

static enum hrtimer_restart devalarm_hrthandler(struct hrtimer *hrt)
{
	struct devalarm *devalrm = container_of(hrt, struct devalarm, u.hrt);

	devalarm_triggered(devalrm);
	return HRTIMER_NORESTART;
}

static enum alarmtimer_restart devalarm_alarmhandler(struct alarm *alrm,
							ktime_t now)
{
	struct devalarm *devalrm = container_of(alrm, struct devalarm, u.alrm);

	devalarm_triggered(devalrm);
	return ALARMTIMER_NORESTART;
}


=======
>>>>>>> v3.4.6
static const struct file_operations alarm_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = alarm_ioctl,
	.open = alarm_open,
	.release = alarm_release,
};

static struct miscdevice alarm_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "alarm",
	.fops = &alarm_fops,
};

static int __init alarm_dev_init(void)
{
	int err;
	int i;

	err = misc_register(&alarm_device);
	if (err)
		return err;

<<<<<<< HEAD
	pwr_rtc_dev = alarmpwr_get_rtcdev();

	alarm_init(&alarms[ANDROID_ALARM_RTC_WAKEUP].u.alrm,
			ALARM_REALTIME, devalarm_alarmhandler);
	hrtimer_init(&alarms[ANDROID_ALARM_RTC].u.hrt,
			CLOCK_REALTIME, HRTIMER_MODE_ABS);
	alarm_init(&alarms[ANDROID_ALARM_ELAPSED_REALTIME_WAKEUP].u.alrm,
			ALARM_BOOTTIME, devalarm_alarmhandler);
	hrtimer_init(&alarms[ANDROID_ALARM_ELAPSED_REALTIME].u.hrt,
			CLOCK_BOOTTIME, HRTIMER_MODE_ABS);
	hrtimer_init(&alarms[ANDROID_ALARM_SYSTEMTIME].u.hrt,
			CLOCK_MONOTONIC, HRTIMER_MODE_ABS);

	/* power up alarm */
	alarm_init(&alarms[ANDROID_ALARM_POWER_UP].u.alrm,
			ALARM_REALTIME, devalarm_alarmhandler);

	for (i = 0; i < ANDROID_ALARM_TYPE_COUNT; i++) {
		alarms[i].type = i;
		if (!is_wakeup(i))
			alarms[i].u.hrt.function = devalarm_hrthandler;
	}

	wake_lock_init(&alarm_wake_lock, WAKE_LOCK_SUSPEND, "alarm");


=======
	for (i = 0; i < ANDROID_ALARM_TYPE_COUNT; i++)
		android_alarm_init(&alarms[i], i, alarm_triggered);
	wake_lock_init(&alarm_wake_lock, WAKE_LOCK_SUSPEND, "alarm");

>>>>>>> v3.4.6
	return 0;
}

static void  __exit alarm_dev_exit(void)
{
	misc_deregister(&alarm_device);
	wake_lock_destroy(&alarm_wake_lock);
}

module_init(alarm_dev_init);
module_exit(alarm_dev_exit);

