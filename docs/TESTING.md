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

## Level 4 — end-to-end through a real fcitx5 `Instance`

`vendor/fcitx5-hangul/test/testhangulsunarae.cpp` builds a full
`fcitx::Instance` (the same one used by fcitx5-hangul's own upstream
test), registers the `hangul` input method, selects
`Keyboard=Dubeolsik Sun-arae` via `RawConfig` — the exact mechanism a
real `~/.config/fcitx5/conf/hangul.conf` uses — and drives key events
through `testfrontend`, asserting the five worked examples commit
correctly. Runs via `ctest` in `vendor/fcitx5-hangul/build`, and again
automatically as the `check()` step of
`packaging/fcitx5-hangul/PKGBUILD`. Passing.

## Level 5 — installed system-wide, real desktop session (2026-09-12)

Both patched packages installed (`libhangul 0.2.0-100`,
`fcitx5-hangul 5.1.11-100`), `Keyboard=Dubeolsik Sun-arae` set in the
live `~/.config/fcitx5/conf/hangul.conf`, fcitx5 restarted. Typing
`emmt` (no Shift) into a real text field produced 뜻 — confirmed by the
user directly.

**Debugging note:** the first restart attempt (`fcitx5 -r &` run
manually) looked like it silently reverted the config back to
`# Keyboard=Dubeolsik` (commented out). Root cause turned out to be
unrelated to the patches: this machine's `omarchy-fcitx5.service`
(systemd user unit) and D-Bus service activation
(`/usr/share/dbus-1/services/org.fcitx.Fcitx5.service`) both try to
own the `org.fcitx.Fcitx5` bus name. Running `fcitx5 -r &` by hand
raced with the systemd unit; the loser crash-loops
(`Failed to create addon: dbus ... Is there another fcitx already
running?`), and whichever instance actually holds the config in memory
periodically calls `updateAction()` → `safeSaveAsIni()` (see
`fcitx5-hangul/src/engine.cpp`), which writes its **current in-memory
config** back to disk on every input-method activation — so an
instance that started before the file was edited will happily
overwrite a good edit with its own stale default. Fixed by `pkill -9
fcitx5` then `systemctl --user restart omarchy-fcitx5.service` to get
exactly one instance, re-applying the config, and testing before any
other activation could re-save a stale value. See
`docs/NEXT-STEPS.md` for the (separate, pre-existing) systemd/D-Bus
race itself.

## Level 6 — exhaustive combination matrix (`tests/test_matrix.c`)

Prompted by the request to verify as many combinations as possible.
Rather than hand-typing expected Hangul glyphs (error-prone — see the
transcription mistakes caught and fixed earlier in this log), this
harness cross-validates: for every Sun-arae no-Shift key sequence, it
also types the *same target syllable* via the ordinary Shift-based
standard `"2"` keyboard, and asserts the two outputs are byte-for-byte
identical. The standard keyboard's Shift-based composition is
separate, pre-existing, unmodified code, so a match is real evidence
of correctness, not a self-fulfilling comparison against the same
logic being tested.

**62/62 checks pass**, run against both the dev build
(`vendor/libhangul`) and the installed system `libhangul`. Coverage:

| Group | Count | What it checks |
|---|---|---|
| Rule 2.1, simple vowel | 10 | All 5 tense consonants (ㄲㄸㅃㅆㅉ) × 2 vowels (ㅏ, ㅓ) |
| Rule 2.1 + batchim | 5 | Tense consonant syllable followed by a plain batchim |
| Rule 2.1, diphthong doubling | 4 | Doubling the *first* vowel of a diphthong (ㅘ/ㅝ/ㅟ-forming) still tenses the consonant |
| Rule 2.2 | 4 | ㅑ+ㅣ=ㅒ, ㅕ+ㅣ=ㅖ, incl. with a following consonant/choseong and batchim |
| Rule 2.3 | 4 | Doubled batchim key → ㄲ/ㅆ batchim, in different syllables |
| Rule 2.4 | 4 | All four ways to type 깎 (shift/shift, tensify/shift, shift/tensify, tensify/tensify) are identical |
| Diphthong regression | 7 | ㅘㅙㅚㅝㅞㅟㅢ still compose exactly as on standard dubeolsik |
| Compound batchim regression | 10 | ㄳㄵㄶㄺㄻㄽㄾㄿㅀㅄ still compose exactly as on standard dubeolsik |
| Non-tensable consonant regression | 9 | ㄴㄹㅁㅇㅎㅋㅌㅍㅊ (no tense form) doubling a vowel behaves *identically* to standard dubeolsik — proves the new tensify check doesn't change anything when it doesn't apply |
| Broader words | 5 | 꽃, 떨다, 싸다, 짰다, 깩 — multi-syllable, mixing rules |

Run it:

```sh
gcc tests/test_matrix.c $(pkg-config --cflags --libs libhangul) -o /path/under/HOME/test_matrix
/path/under/HOME/test_matrix
```

(or link against `vendor/libhangul/hangul/.libs` instead of the
system's `pkg-config` output, to test a not-yet-installed build —
same `/tmp` gotcha from Level 2 applies.)

## Not yet tested

- Real keyboard *hardware* behavior under Hyprland — key repeat (a
  physically held-down key auto-repeating) interacting with the
  double-keystroke rules, IME candidate window interaction, and other
  input methods/apps (browser, terminal, GTK/Qt text fields) beyond
  the one field already checked in Level 5.
- The Windows "24-key correspondence" alternate forms of rule 2.1
  (doubling the *second* half of a diphthong, or dropping ㅐ/ㅔ
  entirely) — known, documented non-goal; see `docs/ALGORITHM.md`.
