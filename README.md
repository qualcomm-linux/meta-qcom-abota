# meta-qcom-abota

`meta-qcom-abota` adds A/B (seamless) OTA update support for Qualcomm®
platforms on top of [`meta-qcom`](https://github.com/qualcomm-linux/meta-qcom).
It uses [SWUpdate](https://github.com/sbabic/meta-swupdate) to copy the root
filesystem and system contents from an `.swu` package into the selected
slot.

This guide explains how to sync the required Yocto layers, configure the
A/B OTA integration, build the Qualcomm multimedia image, flash the target
device.

Currently supported machine: `qcs8300-ride-sx`.

## Prerequisites

- A Yocto build environment with `kas` and BitBake installed.
- Access to the Qualcomm Linux layers and the required branch/revision for
  the target release.

## 1. Synchronize the required layers

Clone the Qualcomm BSP, A/B OTA, and SWUpdate layers into the workspace.

```sh
git clone https://github.com/qualcomm-linux/meta-qcom.git
git clone https://github.com/qualcomm-linux/meta-qcom-abota
git clone https://github.com/sbabic/meta-swupdate.git
```


## 2. Configure the local layers

Create `abota-local-layers.yml` under `meta-qcom/ci`. This file adds the
A/B OTA and SWUpdate layers to the KAS configuration.

```sh
cat > meta-qcom/ci/abota-local-layers.yml <<'EOF'
header:
  version: 14
repos:
  meta-qcom-abota:
    path: meta-qcom-abota
  meta-swupdate:
    path: meta-swupdate
EOF
```

## 3. Start the build environment

From the workspace containing the cloned repositories, start a KAS shell
using the QCS8300 RIDE-SX machine configuration, for example:

```sh
kas shell meta-qcom/ci/qcs8300-ride-sx.yml:meta-qcom/ci/qcom-distro.yml:meta-qcom/ci/abota-local-layers.yml
```

## 4. Build the image

Run the following command inside the KAS shell to build the Qualcomm
multimedia image.

```sh
bitbake qcom-multimedia-image
```

The build produces the deployable artifacts under the image deployment
directory, including the initial flashing image and the SWUpdate package,
since the A/B OTA configuration is enabled.

## 5. Flash the device

Use the generated Qualcomm flashing image to provision or update the
target device. Locate
`qcom-multimedia-image-qcs8300-ride-sx.rootfs.qcomflash` in the build
output and flash it as you would any `meta-qcom` image.


## Getting in Contact

* [Report an Issue on GitHub](../../issues)
* [Open a Discussion on GitHub](../../discussions)

## License

`meta-qcom-abota` is licensed under [BSD-3-Clause-Clear](https://spdx.org/licenses/BSD-3-Clause-Clear.html).
See [LICENSE.txt](LICENSE.txt) for the full license text.
