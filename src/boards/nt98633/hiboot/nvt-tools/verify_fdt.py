#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import libfdt
import struct
import json
import argparse
# some oaalnx has no colorama
#import colorama


def dbg_err(msg):
    #print(colorama.Fore.RED+colorama.Style.BRIGHT+msg+colorama.Style.RESET_ALL, file=sys.stderr)
    print(msg, file=sys.stderr)


def dbg_wrn(msg):
    #print(colorama.Fore.YELLOW+colorama.Style.BRIGHT+msg+colorama.Style.RESET_ALL, file=sys.stderr)
    print(msg, file=sys.stderr)


def parse_nvt_info(fdt):
    nvt_info = dict()
    nodeoffset = fdt.path_offset("/nvt_info")
    if nodeoffset < 0:
        dbg_err("unable to find /nvt_info")
        return -1
    nodeoffset = fdt.first_property_offset(nodeoffset, libfdt.QUIET_NOTFOUND)
    while nodeoffset > 0:
        prop = fdt.get_property_by_offset(nodeoffset)
        nvt_info[prop.name] = prop.as_str()
        nodeoffset = fdt.next_property_offset(
            nodeoffset, libfdt.QUIET_NOTFOUND)
    return nvt_info


def parse_flash_partition(fdt, flash_path):
    partitions = []
    nodeoffset = fdt.path_offset(flash_path, libfdt.QUIET_NOTFOUND)
    if nodeoffset < 0:
        return partitions

    nodeoffset = fdt.first_subnode(nodeoffset, libfdt.QUIET_NOTFOUND)
    while nodeoffset > 0:
        name = fdt.get_name(nodeoffset)
        if "partition_" not in name:
            nodeoffset = fdt.next_subnode(nodeoffset, libfdt.QUIET_NOTFOUND)
            continue
        partition = dict()
        prop = fdt.getprop(nodeoffset, "label")
        partition["name"] = name
        partition["label"] = prop.as_str()
        prop = fdt.getprop(nodeoffset, "reg")
        reg = struct.unpack('>QQ', prop)
        partition["ofs"] = reg[0]
        partition["size"] = reg[1]
        partitions.append(partition)
        nodeoffset = fdt.next_subnode(nodeoffset, libfdt.QUIET_NOTFOUND)
    return partitions


def parse_nvt_memory_cfg(fdt):
    nvt_memory_cfgs = []
    nodeoffset = fdt.path_offset(r"/nvt_memory_cfg", libfdt.QUIET_NOTFOUND)
    if nodeoffset < 0:
        return nvt_memory_cfgs

    nodeoffset = fdt.first_subnode(nodeoffset, libfdt.QUIET_NOTFOUND)
    while nodeoffset > 0:
        name = fdt.get_name(nodeoffset)
        nvt_memory_cfg = dict()
        nvt_memory_cfg["name"] = name
        prop = fdt.getprop(nodeoffset, "reg")
        reg = struct.unpack('>LL', prop)
        nvt_memory_cfg["ofs"] = reg[0]
        nvt_memory_cfg["size"] = reg[1]
        nvt_memory_cfgs.append(nvt_memory_cfg)
        nodeoffset = fdt.next_subnode(nodeoffset, libfdt.QUIET_NOTFOUND)
    return nvt_memory_cfgs


def get_reg_array(address_cell, prop):
    reg = []
    if address_cell == 2:  # 64 bit
        if len(prop) % 16 != 0:
            dbg_err("/memory/reg must be 64 bit alignment")
            return reg
        group_cnt = int(len(prop) / 16)
        # each group include 2 Quads (addr, size), so need *2 here
        reg = struct.unpack('>{:d}Q'.format(int(group_cnt*2)), prop)
    else:  # 32 bit
        if len(prop) % 8 != 0:
            dbg_err("/memory/reg must be 32 bit alignment")
            return -1
        group_cnt = int(len(prop) / 8)
        # each group include 2 Words (addr, size), so need *2 here
        reg = struct.unpack('>{:d}L'.format(int(group_cnt*2)), prop)
    return reg


def parse_linux_memory(fdt):
    linux_memories = []
    nodeoffset = fdt.path_offset(r"/", libfdt.QUIET_NOTFOUND)
    address_cell = fdt.getprop(nodeoffset, "#address-cells")

    nodeoffset = fdt.path_offset(r"/memory", libfdt.QUIET_NOTFOUND)
    if nodeoffset < 0:
        dbg_wrn("/memory is not existing, skip it")
        return linux_memories

    prop = fdt.getprop(nodeoffset, "reg")
    reg = get_reg_array(address_cell.as_uint32(), prop)

    for idx in range(int(len(reg)/2)):
        linux_memory = dict()
        linux_memory["ofs"] = reg[idx*2]
        linux_memory["size"] = reg[idx*2+1]
        linux_memories.append(linux_memory)

    return linux_memories


def parse_reserved_memory(fdt):
    reserved_memories = []
    nodeoffset = fdt.path_offset(r"/reserved-memory", libfdt.QUIET_NOTFOUND)
    if nodeoffset < 0:
        dbg_wrn("/reserved-memory is not existing, skip it")
        return reserved_memories

    address_cell = fdt.getprop(
        nodeoffset, "#address-cells", libfdt.QUIET_NOTFOUND)
    if address_cell == -libfdt.FDT_ERR_NOTFOUND:
        dbg_err("/reserved-memory lacks #address-cells")
        return reserved_memories

    nodeoffset = fdt.first_subnode(nodeoffset, libfdt.QUIET_NOTFOUND)
    while nodeoffset > 0:
        name = fdt.get_name(nodeoffset)
        prop = fdt.getprop(nodeoffset, "reg")
        reg = get_reg_array(address_cell.as_uint32(), prop)

        for idx in range(int(len(reg)/2)):
            reserved_memory = dict()
            reserved_memory["name"] = name
            reserved_memory["ofs"] = reg[idx*2]
            reserved_memory["size"] = reg[idx*2+1]
            reserved_memories.append(reserved_memory)

        nodeoffset = fdt.next_subnode(nodeoffset, libfdt.QUIET_NOTFOUND)
    return reserved_memories


def parse_hdal_memory(fdt):
    hdal_memory = dict()

    nodeoffset = fdt.path_offset(r"/hdal-memory", libfdt.QUIET_NOTFOUND)
    if nodeoffset < 0:
        dbg_wrn("/hdal-memory is not existing, skip it")
        return hdal_memory

    address_cell = fdt.getprop(
        nodeoffset, "#address-cells", libfdt.QUIET_NOTFOUND)
    if address_cell == -libfdt.FDT_ERR_NOTFOUND:
        dbg_err("/hdal-memory lacks #address-cells")
        return hdal_memory

    size_cell = fdt.getprop(
        nodeoffset, "#size-cells", libfdt.QUIET_NOTFOUND)
    if size_cell == -libfdt.FDT_ERR_NOTFOUND:
        dbg_err("/hdal-memory lacks #size-cells")
        return hdal_memory

    nodeoffset = fdt.path_offset(r"/hdal-memory/media", libfdt.QUIET_NOTFOUND)
    if nodeoffset < 0:
        dbg_wrn("/hdal-memory/media is not existing, skip it")
        return hdal_memory

    address_cell = address_cell.as_uint32()
    size_cell = size_cell.as_uint32()
    prop = fdt.getprop(nodeoffset, "reg")
    bytes_per_group = (address_cell + size_cell) * 4
    if len(prop) % bytes_per_group != 0:
        dbg_err("/hdal-memory/media/reg must be {} word alignment".format((address_cell + size_cell)))
        return hdal_memory

    group_cnt = int(len(prop) / bytes_per_group)
    # there are 4 types to process
    if address_cell == 1 and size_cell == 1: # 32-bit for ipcam
        # [ADDR] [SIZE] => 2 elements
        reg = struct.unpack('>{:d}L'.format(int(group_cnt*2)), prop)
        # just parse first hdal memory for dram1 check
        hdal_memory["ofs"] = reg[0]
        hdal_memory["size"] = reg[1]
    elif address_cell == 2 and size_cell == 1: # 32-bit for xvr
        # [DDRID] [ADDR] [SIZE] => 3 elements
        reg = struct.unpack('>{:d}L'.format(int(group_cnt*3)), prop)
        # just parse first hdal memory for dram1 check
        hdal_memory["ofs"] = reg[1]
        hdal_memory["size"] = reg[2]
    elif address_cell == 2 and size_cell == 2: # 64-bit for ipcam
        # [ADDR] [SIZE] => 2 elements
        reg = struct.unpack('>{:d}Q'.format(int(group_cnt*2)), prop)
        # just parse first hdal memory for dram1 check
        hdal_memory["ofs"] = reg[0]
        hdal_memory["size"] = reg[1]
    elif address_cell == 4 and size_cell == 2: # 64-bit for xvr
        # [DDRID] [ADDR] [SIZE] => 3 elements
        reg = struct.unpack('>{:d}Q'.format(int(group_cnt*3)), prop)
        # just parse first hdal memory for dram1 check
        hdal_memory["ofs"] = reg[1]
        hdal_memory["size"] = reg[2]
    else:
        dbg_err("/hdal-memory/media unknown cells pair [{}, {}]".format((address_cell, size_cell)))
        return hdal_memory

    return hdal_memory


def parse_nvtpack(fdt, flash_path):
    nvtpack_items = []

    nvtpack_path = flash_path + r"/nvtpack/index"
    nodeoffset = fdt.path_offset(nvtpack_path, libfdt.QUIET_NOTFOUND)
    if nodeoffset < 0:
        return nvtpack_items

    nodeoffset = fdt.first_subnode(nodeoffset, libfdt.QUIET_NOTFOUND)
    while nodeoffset > 0:
        name = fdt.get_name(nodeoffset)
        prop = fdt.getprop(nodeoffset, "partition_name")
        nvtpack_item = dict()
        nvtpack_item["id"] = name
        nvtpack_item["partition_name"] = prop.as_str()
        nvtpack_items.append(nvtpack_item)
        nodeoffset = fdt.next_subnode(nodeoffset, libfdt.QUIET_NOTFOUND)
    return nvtpack_items


def parse_top(fdt):
    top = dict()

    nodeoffset = fdt.path_offset(r"/top", libfdt.QUIET_NOTFOUND)
    if nodeoffset < 0:
        dbg_wrn("/top is not existing, skip it")
        return top

    nodeoffset = fdt.first_subnode(nodeoffset, libfdt.QUIET_NOTFOUND)
    while nodeoffset > 0:
        name = fdt.get_name(nodeoffset)
        prop = fdt.getprop(nodeoffset, "pinmux", libfdt.QUIET_NOTFOUND)
        if prop == -libfdt.FDT_ERR_NOTFOUND:
            nodeoffset = fdt.next_subnode(nodeoffset, libfdt.QUIET_NOTFOUND)
            continue
        top[name] = prop.as_uint32()
        nodeoffset = fdt.next_subnode(nodeoffset, libfdt.QUIET_NOTFOUND)
    return top


def parse_clock(fdt):
    clocks = []

    nodeoffset = fdt.path_offset(r"/nvt-clk", libfdt.QUIET_NOTFOUND)
    if nodeoffset < 0:
        dbg_wrn("nvt-clk is not existing, skip it")
        return clocks

    nodeoffset = fdt.first_subnode(nodeoffset, libfdt.QUIET_NOTFOUND)
    while nodeoffset > 0:
        name = fdt.get_name(nodeoffset)
        prop = fdt.getprop(nodeoffset, "clock-output-names",
                           libfdt.QUIET_NOTFOUND)
        clock = dict()
        clock["name"] = name
        if prop != -libfdt.FDT_ERR_NOTFOUND:
            clock["clock-output-names"] = prop.as_str()
        else:
            clock["clock-output-names"] = name
        clocks.append(clock)
        nodeoffset = fdt.next_subnode(nodeoffset, libfdt.QUIET_NOTFOUND)
    return clocks


def check_nvt_info_embmem(nvt_info):
    if nvt_info["EMBMEM"] == "EMBMEM_NONE":
        return 0  # no embmem
    if nvt_info["EMBMEM"] == "EMBMEM_NAND":
        dbg_err("EMBMEM_NAND has deprecated.".format(
            nvt_info["EMBMEM"], nvt_info["EMBMEM_BLK_SIZE"]))
        return -1
    if nvt_info["EMBMEM"] == "EMBMEM_SPI_NOR" and nvt_info["EMBMEM_BLK_SIZE"] != "0x10000":
        dbg_err("EMBMEM({}) and EMBMEM_BLK_SIZE({}) are not matched".format(
            nvt_info["EMBMEM"], nvt_info["EMBMEM_BLK_SIZE"]))
        return -1
    if nvt_info["EMBMEM"] == "EMBMEM_SPI_NAND" and nvt_info["EMBMEM_BLK_SIZE"] != "0x20000" and nvt_info["EMBMEM_BLK_SIZE"] != "0x40000":
        dbg_err("EMBMEM({}) and EMBMEM_BLK_SIZE({}) are not matched".format(
            nvt_info["EMBMEM"], nvt_info["EMBMEM_BLK_SIZE"]))
        return -1
    if nvt_info["EMBMEM"] == "EMBMEM_EMMC" and nvt_info["EMBMEM_BLK_SIZE"] != "0x200":
        dbg_err("EMBMEM({}) and EMBMEM_BLK_SIZE({}) are not matched".format(
            nvt_info["EMBMEM"], nvt_info["EMBMEM_BLK_SIZE"]))
        return -1
    # check NVT_ROOTFS_TYPE
    if nvt_info["NVT_ROOTFS_TYPE"] == "NVT_ROOTFS_TYPE_RAMDISK":
        pass
    elif nvt_info["NVT_ROOTFS_TYPE"] == "NVT_ROOTFS_TYPE_SQUASH":
        pass
    elif nvt_info["EMBMEM"] == "EMBMEM_SPI_NOR" and "NVT_ROOTFS_TYPE_NOR" not in nvt_info["NVT_ROOTFS_TYPE"]:
        dbg_err("NVT_ROOTFS_TYPE must be NVT_ROOTFS_TYPE_RAMDISK or NVT_ROOTFS_TYPE_NOR* or NVT_ROOTFS_TYPE_SQUASH")
        return -1
    elif nvt_info["EMBMEM"] == "EMBMEM_SPI_NAND" and "NVT_ROOTFS_TYPE_NAND" not in nvt_info["NVT_ROOTFS_TYPE"]:
        dbg_err("NVT_ROOTFS_TYPE must be NVT_ROOTFS_TYPE_RAMDISK or NVT_ROOTFS_TYPE_NAND* or NVT_ROOTFS_TYPE_SQUASH")
        return -1
    elif nvt_info["EMBMEM"] == "EMBMEM_EMMC" and "NVT_ROOTFS_TYPE_EXT4" not in nvt_info["NVT_ROOTFS_TYPE"]:
        dbg_err("NVT_ROOTFS_TYPE must be NVT_ROOTFS_TYPE_RAMDISK or NVT_ROOTFS_TYPE_EXT4* or NVT_ROOTFS_TYPE_SQUASH")
        return -1
    return 0


def check_flash_partition(nvt_info, partitions):
    if nvt_info["EMBMEM"] == "EMBMEM_NONE":
        return 0  # no flash

    # check dual flash with their own partition (too complex to check)
    if nvt_info["EMBMEM"] == "EMBMEM_SPI_NOR_NAND":
        dbg_wrn("skip check flash partition, because of dual flash partition")
        return 0

    # check partition flash type matched with EMBMEM
    if len(partitions[nvt_info["EMBMEM"]]) == 0:
        dbg_err(
            "storage-partition type is not matched to {} ".format(nvt_info["EMBMEM"]))
        return -1

    # check partition size block
    blk_size = int(nvt_info["EMBMEM_BLK_SIZE"], 0)
    partition = partitions[nvt_info["EMBMEM"]]

    if len(partition) < 3:
        dbg_err("flash partition counts are less than 3. it doesn't make scene")
        return -1
    # check first must be loader or mbr and their size follow our spec
    if nvt_info["EMBMEM"] == "EMBMEM_EMMC":
        if partition[1]["name"] != "partition_fdt":
            dbg_err("2nd flash partition must be partition_fdt")
            return -1
        if partition[1]["ofs"] != 0x40000:
            dbg_err("2nd flash partition offset must be 0x40000 for EMMC")
            return -1
    else:
        if partition[0]["name"] != "partition_loader":
            dbg_err("first flash partition must be partition_loader")
            return -1
        if partition[1]["name"] != "partition_fdt":
            dbg_err("2nd flash partition must be partition_fdt")
            return -1
        if nvt_info["EMBMEM"] == "EMBMEM_SPI_NAND":
            if partition[1]["ofs"] != blk_size*2:
                dbg_err("2nd flash partition offset must be 0x{:X} for EMBMEM_SPI_NAND".format(
                    blk_size*2))
                return -1
        else:
            if partition[1]["ofs"] != blk_size:
                dbg_err(
                    "2nd flash partition offset must be 0x{:X} for EMBMEM_SPI_NOR".format(blk_size))
                return -1
    # check if partition_fdt.restore existing
    if partition[2]["name"] != "partition_fdt.restore":
        dbg_err("3nd flash partition suggest partition_fdt.restore")
    for item in partition:
        if item["size"] % blk_size != 0:
            dbg_err("flash partition {} is not {} bytes aligned.".format(
                item["label"], blk_size))
            return -1
    # check partition if in order and overlapped
    for idx in range(len(partition)):
        if idx == 0:
            continue
        if partition[idx]["ofs"] < partition[idx-1]["ofs"]+partition[idx-1]["size"] and partition[idx]["label"] != "all":
            dbg_err("partition {}@0x{:X}, 0x{:X} overlapped with {}@0x{:X}, 0x{:X}".format(
                partition[idx]["label"],
                partition[idx]["ofs"],
                partition[idx]["size"],
                partition[idx-1]["label"],
                partition[idx-1]["ofs"],
                partition[idx-1]["size"]))
            return -1
    return 0


def check_nvtpack(nvt_info, partitions, nvtpack_items):
    if nvt_info["EMBMEM"] == "EMBMEM_NONE":
        return 0  # no flash

    # check dual flash with their own partition (too complex to check)
    if nvt_info["EMBMEM"] == "EMBMEM_SPI_NOR_NAND":
        dbg_wrn("skip check nvtpack, because of dual flash partition")
        return 0

    # check partition flash type matched with EMBMEM
    if len(nvtpack_items[nvt_info["EMBMEM"]]) == 0:
        dbg_err(
            "nvtpack-partition type is not matched to {} ".format(nvt_info["EMBMEM"]))
        return -1
    # check if id's partition_name is matched to storage partition
    partition = partitions[nvt_info["EMBMEM"]]
    nvtpack_item = nvtpack_items[nvt_info["EMBMEM"]]
    n_item = len(partition) if len(partition) < len(
        nvtpack_item) else len(nvtpack_item)
    for idx in range(n_item):
        nvtpack_partition_name = "partition_" + \
            nvtpack_item[idx]["partition_name"]
        if nvtpack_partition_name != partition[idx]["name"]:
            dbg_err("nvtpack index.{} partition name '{}' but '{}' is excepted.".format(
                idx,
                nvtpack_item[idx]["partition_name"],
                partition[idx]["name"].replace("partition_", "")))
            return -1
    return 0


def check_memory(nvt_memory_cfgs, linux_memories, reserved_memories, hdal_memory):
    fdt = None
    rtos = None
    bridge = None
    linuxtmp = None
    uboot = None
    # nvt_memory_cfgs first must be dram size
    if nvt_memory_cfgs[0]["name"] != "dram":
        dbg_err("nvt_memory_cfg first item must be 'dram' but rather '{}'".format(
            nvt_memory_cfgs[0]["name"]))
        return -1
    dram_size = nvt_memory_cfgs[0]["size"]
    # check each element if is over range
    for nvt_memory_cfg in nvt_memory_cfgs:
        if nvt_memory_cfg["ofs"]+nvt_memory_cfg["size"] > dram_size:
            dbg_err("nvt_memory_cfg's '{}' is out of dram size".format(
                nvt_memory_cfg["name"]))
            return -1
        # keep fdt, rtos, bridge info for checking fastboot sanity later
        if nvt_memory_cfg["name"] == "fdt":
            fdt = nvt_memory_cfg
        elif nvt_memory_cfg["name"] == "rtos":
            rtos = nvt_memory_cfg
        elif nvt_memory_cfg["name"] == "bridge":
            bridge = nvt_memory_cfg
        elif nvt_memory_cfg["name"] == "linuxtmp":
            linuxtmp = nvt_memory_cfg
        elif nvt_memory_cfg["name"] == "uboot":
            uboot = nvt_memory_cfg
    # check each element if in order and overlapped (disable it, because rtos DEMO_EVB, rtos can overlap loader)
    """
    for idx in range(len(nvt_memory_cfgs)):
        if idx == 0:
            continue
        if nvt_memory_cfgs[idx]["ofs"] < nvt_memory_cfgs[idx-1]["ofs"]+nvt_memory_cfgs[idx-1]["size"] and nvt_memory_cfgs[idx-1]["name"] != "dram":
            dbg_err("nvt_memory_cfg {}@0x{:X}, 0x{:X} overlapped with {}@0x{:X}, 0x{:X}".format(
                nvt_memory_cfgs[idx]["name"],
                nvt_memory_cfgs[idx]["ofs"],
                nvt_memory_cfgs[idx]["size"],
                nvt_memory_cfgs[idx-1]["name"],
                nvt_memory_cfgs[idx-1]["ofs"],
                nvt_memory_cfgs[idx-1]["size"]))
            return -1
    """
    # check if linux memory is over range
    for linux_memory in linux_memories:
        # linux_memory["ofs"] < dram_size for skipping dram2
        if linux_memory["ofs"]+linux_memory["size"] > dram_size and linux_memory["ofs"] < dram_size:
            dbg_err("linux memory is out of dram size")
            return -1
    # check if hdal memory is over range
    # hdal_memory["ofs"] < dram_size for skipping dram2
    if hdal_memory["ofs"]+hdal_memory["size"] > dram_size and hdal_memory["ofs"] < dram_size:
        dbg_err("hdal memory is out of dram size")
        return -1

    # uboot memory cannot be overlapped with linux system memory region
    # disable it, we allow overlap linux and uboot. uboot runtime will check linux-text if overlapped with uboot
    """
    for linux_memory in linux_memories:
        if uboot["ofs"] > linux_memory["ofs"] and uboot["ofs"] < linux_memory["ofs"]+linux_memory["size"]:
            dbg_err(
                "uboot memory cannot be overlapped with linux system memory region")
            return -1
        if uboot["ofs"] < linux_memory["ofs"] and uboot["ofs"]+uboot["size"] > linux_memory["ofs"]:
            dbg_err(
                "uboot memory cannot be overlapped with linux system memory region")
            return -1
    """

    # check reserved_memories must be in range of linux-memory and 4MB alignment
    for reserved_memory in reserved_memories:
        valid = 1
        for linux_memory in linux_memories:
            valid = 1
            if reserved_memory["ofs"] < linux_memory["ofs"]:
                valid = 0
                continue
            if reserved_memory["ofs"]+reserved_memory["size"] > linux_memory["ofs"]+linux_memory["size"]:
                valid = 0
                continue
            if valid:
                break
        if not valid:
            dbg_err(
                "{} is out of linux-memory".format(reserved_memory["name"]))
            return -1
    # check if hdal memory overlap with linux-memory
    for linux_memory in linux_memories:
        if hdal_memory["ofs"] > linux_memory["ofs"] and hdal_memory["ofs"] < linux_memory["ofs"]+linux_memory["size"]:
            dbg_err(
                "hdal_memory memory cannot be overlapped with linux system memory region")
            return -1
        if hdal_memory["ofs"] < linux_memory["ofs"] and hdal_memory["ofs"]+hdal_memory["size"] > linux_memory["ofs"]:
            dbg_err(
                "uboot memory cannot be overlapped with linux system memory region")
            return -1
    # if bridge exist, check fastboot requirement
    if bridge is not None:
        # check1: fdt, rtos and bridge memory region must be contiguous.
        if fdt["ofs"] + fdt["size"] != rtos["ofs"]:
            dbg_err("fdt, rtos and bridge memory region must be contiguous.")
            return -1
        if rtos["ofs"] + rtos["size"] != bridge["ofs"]:
            dbg_err("fdt, rtos and bridge memory region must be contiguous.")
            return -1
        hotplug_mem = dict()
        hotplug_mem["ofs"] = fdt["ofs"]
        hotplug_mem["size"] = fdt["size"] + rtos["size"] + bridge["size"]
        # check2: fdt, rtos and bridge memory region cannot overlap with hdal and linux memory
        for linux_memory in linux_memories:
            if hotplug_mem["ofs"] > linux_memory["ofs"] and hotplug_mem["ofs"] < linux_memory["ofs"]+linux_memory["size"]:
                dbg_err(
                    "fdt, rtos and bridge memory region cannot overlap with hdal and linux memory")
                return -1
            if hotplug_mem["ofs"] < linux_memory["ofs"] and hotplug_mem["ofs"]+hotplug_mem["size"] > linux_memory["ofs"]:
                dbg_err(
                    "fdt, rtos and bridge memory region cannot overlap with hdal and linux memory")
                return -1
        # check3: fdt, rtos and bridge memory region cannot overlap with linuxtmp
        if hotplug_mem["ofs"] > linuxtmp["ofs"] and hotplug_mem["ofs"] < linuxtmp["ofs"]+linuxtmp["size"]:
            dbg_err("fdt, rtos and bridge memory region cannot overlap with linuxtmp")
            return -1
        if hotplug_mem["ofs"] < linuxtmp["ofs"] and hotplug_mem["ofs"]+hotplug_mem["size"] > linuxtmp["ofs"]:
            dbg_err("fdt, rtos and bridge memory region cannot overlap with linuxtmp")
            return -1
        # check4: fdt, rtos and bridge memory have to start at 8MB alignment, and the size also needs 8MB alignment
        if (hotplug_mem["ofs"] % 0x800000) != 0:
            dbg_err(
                "fdt, rtos and bridge memory have to start at 8MB alignment, and the size also needs 8MB alignment")
            return -1
        if (hotplug_mem["size"] % 0x800000) != 0:
            dbg_err(
                "fdt, rtos and bridge memory have to start at 8MB alignment, and the size also needs 8MB alignment")
            return -1
    return 0


def check_ep_memory(rc_nvt_memory_cfgs, ep_nvt_memory_cfgs):
    if ep_nvt_memory_cfgs is None:
        dbg_wrn("skip ep_nvt_memory_cfgs check.")
        return 0
    # check item counts if match
    if len(rc_nvt_memory_cfgs) != len(ep_nvt_memory_cfgs):
        dbg_err("ep nvt_memory_cfg counts not match rc({}) v.s. ep({})".format(
            len(rc_nvt_memory_cfgs), len(ep_nvt_memory_cfgs)))
        return -1

    idx = 0
    for rc, ep in zip(rc_nvt_memory_cfgs, ep_nvt_memory_cfgs):
        if rc["name"] != ep["name"]:
            dbg_err("nvt_memory_cfg[{}]: name not matched rc({}) ep({})".format(
                idx, rc["name"], ep["name"]))
            return -1
        if rc["ofs"] != ep["ofs"]:
            dbg_err("nvt_memory_cfg[{}]: ofs not matched rc(0x{:08X}) ep(0x{:08X})".format(
                idx, rc["ofs"], ep["ofs"]))
            return -1
        if rc["size"] != ep["size"]:
            dbg_err("nvt_memory_cfg[{}]: size not matched rc(0x{:08x}) ep(0x{:08x})".format(
                idx, rc["size"], ep["size"]))
            return -1
        idx += 1
        pass
    return 0


def check_clock(clocks):
    er = 0
    # check clock-output-names if limit in 15 characters
    for clock in clocks:
        if len(clock["clock-output-names"]) > 15:
            dbg_err("{}/clock-output-names = {} is larger than 15 characters".format(
                clock["name"], clock["clock-output-names"]))
            er = -1
    # check if duplicated clock-output-names
    cnt_clock = len(clocks)
    for idx_i in range(cnt_clock):
        clock1 = clocks[idx_i]
        clock1_output_name = clock1["clock-output-names"]
        for idx_j in range(idx_i + 1, cnt_clock):
            clock2 = clocks[idx_j]
            clock2_output_name = clock2["clock-output-names"]
            if clock1_output_name == clock2_output_name:
                dbg_err("{}/clock-output-names is the same with {}/clock-output-names named {}".format(
                    clock1["name"],
                    clock2["name"],
                    clock1_output_name,
                ))
                er = -1

    return er


def get_emmc_path(fdt):
    emmc_names = ["mmc2", "mmc1", "mmc0"]
    for emmc_name in emmc_names:
        emmc_path = fdt.get_alias(emmc_name)
        if emmc_path:
            # find any existing partition_x
            nodeoffset = fdt.path_offset(emmc_path, libfdt.QUIET_NOTFOUND)
            if nodeoffset < 0:
                continue
            nodeoffset = fdt.first_subnode(nodeoffset, libfdt.QUIET_NOTFOUND)
            while nodeoffset > 0:
                name = fdt.get_name(nodeoffset)
                if "partition_" not in name:
                    nodeoffset = fdt.next_subnode(
                        nodeoffset, libfdt.QUIET_NOTFOUND)
                    continue
                else:
                    return emmc_path
    return None


def parse_args(argv):
    parser = argparse.ArgumentParser(description='fdt checker')
    parser.add_argument('-f', '--file', metavar='.dtb/.bin', required=True,
                        dest='file', action='store',
                        help='fdt binary file')
    parser.add_argument('-e', '--ep', metavar='.dtb/.bin',
                        dest='ep_file', action='store',
                        help='ep-fdt binary file')
    parser.add_argument('-t', '--type', metavar='',
                        dest='type', action='store',
                        help='ic name')
    args = parser.parse_args()
    return args


def main(argv):
    if 0:
        argv.append(r"-f")
        # argv.append(r"d:\tmp\nvt-na51055-evb.bin")
        # argv.append(r"/home/ubuntu/firmware/na51090_dual_sdk/na51090_linux_sdk/output/nvt-evb.dtb")
        argv.append(r"/home/ubuntu/firmware/na51102_dual_sdk/na51102_linux_sdk/output/nvt-evb.bin")

    args = parse_args(argv)

    if not os.path.isfile(args.file):
        print("cannot find {}".format(args.file), file=sys.stderr)
        return -1

    print("verify {}".format(args.file))

    fdt = libfdt.Fdt(open(args.file, mode='rb').read())

    ep_nvt_memory_cfgs = None
    if args.ep_file and os.path.isfile(args.ep_file):
        ep_fdt = libfdt.Fdt(open(args.ep_file, mode='rb').read())
        ep_nvt_memory_cfgs = parse_nvt_memory_cfg(ep_fdt)

    nvt_info = parse_nvt_info(fdt)
    emmc_path = get_emmc_path(fdt)
    partitions = dict()
    partitions["EMBMEM_SPI_NAND"] = parse_flash_partition(fdt, r"/nand")
    partitions["EMBMEM_SPI_NOR"] = parse_flash_partition(fdt, r"/nor")
    if emmc_path:
        partitions["EMBMEM_EMMC"] = parse_flash_partition(fdt, emmc_path)
    else:
        partitions["EMBMEM_EMMC"] = []

    nvt_memory_cfgs = parse_nvt_memory_cfg(fdt)
    linux_memories = parse_linux_memory(fdt)
    reserved_memories = parse_reserved_memory(fdt)
    hdal_memory = parse_hdal_memory(fdt)
    nvtpack_items = dict()
    nvtpack_items["EMBMEM_SPI_NAND"] = parse_nvtpack(fdt, r"/nand")
    nvtpack_items["EMBMEM_SPI_NOR"] = parse_nvtpack(fdt, r"/nor")
    if emmc_path:
        nvtpack_items["EMBMEM_EMMC"] = parse_nvtpack(fdt, emmc_path)
    else:
        nvtpack_items["EMBMEM_EMMC"] = []

    top = parse_top(fdt)
    clocks = parse_clock(fdt)

    if check_nvt_info_embmem(nvt_info) != 0:
        return -1
    if check_flash_partition(nvt_info, partitions) != 0:
        return -1
    if check_nvtpack(nvt_info, partitions, nvtpack_items) != 0:
        return -1
    if check_memory(nvt_memory_cfgs, linux_memories, reserved_memories, hdal_memory) != 0:
        return -1
    if check_ep_memory(nvt_memory_cfgs, ep_nvt_memory_cfgs) != 0:
        return -1
    if check_clock(clocks) != 0:
        return -1
    return 0


if __name__ == '__main__':
    try:
        # colorama.init()
        er = main(sys.argv)
    except Exception as exp:
        er = -1
        print(exp, file=sys.stderr)
        raise exp
    sys.exit(er)
