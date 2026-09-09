FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://0001-initramfs-update-slot-based-on-efivar-bits-and-force.patch"

RDEPENDS:initramfs-module-rootfs += "efivar"
