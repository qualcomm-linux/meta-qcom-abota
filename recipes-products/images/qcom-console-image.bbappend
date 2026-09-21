#Enable read only rootfs
IMAGE_FEATURES += " read-only-rootfs "

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

inherit qcom-swupdate-abota-image

CORE_IMAGE_EXTRA_INSTALL += "overlayfs-qcom-paths"

CORE_IMAGE_BASE_INSTALL:remove = "var-persist-mount"

CORE_IMAGE_BASE_INSTALL:remove = "qcom-resize-partitions"


CORE_IMAGE_BASE_INSTALL += " \
    qcom-resize-partitions-abota \
    qcom-efiabctl-abota \
    swupdate \
    "

QCOM_PARTITION_FILES_SUBDIR:qcs8300-ride-sx = "partitions/qcs8300-ride-sx/emmc"
