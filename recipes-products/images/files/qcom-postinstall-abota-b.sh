#!/bin/sh
# Postinstall script bundled into the .swu package.
# Invoked by swupdate (shellscript handler) after the images: entries
# for the matched hardware-compatibility stanza have been copied
# successfully. Arms trial-boot for the slot that was just written and
# flips the active-slot pointer so the bootloader/EFI state machine boots
# into it next.

set -e

active_line="$(abctl --get_active)"
case "$active_line" in
    *"slot is a"*) active_slot="a" ;;
    *"slot is b"*) active_slot="b" ;;
    *) echo "qcom-postinstall-abota: unable to determine active slot: $active_line" >&2; exit 1 ;;
esac

if [ "$active_slot" = "a" ]; then
    new_slot="1"
else
    new_slot="0"
fi

abctl --setIsSlotUpdated 1
abctl --setTrailBootCount 0
abctl --setTrailBootMaxCount 3
abctl --setTrailBootEnabled 1
abctl --set_active 1 
