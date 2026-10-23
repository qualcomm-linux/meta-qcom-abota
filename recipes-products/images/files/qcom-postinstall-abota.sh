#!/bin/sh
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear
#
# Postinstall script bundled into the .swu package.
# Invoked by swupdate (shellscript handler) after the images: entries
# for the matched hardware-compatibility stanza have been copied
# successfully. Arms trial-boot for the slot that was just written and
# flips the active-slot pointer so the bootloader/EFI state machine boots
# into it next.
#
# Slot-agnostic: the same script serves both slot_a and slot_b stanzas.
# It reads the currently active slot and flips to the other one, so there
# is no per-slot copy to keep in sync.

set -e

# abctl --get_active reports the active slot as its EXIT CODE, not on stdout:
#   0 = slot a, 1 = slot b, anything else = genuine failure
# (see do_get_active() in recipes-bsp/ab-update/files/abctl/src/abctl.cpp).
# Exit code 1 therefore means "slot b is active", not an error -- capture it
# with `|| active_slot=$?` so `set -e` does not abort on the slot-b case.
# (A bare `active_line="$(abctl --get_active)"` aborts here whenever b is
# active, which is exactly the bug this replaces.)
active_slot=0
abctl --get_active || active_slot=$?

case "$active_slot" in
    0) new_slot=1 ;;   # active a -> boot b next
    1) new_slot=0 ;;   # active b -> boot a next
    *)
        echo "qcom-postinstall-abota: unable to determine active slot (abctl --get_active exit $active_slot)" >&2
        exit 1
        ;;
esac

abctl --setIsSlotUpdated 1
abctl --setTrailBootCount 0
abctl --setTrailBootMaxCount 3
abctl --setTrailBootEnabled 1
abctl --set_active "$new_slot"
