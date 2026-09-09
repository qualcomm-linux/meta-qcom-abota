// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#pragma once

#include <cstdint>

int efi_setActive(unsigned int slot);
int efi_setUnbootable(unsigned int slot);
int efi_getActiveStatus(unsigned int slot);
int efi_getActiveSlot();
int efi_getTrailBootMaxCount();
int efi_setTrailBootMaxCount(uint16_t count);
int efi_getTrailBootCount();
int efi_setTrailBootCount(uint16_t count);
int efi_getTrailBootEnabled();
int efi_setTrailBootEnabled(uint16_t value);
int efi_getOsUpdateType();
int efi_setOsUpdateType(uint16_t value);
int efi_getIsSlotUpdate();
int efi_setIsSlotUpdate(uint16_t value);
int efi_getHlosStateMachine(uint16_t &osStateMachine, uint16_t &status);
int efi_setHlosStateMachine(uint16_t osStateMachine, uint16_t status);
int efi_getNhlosStateMachine(uint16_t &nhlosStateMachine, uint16_t &status);
int efi_setNhlosStateMachine(uint16_t nhlosStateMachine, uint16_t status);
int efi_setOsIndications(uint64_t value);
uint64_t efi_getOsIndications();
int efi_isTrailBootSuccessful();
int efi_getCapsuleFailureReason();
int efi_getBootSlot();
int efi_saveEfivars();
