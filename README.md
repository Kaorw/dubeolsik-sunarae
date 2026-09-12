# dubeolsik-sunarae

두벌식 순아래 (Dubeolsik Sun-arae) — a Shift-free variant of the standard
two-set (두벌식) Korean keyboard layout — packaged for **Omarchy** via
**Fcitx5** (`fcitx5-hangul` + `libhangul`).

Same physical key layout as standard 두벌식. The difference is entirely in
how repeated keystrokes are interpreted, so tense consonants (ㄲㄸㅃㅆㅉ)
and ㅒ/ㅖ never need Shift. See [`docs/ALGORITHM.md`](docs/ALGORITHM.md)
for the full rules and worked examples, and
[`docs/RESEARCH.md`](docs/RESEARCH.md) for why this ended up as a
`libhangul` patch instead of a `kime` layout.

## Status

**Working and installed.** Both `libhangul` (adds the `2sunarae`
keyboard) and `fcitx5-hangul` (adds it as a selectable `Keyboard`
option) are patched, built, packaged, and installed on this machine —
see [`packaging/README.md`](packaging/README.md). Typing through the
real fcitx5 input method with `Keyboard=Dubeolsik Sun-arae` selected
produces the worked examples correctly (뜻 confirmed live; see
[`docs/TESTING.md`](docs/TESTING.md) for the full test log, including
a debugging note about a systemd/D-Bus activation race that briefly
made this look broken).

## Layout

Standard 두벌식 key positions (same as `Keyboard=Dubeolsik` in fcitx5).
What changes is composition:

| Rule | Effect | Example |
|---|---|---|
| 2.1 | Same **vowel** key twice right after a plain consonant → tenses that consonant | `ㄷㅡㅡㅅ` → 뜻 |
| 2.2 | `ㅑ+ㅣ`/`ㅕ+ㅣ`/`ㅘ+ㅣ`/`ㅝ+ㅣ` → `ㅒ`/`ㅖ`/`ㅙ`/`ㅞ`, no Shift (plus `ㅏ+ㅣ`/`ㅓ+ㅣ` → `ㅐ`/`ㅔ` as extras) | `ㅇㅕㅣㅅ` → 옛 |
| 2.3 | Same **batchim** key twice → tense batchim | `ㄱㅓㄱㄱ` → 걲 |
| 2.4 | Old Shift-based input still works; can be mixed | `ㄲㅓㄲ` / `ㄲㅓㄱㄱ` / `ㄱㅓㅓㄲ` → 꺾 |

## Repo layout

```
docs/                 design notes, algorithm spec, research log, test log
patches/              the two patches (libhangul, fcitx5-hangul), as standalone reviewable diffs
vendor/libhangul      patched libhangul source tree (patch already applied, builds as-is)
vendor/fcitx5-hangul  patched fcitx5-hangul source tree (same)
packaging/libhangul   Arch PKGBUILD to build+install the patched libhangul
packaging/fcitx5-hangul  Arch PKGBUILD to build+install the patched fcitx5-hangul
fcitx5/               fcitx5-hangul config snippet to select the new layout
tests/                C test harness exercising libhangul's HangulInputContext API directly
```

## How it's implemented

A new libhangul keyboard, id `2sunarae` ("Dubeolsik Sun-arae"), added
directly to `libhangul` (not `kime` — see
[`docs/RESEARCH.md`](docs/RESEARCH.md) for why). It reuses the existing
"2" (dubeolsik) key table and adds one new automaton function plus two
small combination tables. No other keyboard's behavior changes.

`fcitx5-hangul` wraps `libhangul` and exposes a `Keyboard=` setting in
`~/.config/fcitx5/conf/hangul.conf`, but that setting was a fixed,
hardcoded enum (`HangulKeyboard` in `fcitx5-hangul/src/engine.h`), not
a passthrough to whatever `libhangul` reports — so `fcitx5-hangul`
needed its own small patch too (two lists, each gaining one entry, both
appends so no saved config is renumbered) to make `2sunarae` selectable
as "Dubeolsik Sun-arae". See
[`patches/0002-fcitx5-hangul-add-sunarae-option.patch`](patches/0002-fcitx5-hangul-add-sunarae-option.patch)
and `test/testhangulsunarae.cpp` in `vendor/fcitx5-hangul` for an
end-to-end test through a real fcitx5 `Instance`.

## Building

```sh
cd vendor/libhangul
autoreconf -fi        # only needed once, or after patches/ changes
./configure --prefix=/usr --libdir=/usr/lib
make -j$(nproc)
```

Sanity-check the automaton without touching your system:

```sh
gcc tests/test_sunarae.c -Ivendor/libhangul/hangul -o /tmp/test_sunarae \
    -Lvendor/libhangul/hangul/.libs -lhangul
LD_LIBRARY_PATH=vendor/libhangul/hangul/.libs /tmp/test_sunarae
```

## Installing

This replaces two system packages, so both are packaged as proper
pacman packages rather than a bare `make`/`cmake --install` (which
pacman can't track and a later `pacman -Syu` would silently fight
with). See [`packaging/README.md`](packaging/README.md) for the full
build/install/upgrade-pinning steps, and
[`fcitx5/hangul.conf`](fcitx5/hangul.conf) for the config change that
actually switches your input method to Sun-arae afterwards.

## Upstream

There's an open feature request for exactly this
(<https://github.com/libhangul/libhangul/issues/31>). `patches/0001-*.patch`
is written to be legible as a standalone contribution if upstream wants
it.
