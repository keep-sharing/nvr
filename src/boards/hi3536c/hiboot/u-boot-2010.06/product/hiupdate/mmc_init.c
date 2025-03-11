/*
 * Copyright (c) 2016 HiSilicon Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

static int mmc_stor_init(void)
{
	struct mmc *mmc;

#if (defined CONFIG_HI3516CV300 || defined CONFIG_ARCH_HI3519 || \
		defined CONFIG_ARCH_HI3519V101 || defined CONFIG_ARCH_HI3559 || defined CONFIG_ARCH_HI3556 || \
		defined CONFIG_ARCH_HI3516AV200)
	mmc = find_mmc_device(2);
#else
	mmc = find_mmc_device(0);
#endif
	if (NULL == mmc) {
		printf("No mmc driver found!\n");
		return -1;
	}

	if ((0 == (unsigned int)mmc->block_dev.vendor[0])
			|| (0 == (unsigned int)mmc->block_dev.product[0])) {
		printf("*No SD card found!\n");
		return -1;
	}

	return 0;
}

static void mmc_stor_exit(void)
{
}
