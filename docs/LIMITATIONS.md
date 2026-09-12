# 한계점과 개선 여지

## 알려진 이슈: Omarchy의 systemd/D-Bus 경합

Omarchy 환경에서는 `omarchy-fcitx5.service`(systemd 사용자 유닛)와 D-Bus 서비스 활성화(`/usr/share/dbus-1/services/org.fcitx.Fcitx5.service`)가 각각 독립적으로 `org.fcitx.Fcitx5` 버스 이름을 점유하려고 시도할 수 있습니다. 둘 중 하나가 지면 `activating (auto-restart)` 크래시 루프(`Failed to create addon: dbus ... Is there another fcitx already running?`)에 빠집니다. 이는 Omarchy의 기존 설정 문제로 이 프로젝트의 패치와는 무관하지만, fcitx5를 재시작할 때 혼란을 줄 수 있어 기록해둡니다(수동으로 실행한 `fcitx5 -r &`가 systemd 유닛과 경합하면서 마치 패치가 조용히 되돌아가는 것처럼 보일 수 있습니다). 해결 방법: `pkill -9 fcitx5` 실행 후 `systemctl --user restart omarchy-fcitx5.service`로 인스턴스를 하나만 남깁니다.

## 아직 검증하지 않은 범위

실제 키보드 *하드웨어* 동작(Hyprland 환경)은 아직 확인하지 않았습니다: 키를 길게 눌렀을 때의 자동 반복(key repeat)이 이중 입력 규칙과 어떻게 상호작용하는지, IME 후보 창과의 상호작용, 그리고 지금까지 확인한 곳 외의 다른 입력 대상(브라우저, 터미널, GTK/Qt 텍스트 필드 등)이 대표적입니다. 전체 테스트 범위는 [`TESTING.md`](TESTING.md)를 참고하세요.

## 개선 여지: 업스트림 기여

두 패치 모두 독립적인 PR로 제출할 수 있도록 작성되어 있습니다.

- `patches/0001-add-dubeolsik-sunarae-keyboard.patch` → <https://github.com/libhangul/libhangul> (이슈 #31에서 정확히 이 기능을 요청 중).
- `patches/0002-fcitx5-hangul-add-sunarae-option.patch` → <https://github.com/fcitx/fcitx5-hangul>. libhangul 패치가 먼저 머지된 뒤에나 의미가 있음.
