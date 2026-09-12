# Testing log

## Level 1 — assumption check against the unpatched, installed `libhangul`

Before writing any code, checked (via `tests/test_hangul.c`) that the
standard `"2"` keyboard on the **system** `libhangul 0.2.0-1` really
doesn't already do any of this:

- `gg` (no vowel yet) does not combine into `ㄲ` — confirms
  `option_combi_on_double_stroke` is off by default.
- `ya`,`i` does not combine into `ㅒ` — confirms
  `hangul_combination_table_default` has no such entry.
- batchim doubling (`g,a` then `g,g`) does not combine into a tense
  batchim either, same option gate.

This ruled out "maybe stock libhangul already supports this" and gave
a same-process reference baseline that `tests/test_sunarae.c` reuses
(every Sun-arae check has a `"2"`-keyboard control run alongside it).

## Level 2 — the new automaton, via libhangul's public API directly

`tests/test_sunarae.c` drives `HangulInputContext` (`hangul_ic_new`,
`hangul_ic_process`, `hangul_ic_get_commit_string`, `hangul_ic_flush`)
against keyboard id `2sunarae`, reproducing every worked example from
`docs/ALGORITHM.md`:

| Input keys | Rule | Expected | Got |
|---|---|---|---|
| `emmt` (d,eu,eu,s) | 2.1 | 뜻 | 뜻 ✅ |
| `rhhor` (g,o,o,ae,g) | 2.1 | 꽥 | 꽥 ✅ |
| `rhoor` (g,o,ae,ae,g) | 2.1, 24-key alt | 꽥 | 괘ㅐㄱ ❌ (expected — see below) |
| `il` (ya,i) | 2.2 | ㅒ | ㅒ ✅ |
| `ul` (yeo,i) | 2.2 | ㅖ | ㅖ ✅ |
| `dult` (ieung,yeo,i,s) | 2.2 | 옛 | 옛 ✅ |
| `rjrr` (g,eo,g,g) | 2.3 | 걲 | 걲 ✅ |
| `rjtt` (g,eo,s,s) | 2.3 | 겄 | 겄 ✅ |
| `rjjrr` (g,eo,eo,g,g) | 2.1 + 2.3 | 꺾 | 꺾 ✅ |
| `rk` (g,a) | sanity | 가 | 가 ✅ |

The one intentional non-match (`rhoor`, doubling the *second* half of
a diphthong instead of the first) is the Windows "24-key
correspondence" variant mentioned in the spec, not the primary form —
see `docs/ALGORITHM.md` rule 2.1 for why this implementation doesn't
attempt it, and `docs/NEXT-STEPS.md`/upstream discussion if that's
ever wanted.

Every check above also ran against the plain `"2"` keyboard as a
control in the same process, confirming the standard keyboard's
behavior is unchanged (byte-for-byte identical output to the Level 1
baseline).

Run it yourself:

```sh
gcc tests/test_sunarae.c -Ivendor/libhangul/hangul -o /path/under/HOME/test_sunarae \
    -Lvendor/libhangul/hangul/.libs -lhangul
LD_LIBRARY_PATH=vendor/libhangul/hangul/.libs /path/under/HOME/test_sunarae
```

**Gotcha found while testing:** build/run this under your home
directory (or anywhere not `/tmp`) — on this machine, a binary linked
against `-lhangul` and run with `LD_LIBRARY_PATH` pointing at a
`/tmp/...` path *silently* falls back to the system `libhangul.so.1`
instead (no error, no warning — `hangul_keyboard_list_get_count()`
just quietly returns the stock count). Confirmed by `md5sum`-comparing
the exact same `.so` file tested from both locations: identical bytes,
different behavior purely based on the directory it's loaded from.
Almost certainly some hardening around loading libraries from
world-writable paths. Not investigated further since it's a
non-issue once you stop using `/tmp`.

## Level 3 — the built package

```sh
cd packaging && makepkg -f
```

succeeds and produces `libhangul-0.2.0-100-x86_64.pkg.tar.zst` with
the exact same 21-file layout as the real Arch `libhangul` package
(compared via `pacman -Ql libhangul`), same soname
(`libhangul.so.1.1.0`). Verified (from a `~/...` path, not `/tmp`, per
the gotcha above) that:

- `usr/bin/hangul --list` from the built package lists `2sunarae
  Dubeolsik Sun-arae` alongside the 9 stock keyboards.
- `usr/bin/hangul -k 2sunarae -i "emmtrhhordultrjrrrjjrr"` outputs
  `뜻꽥옛걲꺾` — the five worked examples concatenated, matching Level 2
  exactly, confirming the *packaged* binary (not just the build
  directory) behaves correctly.

## Not yet tested

- **Through fcitx5 itself** — blocked on the `fcitx5-hangul` patch
  (see `docs/NEXT-STEPS.md`); the new keyboard isn't reachable from
  fcitx5's config yet.
- **Installed system-wide** — the package hasn't been installed via
  `pacman -U` yet; see `docs/NEXT-STEPS.md` for why (needs explicit
  user confirmation since it replaces a system library).
- Real keyboard/IME behavior under Hyprland (key repeat interacting
  with the double-keystroke rules, IME candidate window, etc.) —
  can only be checked once the above two are done.
