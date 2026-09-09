# Autotools, Makefile.am & configure.ac — How `libabctl.so` Gets Built

This is a learning reference for the `abctl` module (`files/abctl/`), explaining what
`configure.ac` and `Makefile.am` actually do, and how source files turn into
`libabctl.so`, `libabctl.a`, and the `abctl` binary.

---

## 1. The big picture: 4 stages

Autotools is a *generator of generators*. You write two small hand-authored files
(`configure.ac`, `Makefile.am`), and a chain of tools expands them into the real
build system:

```
configure.ac  ──┐
Makefile.am   ──┼──► autoreconf ──► configure (shell script) + Makefile.in
                │
                └──► ./configure ──► Makefile (final, machine-specific)
                                          │
                                          ▼
                                        make ──► compiler (g++) ──► .o / .lo files
                                                       │
                                                       ▼
                                                  linker (ld, via libtool)
                                                       │
                                                       ▼
                                        libabctl.so.1.0.0 (+ symlinks), libabctl.a, abctl
```

- **`autoreconf -fi`** reads `configure.ac` + `Makefile.am` and *generates*
  `configure` (a portable shell script) and `Makefile.in` (a template Makefile).
  It also copies in libtool helper scripts (`ltmain.sh`, `config.guess`, etc.)
  into `m4/` and `build-aux/` — that's why those empty directories exist in
  `files/abctl/`.
- **`./configure`** (generated, not hand-written) probes the system — is there a
  C++ compiler? does `pkg-config` know about `efivar`? — and writes a real
  `Makefile` from `Makefile.in`, substituting things like `@EFIVAR_CFLAGS@` with
  actual values (e.g. `-I/usr/include/efivar`).
- **`make`** reads the generated `Makefile` and actually invokes the compiler and
  linker.
- In Yocto, `inherit autotools` (used in `qcom-efiabctl-abota_1.0.bb`) runs all of
  this for you automatically: `do_configure` runs `autoreconf` + `./configure`,
  `do_compile` runs `make`, `do_install` runs `make install`.

So: **you never write `Makefile` or `configure` by hand** — you write the `.am`
(automake) and `.ac` (autoconf) *source* files, and the tools expand them.

---

## 2. `configure.ac` line by line

```m4
AC_INIT([abctl], [1.0], [])
```
- Autoconf macro. Declares the package name (`abctl`) and version (`1.0`). The
  third arg (empty here) is a bug-report email. This becomes `PACKAGE_NAME`,
  `PACKAGE_VERSION` etc., usable in code via `config.h` if you generate one.

```m4
AM_INIT_AUTOMAKE([foreign subdir-objects])
```
- Turns on **Automake** (the layer that understands `Makefile.am`).
- `foreign`: don't require GNU-project files like `NEWS`, `AUTHORS`, `ChangeLog`
  to exist (we're not a GNU package).
- `subdir-objects`: allow `.cpp` sources to live in subdirectories (`src/`,
  `inc/`) instead of forcing everything into the top-level directory — this is
  why `src/abctl.cpp` works instead of needing a flat layout.

```m4
AC_CONFIG_MACRO_DIR([m4])
```
- Tells autoconf where to look for/store third-party `.m4` macro files
  (`libtool.m4`, `pkg.m4` for `PKG_CHECK_MODULES`, etc.), which `autoreconf`
  copies into the `m4/` directory.

```m4
AC_PROG_CC
AC_PROG_CXX
```
- Autoconf macros that **detect the C and C++ compilers** (`gcc`/`clang`,
  `g++`/`clang++`) and set the `CC`/`CXX` variables the generated `Makefile`
  will use to invoke them. In cross-compilation (Yocto), these pick up the
  cross toolchain (e.g. `aarch64-oe-linux-g++`) from environment variables the
  BitBake `autotools` class exports.

```m4
LT_INIT
```
- Initializes **Libtool**. This is the macro that makes `lib_LTLIBRARIES` in
  `Makefile.am` mean something — without it, Automake wouldn't know how to
  build a `.la`/`.so`/`.a` at all. It also enables `-version-info` handling
  (see §4).

```m4
PKG_CHECK_MODULES([EFIVAR], [efivar >= 37])
```
- Uses `pkg-config` to find the `efivar` library (version ≥ 37) and defines two
  variables for use in `Makefile.am`:
  - `EFIVAR_CFLAGS` → e.g. `-I/usr/include/efivar`
  - `EFIVAR_LIBS` → e.g. `-lefivar`
  If `efivar.pc` isn't found, `configure` fails with an error — this is your
  dependency check, equivalent to `DEPENDS += "efivar"` in the `.bb` recipe.

```m4
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
```
- `AC_CONFIG_FILES([Makefile])`: says "generate a `Makefile` from
  `Makefile.in`" (which Automake itself generates from your `Makefile.am`).
- `AC_OUTPUT`: the macro that actually emits the `configure` script's final
  code — writes out all the files declared above. Every `configure.ac` ends
  with this.

---

## 3. `Makefile.am` line by line

```make
AUTOMAKE_OPTIONS = foreign subdir-objects
```
- Same meaning as the `AM_INIT_AUTOMAKE` flags above, just settable per-Makefile
  too (belt-and-suspenders).

```make
AM_CPPFLAGS = -I$(srcdir)/inc @EFIVAR_CFLAGS@
```
- `AM_CPPFLAGS`: extra preprocessor flags applied to **every** compile in this
  Makefile. `-I$(srcdir)/inc` adds our own `inc/` folder to the include path,
  so `#include "libabctl.h"` resolves. `@EFIVAR_CFLAGS@` is substituted by
  `configure` at configure-time with whatever `PKG_CHECK_MODULES` found (e.g.
  `-I/usr/include/efivar`).

```make
lib_LTLIBRARIES = libabctl.la
```
- Declares a **libtool library target** named `libabctl`. `lib_` means "install
  to `$(libdir)`" (typically `/usr/lib`). The `.la` suffix is the libtool
  convention — it's a build-time placeholder name; the real output is
  `.so`/`.a` (see §4).

```make
libabctl_la_SOURCES = src/libabctl.cpp src/efiabctl.cpp
```
- Automake convention: `<target>_SOURCES` (with `.` and `-` turned into `_`)
  lists the source files compiled into that target. Both `libabctl.cpp`
  (public wrapper) and `efiabctl.cpp` (EFI backend) get compiled and linked
  into the *same* `libabctl.so` — `efiabctl.cpp`'s symbols end up inside the
  library but its header (`efiabctl.h`) is never installed, so callers outside
  this module can't see them. That's how "efiabctl is internal to libabctl" is
  enforced at the build level.

```make
libabctl_la_LIBADD = @EFIVAR_LIBS@
```
- `<target>_LIBADD`: extra libraries to link **into** `libabctl.la` itself
  (transitively linked into anything that links `libabctl.la`). `@EFIVAR_LIBS@`
  expands to `-lefivar`.

```make
libabctl_la_LDFLAGS = -version-info 1:0:0 -no-undefined
```
- `<target>_LDFLAGS`: linker flags for this target.
  - `-version-info current:revision:age` — libtool's ABI versioning scheme,
    explained fully in §4.
  - `-no-undefined` — tells the linker the shared library must have **no**
    unresolved symbols after linking (all `efi_*` calls from `libefivar` must
    actually resolve via `-lefivar`). This is required to build a `.so` at all
    on some platforms (e.g. Windows/mingw), and is good practice everywhere:
    catches missing `-l` flags at build time instead of at `dlopen()` time.

```make
libabctlincludedir = $(includedir)
libabctlinclude_HEADERS = inc/libabctl.h
```
- `<name>dir = <path>` + `<name>_HEADERS = <files>` is the Automake pattern for
  "install these header files to this directory." Here it installs
  `libabctl.h` to `$(includedir)` (`/usr/include`) — the **public API**. Note
  `efiabctl.h` is *not* listed here, so it never gets installed system-wide;
  it's purely an internal implementation header.

```make
sbin_PROGRAMS = abctl
```
- Declares an **executable** target `abctl`, installed to `$(sbindir)`
  (`/usr/sbin`).

```make
abctl_SOURCES = src/abctl.cpp
abctl_LDADD = libabctl.la
```
- `abctl` is built from just `abctl.cpp`, and linked against `libabctl.la`
  (the library target defined above). This is the whole point of the
  three-tier design: the CLI's object code contains **no** `efi_*` symbol
  references at all — only `libabctl_*` calls — because it only links against
  `libabctl.la`, never touches `efiabctl.o`/`efiabctl.h` directly.

```make
sbin_SCRIPTS = scripts/abctl-helper.sh
```
- `<dir>_SCRIPTS` installs a file as-is (with executable bit) to `<dir>` — no
  compilation step, since it's a shell script. Same `sbin` install location as
  the `abctl` binary it wraps.

---

## 4. How the `.so` actually gets produced (compiler + linker + libtool)

For each `.cpp` in `libabctl_la_SOURCES`, `make` runs roughly:

```sh
libtool --mode=compile g++ -DPIC -fPIC -I./inc <EFIVAR_CFLAGS> -c src/efiabctl.cpp -o src/efiabctl.lo
```

- **`-fPIC`**: "Position Independent Code" — required for any code going into a
  shared library, so it can be mapped at any address in different processes.
  Libtool adds this automatically for `_LTLIBRARIES` targets; you never write
  it yourself.
- **`.lo`**: a libtool wrapper file (a small text file), not raw machine code.
  It records the paths to the *real* object files libtool creates underneath
  (`.libs/efiabctl.o` for the PIC version used in the `.so`, and sometimes a
  second non-PIC `.o` if a static archive is also being built).

Once all sources are compiled, libtool **links**:

```sh
libtool --mode=link g++ -o libabctl.la src/libabctl.lo src/efiabctl.lo \
    -lefivar -no-undefined -version-info 1:0:0 -rpath /usr/lib
```

Under the hood this becomes something like:

```sh
g++ -shared -fPIC -Wl,-soname,libabctl.so.1 \
    -o .libs/libabctl.so.1.0.0 \
    .libs/efiabctl.o .libs/libabctl.o -lefivar
```

- **`-shared`**: tells `g++`/`ld` to produce a shared object rather than a
  final executable.
- **`-Wl,-soname,libabctl.so.1`**: embeds the **SONAME** into the binary — the
  name other binaries record as "the library I need at runtime." This is
  where `-version-info` matters:

  `-version-info current:revision:age` → soname suffix = `current - age`.
  With `1:0:0` (current=1, revision=0, age=0): soname number = `1 - 0 = 1` →
  `libabctl.so.1`. The real file on disk is named
  `libabctl.so.<current-age>.<age>.<revision>` = `libabctl.so.1.0.0`.

  Libtool then creates the symlink chain:
  ```
  libabctl.so       -> libabctl.so.1.0.0   (used at link time, -labctl)
  libabctl.so.1     -> libabctl.so.1.0.0   (the SONAME, used at runtime by ld.so)
  libabctl.so.1.0.0                        (the real ELF shared object)
  ```
  This triplet is *the* ABI-compatibility contract: if you add a function
  without breaking existing ones, bump `revision`; if you add functions in a
  backward-compatible way, bump `current` and `age`; if you break ABI, bump
  `current` and reset `age`/`revision` to 0. Programs linked against
  `libabctl.so.1` keep working as long as the soname number doesn't change.
- Libtool **also** builds `libabctl.a` (static archive) unless you pass
  `LT_INIT([disable-static])` in `configure.ac` — this is a plain `ar rcs
  libabctl.a efiabctl.o libabctl.o` (no `-fPIC` needed for a static archive
  destined only for static linking, though libtool by default reuses the PIC
  objects for simplicity on ELF systems).

Then for `abctl`:

```sh
g++ -o .libs/abctl src/abctl.o -L.libs -labctl -lefivar \
    -Wl,-rpath,/usr/lib
```
- The **linker** resolves every `libabctl_*` symbol referenced in
  `abctl.o` against `libabctl.so`'s **dynamic symbol table**. It does *not*
  need to see `efiabctl.o` at all — `efi_*` symbols were already resolved
  *inside* `libabctl.so` at the time *that* library was linked. This is the
  literal mechanism enforcing "abctl.cpp never touches efi_* directly": even
  if someone typo'd a call to `efi_getBootSlot()` in `abctl.cpp`, the linker
  would fail because `efiabctl.h` was never included and no object exposing
  that symbol to the linker was passed on `abctl`'s link line.
- At **runtime**, the dynamic linker (`ld.so`) reads `abctl`'s `NEEDED` entry
  (`libabctl.so.1`), searches `LD_LIBRARY_PATH`/`/etc/ld.so.conf`/rpath, and
  maps `libabctl.so.1.0.0` into the process, resolving `libabctl_*` symbol
  addresses via the **PLT/GOT** (lazy binding) the compiler generated.

---

## 5. How it all composes end-to-end

```
efiabctl.cpp ──┐                                      abctl.cpp
efiabctl.h    ─┼─► compiled into ──► libabctl.so.1.0.0 ◄── links against ──► abctl
libabctl.cpp  ─┘        (efi_* symbols hidden           (only calls
libabctl.h                inside, not installed)         libabctl_* — never
  (only this header                                       sees efi_* at all)
   is installed)
```

- **Compile-time boundary**: `abctl.cpp` only `#include "libabctl.h"`. It
  physically cannot call `efi_getBootSlot()` because that declaration isn't
  visible — enforced by the header, not just convention.
- **Link-time boundary**: even if it somehow declared its own prototype for an
  `efi_*` function, the linker for the `abctl` binary is never given
  `efiabctl.o`/`efiabctl.lo` or told to link `-lefiabctl` — only `-labctl`
  (i.e. `libabctl.la`). So the symbol simply wouldn't resolve.
- **Runtime boundary**: at runtime, `abctl`'s dynamic symbol needs are
  satisfied entirely by `libabctl.so.1`; `efi_*` symbols live inside that
  `.so`'s own internal (non-exported-to-abctl but still present) symbol table
  and are called *within* `libabctl.so` itself when `libabctl_getBootSlot()`
  calls `efi_getBootSlot()`.

## 6. How Yocto drives all of this (`qcom-efiabctl-abota_1.0.bb`)

`inherit autotools pkgconfig` in the `.bb` maps BitBake tasks to the stages
in §1:

| BitBake task   | What it runs                                             |
|----------------|-----------------------------------------------------------|
| `do_configure` | `autoreconf -fi` then `./configure` (cross-compile flags, `--prefix=/usr`, `--host=<triplet>` etc. injected automatically) |
| `do_compile`   | `make` (invokes the cross `g++`, e.g. `aarch64-oe-linux-g++`) |
| `do_install`   | `make install DESTDIR=<image-staging-dir>` — copies `libabctl.so.*`, `libabctl.h`, `abctl`, `abctl-helper.sh` into the staged rootfs layout |
| packaging      | splits install output into `${PN}` (runtime `.so.*`, `abctl`, script) and `${PN}-dev` (`.so` symlink + `libabctl.h`), `${PN}-staticdev` (`.a`) automatically, based on Yocto's default `FILES` globs |

`DEPENDS += "efivar"` ensures `efivar`'s headers/`.pc` file are present in the
sysroot *before* `do_configure` runs, so `PKG_CHECK_MODULES([EFIVAR], ...)`
inside `configure.ac` succeeds during the build.

---

## 7. `abctl` command reference

Once built and installed, `abctl` (in `$(sbindir)`, typically `/usr/sbin`) is
invoked as `abctl <option> [args]`. Each option below maps directly to one
`do_*` handler in `src/abctl.cpp`, which calls the matching `libabctl_*`
function (never `efi_*` directly — see §5).

| Command                                         | Calls (`libabctl_*`)              | Effect |
|--------------------------------------------------|------------------------------------|--------|
| `abctl --help`                                   | *(usage only)*                     | Prints usage text |
| `abctl --get_boot_slot`                          | `libabctl_getBootSlot()`           | Prints the slot the device actually booted from (0=`_a`, 1=`_b`) |
| `abctl --get_active`                             | `libabctl_getActiveSlot()`         | Prints the slot marked active for next boot |
| `abctl --activestatus <slot>`                    | `libabctl_getActiveStatus(slot)`   | Prints whether `<slot>` is the active slot (0=active) |
| `abctl --set_active <slot>`                      | `libabctl_setActive(slot)`         | Marks `<slot>` as the active/next-boot slot |
| `abctl --set_unbootable <slot>`                  | `libabctl_setUnbootable(slot)`     | Marks `<slot>` unbootable |
| `abctl --getIsSlotUpdated`                       | `libabctl_getIsSlotUpdated()`      | Prints the `IsSlotUpdate` flag |
| `abctl --setIsSlotUpdated <value>`               | `libabctl_setIsSlotUpdated(value)` | Sets the `IsSlotUpdate` flag |
| `abctl --getTrailBootMaxCount`                   | `libabctl_getTrailBootMaxCount()`  | Prints max trial-boot attempts allowed |
| `abctl --setTrailBootMaxCount <count>`           | `libabctl_setTrailBootMaxCount(count)` | Sets max trial-boot attempts |
| `abctl --getTrailBootCount`                      | `libabctl_getTrailBootCount()`     | Prints current trial-boot attempt count |
| `abctl --setTrailBootCount <count>`              | `libabctl_setTrailBootCount(count)`| Sets current trial-boot attempt count |
| `abctl --getTrailBootEnabled`                    | `libabctl_getTrailBootEnabled()`   | Prints whether trial boot is enabled |
| `abctl --setTrailBootEnabled <value>`            | `libabctl_setTrailBootEnabled(value)` | Enables/disables trial boot |
| `abctl --getOsUpdateType`                        | `libabctl_getOsUpdateType()`       | Prints the OS update type |
| `abctl --setOsUpdateType <value>`                | `libabctl_setOsUpdateType(value)`  | Sets the OS update type |
| `abctl --getOsIndications`                       | `libabctl_getOsIndications()`      | Prints the 64-bit `OsIndications` EFI variable |
| `abctl --setOsIndications <value>`               | `libabctl_setOsIndications(value)` | Sets `OsIndications` |
| `abctl --getCapsuleFailureReason`                | `libabctl_getCapsuleFailureReason()` | Prints capsule-update failure reason from ESRT |
| `abctl --isTrailBootSuccessful`                  | `libabctl_isTrailBootSuccessful()` | Reports whether the current trial boot is considered successful |
| `abctl --getHlosStateMachine`                    | `libabctl_getHlosStateMachine(...)` | Prints HLOS state-machine value + status |
| `abctl --setHlosStateMachine <state> <status>`   | `libabctl_setHlosStateMachine(...)` | Sets HLOS state-machine value + status |
| `abctl --getNhlosStateMachine`                   | `libabctl_getNhlosStateMachine(...)` | Prints NHLOS state-machine value + status |
| `abctl --setNhlosStateMachine <state> <status>`  | `libabctl_setNhlosStateMachine(...)` | Sets NHLOS state-machine value + status |
| `abctl --saveEfivars`                            | `libabctl_saveEfivars()`           | Saves EFI variables (no-op stub in this module — see `efiabctl.cpp`) |

Example session:

```sh
# Which slot did we boot from, and which is active?
abctl --get_boot_slot
abctl --get_active

# Promote slot 1 (_b) to active, then check it took effect
abctl --set_active 1
abctl --activestatus 1        # prints 0 (is active) once set

# Trial-boot bookkeeping around an OTA update
abctl --setTrailBootEnabled 1
abctl --setTrailBootMaxCount 3
abctl --setTrailBootCount 0
abctl --isTrailBootSuccessful
```

### Using the shell dispatcher instead

`scripts/abctl-helper.sh` (installed to the same `sbin_SCRIPTS` location, see
§3) wraps every `abctl --option` above behind a friendlier, hyphenated verb —
useful for scripting without remembering exact flag spellings:

```sh
abctl-helper.sh get-boot-slot
abctl-helper.sh set-active 1
abctl-helper.sh active-status 1
abctl-helper.sh set-trailboot-enabled 1
abctl-helper.sh set-hlos-state-machine 2 0
abctl-helper.sh help              # lists every action
```

Internally each branch of its `case` statement just shells out to
`"$ABCTL_BIN" --<Option> "$@"` (see §3's `sbin_SCRIPTS` entry) — so it adds no
new EFI logic, only a scripting-friendly command surface over the same
`abctl` → `libabctl` → `efiabctl` chain described in §5. Set `ABCTL_BIN` in
the environment if `abctl` isn't on `$PATH` (e.g. `ABCTL_BIN=/usr/sbin/abctl
abctl-helper.sh get-active`).
