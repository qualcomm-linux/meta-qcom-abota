// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#pragma once

#include <cstdint>

/*
 *  libabctl_getBootSlot returns current slot number from which device booted.
 *  Return values should be interpreted as follows:
 *    0: When device booted from slot suffixed "_a".
 *    1: When device booted from slot suffixed "_b".
 *   -1: If system does not support A/B, or in case of any error.
 */
int libabctl_getBootSlot();

/*
 *  libabctl_getActiveSlot returns current slot number from which device will boot next time.
 *  Return values should be interpreted as follows:
 *    0: When device booted from slot suffixed "_a".
 *    1: When device booted from slot suffixed "_b".
 *   -1: If system does not support A/B, or in case of any error.
 */
int libabctl_getActiveSlot();

/*
 *  libabctl_getActiveStatus returns whether <slot> is the active slot.
 *  Return values should be interpreted as follows:
 *    0: If <slot> is the active slot.
 *    1: If <slot> is not the active slot.
 *   -1: If system does not support A/B, or in case of any error.
 */
int libabctl_getActiveStatus(unsigned int slot);

/*
 *  libabctl_setActive sets <slot> as the active slot.
 *  Return values should be interpreted as follows:
 *    0: If the slot is updated.
 *   -1: If system does not support A/B, or in case of any error.
 */
int libabctl_setActive(unsigned int slot);

/*
 *  libabctl_setUnbootable marks <slot> as unbootable.
 *  Return values should be interpreted as follows:
 *    0: If the slot is updated.
 *   -1: If system does not support A/B, or in case of any error.
 */
int libabctl_setUnbootable(unsigned int slot);

/*
 *  libabctl_getTrailBootMaxCount returns current value of TrailBootMaxCount.
 *  Return values should be interpreted as follows:
 *    Any non-negative integer will value of it.
 *   -1: If system does not support A/B, or in case of any error.
 */
int libabctl_getTrailBootMaxCount();

/*
 *  libabctl_setTrailBootMaxCount sets value of TrailBootMaxCount to <count>.
 *  Return values should be interpreted as follows:
 *    0 If value is updated.
 *   -1: If system does not support A/B, or in case of any error.
 */
int libabctl_setTrailBootMaxCount(uint16_t count);

/*
 *  libabctl_getTrailBootCount returns current value of TrailBootCount.
 *  Return values should be interpreted as follows:
 *    Any non-negative integer will value of it.
 *   -1: If system does not support A/B, or in case of any error.
 */
int libabctl_getTrailBootCount();

/*
 *  libabctl_setTrailBootCount sets current value of TrailBootCount to <count>.
 *  Return values should be interpreted as follows:
 *    0: If successfully sets the value to <count>.
 *   -1: If system does not support A/B, or in case of any error.
 */
int libabctl_setTrailBootCount(uint16_t count);

/*
 *  libabctl_getTrailBootEnabled returns current value of TrailBootEnabled.
 *  Return values should be interpreted as follows:
 *    Any non-negative integer will value of it.
 *   -1: If system does not support A/B, or in case of any error.
 */
int libabctl_getTrailBootEnabled();

/*
 *  libabctl_setTrailBootEnabled sets value of TrailBootEnabled to <value>.
 *  Return values should be interpreted as follows:
 *    0 If value is updated.
 *   -1: If system does not support A/B, or in case of any error.
 */
int libabctl_setTrailBootEnabled(uint16_t value);

/*
 *  libabctl_getOsUpdateType returns current value of OsUpdateType.
 *  Return values should be interpreted as follows:
 *    Any non-negative integer will value of it.
 *   -1: If system does not support A/B, or in case of any error.
 */
int libabctl_getOsUpdateType();

/*
 *  libabctl_setOsUpdateType sets value of OsUpdateType to <value>.
 *  Return values should be interpreted as follows:
 *    0 If value is updated.
 *   -1: If system does not support A/B, or in case of any error.
 */
int libabctl_setOsUpdateType(uint16_t value);

/*
 *  libabctl_getIsSlotUpdated returns current value of IsSlotUpdated.
 *  Return values should be interpreted as follows:
 *    Any non-negative integer will value of it.
 *   -1: If system does not support A/B, or in case of any error.
 */
int libabctl_getIsSlotUpdated();

/*
 *  libabctl_setIsSlotUpdated sets value of IsSlotUpdated to <value>.
 *  Return values should be interpreted as follows:
 *    0: If value is updated.
 *   -1: If system does not support A/B, or in case of any error.
 */
int libabctl_setIsSlotUpdated(uint16_t value);

/*
 *  libabctl_getHlosStateMachine stores current values of osStateMachine and status into passed respective arguments.
 *  Return values should be interpreted as follows:
 *    0: If values are read
 *   -1: If system does not support A/B, or in case of any error.
 */
int libabctl_getHlosStateMachine(uint16_t &osStateMachine, uint16_t &status);

/*
 *  libabctl_setHlosStateMachine sets values of osStateMachine and status from respective arguments.
 *  Return values should be interpreted as follows:
 *    0: If values are updated
 *   -1: If system does not support A/B, or in case of any error.
 */
int libabctl_setHlosStateMachine(uint16_t osStateMachine, uint16_t status);

/*
 *  libabctl_getNhlosStateMachine stores current values of nhlosStateMachine and status into passed respective arguments.
 *  Return values should be interpreted as follows:
 *    0: If values are read
 *   -1: If system does not support A/B, or in case of any error.
 */
int libabctl_getNhlosStateMachine(uint16_t &nhlosStateMachine, uint16_t &status);

/*
 *  libabctl_setNhlosStateMachine sets values of nhlosStateMachine and status from respective arguments.
 *  Return values should be interpreted as follows:
 *    0: If values are updated
 *   -1: If system does not support A/B, or in case of any error.
 */
int libabctl_setNhlosStateMachine(uint16_t nhlosStateMachine, uint16_t status);

/*
 *  libabctl_setOsIndications sets value of OsIndications to <value>.
 *  Return values should be interpreted as follows:
 *    0: If value is updated.
 *   -1: If system does not support A/B, or in case of any error.
 */
int libabctl_setOsIndications(uint64_t value);

/*
 *  libabctl_getOsIndications gets value of OsIndications efi variable.
 *  Return values should be interpreted as follows:
 *    Non-negative integer: If read value successfully.
 *   -1: If system does not support A/B, or in case of any error.
 */
uint64_t libabctl_getOsIndications();

/*
 *  libabctl_isTrailBootSuccessful checks if TrialBootEnabled is enabled, count < max count, isSlotUpdate
 *  is false and last_attempt = 0 in esrt then returns 1, else returns 0.
 *   -1: If system does not support A/B, or in case of any error.
 */
int libabctl_isTrailBootSuccessful();

/*
 *  libabctl_getCapsuleFailureReason reads esrt and returns failure reasons if it failed, 0 if no failures.
 *  esrt path: /sys/firmware/efi/esrt/entries/entry0/last_attempt_status
 *   -1: If system does not support A/B, or in case of any error.
 */
int libabctl_getCapsuleFailureReason();

/*
 *  libabctl_saveEfivars saves EFI variables.
 *  Return values should be interpreted as follows:
 *    0: On success.
 *   -1: If system does not support A/B, or in case of any error.
 */
int libabctl_saveEfivars();
