// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cinttypes>

#include "libabctl.h"

typedef int (*action_func_t)(void *p, void *q);

typedef struct {
    const char *option_string;
    size_t opt_str_len;
    action_func_t action_func;
    int num_params;
} opt_func_map;

static void usage()
{
    printf("Usage: abctl <option> [args]\n\n");
    printf("   --get_boot_slot:\n");
    printf("         gets the slot from which device booted.\n");
    printf("   --get_active:\n");
    printf("         gets the slot marked as active.\n");
    printf("   --activestatus <slot number>:\n");
    printf("         gets whether <slot number> is the active slot (0 = active).\n");
    printf("   --set_active <slot number>:\n");
    printf("         sets <slot number> as the active slot.\n");
    printf("   --set_unbootable <slot number>:\n");
    printf("         marks <slot number> as unbootable.\n");
    printf("   --setIsSlotUpdated <value>:\n");
    printf("         sets the IsSlotUpdate field to <value>.\n");
    printf("   --getIsSlotUpdated:\n");
    printf("         gets the IsSlotUpdate value.\n");
    printf("   --getTrailBootMaxCount:\n");
    printf("         gets the TrailBootMaxCount.\n");
    printf("   --setTrailBootMaxCount <count>:\n");
    printf("         sets the TrailBootMaxCount.\n");
    printf("   --getTrailBootCount:\n");
    printf("         gets the TrailBootCount.\n");
    printf("   --setTrailBootCount <count>:\n");
    printf("         sets the TrailBootCount.\n");
    printf("   --getTrailBootEnabled:\n");
    printf("         gets the TrailBootEnabled flag.\n");
    printf("   --setTrailBootEnabled <value>:\n");
    printf("         sets the TrailBootEnabled flag.\n");
    printf("   --getOsUpdateType:\n");
    printf("         gets the OsUpdateType.\n");
    printf("   --setOsUpdateType <value>:\n");
    printf("         sets the OsUpdateType.\n");
    printf("   --getOsIndications:\n");
    printf("         gets the OsIndications.\n");
    printf("   --setOsIndications <value>:\n");
    printf("         sets the OsIndications.\n");
    printf("   --getCapsuleFailureReason:\n");
    printf("         gets the capsule update failure reason from ESRT.\n");
    printf("   --isTrailBootSuccessful:\n");
    printf("         reports whether the current trial boot is successful.\n");
    printf("   --getHlosStateMachine:\n");
    printf("         gets the HLOS state-machine <state> <status> values.\n");
    printf("   --setHlosStateMachine <state> <status>:\n");
    printf("         sets the HLOS state-machine <state> <status> values.\n");
    printf("   --getNhlosStateMachine:\n");
    printf("         gets the NHLOS state-machine <state> <status> values.\n");
    printf("   --setNhlosStateMachine <state> <status>:\n");
    printf("         sets the NHLOS state-machine <state> <status> values.\n");
    printf("   --saveEfivars:\n");
    printf("         saves EFI variables.\n");
    printf("\n");
}

static int do_get_boot_slot(void *p, void *q)
{
    int slot = libabctl_getBootSlot();
    printf("Boot slot is %d\n", slot);
    return slot;
}

static int do_get_active(void *p, void *q)
{
    int ret = libabctl_getActiveSlot();
    if (ret == 0)
        printf("Active slot is a\n");
    else if (ret == 1)
        printf("Active slot is b\n");
    else
        printf("Failed to get active slot\n");
    return ret;
}

static int do_get_active_status(void *p, void *q)
{
    unsigned int slot = atoi((char *)p);
    int ret = libabctl_getActiveStatus(slot);
    printf("ActiveStatus value is %d\n", ret);
    return ret;
}

static int do_set_active(void *p, void *q)
{
    unsigned int slot = atoi((char *)p);
    return libabctl_setActive(slot);
}

static int do_set_unbootable(void *p, void *q)
{
    unsigned int slot = atoi((char *)p);
    return libabctl_setUnbootable(slot);
}

static int do_getIsSlotUpdate(void *p, void *q)
{
    int ret = libabctl_getIsSlotUpdated();
    printf("IsSlotUpdated value is %d\n", ret);
    return ret;
}

static int do_setIsSlotUpdate(void *p, void *q)
{
    uint16_t value = atoi((char *)p);
    int ret = libabctl_setIsSlotUpdated(value);
    if (ret == -1)
        printf("Failed to set IsSlotUpdate\n");
    return ret;
}

static int do_getTrailBootMaxCount(void *p, void *q)
{
    int ret = libabctl_getTrailBootMaxCount();
    if (ret == -1)
        printf("Failed to get TrailBootMaxCount\n");
    else
        printf("TrailBootMaxCount is %d\n", ret);
    return ret;
}

static int do_setTrailBootMaxCount(void *p, void *q)
{
    uint16_t count = atoi((char *)p);
    int ret = libabctl_setTrailBootMaxCount(count);
    if (ret == -1)
        printf("Failed to set TrailBootMaxCount\n");
    return ret;
}

static int do_getTrailBootCount(void *p, void *q)
{
    int ret = libabctl_getTrailBootCount();
    if (ret == -1)
        printf("Failed to get TrailBootCount\n");
    else
        printf("TrailBootCount is %d\n", ret);
    return ret;
}

static int do_setTrailBootCount(void *p, void *q)
{
    uint16_t count = atoi((char *)p);
    int ret = libabctl_setTrailBootCount(count);
    if (ret == -1)
        printf("Failed to set TrailBootCount\n");
    return ret;
}

static int do_getTrailBootEnabled(void *p, void *q)
{
    int ret = libabctl_getTrailBootEnabled();
    if (ret == -1)
        printf("Failed to get TrailBootEnabled\n");
    else
        printf("TrailBootEnabled is %d\n", ret);
    return ret;
}

static int do_setTrailBootEnabled(void *p, void *q)
{
    uint16_t value = atoi((char *)p);
    int ret = libabctl_setTrailBootEnabled(value);
    if (ret == -1)
        printf("Failed to set TrailBootEnabled\n");
    return ret;
}

static int do_getOsUpdateType(void *p, void *q)
{
    int ret = libabctl_getOsUpdateType();
    if (ret == -1)
        printf("Failed to get OsUpdateType\n");
    else
        printf("OsUpdateType is %d\n", ret);
    return ret;
}

static int do_setOsUpdateType(void *p, void *q)
{
    uint16_t value = atoi((char *)p);
    int ret = libabctl_setOsUpdateType(value);
    if (ret == -1)
        printf("Failed to set OsUpdateType\n");
    return ret;
}

static int do_getOsIndications(void *p, void *q)
{
    uint64_t ret = libabctl_getOsIndications();
    printf("OsIndications is %" PRIu64 "\n", ret);
    return 0;
}

static int do_setOsIndications(void *p, void *q)
{
    uint64_t value = strtoull((char *)p, nullptr, 0);
    int ret = libabctl_setOsIndications(value);
    if (ret == -1)
        printf("Failed to set OsIndications\n");
    return ret;
}

static int do_getCapsuleFailureReason(void *p, void *q)
{
    int ret = libabctl_getCapsuleFailureReason();
    if (ret == -1)
        printf("Failed to get CapsuleFailureReason\n");
    else
        printf("CapsuleFailureReason is %d\n", ret);
    return ret;
}

static int do_isTrailBootSuccessful(void *p, void *q)
{
    int ret = libabctl_isTrailBootSuccessful();
    if (ret == -1)
        printf("Failed to get isTrailBootSuccessful\n");
    else
        printf("isTrailBootSuccessful is %d\n", ret);
    return ret;
}

static int do_getHlosStateMachine(void *p, void *q)
{
    uint16_t osStateMachine = 0, status = 0;
    int ret = libabctl_getHlosStateMachine(osStateMachine, status);
    if (ret == -1)
        printf("Failed to get HlosStateMachine\n");
    else
        printf("osStateMachine is %d and status is %d\n", osStateMachine, status);
    return ret;
}

static int do_setHlosStateMachine(void *p, void *q)
{
    uint16_t osStateMachine = atoi((char *)p);
    uint16_t status = atoi((char *)q);
    int ret = libabctl_setHlosStateMachine(osStateMachine, status);
    if (ret == -1)
        printf("Failed to set HlosStateMachine\n");
    else
        printf("set HlosStateMachine succeeded\n");
    return ret;
}

static int do_getNhlosStateMachine(void *p, void *q)
{
    uint16_t nhlosStateMachine = 0, status = 0;
    int ret = libabctl_getNhlosStateMachine(nhlosStateMachine, status);
    if (ret == -1)
        printf("Failed to get NhlosStateMachine\n");
    else
        printf("nhlosStateMachine is %d and status is %d\n", nhlosStateMachine, status);
    return ret;
}

static int do_setNhlosStateMachine(void *p, void *q)
{
    uint16_t nhlosStateMachine = atoi((char *)p);
    uint16_t status = atoi((char *)q);
    int ret = libabctl_setNhlosStateMachine(nhlosStateMachine, status);
    if (ret == -1)
        printf("Failed to set NhlosStateMachine\n");
    else
        printf("set NhlosStateMachine succeeded\n");
    return ret;
}

static int do_saveEfivars(void *p, void *q)
{
    int ret = libabctl_saveEfivars();
    if (ret == 0)
        printf("Saved efivars\n");
    return ret;
}

static int print_help(void *p, void *q)
{
    usage();
    return 0;
}

static opt_func_map opt_func_list[] = {
    {"--help",                  strlen("--help"), &print_help, 0},
    {"--get_boot_slot",         strlen("--get_boot_slot"), &do_get_boot_slot, 0},
    {"--get_active",            strlen("--get_active"), &do_get_active, 0},
    {"--activestatus",          strlen("--activestatus"), &do_get_active_status, 1},
    {"--set_active",            strlen("--set_active"), &do_set_active, 1},
    {"--set_unbootable",        strlen("--set_unbootable"), &do_set_unbootable, 1},
    {"--getIsSlotUpdated",      strlen("--getIsSlotUpdated"), &do_getIsSlotUpdate, 0},
    {"--setIsSlotUpdated",      strlen("--setIsSlotUpdated"), &do_setIsSlotUpdate, 1},
    {"--getTrailBootMaxCount",  strlen("--getTrailBootMaxCount"), &do_getTrailBootMaxCount, 0},
    {"--setTrailBootMaxCount",  strlen("--setTrailBootMaxCount"), &do_setTrailBootMaxCount, 1},
    {"--getTrailBootCount",     strlen("--getTrailBootCount"), &do_getTrailBootCount, 0},
    {"--setTrailBootCount",     strlen("--setTrailBootCount"), &do_setTrailBootCount, 1},
    {"--getTrailBootEnabled",   strlen("--getTrailBootEnabled"), &do_getTrailBootEnabled, 0},
    {"--setTrailBootEnabled",   strlen("--setTrailBootEnabled"), &do_setTrailBootEnabled, 1},
    {"--getOsUpdateType",       strlen("--getOsUpdateType"), &do_getOsUpdateType, 0},
    {"--setOsUpdateType",       strlen("--setOsUpdateType"), &do_setOsUpdateType, 1},
    {"--getOsIndications",      strlen("--getOsIndications"), &do_getOsIndications, 0},
    {"--setOsIndications",      strlen("--setOsIndications"), &do_setOsIndications, 1},
    {"--getCapsuleFailureReason", strlen("--getCapsuleFailureReason"), &do_getCapsuleFailureReason, 0},
    {"--isTrailBootSuccessful", strlen("--isTrailBootSuccessful"), &do_isTrailBootSuccessful, 0},
    {"--getHlosStateMachine",   strlen("--getHlosStateMachine"), &do_getHlosStateMachine, 0},
    {"--setHlosStateMachine",   strlen("--setHlosStateMachine"), &do_setHlosStateMachine, 2},
    {"--getNhlosStateMachine",  strlen("--getNhlosStateMachine"), &do_getNhlosStateMachine, 0},
    {"--setNhlosStateMachine",  strlen("--setNhlosStateMachine"), &do_setNhlosStateMachine, 2},
    {"--saveEfivars",           strlen("--saveEfivars"), &do_saveEfivars, 0},
};

int main(int argc, char *argv[])
{
    if (argc < 2) {
        usage();
        return -1;
    }

    for (size_t i = 0; i < sizeof(opt_func_list) / sizeof(opt_func_map); i++) {
        if (strncmp(argv[1], opt_func_list[i].option_string, opt_func_list[i].opt_str_len) != 0)
            continue;

        if (opt_func_list[i].num_params == 2) {
            if (argv[2] && argv[3])
                return opt_func_list[i].action_func(argv[2], argv[3]);
            usage();
            return -1;
        } else if (opt_func_list[i].num_params == 1) {
            if (argv[2])
                return opt_func_list[i].action_func(argv[2], nullptr);
            usage();
            return -1;
        } else {
            return opt_func_list[i].action_func(nullptr, nullptr);
        }
    }

    usage();
    return -1;
}
