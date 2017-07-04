/****************************************************************

Siano Mobile Silicon, Inc.
MDTV receiver kernel modules.
<<<<<<< HEAD
Copyright(C) 2006-2010, Erez Cohen
=======
Copyright (C) 2005-2009, Uri Shkolnik, Anatoly Greenblat
>>>>>>> v3.4.6

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

 This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

****************************************************************/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/usb.h>
#include <linux/firmware.h>
#include <linux/slab.h>
<<<<<<< HEAD
#include <asm/byteorder.h>
#include <linux/module.h>
=======
#include <linux/module.h>

>>>>>>> v3.4.6
#include "smscoreapi.h"
#include "sms-cards.h"
#include "smsendian.h"

<<<<<<< HEAD
=======
static int sms_dbg;
module_param_named(debug, sms_dbg, int, 0644);
MODULE_PARM_DESC(debug, "set debug level (info=1, adv=2 (or-able))");

>>>>>>> v3.4.6
#define USB1_BUFFER_SIZE		0x1000
#define USB2_BUFFER_SIZE		0x4000

#define MAX_BUFFERS		50
#define MAX_URBS		10

<<<<<<< HEAD
/*
 * smsusb_worker_thread handles all urb's that have completed data reception.
 * These urb's need to be submitted again with a new common buffer allocation.
 * The worker thread, which handles the surb's list g_smsusb_surbs, is called
 * from an interrupt context (smsusb_onresponse) and implememtns the bottom
 * half of the interrupt handler in a thread context (allowing blocking
 * operations like schedule()).
 */
struct list_head g_smsusb_surbs;
spinlock_t g_surbs_lock;
static void smsusb_worker_thread(void *arg);
static DECLARE_WORK(smsusb_work_queue, (void *)smsusb_worker_thread);

enum smsusb_state {
	SMSUSB_DISCONNECTED,
	SMSUSB_SUSPENDED,
	SMSUSB_ACTIVE
};

struct smsusb_device_t;

struct smsusb_urb_t {
	struct list_head entry;
	struct smscore_buffer_t *cb;
	struct smsusb_device_t *dev;
=======
struct smsusb_device_t;

struct smsusb_urb_t {
	struct smscore_buffer_t *cb;
	struct smsusb_device_t	*dev;
>>>>>>> v3.4.6

	struct urb urb;
};

struct smsusb_device_t {
	struct usb_device *udev;
	struct smscore_device_t *coredev;
<<<<<<< HEAD
	struct smsusb_urb_t surbs[MAX_URBS];
	int surbs_active;

	int response_alignment;
	int buffer_size;
	unsigned char in_ep;
	unsigned char out_ep;
	enum smsusb_state state;
=======

	struct smsusb_urb_t 	surbs[MAX_URBS];

	int		response_alignment;
	int		buffer_size;
>>>>>>> v3.4.6
};

static int smsusb_submit_urb(struct smsusb_device_t *dev,
			     struct smsusb_urb_t *surb);

<<<<<<< HEAD
/**
 * Completing URB's callback handler - top half (interrupt context)

 * adds completing sms urb to the global surbs list and activtes the
 * worker thread the surb
 * IMPORTANT - blocking functions must not be called from here !!!

 * @param urb pointer to a completing urb object
 */

static void smsusb_onresponse(struct urb *urb)
{
	struct smsusb_urb_t *surb = (struct smsusb_urb_t *)urb->context;
	unsigned long flags;

	spin_lock_irqsave(&g_surbs_lock, flags);
	list_add_tail(&surb->entry, &g_smsusb_surbs);
	spin_unlock_irqrestore(&g_surbs_lock, flags);
	schedule_work(&smsusb_work_queue);
}

/**
 * Completing Urb's callback handler - bottom half (worker thread context)
 *
 * 1. sends old core buffer to smscore.
 *    assumes that after return from smsmcore_onsresponse, the core buffer is
 *    either on pendint data (smschar) or in available buffers list
 *    (smschar, smsdvb)
 * 2. acquires new available core buffer and submits the urb to the usb core
 *
 * @param surb pointer to a completing surb object
 */
static void smsusb_handle_surb(struct smsusb_urb_t *surb)
{
	struct smsusb_device_t *dev = surb->dev;
	struct urb *urb = &surb->urb;
=======
static void smsusb_onresponse(struct urb *urb)
{
	struct smsusb_urb_t *surb = (struct smsusb_urb_t *) urb->context;
	struct smsusb_device_t *dev = surb->dev;
>>>>>>> v3.4.6

	if (urb->status == -ESHUTDOWN) {
		sms_err("error, urb status %d (-ESHUTDOWN), %d bytes",
			urb->status, urb->actual_length);
		return;
	}

<<<<<<< HEAD
	/*
	 * in case that the urb was killed during
     * smsusb_stop_streaming, the status is ENOENT
     */
	if (urb->status == -ENOENT) {
		sms_err("error, urb status %d (-ENOENT), %d bytes",
			urb->status, urb->actual_length);
		return;
	}

	if (!dev->surbs_active) {
		sms_err("error, surbs are not active, urb status %d , %d bytes",
			urb->status, urb->actual_length);
		return;
	}

=======
>>>>>>> v3.4.6
	if ((urb->actual_length > 0) && (urb->status == 0)) {
		struct SmsMsgHdr_ST *phdr = (struct SmsMsgHdr_ST *)surb->cb->p;

		smsendian_handle_message_header(phdr);
		if (urb->actual_length >= phdr->msgLength) {
			surb->cb->size = phdr->msgLength;

			if (dev->response_alignment &&
			    (phdr->msgFlags & MSG_HDR_FLAG_SPLIT_MSG)) {

				surb->cb->offset =
<<<<<<< HEAD
				    dev->response_alignment +
				    ((phdr->msgFlags >> 8) & 3);

				/* sanity check */
				if (((int)phdr->msgLength +
=======
					dev->response_alignment +
					((phdr->msgFlags >> 8) & 3);

				/* sanity check */
				if (((int) phdr->msgLength +
>>>>>>> v3.4.6
				     surb->cb->offset) > urb->actual_length) {
					sms_err("invalid response "
						"msglen %d offset %d "
						"size %d",
						phdr->msgLength,
						surb->cb->offset,
						urb->actual_length);
<<<<<<< HEAD
					goto resubmit_and_exit;
=======
					goto exit_and_resubmit;
>>>>>>> v3.4.6
				}

				/* move buffer pointer and
				 * copy header to its new location */
<<<<<<< HEAD
				memcpy((char *)phdr + surb->cb->offset,
=======
				memcpy((char *) phdr + surb->cb->offset,
>>>>>>> v3.4.6
				       phdr, sizeof(struct SmsMsgHdr_ST));
			} else
				surb->cb->offset = 0;

			smscore_onresponse(dev->coredev, surb->cb);
			surb->cb = NULL;
		} else {
			sms_err("invalid response "
				"msglen %d actual %d",
				phdr->msgLength, urb->actual_length);
		}
	} else
		sms_err("error, urb status %d, %d bytes",
			urb->status, urb->actual_length);

<<<<<<< HEAD
resubmit_and_exit:
	if (smsusb_submit_urb(dev, surb) < 0) {
		/* */
		sms_err("smsusb_submit_urb failed");
	}

}

/**
 * Completing Urb's callback handler - bottom half (worker thread context)
 *
 * extracts and handles sms urb from the global surbs list (fifo).
 * access to the list is locked. however, the lock is released during
 * surb handle.
 * in practice, new surbs may be added to the list during the worker's run
 *
 * @param args not used
 */
static void smsusb_worker_thread(void *args)
{
	struct smsusb_urb_t *surb;
	unsigned long flags;

	spin_lock_irqsave(&g_surbs_lock, flags);

	while (!list_empty(&g_smsusb_surbs)) {
		surb = (struct smsusb_urb_t *) g_smsusb_surbs.next;
		list_del(&surb->entry);

		/*
		 * release the lock, since smsusb_submit_urb might block
		 */
		spin_unlock_irqrestore(&g_surbs_lock, flags);

		smsusb_handle_surb(surb);

		/*
		 * acquire the lock again
		 */
		spin_lock_irqsave(&g_surbs_lock, flags);
	}

	spin_unlock_irqrestore(&g_surbs_lock, flags);

=======

exit_and_resubmit:
	smsusb_submit_urb(dev, surb);
>>>>>>> v3.4.6
}

static int smsusb_submit_urb(struct smsusb_device_t *dev,
			     struct smsusb_urb_t *surb)
{
	if (!surb->cb) {
		surb->cb = smscore_getbuffer(dev->coredev);
		if (!surb->cb) {
			sms_err("smscore_getbuffer(...) returned NULL");
			return -ENOMEM;
		}
	}

	usb_fill_bulk_urb(
		&surb->urb,
		dev->udev,
<<<<<<< HEAD
		usb_rcvbulkpipe(dev->udev, dev->in_ep),
=======
		usb_rcvbulkpipe(dev->udev, 0x81),
>>>>>>> v3.4.6
		surb->cb->p,
		dev->buffer_size,
		smsusb_onresponse,
		surb
	);
	surb->urb.transfer_dma = surb->cb->phys;
	surb->urb.transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	return usb_submit_urb(&surb->urb, GFP_ATOMIC);
}

static void smsusb_stop_streaming(struct smsusb_device_t *dev)
{
	int i;

<<<<<<< HEAD
	dev->surbs_active = 0;
=======
>>>>>>> v3.4.6
	for (i = 0; i < MAX_URBS; i++) {
		usb_kill_urb(&dev->surbs[i].urb);

		if (dev->surbs[i].cb) {
			smscore_putbuffer(dev->coredev, dev->surbs[i].cb);
			dev->surbs[i].cb = NULL;
		}
	}
}

static int smsusb_start_streaming(struct smsusb_device_t *dev)
{
	int i, rc;

<<<<<<< HEAD
	dev->surbs_active = 1;
=======
>>>>>>> v3.4.6
	for (i = 0; i < MAX_URBS; i++) {
		rc = smsusb_submit_urb(dev, &dev->surbs[i]);
		if (rc < 0) {
			sms_err("smsusb_submit_urb(...) failed");
			smsusb_stop_streaming(dev);
			break;
		}
	}

	return rc;
}

static int smsusb_sendrequest(void *context, void *buffer, size_t size)
{
<<<<<<< HEAD
	struct smsusb_device_t *dev = (struct smsusb_device_t *)context;
	int dummy;

	if (dev->state != SMSUSB_ACTIVE)
		return -ENOENT;

	smsendian_handle_message_header((struct SmsMsgHdr_ST *)buffer);
	return usb_bulk_msg(dev->udev, usb_sndbulkpipe(dev->udev, dev->out_ep),
=======
	struct smsusb_device_t *dev = (struct smsusb_device_t *) context;
	int dummy;

	smsendian_handle_message_header((struct SmsMsgHdr_ST *)buffer);
	return usb_bulk_msg(dev->udev, usb_sndbulkpipe(dev->udev, 2),
>>>>>>> v3.4.6
			    buffer, size, &dummy, 1000);
}

static char *smsusb1_fw_lkup[] = {
	"dvbt_stellar_usb.inp",
	"dvbh_stellar_usb.inp",
	"tdmb_stellar_usb.inp",
	"none",
	"dvbt_bda_stellar_usb.inp",
};

<<<<<<< HEAD
static inline char *smsusb1_get_fw_name(int mode, int board_id)
=======
static inline char *sms_get_fw_name(int mode, int board_id)
>>>>>>> v3.4.6
{
	char **fw = sms_get_board(board_id)->fw;
	return (fw && fw[mode]) ? fw[mode] : smsusb1_fw_lkup[mode];
}

static int smsusb1_load_firmware(struct usb_device *udev, int id, int board_id)
{
	const struct firmware *fw;
	u8 *fw_buffer;
	int rc, dummy;
	char *fw_filename;

	if (id < DEVICE_MODE_DVBT || id > DEVICE_MODE_DVBT_BDA) {
		sms_err("invalid firmware id specified %d", id);
		return -EINVAL;
	}

<<<<<<< HEAD
	fw_filename = smsusb1_get_fw_name(id, board_id);
=======
	fw_filename = sms_get_fw_name(id, board_id);
>>>>>>> v3.4.6

	rc = request_firmware(&fw, fw_filename, &udev->dev);
	if (rc < 0) {
		sms_warn("failed to open \"%s\" mode %d, "
			 "trying again with default firmware", fw_filename, id);

		fw_filename = smsusb1_fw_lkup[id];
		rc = request_firmware(&fw, fw_filename, &udev->dev);
		if (rc < 0) {
			sms_warn("failed to open \"%s\" mode %d",
				 fw_filename, id);

			return rc;
		}
	}

	fw_buffer = kmalloc(fw->size, GFP_KERNEL);
	if (fw_buffer) {
		memcpy(fw_buffer, fw->data, fw->size);

		rc = usb_bulk_msg(udev, usb_sndbulkpipe(udev, 2),
				  fw_buffer, fw->size, &dummy, 1000);

		sms_info("sent %zd(%d) bytes, rc %d", fw->size, dummy, rc);

		kfree(fw_buffer);
	} else {
		sms_err("failed to allocate firmware buffer");
		rc = -ENOMEM;
	}
	sms_info("read FW %s, size=%zd", fw_filename, fw->size);

	release_firmware(fw);

	return rc;
}

static void smsusb1_detectmode(void *context, int *mode)
{
	char *product_string =
<<<<<<< HEAD
	    ((struct smsusb_device_t *)context)->udev->product;
=======
		((struct smsusb_device_t *) context)->udev->product;
>>>>>>> v3.4.6

	*mode = DEVICE_MODE_NONE;

	if (!product_string) {
		product_string = "none";
		sms_err("product string not found");
	} else if (strstr(product_string, "DVBH"))
		*mode = 1;
	else if (strstr(product_string, "BDA"))
		*mode = 4;
	else if (strstr(product_string, "DVBT"))
		*mode = 0;
	else if (strstr(product_string, "TDMB"))
		*mode = 2;

	sms_info("%d \"%s\"", *mode, product_string);
}

static int smsusb1_setmode(void *context, int mode)
{
	struct SmsMsgHdr_ST Msg = { MSG_SW_RELOAD_REQ, 0, HIF_TASK,
			     sizeof(struct SmsMsgHdr_ST), 0 };

	if (mode < DEVICE_MODE_DVBT || mode > DEVICE_MODE_DVBT_BDA) {
		sms_err("invalid firmware id specified %d", mode);
		return -EINVAL;
	}

	return smsusb_sendrequest(context, &Msg, sizeof(Msg));
}

static void smsusb_term_device(struct usb_interface *intf)
{
<<<<<<< HEAD
	struct smsusb_device_t *dev =
	    (struct smsusb_device_t *)usb_get_intfdata(intf);

	if (dev) {
		dev->state = SMSUSB_DISCONNECTED;

=======
	struct smsusb_device_t *dev = usb_get_intfdata(intf);

	if (dev) {
>>>>>>> v3.4.6
		smsusb_stop_streaming(dev);

		/* unregister from smscore */
		if (dev->coredev)
			smscore_unregister_device(dev->coredev);

<<<<<<< HEAD
		kfree(dev);

		sms_info("device 0x%p destroyed", dev);
=======
		sms_info("device %p destroyed", dev);
		kfree(dev);
>>>>>>> v3.4.6
	}

	usb_set_intfdata(intf, NULL);
}

static int smsusb_init_device(struct usb_interface *intf, int board_id)
{
	struct smsdevice_params_t params;
	struct smsusb_device_t *dev;
	int i, rc;

	/* create device object */
	dev = kzalloc(sizeof(struct smsusb_device_t), GFP_KERNEL);
	if (!dev) {
		sms_err("kzalloc(sizeof(struct smsusb_device_t) failed");
		return -ENOMEM;
	}

	memset(&params, 0, sizeof(params));
	usb_set_intfdata(intf, dev);
	dev->udev = interface_to_usbdev(intf);
<<<<<<< HEAD
	dev->surbs_active = 0;
	dev->state = SMSUSB_DISCONNECTED;
=======
>>>>>>> v3.4.6

	params.device_type = sms_get_board(board_id)->type;

	switch (params.device_type) {
	case SMS_STELLAR:
		dev->buffer_size = USB1_BUFFER_SIZE;

		params.setmode_handler = smsusb1_setmode;
		params.detectmode_handler = smsusb1_detectmode;
		break;
	default:
		sms_err("Unspecified sms device type!");
		/* fall-thru */
	case SMS_NOVA_A0:
	case SMS_NOVA_B0:
	case SMS_VEGA:
<<<<<<< HEAD
	case SMS_VENICE:
=======
>>>>>>> v3.4.6
		dev->buffer_size = USB2_BUFFER_SIZE;
		dev->response_alignment =
		    le16_to_cpu(dev->udev->ep_in[1]->desc.wMaxPacketSize) -
		    sizeof(struct SmsMsgHdr_ST);

		params.flags |= SMS_DEVICE_FAMILY2;
		break;
	}

<<<<<<< HEAD
	for (i = 0; i < intf->cur_altsetting->desc.bNumEndpoints; i++)
		if (intf->cur_altsetting->endpoint[i].desc.
			bEndpointAddress & USB_DIR_IN)
			dev->in_ep = intf->cur_altsetting->endpoint[i].desc.
			bEndpointAddress;
		else
			dev->out_ep = intf->cur_altsetting->endpoint[i].desc.
			bEndpointAddress;

	sms_info("in_ep = %02x, out_ep = %02x",
		dev->in_ep, dev->out_ep);

=======
>>>>>>> v3.4.6
	params.device = &dev->udev->dev;
	params.buffer_size = dev->buffer_size;
	params.num_buffers = MAX_BUFFERS;
	params.sendrequest_handler = smsusb_sendrequest;
	params.context = dev;
<<<<<<< HEAD
	snprintf(params.devpath, sizeof(params.devpath),
		 "usb\\%d-%s", dev->udev->bus->busnum, dev->udev->devpath);
=======
	usb_make_path(dev->udev, params.devpath, sizeof(params.devpath));
>>>>>>> v3.4.6

	/* register in smscore */
	rc = smscore_register_device(&params, &dev->coredev);
	if (rc < 0) {
		sms_err("smscore_register_device(...) failed, rc %d", rc);
		smsusb_term_device(intf);
		return rc;
	}

	smscore_set_board_id(dev->coredev, board_id);

	/* initialize urbs */
	for (i = 0; i < MAX_URBS; i++) {
		dev->surbs[i].dev = dev;
		usb_init_urb(&dev->surbs[i].urb);
	}

<<<<<<< HEAD
	rc = smsusb_start_streaming(dev);
	if (rc < 0) {
		sms_err("smsusb_start_streaming failed");
=======
	sms_info("smsusb_start_streaming(...).");
	rc = smsusb_start_streaming(dev);
	if (rc < 0) {
		sms_err("smsusb_start_streaming(...) failed");
>>>>>>> v3.4.6
		smsusb_term_device(intf);
		return rc;
	}

<<<<<<< HEAD
	dev->state = SMSUSB_ACTIVE;

	rc = smscore_start_device(dev->coredev);
	if (rc < 0) {
		sms_err("smscore_start_device ailed");
=======
	rc = smscore_start_device(dev->coredev);
	if (rc < 0) {
		sms_err("smscore_start_device(...) failed");
>>>>>>> v3.4.6
		smsusb_term_device(intf);
		return rc;
	}

<<<<<<< HEAD
	sms_info("device 0x%p created", dev);
=======
	sms_info("device %p created", dev);
>>>>>>> v3.4.6

	return rc;
}

<<<<<<< HEAD
static int smsusb_probe(struct usb_interface *intf,
=======
static int __devinit smsusb_probe(struct usb_interface *intf,
>>>>>>> v3.4.6
			const struct usb_device_id *id)
{
	struct usb_device *udev = interface_to_usbdev(intf);
	char devpath[32];
	int i, rc;

<<<<<<< HEAD
	sms_info("interface number %d",
		 intf->cur_altsetting->desc.bInterfaceNumber);

	if (sms_get_board(id->driver_info)->intf_num !=
		intf->cur_altsetting->desc.bInterfaceNumber) {
		sms_err("interface number is %d expecting %d",
			sms_get_board(id->driver_info)->intf_num,
			intf->cur_altsetting->desc.bInterfaceNumber);
		return -ENODEV;
	}

	if (intf->num_altsetting > 1) {
		rc = usb_set_interface(udev,
				intf->cur_altsetting->desc.
				bInterfaceNumber, 0);
=======
	rc = usb_clear_halt(udev, usb_rcvbulkpipe(udev, 0x81));
	rc = usb_clear_halt(udev, usb_rcvbulkpipe(udev, 0x02));

	if (intf->num_altsetting > 0) {
		rc = usb_set_interface(
			udev, intf->cur_altsetting->desc.bInterfaceNumber, 0);
>>>>>>> v3.4.6
		if (rc < 0) {
			sms_err("usb_set_interface failed, rc %d", rc);
			return rc;
		}
	}

<<<<<<< HEAD
	for (i = 0; i < intf->cur_altsetting->desc.bNumEndpoints; i++) {
		sms_info("endpoint %d bEndpointAddress=%02x",
			i,
			intf->cur_altsetting->endpoint[i].desc.bEndpointAddress
			);
		sms_info("bmAttributes=%02x wMaxPacketSize=%d",
			intf->cur_altsetting->endpoint[i].desc.bmAttributes,
			intf->cur_altsetting->endpoint[i].desc.wMaxPacketSize);
		if (intf->cur_altsetting->endpoint[i].desc.
			bEndpointAddress & USB_DIR_IN)
			rc = usb_clear_halt(udev, usb_rcvbulkpipe(udev,
				intf->cur_altsetting->endpoint[i].desc.
				bEndpointAddress));
		else
			rc = usb_clear_halt(udev, usb_sndbulkpipe(udev,
				intf->cur_altsetting->endpoint[i].desc.
				bEndpointAddress));
	}

	if (id->driver_info == SMS1XXX_BOARD_SIANO_STELLAR_ROM) {
		sms_info("stellar device was found.");
		snprintf(devpath, sizeof(devpath), "usb\\%d-%s",
			 udev->bus->busnum, udev->devpath);
		return smsusb1_load_firmware(udev,
					     smscore_registry_getmode(devpath),
					     id->driver_info);
=======
	sms_info("smsusb_probe %d",
	       intf->cur_altsetting->desc.bInterfaceNumber);
	for (i = 0; i < intf->cur_altsetting->desc.bNumEndpoints; i++)
		sms_info("endpoint %d %02x %02x %d", i,
		       intf->cur_altsetting->endpoint[i].desc.bEndpointAddress,
		       intf->cur_altsetting->endpoint[i].desc.bmAttributes,
		       intf->cur_altsetting->endpoint[i].desc.wMaxPacketSize);

	if ((udev->actconfig->desc.bNumInterfaces == 2) &&
	    (intf->cur_altsetting->desc.bInterfaceNumber == 0)) {
		sms_err("rom interface 0 is not used");
		return -ENODEV;
	}

	if (intf->cur_altsetting->desc.bInterfaceNumber == 1) {
		snprintf(devpath, sizeof(devpath), "usb\\%d-%s",
			 udev->bus->busnum, udev->devpath);
		sms_info("stellar device was found.");
		return smsusb1_load_firmware(
				udev, smscore_registry_getmode(devpath),
				id->driver_info);
>>>>>>> v3.4.6
	}

	rc = smsusb_init_device(intf, id->driver_info);
	sms_info("rc %d", rc);
<<<<<<< HEAD
	return rc;
}

static void smsusb_flush_surbs(struct usb_interface *intf)
{
	struct smsusb_device_t *dev =
	    (struct smsusb_device_t *)usb_get_intfdata(intf);
	struct list_head *next;
	struct smsusb_urb_t *surb;
	unsigned long flags;

	sms_debug("");
	spin_lock_irqsave(&g_surbs_lock, flags);
	for (next = (struct list_head *)&g_smsusb_surbs;
			next != &g_smsusb_surbs; next = next->next) {
		surb = (struct smsusb_urb_t *) next;
		sms_debug("surb->dev = 0x%p, dev = 0x%p", surb->dev, dev);
		if (surb->dev == dev)
			list_del(&surb->entry);
	}
	spin_unlock_irqrestore(&g_surbs_lock, flags);
}

static void smsusb_disconnect(struct usb_interface *intf)
{
	smsusb_flush_surbs(intf);
=======
	sms_board_load_modules(id->driver_info);
	return rc;
}

static void smsusb_disconnect(struct usb_interface *intf)
{
>>>>>>> v3.4.6
	smsusb_term_device(intf);
}

static int smsusb_suspend(struct usb_interface *intf, pm_message_t msg)
{
<<<<<<< HEAD
	struct smsusb_device_t *dev =
	    (struct smsusb_device_t *)usb_get_intfdata(intf);
	printk(KERN_INFO "%s  Entering status %d.\n", __func__, msg.event);
	dev->state = SMSUSB_SUSPENDED;
	/*smscore_set_power_mode(dev, SMS_POWER_MODE_SUSPENDED);*/
=======
	struct smsusb_device_t *dev = usb_get_intfdata(intf);
	printk(KERN_INFO "%s: Entering status %d.\n", __func__, msg.event);
>>>>>>> v3.4.6
	smsusb_stop_streaming(dev);
	return 0;
}

static int smsusb_resume(struct usb_interface *intf)
{
	int rc, i;
<<<<<<< HEAD
	struct smsusb_device_t *dev =
	    (struct smsusb_device_t *)usb_get_intfdata(intf);
	struct usb_device *udev = interface_to_usbdev(intf);

	printk(KERN_INFO "%s  Entering.\n", __func__);
	usb_clear_halt(udev, usb_rcvbulkpipe(udev, dev->in_ep));
	usb_clear_halt(udev, usb_sndbulkpipe(udev, dev->out_ep));
=======
	struct smsusb_device_t *dev = usb_get_intfdata(intf);
	struct usb_device *udev = interface_to_usbdev(intf);

	printk(KERN_INFO "%s: Entering.\n", __func__);
	usb_clear_halt(udev, usb_rcvbulkpipe(udev, 0x81));
	usb_clear_halt(udev, usb_rcvbulkpipe(udev, 0x02));
>>>>>>> v3.4.6

	for (i = 0; i < intf->cur_altsetting->desc.bNumEndpoints; i++)
		printk(KERN_INFO "endpoint %d %02x %02x %d\n", i,
		       intf->cur_altsetting->endpoint[i].desc.bEndpointAddress,
		       intf->cur_altsetting->endpoint[i].desc.bmAttributes,
		       intf->cur_altsetting->endpoint[i].desc.wMaxPacketSize);

	if (intf->num_altsetting > 0) {
		rc = usb_set_interface(udev,
				       intf->cur_altsetting->desc.
				       bInterfaceNumber, 0);
		if (rc < 0) {
<<<<<<< HEAD
			printk(KERN_INFO
			       "%s usb_set_interface failed, rc %d\n",
			       __func__, rc);
=======
			printk(KERN_INFO "%s usb_set_interface failed, "
			       "rc %d\n", __func__, rc);
>>>>>>> v3.4.6
			return rc;
		}
	}

<<<<<<< HEAD
	dev->state = SMSUSB_ACTIVE;
	rc = smsusb_start_streaming(dev);
	if (rc < 0) {
		sms_err("smsusb_start_streaming, error %d", rc);
		smsusb_term_device(intf);
	}
	smscore_set_power_mode(SMS_POWER_MODE_ACTIVE);
	return rc;
}

static struct usb_driver smsusb_driver = {
	.name = "smsusb",
	.probe = smsusb_probe,
	.disconnect = smsusb_disconnect,
	.suspend = smsusb_suspend,
	.resume = smsusb_resume,
	.id_table = smsusb_id_table,
};

int smsusb_register(void)
{
	int rc;

	INIT_LIST_HEAD(&g_smsusb_surbs);
	spin_lock_init(&g_surbs_lock);

	rc = usb_register(&smsusb_driver);
	if (rc)
		sms_err("usb_register failed. Error number %d", rc);

	return rc;
}

void smsusb_unregister(void)
{
	/* Regular USB Cleanup */
	usb_deregister(&smsusb_driver);
}

MODULE_DESCRIPTION("Driver for the Siano SMS1xxx USB dongle");
MODULE_AUTHOR("Siano Mobile Silicon, INC. (erezc@siano-ms.com)");
=======
	smsusb_start_streaming(dev);
	return 0;
}

static const struct usb_device_id smsusb_id_table[] __devinitconst = {
	{ USB_DEVICE(0x187f, 0x0010),
		.driver_info = SMS1XXX_BOARD_SIANO_STELLAR },
	{ USB_DEVICE(0x187f, 0x0100),
		.driver_info = SMS1XXX_BOARD_SIANO_STELLAR },
	{ USB_DEVICE(0x187f, 0x0200),
		.driver_info = SMS1XXX_BOARD_SIANO_NOVA_A },
	{ USB_DEVICE(0x187f, 0x0201),
		.driver_info = SMS1XXX_BOARD_SIANO_NOVA_B },
	{ USB_DEVICE(0x187f, 0x0300),
		.driver_info = SMS1XXX_BOARD_SIANO_VEGA },
	{ USB_DEVICE(0x2040, 0x1700),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_CATAMOUNT },
	{ USB_DEVICE(0x2040, 0x1800),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_OKEMO_A },
	{ USB_DEVICE(0x2040, 0x1801),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_OKEMO_B },
	{ USB_DEVICE(0x2040, 0x2000),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_TIGER_MINICARD },
	{ USB_DEVICE(0x2040, 0x2009),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_TIGER_MINICARD_R2 },
	{ USB_DEVICE(0x2040, 0x200a),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_TIGER_MINICARD },
	{ USB_DEVICE(0x2040, 0x2010),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_TIGER_MINICARD },
	{ USB_DEVICE(0x2040, 0x2011),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_TIGER_MINICARD },
	{ USB_DEVICE(0x2040, 0x2019),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_TIGER_MINICARD },
	{ USB_DEVICE(0x2040, 0x5500),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_WINDHAM },
	{ USB_DEVICE(0x2040, 0x5510),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_WINDHAM },
	{ USB_DEVICE(0x2040, 0x5520),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_WINDHAM },
	{ USB_DEVICE(0x2040, 0x5530),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_WINDHAM },
	{ USB_DEVICE(0x2040, 0x5580),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_WINDHAM },
	{ USB_DEVICE(0x2040, 0x5590),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_WINDHAM },
	{ USB_DEVICE(0x187f, 0x0202),
		.driver_info = SMS1XXX_BOARD_SIANO_NICE },
	{ USB_DEVICE(0x187f, 0x0301),
		.driver_info = SMS1XXX_BOARD_SIANO_VENICE },
	{ USB_DEVICE(0x2040, 0xb900),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_WINDHAM },
	{ USB_DEVICE(0x2040, 0xb910),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_WINDHAM },
	{ USB_DEVICE(0x2040, 0xb980),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_WINDHAM },
	{ USB_DEVICE(0x2040, 0xb990),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_WINDHAM },
	{ USB_DEVICE(0x2040, 0xc000),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_WINDHAM },
	{ USB_DEVICE(0x2040, 0xc010),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_WINDHAM },
	{ USB_DEVICE(0x2040, 0xc080),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_WINDHAM },
	{ USB_DEVICE(0x2040, 0xc090),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_WINDHAM },
	{ USB_DEVICE(0x2040, 0xc0a0),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_WINDHAM },
	{ USB_DEVICE(0x2040, 0xf5a0),
		.driver_info = SMS1XXX_BOARD_HAUPPAUGE_WINDHAM },
	{ } /* Terminating entry */
	};

MODULE_DEVICE_TABLE(usb, smsusb_id_table);

static struct usb_driver smsusb_driver = {
	.name			= "smsusb",
	.probe			= smsusb_probe,
	.disconnect		= smsusb_disconnect,
	.id_table		= smsusb_id_table,

	.suspend		= smsusb_suspend,
	.resume			= smsusb_resume,
};

module_usb_driver(smsusb_driver);

MODULE_DESCRIPTION("Driver for the Siano SMS1xxx USB dongle");
MODULE_AUTHOR("Siano Mobile Silicon, INC. (uris@siano-ms.com)");
>>>>>>> v3.4.6
MODULE_LICENSE("GPL");
