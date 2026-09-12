# Next steps

## 1. Patch `fcitx5-hangul` so the layout is actually selectable

Confirmed by reading `fcitx5-hangul`'s source (tag `5.1.11`, matching
the installed `fcitx5-hangul 5.1.11-1`): its `Keyboard` config option
is a fixed `enum class HangulKeyboard` (`src/engine.h`) mapped through
a parallel `static const char *keyboardId[]` array (`src/engine.cpp`)
— it does **not** enumerate whatever `libhangul` reports at runtime.
So a patched `libhangul` with `2sunarae` registered is necessary but
not sufficient; `fcitx5-hangul` needs one small patch too:

- append `Dubeolsik_Sunarae` to the `HangulKeyboard` enum
- append `N_("Dubeolsik Sun-arae")` to the matching
  `FCITX_CONFIG_ENUM_NAME_WITH_I18N` name list
- append `"2sunarae"` to `keyboardId[]`

All three are **appends**, not insertions, so nobody's saved
`Keyboard=` config value (stored as enum name, not index) breaks.

The exact diff is drafted (not yet built or tested) in
`patches/0002-fcitx5-hangul-add-sunarae-option.patch.DRAFT`.

This was scoped out of the current pass because it needs `cmake` and
`extra-cmake-modules`, which aren't installed on this machine, and
installing packages needs the user's `sudo` password interactively.
When ready:

```sh
sudo pacman -S --needed cmake extra-cmake-modules
```

then clone `fcitx5-hangul` at tag `5.1.11` (or whatever's installed —
check with `pacman -Qi fcitx5-hangul`), apply the finished patch,
build with CMake, and package the same way `packaging/PKGBUILD` does
for `libhangul` (a small second PKGBUILD, `provides`/`conflicts`
against the real `fcitx5-hangul`).

## 2. Install the patched `libhangul` system-wide

`packaging/PKGBUILD` is built and ready
(`makepkg` succeeds — see `docs/TESTING.md`). Installing it replaces
the system `libhangul` package, which is a change worth confirming
with the user explicitly before running, since:

- it affects every application that links `libhangul` (any Fcitx5 or
  ibus Korean input, not just this project)
- it needs `sudo pacman -U`
- it needs `IgnorePkg = libhangul` in `/etc/pacman.conf` to stay
  installed across `pacman -Syu` (see `packaging/README.md`)

## 3. End-to-end test once both patches are installed

- `fcitx5-hangul` selectable via `fcitx5-configtool` or
  `fcitx5/hangul.conf`.
- Type the worked examples from `docs/ALGORITHM.md` (뜻, 꽥, 옛, 걲, 꺾)
  into a real text field (terminal, browser) under Hyprland/Omarchy,
  not just through the library-level test harness.

## 4. Optional: propose upstream

`patches/0001-add-dubeolsik-sunarae-keyboard.patch` is written to be
legible as a standalone PR against
<https://github.com/libhangul/libhangul>, which has an open issue
asking for exactly this (#31). Worth opening a PR there so this
doesn't need to be carried as a local patch indefinitely.
