# Packaging & install

Two packages, each a small patch on top of the real Arch package,
same files/soname otherwise:

- `libhangul/` — adds the `2sunarae` keyboard.
- `fcitx5-hangul/` — adds "Dubeolsik Sun-arae" as a selectable
  `Keyboard` option (needs the patched `libhangul` above to point at;
  builds fine against stock `libhangul` too, it just has nothing to
  select yet).

Both installed and in use as of 2026-09-12 — see
[`../docs/TESTING.md`](../docs/TESTING.md) for what's been verified.

## Build

```sh
cd packaging/libhangul && makepkg -si
cd ../fcitx5-hangul && makepkg -si
```

`-s` pulls build deps via pacman if missing, `-i` installs the result
straight away. Without `-i`, install the built packages manually:

```sh
sudo pacman -U packaging/libhangul/libhangul-0.2.0-100-x86_64.pkg.tar.zst
sudo pacman -U packaging/fcitx5-hangul/fcitx5-hangul-5.1.11-100-x86_64.pkg.tar.zst
```

## Selecting the layout

```sh
cp fcitx5/hangul.conf ~/.config/fcitx5/conf/hangul.conf
```

then restart fcitx5 so it re-reads the config. On this Omarchy machine
that's `systemctl --user restart omarchy-fcitx5.service` — **not** a
bare `fcitx5 -r &`, which raced with the systemd-managed instance over
the D-Bus name and made things worse (see the debugging notes in
`docs/TESTING.md`). If in doubt, `pkill -9 fcitx5` first, then let
systemd (or however Omarchy manages it) restart it fresh.

## Keeping both installed across `pacman -Syu`

Each `pkgrel` is set higher than the real package's (`100` vs. the
real `1`), so pacman won't try to replace either build with the
identical upstream version on your next sync. That's **not** protected
against upstream shipping a genuinely newer version (a `0.2.1`
`libhangul` or `5.1.12` `fcitx5-hangul`, say) — pacman will offer to
"upgrade" over these builds the moment that happens, silently dropping
the Sun-arae support.

To pin them properly, add to `/etc/pacman.conf`:

```ini
[options]
IgnorePkg = libhangul fcitx5-hangul
```

(Already done on this machine.) Then `pacman -Syu` will report both as
"ignoring package upgrade" each time upstream releases, rather than
replacing them — a reminder to rebase the relevant `patches/*.patch`
onto the new version (usually a clean `patch -p1` apply) and rebuild
both.

## Uninstalling / reverting to stock

```sh
sudo pacman -S libhangul fcitx5-hangul   # reinstalls the real extra/ packages
```

(Remove the `IgnorePkg` line first if you added it, and reset
`~/.config/fcitx5/conf/hangul.conf`'s `Keyboard=` line back to
`Dubeolsik` or delete it.)
