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

- `libhangul`: implemented, builds, and passes local functional tests
  (see [`docs/TESTING.md`](docs/TESTING.md)), including via the
  bundled `hangul` CLI tool. Not yet installed system-wide — see
  [Installing](#installing).
- `fcitx5-hangul`: **not yet patched**. It hardcodes its selectable
  keyboards as a fixed list and doesn't read libhangul's keyboard
  registry, so the new layout isn't reachable from fcitx5's config yet
  even after installing the patched `libhangul`. See
  [`docs/NEXT-STEPS.md`](docs/NEXT-STEPS.md) for exactly what's needed
  and why it's deferred (build tooling not yet installed on this
  machine).

## Layout

Standard 두벌식 key positions (same as `Keyboard=Dubeolsik` in fcitx5).
What changes is composition:

| Rule | Effect | Example |
|---|---|---|
| 2.1 | Same **vowel** key twice right after a plain consonant → tenses that consonant | `ㄷㅡㅡㅅ` → 뜻 |
| 2.2 | `ㅑ+ㅣ` / `ㅕ+ㅣ` → `ㅒ`/`ㅖ`, no Shift | `ㅇㅕㅣㅅ` → 옛 |
| 2.3 | Same **batchim** key twice → tense batchim | `ㄱㅓㄱㄱ` → 걲 |
| 2.4 | Old Shift-based input still works; can be mixed | `ㄲㅓㄲ` / `ㄲㅓㄱㄱ` / `ㄱㅓㅓㄲ` → 꺾 |

## Repo layout

```
docs/            design notes, algorithm spec, research log, test log
patches/         the libhangul patch, as a standalone reviewable diff
vendor/libhangul patched libhangul source tree (patch already applied, builds as-is)
packaging/       Arch PKGBUILD to build+install the patched libhangul
fcitx5/          fcitx5-hangul config snippet to select the new layout
tests/           C test harness exercising libhangul's HangulInputContext API directly
```

## How it's implemented

A new libhangul keyboard, id `2sunarae` ("Dubeolsik Sun-arae"), added
directly to `libhangul` (not `kime` — see
[`docs/RESEARCH.md`](docs/RESEARCH.md) for why). It reuses the existing
"2" (dubeolsik) key table and adds one new automaton function plus two
small combination tables. No other keyboard's behavior changes.

`fcitx5-hangul` wraps `libhangul` and exposes a `Keyboard=` setting in
`~/.config/fcitx5/conf/hangul.conf`, but that setting is a fixed,
hardcoded enum (`HangulKeyboard` in `fcitx5-hangul/src/engine.h`), not
a passthrough to whatever `libhangul` reports — so `fcitx5-hangul`
itself also needs a small patch (two lists, each gaining one entry) to
make `2sunarae` selectable. See
[`docs/NEXT-STEPS.md`](docs/NEXT-STEPS.md) and
[`patches/0002-fcitx5-hangul-add-sunarae-option.patch.DRAFT`](patches/0002-fcitx5-hangul-add-sunarae-option.patch.DRAFT).

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

This replaces the system `libhangul` package, so it's packaged as a
proper pacman package rather than a bare `make install` (which pacman
can't track and a later `pacman -Syu` would silently fight with). See
[`packaging/PKGBUILD`](packaging/PKGBUILD) and
[`packaging/README.md`](packaging/README.md) for the build/install/
upgrade-pinning steps, and [`fcitx5/hangul.conf`](fcitx5/hangul.conf)
for the config change that actually switches your input method to
Sun-arae afterwards.

## Upstream

There's an open feature request for exactly this
(<https://github.com/libhangul/libhangul/issues/31>). `patches/0001-*.patch`
is written to be legible as a standalone contribution if upstream wants
it.
