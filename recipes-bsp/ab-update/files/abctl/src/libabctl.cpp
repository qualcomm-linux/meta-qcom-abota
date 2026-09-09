// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "libabctl.h"
#include "efiabctl.h"

int libabctl_getBootSlot()
{
    return efi_getBootSlot();
}

int libabctl_getActiveSlot()
{
    return efi_getActiveSlot();
}

int libabctl_getActiveStatus(unsigned int slot)
{
    return efi_getActiveStatus(slot);
}

int libabctl_setActive(unsigned int slot)
{
    return efi_setActive(slot);
}

int libabctl_setUnbootable(unsigned int slot)
{
    return efi_setUnbootable(slot);
}

int libabctl_getTrailBootMaxCount()
{
    return efi_getTrailBootMaxCount();
}

int libabctl_setTrailBootMaxCount(uint16_t count)
{
    return efi_setTrailBootMaxCount(count);
}

int libabctl_getTrailBootCount()
{
    return efi_getTrailBootCount();
}

int libabctl_setTrailBootCount(uint16_t count)
{
    return efi_setTrailBootCount(count);
}

int libabctl_getTrailBootEnabled()
{
    return efi_getTrailBootEnabled();
}

int libabctl_setTrailBootEnabled(uint16_t value)
{
    return efi_setTrailBootEnabled(value);
}

int libabctl_getOsUpdateType()
{
    return efi_getOsUpdateType();
}

int libabctl_setOsUpdateType(uint16_t value)
{
    return efi_setOsUpdateType(value);
}

int libabctl_getIsSlotUpdated()
{
    return efi_getIsSlotUpdate();
}

int libabctl_setIsSlotUpdated(uint16_t value)
{
    return efi_setIsSlotUpdate(value);
}

int libabctl_getHlosStateMachine(uint16_t &osStateMachine, uint16_t &status)
{
    return efi_getHlosStateMachine(osStateMachine, status);
}

int libabctl_setHlosStateMachine(uint16_t osStateMachine, uint16_t status)
{
    return efi_setHlosStateMachine(osStateMachine, status);
}

int libabctl_getNhlosStateMachine(uint16_t &nhlosStateMachine, uint16_t &status)
{
    return efi_getNhlosStateMachine(nhlosStateMachine, status);
}

int libabctl_setNhlosStateMachine(uint16_t nhlosStateMachine, uint16_t status)
{
    return efi_setNhlosStateMachine(nhlosStateMachine, status);
}

int libabctl_setOsIndications(uint64_t value)
{
    return efi_setOsIndications(value);
}

uint64_t libabctl_getOsIndications()
{
    return efi_getOsIndications();
}

int libabctl_isTrailBootSuccessful()
{
    return efi_isTrailBootSuccessful();
}

int libabctl_getCapsuleFailureReason()
{
    return efi_getCapsuleFailureReason();
}

int libabctl_saveEfivars()
{
    return efi_saveEfivars();
}
