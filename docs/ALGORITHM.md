# The 두벌식 순아래 (Dubeolsik Sun-arae) algorithm

Source: <https://sites.google.com/site/tinyduckn/dubeolsig-sun-alae> (by
꼬마집오리 / tinyduck), which credits 우덜(3beol), 팥알, 이호석, and
김용묵 (author of the Windows 날개셋 implementation).

## Why it exists

Standard 두벌식 (KS X 5002) puts consonants on the left hand and vowels
on the right, mapping the 19 basic consonant jamo and most vowel jamo
directly to keys. But it still needs Shift for:

- the 5 tense consonants ㄲㄸㅃㅆㅉ (both as initial *and* as batchim)
- the 2 tense-looking compound vowels ㅒ/ㅖ

순아래 removes that Shift dependency by reinterpreting a **repeated
keystroke** as the "make this tense" signal, instead of needing a
separate Shift-modified key. The physical key layout doesn't change at
all — same keys, same fingers, same hand assignment — only the
composition rules change.

## The rules

### 2.1 — Tense initial consonant via a doubled following vowel

After typing a plain consonant, pressing the *next* vowel key **twice**
(instead of once) retypes that consonant as its tense form. The vowel
itself still only counts once.

- 뜻 (ㄸ+ㅡ+ㅅ) = keys `ㄷ ㅡ ㅡ ㅅ` (d, eu, eu, s) — doubling `ㅡ` tenses
  `ㄷ`→`ㄸ`, then `ㅅ` is the batchim.
- 꽥 (ㄲ+ㅗㅐ+ㄱ) = keys `ㄱ ㅗ ㅗ ㅐ ㄱ` (g, o, o, ae, g) — doubling the
  *first* vowel of the diphthong ㅗㅐ tenses `ㄱ`→`ㄲ`; the diphthong
  ㅗ+ㅐ=ㅙ still composes normally afterward.
  - The Windows "24-key correspondence" variant of Sun-arae also allows
    doubling the *second* half of a diphthong instead: `ㄱ ㅗ ㅐ ㅐ ㄱ`
    (g, o, ae, ae, g) tenses the same way. Implemented for every
    diphthong that has no dedicated key of its own — ㅘㅙㅚㅝㅞㅟㅒㅖㅢ —
    since a buffered value like `WAE` can only ever have come from a
    two-keystroke combine, so doubling either half is unambiguous.
    **Not** implemented for `ㅐ`/`ㅔ` specifically: those two *do* have a
    dedicated key, so a buffered `ㅐ` might be a single keystroke or the
    result of `ㅏ+ㅣ` — there's no way to tell which, and treating every
    lone `ㅐ` followed by `ㅣ` as a tense-choseong signal would misfire
    on ordinary text (typing 개 then a separate ㅣ). See
    `hangul_ic_sunarae_is_second_half()` in the patch and
    `tests/test_matrix.c`'s "24-key correspondence" section for the
    full reasoning and the regression tests that pin this down.

### 2.2 — ㅒ/ㅖ/ㅙ/ㅞ without Shift

`ㅑ+ㅣ` composes to `ㅒ`, and `ㅕ+ㅣ` composes to `ㅖ` — the same way
`ㅗ+ㅏ` already composes to `ㅘ` in standard dubeolsik. No Shift key
involved. The page states this rule covers two more pairs the same
way: `ㅘ+ㅣ`→`ㅙ` and `ㅝ+ㅣ`→`ㅞ` (i.e. finish typing the diphthong,
then press `ㅣ` to extend it).

- 옛 (ㅇ+ㅖ+ㅅ) = keys `ㅇ ㅕ ㅣ ㅅ` (ieung, yeo, i, s).
- 왜 (ㅇ+ㅙ) = keys `ㅇ ㅗ ㅏ ㅣ` (ieung, o, a, i) — forms `ㅘ`, then `ㅣ`
  extends it to `ㅙ`. (The direct `ㅇ ㅗ ㅐ` spelling using the
  dedicated `ㅐ` key still works too, as always.)

The page separately describes a Windows-only "24-key correspondence"
variant that drops the dedicated `ㅐ`/`ㅔ` keys entirely, replacing them
with `ㅏ+ㅣ`→`ㅐ` and `ㅓ+ㅣ`→`ㅔ`. This implementation doesn't do the
full 24-key remap (the `ㅐ`/`ㅔ` keys still exist and still work), but
adds those two combinations anyway as harmless extras on top — pressing
`ㅏ` then `ㅣ` gives `ㅐ` in addition to the dedicated key still working.

### 2.3 — Tense batchim via a doubled batchim key

Pressing the same consonant key twice while it's acting as a batchim
(final consonant, not a new initial) tenses it, for the two batchim
that have a tense form (ㄲ, ㅆ).

- 걲 (거 + ㄲ batchim) = keys `ㄱ ㅓ ㄱ ㄱ` (g, eo, g, g).

### 2.4 — Fully compatible with the existing Shift-based input

Nothing above *removes* the old Shift keys for ㄲㄸㅃㅆㅉ/ㅒㅖ — it just
adds a second way to type them. The two methods can even be mixed in
the same syllable.

- 꺾 (ㄲ+ㅓ+ㄲ) can be typed as:
  - `ㄲ ㅓ ㄲ` — fully Shift-based (standard dubeolsik)
  - `ㄲ ㅓ ㄱ ㄱ` — Shift for the initial, doubling for the batchim
  - `ㄱ ㅓ ㅓ ㄲ` — doubling for the initial, Shift for the batchim
  - `ㄱ ㅓ ㅓ ㄱ ㄱ` — fully Shift-free (both rules 2.1 and 2.3)

## How this maps onto libhangul's automaton

libhangul already has almost everything needed as *existing, unused-by
default* machinery:

- `hangul_combination_table_default` (the table the standard "2"
  keyboard already uses) already contains choseong+choseong→ssang and
  jongseong+jongseong→ssang pairs (e.g. ㄱ+ㄱ→ㄲ at both choseong and
  jongseong level). They're just never reached in practice: the
  wrapper `hangul_ic_combine()` explicitly refuses to combine two
  *identical* jamo unless `option_combi_on_double_stroke` is turned on
  — and even then, that's the wrong mechanism for rule 2.1, which
  tenses a *different* jamo (the preceding choseong) than the one
  being doubled (the following jungseong).
- What's missing is rule 2.1's mechanism specifically: retyping the
  *choseong* in response to a *repeated jungseong* keystroke. That
  needs a new lookup table (keyed on a single jamo, not a pair) and a
  few lines checking "did the caller just press the same key as the
  one on top of the stack".
- Rule 2.2 just needs more rows in the jungseong combination table —
  pure data, no new logic. Shipped with `YA+I=YAE` and `YEO+I=YE`
  initially; `WA+I=WAE`, `WEO+I=WE`, `A+I=AE`, and `EO+I=E` were added
  after walking through every example on the spec page and testing
  live (see `docs/TESTING.md`) turned up the gap — the 3beol reference
  implementation this was ported from didn't have them either.
- Rule 2.3 needs jongseong+jongseong doubling to actually fire, i.e.
  the new automaton must call the raw table lookup
  (`hangul_keyboard_combine`) directly instead of going through the
  option-gated `hangul_ic_combine()` wrapper — but only for *this*
  keyboard, so every other keyboard keeps requiring
  `option_combi_on_double_stroke` (default off) as before.

See `patches/0001-add-dubeolsik-sunarae-keyboard.patch` for the actual
diff, and `docs/RESEARCH.md` for how this was found (an existing,
tested implementation in a libhangul fork, ported onto current
upstream rather than copied wholesale).
