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
- **Incremental OTA only:** `rdiff` and `cpio` on the host that runs
  `generate_delta_package.sh`. These are **not** required to build images or
  to produce or apply a full OTA — only to generate an incremental
  (differential) package (see
  [OTA package generation and update](#6-ota-package-generation-and-update)).
  Install them with your distro's package manager, e.g.:

  ```sh
  # Debian/Ubuntu
  sudo apt-get install librsync-bin cpio

  # Fedora/RHEL
  sudo dnf install librsync cpio
  ```

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


## 6. OTA package generation and update

Two kinds of update package can be applied to a flashed device: a **full**
OTA that writes the whole system/EFI image into the inactive slot, and an
**incremental** (differential) OTA that ships only an rdiff patch between the
installed build and the new build. The on-device apply command is identical
for both — only how the `.swu` is produced differs.

### Full OTA package

The full `.swu` is produced automatically by the image build (step 4); no
extra step is required. Find it in the image deploy directory, e.g.
`build/tmp/deploy/images/qcs8300-ride-sx/qcom-multimedia-image-qcs8300-ride-sx.rootfs.swu`.

### Incremental (differential) OTA package

An incremental package is built on the host with
`recipes-products/images/files/generate_delta_package.sh`, which computes an
rdiff patch for each image between a **source** build (what is currently
installed on the device) and a **dest** build (the target you want to update
to), and bundles the slot-flip postinstall scripts so the package behaves like
a production OTA.

1. Build the baseline image (step 4) and flash it to the device (step 5),
   then set the built images aside as the source — the delta is computed
   against exactly these bytes, which must match what is installed on the
   device, so keep a copy before rebuilding:

   ```sh
   mkdir -p ~/abota-ota/source
   cp tmp/deploy/images/qcs8300-ride-sx/system.img ~/abota-ota/source/
   cp tmp/deploy/images/qcs8300-ride-sx/efi.bin    ~/abota-ota/source/
   ```

2. Make your changes and rebuild `qcom-multimedia-image`. The new
   `system.img`/`efi.bin` in `tmp/deploy/images/qcs8300-ride-sx/` are the dest.

3. Generate the differential `.swu` (`--mode differential` is the default; run
   from the script's own directory so `--scripts .` resolves the templates and
   postinstall scripts):

   ```sh
   cd meta-qcom-abota/recipes-products/images/files

   ./generate_delta_package.sh \
       --output ~/abota-ota/test-rdiff.swu \
       --source-dir ~/abota-ota/source \
       --dest-dir /path/to/build/tmp/deploy/images/qcs8300-ride-sx \
       --scripts .
   ```

   By default the delta covers both `system.img` and `efi.bin`; restrict it
   with e.g. `--images "system.img"` if only the rootfs changed. The script
   needs `rdiff` and `cpio` on the host (see [Prerequisites](#prerequisites)).

   To sign the package (required when the device was built with
   `ABOTA_SIGNING = "1"`), add `--sign` with the **same** keypair the image
   build signs with — see [Signed OTA updates](#signed-ota-updates-optional):

   ```sh
   ./generate_delta_package.sh \
       --output ~/abota-ota/test-rdiff.swu --sign \
       --cert /path/to/qcom-swupdate-dev.cert.pem \
       --key  /path/to/qcom-swupdate-dev.key.pem \
       --source-dir ~/abota-ota/source \
       --dest-dir /path/to/build/tmp/deploy/images/qcs8300-ride-sx \
       --scripts .
   ```

### Apply the update on the device

Copy the `.swu` to the device (into `/opt`, which is read/write) and run
SWUpdate against the inactive slot. If the device needs network to copy the
package, bring up Wi-Fi first:

```sh
nmcli dev wifi connect <wifi_name> password <password>
```

Check the currently active slot and select the **other** one as the update
target:

```sh
abctl --get_active
```

Then apply the package (the command is the same for full and incremental
packages).

```sh
swupdate -i /opt/update.swu -H board:qcs8300-ride-sx -m -M -l 3
```

- `-i <path>` — the `.swu` package to install.
- `-H board:qcs8300-ride-sx` — hardware-compatibility string; must match the
  `hardware-compatibility` entry in the package's `sw-description`.
- `-m -M` — required update options for this A/B flow.
- `-l 3` — log level 3 (verbose), to watch the update progress.

On success the postinstall script arms trial boot and flips the active-slot
pointer, so the device boots into the updated slot on the next reboot:

```sh
reboot
```

## Signed OTA updates (optional)

By default the build produces an **unsigned** `.swu` and a SWUpdate binary
that accepts it — no keys required. Signature verification (CMS/PKCS#7 over
`sw-description`) is gated behind a single flag, `ABOTA_SIGNING`, that drives
both the build-side `.swu` signing and the device-side verification so the
two can never drift apart.

### Enable it

Add the flag to your build's `conf/local.conf` and drop a signing keypair into
the signing directory:

```sh
# Turn on signing (build + verify). ABOTA_SIGNING_DIR is optional; it defaults
# to meta-qcom-abota/recipes-products/images/files inside the layer. Override
# it (absolute path) to read the keypair from a product/CI secrets directory.
cat >> conf/local.conf <<'EOF'

# ABOTA signed OTA
ABOTA_SIGNING = "1"
EOF
```

With `ABOTA_SIGNING = "1"`, the build:

- signs `sw-description` with `openssl cms -sign` and bundles
  `sw-description.sig` into the `.swu`,
- builds SWUpdate with `CONFIG_SIGNED_IMAGES=y`,
- installs the public cert on target and points `/etc/swupdate.cfg`'s
  `public-key-file` at it.

### Generate a development keypair

The layer ships **no** cert or key — supply your own. The private key is
git-ignored and must never be committed or bundled into a `.swu`; the public
cert pairs with it and is likewise kept untracked for the dev keypair.

```sh
cd meta-qcom-abota/recipes-products/images/files   # = ABOTA_SIGNING_DIR default

# 1. Private signing key (RSA 2048). Build-side only. Keep mode 600.
openssl genrsa -out qcom-swupdate-dev.key.pem 2048
chmod 600 qcom-swupdate-dev.key.pem

# 2. Matching public cert (self-signed X.509, 10-year validity). Used by both
#    the build (embedded in the CMS signature) and the device (verification).
openssl req -new -x509 -key qcom-swupdate-dev.key.pem \
  -out qcom-swupdate-dev.cert.pem -days 3650 \
  -subj "/CN=qcom-swupdate-dev"
```

CMS is the signature *container*; RSA is the algorithm it carries, so a CMS
build needs both the key and its matching cert. The cert and key are a pair —
regenerate one and you must regenerate the other, or verification fails.

### Customizing names and location

| Variable | Side | Default | Purpose |
| --- | --- | --- | --- |
| `ABOTA_SIGNING` | both | `0` | Master on/off switch. |
| `ABOTA_SIGNING_DIR` | both | `${LAYERDIR}/recipes-products/images/files` | Dir holding the keypair. Point at a CI secrets dir for release builds. |
| `SWUPDATE_CMS_CERT` | build | `${ABOTA_SIGNING_DIR}/qcom-swupdate-dev.cert.pem` | Signer cert embedded in the signature. |
| `SWUPDATE_CMS_KEY` | build | `${ABOTA_SIGNING_DIR}/qcom-swupdate-dev.key.pem` | Private signing key. |
| `SWUPDATE_PUBLIC_CERT` | device | `qcom-swupdate-dev.cert.pem` | Public cert installed on target as `public-key-file`. |

For a release build, point these at a product/CI keypair and install only the
matching **public** cert on the target — never ship the private key.

> **Development only.** The default keypair above is for local testing. Treat
> any key that has been shared or committed as compromised and rotate it.

## Getting in Contact

* [Report an Issue on GitHub](../../issues)
* [Open a Discussion on GitHub](../../discussions)

## License

`meta-qcom-abota` is licensed under [BSD-3-Clause](https://spdx.org/licenses/BSD-3-Clause.html).
See [LICENSE.txt](LICENSE.txt) for the full license text.
