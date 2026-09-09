#Enable read only rootfs
IMAGE_FEATURES += " read-only-rootfs "

CORE_IMAGE_BASE_INSTALL += " \
    qcom-efiabctl-abota \
    "

QCOM_PARTITION_FILES_SUBDIR:qcs8300-ride-sx = "partitions/qcs8300-ride-sx/emmc"
