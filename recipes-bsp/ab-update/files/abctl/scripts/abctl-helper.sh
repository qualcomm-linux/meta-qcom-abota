#!/bin/sh
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear
#
# Shell dispatcher around the abctl CLI (abctl -> libabctl -> efiabctl).
# Not yet wired into any recipe/Makefile.am; invoke manually for now.

ABCTL_BIN="${ABCTL_BIN:-abctl}"

usage() {
    cat <<EOF
Usage: $(basename "$0") <action> [args]

Actions:
  get-boot-slot
  get-active
  active-status <slot>
  set-active <slot>
  set-unbootable <slot>
  get-is-slot-updated
  set-is-slot-updated <value>
  get-trailboot-max-count
  set-trailboot-max-count <count>
  get-trailboot-count
  set-trailboot-count <count>
  get-trailboot-enabled
  set-trailboot-enabled <value>
  get-os-update-type
  set-os-update-type <value>
  get-os-indications
  set-os-indications <value>
  get-capsule-failure-reason
  is-trailboot-successful
  get-hlos-state-machine
  set-hlos-state-machine <state> <status>
  get-nhlos-state-machine
  set-nhlos-state-machine <state> <status>
  save-efivars
EOF
}

action="$1"
[ $# -ge 1 ] && shift

case "$action" in
    get-boot-slot)              "$ABCTL_BIN" --get_boot_slot ;;
    get-active)                 "$ABCTL_BIN" --get_active ;;
    active-status)               "$ABCTL_BIN" --activestatus "$1" ;;
    set-active)                  "$ABCTL_BIN" --set_active "$1" ;;
    set-unbootable)               "$ABCTL_BIN" --set_unbootable "$1" ;;
    get-is-slot-updated)         "$ABCTL_BIN" --getIsSlotUpdated ;;
    set-is-slot-updated)         "$ABCTL_BIN" --setIsSlotUpdated "$1" ;;
    get-trailboot-max-count)     "$ABCTL_BIN" --getTrailBootMaxCount ;;
    set-trailboot-max-count)     "$ABCTL_BIN" --setTrailBootMaxCount "$1" ;;
    get-trailboot-count)         "$ABCTL_BIN" --getTrailBootCount ;;
    set-trailboot-count)         "$ABCTL_BIN" --setTrailBootCount "$1" ;;
    get-trailboot-enabled)       "$ABCTL_BIN" --getTrailBootEnabled ;;
    set-trailboot-enabled)       "$ABCTL_BIN" --setTrailBootEnabled "$1" ;;
    get-os-update-type)          "$ABCTL_BIN" --getOsUpdateType ;;
    set-os-update-type)          "$ABCTL_BIN" --setOsUpdateType "$1" ;;
    get-os-indications)          "$ABCTL_BIN" --getOsIndications ;;
    set-os-indications)          "$ABCTL_BIN" --setOsIndications "$1" ;;
    get-capsule-failure-reason)  "$ABCTL_BIN" --getCapsuleFailureReason ;;
    is-trailboot-successful)     "$ABCTL_BIN" --isTrailBootSuccessful ;;
    get-hlos-state-machine)      "$ABCTL_BIN" --getHlosStateMachine ;;
    set-hlos-state-machine)      "$ABCTL_BIN" --setHlosStateMachine "$1" "$2" ;;
    get-nhlos-state-machine)     "$ABCTL_BIN" --getNhlosStateMachine ;;
    set-nhlos-state-machine)     "$ABCTL_BIN" --setNhlosStateMachine "$1" "$2" ;;
    save-efivars)                "$ABCTL_BIN" --saveEfivars ;;
    ""|help|--help|-h)           usage ;;
    *)
        echo "Unknown action: $action" >&2
        usage
        exit 1
        ;;
esac
