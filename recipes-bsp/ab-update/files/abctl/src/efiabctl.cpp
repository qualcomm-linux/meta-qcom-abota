// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear
//
// EFI-variable backend for abctl. All A/B slot, trial-boot and OS-update
// state is packed into a single 32-bit little-endian bitfield stored in the
// "OsTrialBootStatus" EFI variable, plus a separate 64-bit "OsIndications"
// EFI variable. This file centralizes the read/modify/write boilerplate so
// each accessor below only needs to describe which bits it touches.

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <fcntl.h>
#include <unistd.h>

#include <efivar/efivar.h>

#include "efiabctl.h"

namespace {

constexpr char kGuidStr[] = "8BE4DF61-93CA-11D2-AA0D-00E098032B8C";
constexpr char kTrialBootVar[] = "OsTrialBootStatus";
constexpr char kOsIndicationsVar[] = "OsIndications";
constexpr uint32_t kDefaultAttrs =
    0x00000001u /* NON_VOLATILE */ |
    0x00000002u /* BOOTSERVICE_ACCESS */ |
    0x00000004u /* RUNTIME_ACCESS */;

constexpr unsigned kBitTrailBootMaxCount = 8;   // bits 8-11
constexpr unsigned kBitTrailBootCount    = 12;  // bits 12-15
constexpr unsigned kBitTrailBootEnabled  = 16;
constexpr unsigned kBitOsUpdateType      = 17;
constexpr unsigned kBitActiveSlot        = 18;
constexpr unsigned kBitActiveSlotHi      = 19; // must stay 0
constexpr unsigned kBitIsSlotUpdated     = 20;
constexpr unsigned kBitUnbootableSlot    = 21; // bits 21-23
constexpr unsigned kBitNhlosStateMachine = 24; // bits 24-26, status @27
constexpr unsigned kBitHlosStateMachine  = 28; // bits 28-30, status @31

inline int lsb_get_bit(const uint8_t *buf, size_t nbits, unsigned bit_index) {
    if (!buf || bit_index >= nbits) return -1;
    return (buf[bit_index / 8] >> (bit_index % 8)) & 0x1;
}

inline int lsb_set_bit(uint8_t *buf, size_t nbits, unsigned bit_index, bool value) {
    if (!buf || bit_index >= nbits) return -1;
    const uint8_t mask = static_cast<uint8_t>(1u << (bit_index % 8));
    if (value) buf[bit_index / 8] |= mask;
    else       buf[bit_index / 8] &= static_cast<uint8_t>(~mask);
    return 0;
}

// Reads a contiguous little-endian bitfield of `width` bits starting at
// `start`. Returns -1 on failure.
int get_bitfield(const uint8_t *buf, size_t nbits, unsigned start, unsigned width) {
    int value = 0;
    for (unsigned i = 0; i < width; ++i) {
        int bit = lsb_get_bit(buf, nbits, start + i);
        if (bit < 0) return -1;
        value |= (bit << i);
    }
    return value;
}

// Writes a contiguous little-endian bitfield of `width` bits starting at
// `start`. Returns -1 on failure.
int set_bitfield(uint8_t *buf, size_t nbits, unsigned start, unsigned width, unsigned value) {
    for (unsigned i = 0; i < width; ++i) {
        if (lsb_set_bit(buf, nbits, start + i, (value >> i) & 0x1) != 0) return -1;
    }
    return 0;
}

using EfiBuf = std::unique_ptr<uint8_t, decltype(&free)>;

// Reads `name` under the abctl GUID. On success returns an owning buffer,
// its size in bytes and its current attributes; `min_size` is the smallest
// size this caller can operate on.
int efi_read_var(const char *name, size_t min_size, EfiBuf &out, size_t &size, uint32_t &attrs) {
    efi_guid_t guid{};
    if (efi_str_to_guid(kGuidStr, &guid) != 0) {
        std::fprintf(stderr, "efiabctl: invalid GUID string: %s\n", kGuidStr);
        return -1;
    }

    uint8_t *raw = nullptr;
    size = 0;
    attrs = 0;
    if (efi_get_variable(guid, name, &raw, &size, &attrs) != 0 || raw == nullptr) {
        perror("efi_get_variable");
        if (raw) free(raw);
        return -1;
    }
    out = EfiBuf(raw, &free);

    if (size < min_size) {
        std::fprintf(stderr, "efiabctl: variable '%s' too small (%zu bytes, need >= %zu)\n",
                     name, size, min_size);
        return -1;
    }
    return 0;
}

// Writes `data`/`size` back to `name`, falling back to the standard NV/BS/RT
// attribute set if the variable did not already carry any attributes.
int efi_write_var(const char *name, uint8_t *data, size_t size, uint32_t attrs) {
    efi_guid_t guid{};
    if (efi_str_to_guid(kGuidStr, &guid) != 0) {
        std::fprintf(stderr, "efiabctl: invalid GUID string: %s\n", kGuidStr);
        return -1;
    }

    if (attrs == 0) attrs = kDefaultAttrs;

    if (efi_set_variable(guid, name, data, size, attrs, 0) != 0) {
        perror("efi_set_variable");
        return -1;
    }
    return 0;
}

// Read-modify-write helper for the 32-bit OsTrialBootStatus word: reads the
// variable, hands the buffer to `fn` to inspect/mutate, and writes it back
// if `fn` returns true.
template <typename Fn>
int with_trialboot_word(Fn fn) {
    EfiBuf data(nullptr, &free);
    size_t size = 0;
    uint32_t attrs = 0;
    if (efi_read_var(kTrialBootVar, 4, data, size, attrs) != 0) return -1;

    constexpr size_t nbits = 32;
    int result = fn(data.get(), nbits);
    if (result < 0) return -1;
    if (result == 0) return 0; // read-only path, nothing to write back

    return efi_write_var(kTrialBootVar, data.get(), size, attrs);
}

} // namespace

int efi_getBootSlot()
{
    int fd = open("/proc/mounts", O_RDONLY);
    if (fd == -1) {
        perror("open: /proc/mounts");
        return -1;
    }

    char *buf = static_cast<char *>(malloc(8192));
    if (!buf) {
        perror("malloc");
        close(fd);
        return -1;
    }

    ssize_t read_bytes = read(fd, buf, 8191);
    close(fd);
    if (read_bytes <= 0) {
        perror("read");
        free(buf);
        return -1;
    }
    buf[read_bytes] = '\0';

    int slot = -1;
    char *line = strtok(buf, "\n");
    while (line) {
        char dev[256], mount[256], fstype[64];
        if (sscanf(line, "%255s %255s %63s", dev, mount, fstype) == 3 &&
            strcmp(mount, "/") == 0) {
            if (strcmp(dev, "/dev/disk/by-partlabel/system_b") == 0)
                slot = 1;
            else if (strcmp(dev, "/dev/disk/by-partlabel/system") == 0)
                slot = 0;
            break;
        }
        line = strtok(nullptr, "\n");
    }

    free(buf);
    return slot;
}

int efi_getCapsuleFailureReason()
{
    int fd = open("/sys/firmware/efi/esrt/entries/entry0/last_attempt_status", O_RDONLY);
    if (fd == -1) {
        perror("libabctl: open:/sys/firmware/efi/esrt/entries/entry0/last_attempt_status");
        return -1;
    }

    char *last_attempt_status = static_cast<char *>(malloc(2048 + 1));
    ssize_t read_bytes = read(fd, last_attempt_status, 2048);
    close(fd);
    if (read_bytes == -1) {
        perror("read: last_attempt_status");
        free(last_attempt_status);
        return -1;
    }
    last_attempt_status[read_bytes] = '\0';

    char *trimmed = strtok(last_attempt_status, "\n");
    int value = trimmed ? static_cast<int>(strtol(trimmed, nullptr, 16)) : -1;
    free(last_attempt_status);
    return value;
}

int efi_getActiveSlot()
{
    EfiBuf data(nullptr, &free);
    size_t size = 0;
    uint32_t attrs = 0;
    if (efi_read_var(kTrialBootVar, 4, data, size, attrs) != 0) return -1;

    constexpr size_t nbits = 32;
    int slot = lsb_get_bit(data.get(), nbits, kBitActiveSlot);
    int hi   = lsb_get_bit(data.get(), nbits, kBitActiveSlotHi);
    if (slot < 0 || hi < 0) {
        std::fprintf(stderr, "efi_getActiveSlot: failed to read active-slot bits.\n");
        return -1;
    }
    if (hi != 0) {
        std::fprintf(stderr, "efi_getActiveSlot: unexpected bit%u=%d (expected 0).\n",
                      kBitActiveSlotHi, hi);
        return -1;
    }
    return slot;
}

int efi_getActiveStatus(unsigned int slot)
{
    if (slot != 0 && slot != 1) {
        std::fprintf(stderr, "efi_getActiveStatus: invalid slot (%u).\n", slot);
        return -1;
    }

    EfiBuf data(nullptr, &free);
    size_t size = 0;
    uint32_t attrs = 0;
    if (efi_read_var(kTrialBootVar, 4, data, size, attrs) != 0) return -1;

    constexpr size_t nbits = 32;
    int b18 = lsb_get_bit(data.get(), nbits, kBitActiveSlot);
    int b19 = lsb_get_bit(data.get(), nbits, kBitActiveSlotHi);
    if (b18 < 0 || b19 < 0) {
        std::fprintf(stderr, "efi_getActiveStatus: failed to read active-slot bits.\n");
        return -1;
    }

    const bool matches = (slot == 0) ? (b18 == 0 && b19 == 0) : (b18 == 1 && b19 == 0);
    return matches ? 0 : 1;
}

int efi_setActive(unsigned int slot)
{
    if (slot != 0 && slot != 1) {
        std::fprintf(stderr, "efi_setActive: invalid slot (%u).\n", slot);
        return -1;
    }

    return with_trialboot_word([slot](uint8_t *buf, size_t nbits) -> int {
        if (set_bitfield(buf, nbits, kBitTrailBootMaxCount, 3, 0x7) != 0) return -1;
        if (lsb_set_bit(buf, nbits, kBitOsUpdateType, true) != 0) return -1;
        if (lsb_set_bit(buf, nbits, kBitActiveSlotHi, false) != 0) return -1;
        if (lsb_set_bit(buf, nbits, kBitActiveSlot, slot == 1) != 0) return -1;
        if (lsb_set_bit(buf, nbits, kBitIsSlotUpdated, true) != 0) return -1;
        if (set_bitfield(buf, nbits, kBitUnbootableSlot, 3, 0x1) != 0) return -1;
        if (set_bitfield(buf, nbits, kBitHlosStateMachine, 3, 0x2) != 0) return -1;
        if (lsb_set_bit(buf, nbits, kBitHlosStateMachine + 3 /* status */, true) != 0) return -1;
        return 1; // write back
    });
}

int efi_setUnbootable(unsigned int slot)
{
    if (slot != 0 && slot != 1) {
        std::fprintf(stderr, "efi_setUnbootable: invalid slot (%u).\n", slot);
        return -1;
    }

    return with_trialboot_word([slot](uint8_t *buf, size_t nbits) -> int {
        if (lsb_set_bit(buf, nbits, kBitUnbootableSlot + 2, false) != 0) return -1;
        if (lsb_set_bit(buf, nbits, kBitUnbootableSlot + 1, slot == 1) != 0) return -1;
        if (lsb_set_bit(buf, nbits, kBitUnbootableSlot, slot == 0) != 0) return -1;
        return 1;
    });
}

int efi_getTrailBootMaxCount()
{
    EfiBuf data(nullptr, &free);
    size_t size = 0;
    uint32_t attrs = 0;
    if (efi_read_var(kTrialBootVar, 4, data, size, attrs) != 0) return -1;
    return get_bitfield(data.get(), 32, kBitTrailBootMaxCount, 4);
}

int efi_setTrailBootMaxCount(uint16_t count)
{
    return with_trialboot_word([count](uint8_t *buf, size_t nbits) -> int {
        if (set_bitfield(buf, nbits, kBitTrailBootMaxCount, 4, count) != 0) return -1;
        return 1;
    });
}

int efi_getTrailBootCount()
{
    EfiBuf data(nullptr, &free);
    size_t size = 0;
    uint32_t attrs = 0;
    if (efi_read_var(kTrialBootVar, 4, data, size, attrs) != 0) return -1;
    return get_bitfield(data.get(), 32, kBitTrailBootCount, 4);
}

int efi_setTrailBootCount(uint16_t count)
{
    return with_trialboot_word([count](uint8_t *buf, size_t nbits) -> int {
        if (set_bitfield(buf, nbits, kBitTrailBootCount, 4, count) != 0) return -1;
        return 1;
    });
}

int efi_getTrailBootEnabled()
{
    EfiBuf data(nullptr, &free);
    size_t size = 0;
    uint32_t attrs = 0;
    if (efi_read_var(kTrialBootVar, 4, data, size, attrs) != 0) return -1;
    return get_bitfield(data.get(), 32, kBitTrailBootEnabled, 1);
}

int efi_setTrailBootEnabled(uint16_t value)
{
    return with_trialboot_word([value](uint8_t *buf, size_t nbits) -> int {
        if (lsb_set_bit(buf, nbits, kBitTrailBootEnabled, value & 0x1) != 0) return -1;
        return 1;
    });
}

int efi_getOsUpdateType()
{
    EfiBuf data(nullptr, &free);
    size_t size = 0;
    uint32_t attrs = 0;
    if (efi_read_var(kTrialBootVar, 4, data, size, attrs) != 0) return -1;
    return get_bitfield(data.get(), 32, kBitOsUpdateType, 1);
}

int efi_setOsUpdateType(uint16_t value)
{
    return with_trialboot_word([value](uint8_t *buf, size_t nbits) -> int {
        if (lsb_set_bit(buf, nbits, kBitOsUpdateType, value & 0x1) != 0) return -1;
        return 1;
    });
}

int efi_getIsSlotUpdate()
{
    EfiBuf data(nullptr, &free);
    size_t size = 0;
    uint32_t attrs = 0;
    if (efi_read_var(kTrialBootVar, 4, data, size, attrs) != 0) return -1;
    return get_bitfield(data.get(), 32, kBitIsSlotUpdated, 1);
}

int efi_setIsSlotUpdate(uint16_t value)
{
    return with_trialboot_word([value](uint8_t *buf, size_t nbits) -> int {
        if (lsb_set_bit(buf, nbits, kBitIsSlotUpdated, value != 0) != 0) return -1;
        return 1;
    });
}

int efi_getHlosStateMachine(uint16_t &osStateMachine, uint16_t &status)
{
    EfiBuf data(nullptr, &free);
    size_t size = 0;
    uint32_t attrs = 0;
    if (efi_read_var(kTrialBootVar, 4, data, size, attrs) != 0) return -1;

    int value = get_bitfield(data.get(), 32, kBitHlosStateMachine, 3);
    int st = lsb_get_bit(data.get(), 32, kBitHlosStateMachine + 3);
    if (value < 0 || st < 0) {
        std::fprintf(stderr, "efi_getHlosStateMachine: failed to read state bits.\n");
        return -1;
    }
    osStateMachine = static_cast<uint16_t>(value);
    status = static_cast<uint16_t>(st);
    return 0;
}

int efi_setHlosStateMachine(uint16_t osStateMachine, uint16_t status)
{
    return with_trialboot_word([osStateMachine, status](uint8_t *buf, size_t nbits) -> int {
        if (set_bitfield(buf, nbits, kBitHlosStateMachine, 3, osStateMachine) != 0) return -1;
        if (lsb_set_bit(buf, nbits, kBitHlosStateMachine + 3, status != 0) != 0) return -1;
        return 1;
    });
}

int efi_getNhlosStateMachine(uint16_t &nhlosStateMachine, uint16_t &status)
{
    EfiBuf data(nullptr, &free);
    size_t size = 0;
    uint32_t attrs = 0;
    if (efi_read_var(kTrialBootVar, 4, data, size, attrs) != 0) return -1;

    int value = get_bitfield(data.get(), 32, kBitNhlosStateMachine, 3);
    int st = lsb_get_bit(data.get(), 32, kBitNhlosStateMachine + 3);
    if (value < 0 || st < 0) {
        std::fprintf(stderr, "efi_getNhlosStateMachine: failed to read state bits.\n");
        return -1;
    }
    nhlosStateMachine = static_cast<uint16_t>(value);
    status = static_cast<uint16_t>(st);
    return 0;
}

int efi_setNhlosStateMachine(uint16_t nhlosStateMachine, uint16_t status)
{
    return with_trialboot_word([nhlosStateMachine, status](uint8_t *buf, size_t nbits) -> int {
        if (set_bitfield(buf, nbits, kBitNhlosStateMachine, 3, nhlosStateMachine) != 0) return -1;
        if (lsb_set_bit(buf, nbits, kBitNhlosStateMachine + 3, status != 0) != 0) return -1;
        return 1;
    });
}

int efi_setOsIndications(uint64_t value)
{
    EfiBuf data(nullptr, &free);
    size_t size = 0;
    uint32_t attrs = 0;
    if (efi_read_var(kOsIndicationsVar, sizeof(uint64_t), data, size, attrs) != 0) return -1;

    if (set_bitfield(data.get(), 64, 0, 32, static_cast<uint32_t>(value)) != 0 ||
        set_bitfield(data.get(), 64, 32, 32, static_cast<uint32_t>(value >> 32)) != 0) {
        std::fprintf(stderr, "efi_setOsIndications: failed to set bits.\n");
        return -1;
    }

    return efi_write_var(kOsIndicationsVar, data.get(), size, attrs);
}

uint64_t efi_getOsIndications()
{
    EfiBuf data(nullptr, &free);
    size_t size = 0;
    uint32_t attrs = 0;
    if (efi_read_var(kOsIndicationsVar, sizeof(uint64_t), data, size, attrs) != 0) return 0;

    int lo = get_bitfield(data.get(), 64, 0, 32);
    int hi = get_bitfield(data.get(), 64, 32, 32);
    if (lo < 0 || hi < 0) {
        std::fprintf(stderr, "efi_getOsIndications: failed to read bits.\n");
        return 0;
    }
    return (static_cast<uint64_t>(static_cast<uint32_t>(hi)) << 32) |
           static_cast<uint32_t>(lo);
}

int efi_isTrailBootSuccessful()
{
    int trial_boot_count = efi_getTrailBootCount();
    int trial_boot_max_count = efi_getTrailBootMaxCount();
    int is_slot_updated = efi_getIsSlotUpdate();
    if (trial_boot_count < 0 || trial_boot_max_count < 0 || is_slot_updated < 0)
        return -1;

    int last_attempt_status = efi_getCapsuleFailureReason();

    return (last_attempt_status == 0 && is_slot_updated == 0 &&
            trial_boot_count < trial_boot_max_count) ? 1 : 0;
}

int efi_saveEfivars()
{
    // No-op: the RPMB-sync path (uefisecapp/TZ) is not part of this module.
    return 0;
}
