#!/usr/bin/env bash
# fcitx5-hangul 의 자판을 두벌식 순아래로 바꾸고, 입력기 목록에 한글(hangul)이 없으면 더한다.
#
#   scripts/enable-fcitx5.sh             순아래로 설정
#   scripts/enable-fcitx5.sh --remove    표준 두벌식으로 되돌림 (입력기 목록은 그대로 둠)
#
# ~/.config/fcitx5/conf/hangul.conf 의 다른 설정(한자 키 등)은 건드리지 않는다.
# fcitx5 는 끝날 때 설정을 덮어쓰므로, 돌고 있으면 멈춘 뒤 고치고 다시 띄운다.
# 고치기 전 파일은 *.bak-sunarae 로 남긴다.
set -euo pipefail

action=add
while (($#)); do
    case "$1" in
        --remove) action=remove ;;
        -h|--help) sed -n '2,9p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "알 수 없는 옵션: $1" >&2; exit 2 ;;
    esac
    shift
done

conf_dir="${XDG_CONFIG_HOME:-$HOME/.config}/fcitx5"

if [[ $action == add ]] && ! grep -qs 'Dubeolsik Sun-arae' /usr/lib/fcitx5/libhangul.so /usr/lib/libhangul.so.1; then
    echo "순아래가 들어간 libhangul·fcitx5-hangul 이 설치되어 있지 않습니다. 먼저 packaging/ 의 두 패키지를 설치해 주세요." >&2
    exit 1
fi

# ── 돌고 있는 fcitx5 멈추기 ─────────────────────────────
# systemd 사용자 서비스로 돌면 그 서비스를, 아니면 프로세스를 같은 인자로 다시 띄운다.
# (SUNARAE_NO_RESTART=1 이면 fcitx5 를 건드리지 않는다: 테스트용)
service=
cmdline=()
pid=
if [[ -n ${SUNARAE_NO_RESTART:-} ]]; then
    :
elif command -v systemctl >/dev/null; then
    service=$(systemctl --user list-units --type=service --state=running --no-legend 2>/dev/null \
        | awk '{print $1}' | grep -i fcitx5 | head -n1 || true)
fi
[[ -z ${SUNARAE_NO_RESTART:-} && -z $service ]] && pid=$(pgrep -u "$(id -u)" -x fcitx5 | head -n1 || true)
if [[ -n $service ]]; then
    systemctl --user stop "$service"
elif [[ -n $pid ]]; then
    mapfile -d '' -t cmdline < "/proc/$pid/cmdline"
    kill "$pid"
    for _ in $(seq 50); do kill -0 "$pid" 2>/dev/null || break; sleep 0.1; done
fi

restart() {
    if [[ -n $service ]]; then
        systemctl --user start "$service"
    elif ((${#cmdline[@]})); then
        # -d 로 데몬이 되므로 이 스크립트가 끝나도 남는다
        setsid "${cmdline[@]}" -d >/dev/null 2>&1 < /dev/null || true
    fi
}
trap restart EXIT

# ── 설정 고치기 ──────────────────────────────────────
mkdir -p "$conf_dir/conf"
for f in profile conf/hangul.conf; do
    [[ -f $conf_dir/$f ]] && cp "$conf_dir/$f" "$conf_dir/$f.bak-sunarae"
done

python3 - "$conf_dir" "$action" <<'PY'
import os, re, sys

conf_dir, action = sys.argv[1:]

def read(path):
    """fcitx 설정(INI) → [(섹션, [줄...])]. 섹션 앞의 줄은 섹션 이름 ''."""
    secs = [["", []]]
    if os.path.exists(path):
        for line in open(path, encoding="utf-8").read().splitlines():
            m = re.match(r"^\[(.+)\]\s*$", line)
            if m:
                secs.append([m.group(1), []])
            else:
                secs[-1][1].append(line)
    return secs

def write(path, secs):
    out = []
    for name, lines in secs:
        if name:
            out.append(f"[{name}]")
        while lines and not lines[-1].strip():
            lines.pop()
        out += lines + [""]
    open(path, "w", encoding="utf-8").write("\n".join(out).lstrip("\n"))

def section(secs, name):
    for s in secs:
        if s[0] == name:
            return s
    s = [name, []]
    secs.append(s)
    return s

def setkey(sec, key, value):
    for i, line in enumerate(sec[1]):
        # 주석 처리된 기본값(# Keyboard=...)도 같은 자리에서 바꾼다
        if re.match(rf"^#?\s*{re.escape(key)}=", line):
            sec[1][i] = f"{key}={value}"
            return
    while sec[1] and not sec[1][-1].strip():
        sec[1].pop()
    sec[1].append(f"{key}={value}")

def get(sec, key):
    for line in sec[1]:
        if line.startswith(key + "="):
            return line.split("=", 1)[1]
    return None

# conf/hangul.conf: 자판만 바꾼다
h = os.path.join(conf_dir, "conf", "hangul.conf")
secs = read(h)
setkey(secs[0], "Keyboard", '"Dubeolsik Sun-arae"' if action == "add" else "Dubeolsik")
write(h, secs)
print("한글 자판:", "두벌식 순아래" if action == "add" else "두벌식")

if action != "add":
    sys.exit()

# profile: 첫 그룹(Groups/0)에 hangul 이 없으면 끝에 더한다
p = os.path.join(conf_dir, "profile")
secs = read(p)
item = re.compile(r"^Groups/0/Items/(\d+)$")
items = [(int(item.match(n).group(1)), get([n, l], "Name")) for n, l in secs if item.match(n)]
names = [name for _, name in sorted(items)]
if "hangul" not in names:
    if not names:
        names = ["keyboard-us"]
    names.append("hangul")
    secs = [s for s in secs if not item.match(s[0])]
    g = section(secs, "Groups/0")
    setkey(g, "Name", get(g, "Name") or "Default")
    setkey(g, "Default Layout", get(g, "Default Layout") or "us")
    if not get(g, "DefaultIM"):
        setkey(g, "DefaultIM", "hangul")
    at = secs.index(g) + 1
    for i, name in enumerate(names):
        secs.insert(at + i, [f"Groups/0/Items/{i}", [f"Name={name}", "Layout="]])
    order = section(secs, "GroupOrder")
    if not get(order, "0"):
        setkey(order, "0", get(g, "Name"))
    write(p, secs)
print("입력기 그룹:", " / ".join(names))
PY
