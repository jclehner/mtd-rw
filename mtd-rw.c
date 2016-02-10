/*
 * mtd-rw - Make all MTD partitions writeable.
 *
 * Copyright (C) 2016 Joseph C. Lehner <joseph.c.lehner@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>

/*
 * Inspired by dougg3@electronics.stackexchange.com:
 * https://electronics.stackexchange.com/a/116133/97342
 */

#ifndef MODULE
#error "Must be compiled as a module."
#endif

#define MOD_INFO KERN_INFO "mtd-rw: "
#define MOD_ERR KERN_ERR "mtd-rw: "

static bool i_want_a_brick = false;
module_param(i_want_a_brick, bool, S_IRUGO);
MODULE_PARM_DESC(i_want_a_brick, "Make all partitions writeable");

static int mtd_max = 0;
static uint64_t unlocked = 0;

static int __init mtd_unlocker_init(void)
{
	struct mtd_info *mtd;
	int i, count;

	if (!i_want_a_brick) {
		printk(MOD_ERR "i_want_a_brick not specified; aborting\n");
		return -EINVAL;
	}

	count = 0;

	for (i = 0; i < 8 * sizeof(unlocked); ++i) {
		mtd = get_mtd_device(NULL, i);
		if (IS_ERR(mtd)) {
			if (PTR_ERR(mtd) != -ENODEV) {
				printk(MOD_ERR "mtd%d: error %ld\n", i, PTR_ERR(mtd));
				continue;
			} else {
				break;
			}
		} else if(!(mtd->flags & MTD_WRITEABLE)) {
			printk(MOD_INFO "mtd%d: setting writeable flag\n", i);
			mtd->flags |= MTD_WRITEABLE;
			unlocked |= (1 << i);
			++count;
		}

		put_mtd_device(mtd);
	}

	if (!unlocked) {
		printk(MOD_INFO "no partitions to unlock\n");
		return -ENODEV;
	}

	mtd_max = i;

	printk(MOD_INFO "unlocked %d partitions\n", count);
	return 0;
}

static void __exit mtd_unlocker_exit(void)
{
	struct mtd_info *mtd;
	int i;

	for (i = 0; i < mtd_max; ++i) {
		if (unlocked & (1 << i)) {
			mtd = get_mtd_device(NULL, i);
			if (IS_ERR(mtd)) {
				printk(MOD_ERR "mtd%d: cannot remove writeable flag\n", i);
				continue;
			} else if (mtd->flags & MTD_WRITEABLE) {
				printk(MOD_ERR "mtd%d: removing writeable flag\n", i);
				mtd->flags &= ~MTD_WRITEABLE;
			}

			put_mtd_device(mtd);
		}
	}
}

module_init(mtd_unlocker_init);
module_exit(mtd_unlocker_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Joseph C. Lehner <joseph.c.lehner@gmail.com>");
MODULE_DESCRIPTION("MTD unlocker");
MODULE_VERSION("1");
