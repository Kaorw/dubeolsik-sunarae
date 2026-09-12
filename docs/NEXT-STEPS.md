# Next steps

## Done

1. ~~Patch `fcitx5-hangul`~~ — done, see
   `patches/0002-fcitx5-hangul-add-sunarae-option.patch` and
   `vendor/fcitx5-hangul/test/testhangulsunarae.cpp` (end-to-end test
   through a real fcitx5 `Instance`, passing).
2. ~~Install the patched `libhangul` system-wide~~ — done
   (`0.2.0-100`, pinned via `IgnorePkg` in `/etc/pacman.conf`).
3. ~~Install the patched `fcitx5-hangul` system-wide~~ — done
   (`5.1.11-100`, pinned the same way).
4. ~~End-to-end test through real fcitx5~~ — done: typing `emmt` with
   `Keyboard=Dubeolsik Sun-arae` selected produces 뜻 in a live text
   field (2026-09-12).

## In progress

- **Exhaustive combination testing** — the user asked to test as many
  key combinations as possible beyond the five worked examples already
  covered. See `docs/TESTING.md` for the expanded test matrix (all 5
  tense consonants × several vowel contexts, both tense batchim, all
  standard diphthongs and compound batchim as a regression check, and
  a broader word list).

## Known separate issue (not this project's bug)

This machine's `omarchy-fcitx5.service` (systemd user unit) and D-Bus
service activation (`/usr/share/dbus-1/services/org.fcitx.Fcitx5.service`)
both try to own the `org.fcitx.Fcitx5` bus name independently. Whichever
loses ends up in an `activating (auto-restart)` crash loop
(`Failed to create addon: dbus ... Is there another fcitx already
running?`). Pre-existing Omarchy configuration, unrelated to this
project's patches — noted here because it made testing confusing (a
manually-run `fcitx5 -r &` raced with the systemd unit and looked like
our patch was silently reverting config). Workaround used during
testing: `pkill -9 fcitx5` then `systemctl --user restart
omarchy-fcitx5.service` to get a single, clean instance. Worth a
separate look if it keeps causing trouble, but out of scope here.

## Optional: propose upstream

Both patches are written to be legible as standalone PRs:

- `patches/0001-add-dubeolsik-sunarae-keyboard.patch` against
  <https://github.com/libhangul/libhangul> (open issue #31 asks for
  exactly this).
- `patches/0002-fcitx5-hangul-add-sunarae-option.patch` against
  <https://github.com/fcitx/fcitx5-hangul>, would only make sense once
  (if) the libhangul patch lands upstream first.
