# Research log / decisions

## Goal

Add 두벌식 순아래 (Dubeolsik Sun-arae) as a usable Korean input method on
Omarchy, originally via `kime` (per the initial ask), falling back to
Fcitx5 if `kime` turns out to be impractical.

## Environment found

- Omarchy ships **Fcitx5**, not `kime`. `fcitx5` and `fcitx5-hangul`
  (which wraps `libhangul` 0.2.0) are already installed.
- `kime` is not installed and isn't in play unless explicitly added.

## Why not `kime`

Cloned `Riey/kime` and read `src/engine/backends/hangul/src/{lib,state,
characters}.rs`. Its Hangul automaton is addon-flag driven
(`Addon::ComposeChoseongSsang`, `ComposeJungseongSsang`,
`ComposeJongseongSsang`, `FlexibleComposeOrder`, ...), and two of the
three 순아래 rules map cleanly onto existing addons:

- rule 2.2 (ㅑ+ㅣ=ㅒ, ㅕ+ㅣ=ㅖ) = `Addon::ComposeJungseongSsang`, already
  implemented.
- rule 2.3 (batchim doubling) = `Addon::ComposeJongseongSsang`, already
  implemented.

But rule 2.1 — pressing a **vowel** key twice to tense the *preceding
consonant* — has no equivalent addon. kime's existing
`ComposeChoseongSsang` doubles the **consonant** key itself (before any
vowel), which is a different, simpler no-shift scheme, not Sun-arae.
Implementing rule 2.1 in kime means writing new Rust automaton logic
(a new `Addon` variant, changes to `CharacterState::jung()` in
`state.rs` to detect "the incoming jungseong equals the last jamo
pushed" and reach back into the choseong) — plausible, but new,
untested logic with no reference implementation to check it against.

## Why Fcitx5 + a patched `libhangul`

Searched for prior art and found that 두벌식 순아래 has already been
built, shipped, and used for years — just not in `kime` and not (yet)
in upstream `libhangul`:

- <https://github.com/libhangul/libhangul/issues/31> — an open upstream
  feature request for exactly this layout, unresolved.
- <https://gitlab.com/3beol/libhangul> — a community fork ("3beol",
  a.k.a. 우덜) that actually implements it, as keyboard ids `2noshift`
  (두벌식 순아래) and `2n9256` (a North Korean KPS 9256 variant). Used in
  the wild via `gureum` (macOS) and `HamoniKR` 6.0 (Linux).
- The tinyduck spec page itself references this fork and its
  Windows-side sibling (김용묵's 날개셋).

The 3beol fork's last sync with upstream `libhangul` was **2019-12-30**
(commit `36abb95`), while upstream is actively maintained (`0.1.0` →
`0.2.0`, commits into 2026). Rather than adopt the whole 6-year-stale
fork (losing years of unrelated upstream fixes) or run two libhangul
forks side by side, I isolated just the Sun-arae-relevant commits
(`fb87289` "Dubeolsik Noshift", `8811202`, `5244cb3` "두벌식 순아래") and
**reimplemented the same mechanism against current upstream HEAD**
(`a34aef7`, 2026-01-15 — the same commit the installed Arch
`libhangul 0.2.0-1` package is built from), reusing 3beol's already
sorted-out design (which table slot to use, which stack peek to
compare, which examples must round-trip) as the reference to verify
against, rather than as source to copy wholesale. Details of the
mechanism itself are in `docs/ALGORITHM.md`.

This came out simpler than expected because current upstream
`libhangul` already converged on the same code shape 3beol's patch
assumed (`hangul_ic_process_jamo` today is structurally near-identical
to 3beol's `hangul_ic_process_jamo_dubeol`), and two of three rules
turned out to need *no new logic* — only a data table addition (rule
2.2) or bypassing an existing option gate for one keyboard (rule 2.3).
Only rule 2.1 needed a genuinely new code path.

### Why patch `libhangul` and not `fcitx5-hangul`

`fcitx5-hangul` is a thin wrapper: it calls `hangul_ic_new(keyboard_id)`
and forwards keys to `hangul_ic_process()`. It already reads a
`Keyboard=` setting from `~/.config/fcitx5/conf/hangul.conf` and maps
the human-readable name to whichever keyboard id `libhangul` reports.
So adding a keyboard to `libhangul` and keeping the ABI stable (same
`HangulKeyboard`/`HangulCombination` struct shapes, additive-only
changes) means the installed `fcitx5-hangul` binary needs **no
rebuild** — only `libhangul.so` gets replaced, and a config line
selects the new layout. Confirmed by inspecting the installed
`fcitx5-hangul` addon config (`/usr/share/fcitx5/addon/hangul.conf`,
`~/.config/fcitx5/conf/hangul.conf`) before writing any code.

### Rejected approach: also enabling `option_combi_on_double_stroke`

An earlier idea was to lean on `libhangul`'s existing
`CombiOnDoubleStroke` IC option (exposed in fcitx5-hangul's config) for
rule 2.3, since the default combination table already has the needed
ㄱ+ㄱ→ㄲ / ㅅ+ㅅ→ㅆ jongseong pairs. Rejected because that option is
IC-wide, not per-keyboard — turning it on would also change behavior
for anyone using plain standard dubeolsik (typing `gg` quickly would
silently become `ㄲ`, surprising users who didn't ask for that). The
new `2sunarae` keyboard instead calls the raw combination table
directly for jongseong doubling, bypassing the option entirely, so the
option's default (off) keeps meaning what it already means for every
other keyboard.

## Empirical verification

Before writing any C, built a tiny harness (kept as
`tests/test_hangul.c`) against the **installed, unpatched** system
`libhangul` to check assumptions rather than guess from reading source:

- confirmed the standard `"2"` keyboard does *not* combine `gg`→ㄲ or
  `tt`(batchim)→ㅆⓉ by default (`option_combi_on_double_stroke` is off
  by default, as `~/.config/fcitx5/conf/hangul.conf` also documents).
- confirmed `ㅑ+ㅣ` and `ㅕ+ㅣ` do *not* combine on the standard keyboard
  (no such entries in `hangul_combination_table_default`).

This ruled out "maybe stock libhangul already does this" before
spending time on a patch, and gave a reference baseline (`tests/
test_sunarae.c` runs the same probes against both `"2"` and
`"2sunarae"` side by side).
