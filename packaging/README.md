# Packaging & install

Builds a `libhangul` package identical to Arch's `extra/libhangul`
except for the added `2sunarae` keyboard. Replaces the system package
in place (same soname, same files) — `fcitx5-hangul` does **not** need
to be rebuilt.

## Build

```sh
cd packaging
makepkg -si
```

`-s` pulls build deps via pacman if missing, `-i` installs the result
straight away. Without `-i`, install the built package manually:

```sh
sudo pacman -U libhangul-0.2.0-100-x86_64.pkg.tar.zst
```

## Keeping it installed across `pacman -Syu`

`pkgrel` is set higher than the real package's (`100` vs. the
real `1`), so pacman won't try to replace this build with the identical
`0.2.0-1` from `extra` on your next sync. It's **not** protected against
upstream shipping a genuinely newer version (`0.2.0-2`, `0.2.1`, ...) —
pacman will offer to "upgrade" over this build the moment that happens,
silently dropping the Sun-arae keyboard.

To pin it properly, add to `/etc/pacman.conf`:

```ini
[options]
IgnorePkg = libhangul
```

Then `pacman -Syu` will report `libhangul` as "ignoring package upgrade"
each time upstream releases, rather than replacing it — a reminder to
rebase `patches/0001-*.patch` onto the new version (usually a clean
`patch -p1` apply) and rebuild.

## Uninstalling / reverting to stock

```sh
sudo pacman -S libhangul   # reinstalls the real extra/libhangul package
```

(Remove the `IgnorePkg` line first if you added it.)
