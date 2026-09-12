# 패키징 & 설치

두 패키지 모두, 실제 Arch 패키지 위에 작은 패치 하나를 얹은 형태이며 파일 구성과 soname은 동일합니다.

- `libhangul/`: `2sunarae` 키보드 추가.
- `fcitx5-hangul/`: "Dubeolsik Sun-arae"를 선택 가능한 `Keyboard` 옵션으로 추가 (선택하려면 위 패치된 `libhangul`이 필요하며, stock `libhangul`에 대해서도 빌드는 되지만 선택할 옵션이 아직 없음).

두 패키지 모두 설치되어 사용 중입니다. 검증 내용은 [`../docs/TESTING.md`](../docs/TESTING.md)를 참고하세요.

## 빌드

```sh
cd packaging/libhangul && makepkg -si
cd ../fcitx5-hangul && makepkg -si
```

`-s`는 빌드 의존성을 pacman으로 설치하고, `-i`는 빌드 결과를 바로 설치합니다. `-i` 없이 빌드했다면 다음과 같이 수동 설치하세요.

```sh
sudo pacman -U packaging/libhangul/libhangul-0.2.0-103-x86_64.pkg.tar.zst
sudo pacman -U packaging/fcitx5-hangul/fcitx5-hangul-5.1.11-100-x86_64.pkg.tar.zst
```

## 자판 선택

```sh
cp fcitx5/hangul.conf ~/.config/fcitx5/conf/hangul.conf
```

이후 fcitx5를 재시작해 설정을 다시 읽도록 합니다. Omarchy 환경에서는 `systemctl --user restart omarchy-fcitx5.service`를 사용하세요. 단순히 `fcitx5 -r &`을 실행하면 systemd가 관리하는 인스턴스와 D-Bus 이름을 두고 경합해 오히려 상황이 꼬일 수 있습니다(자세한 디버깅 내용은 `docs/TESTING.md` 참고). 확실하지 않다면 먼저 `pkill -9 fcitx5`로 정리한 뒤 systemd(또는 Omarchy가 관리하는 방식)가 새로 띄우도록 하세요.

**`libhangul` 패키지를 재설치할 때마다**(예: 패치를 업데이트해 다시 빌드한 경우) 같은 방법으로 fcitx5를 재시작해야 합니다. 실행 중인 프로세스는 `pacman -U`가 디스크의 파일을 교체해도 예전 공유 라이브러리를 메모리에 그대로 유지하고 있어서, 재시작 전까지는 변경 사항이 조용히 반영되지 않습니다. 자세한 내용은 `docs/TESTING.md`의 레벨 7 참고.

## `pacman -Syu` 이후에도 두 패키지 유지하기

각 패키지의 `pkgrel`을 실제 패키지보다 높게 설정해두었으므로(`103`/`100` vs. 실제 패키지의 `1`), 다음 `pacman -Syu`에서 동일한 업스트림 버전으로 되돌아가지는 않습니다. 다만 이는 업스트림이 실제로 더 새로운 버전(예: `libhangul` `0.2.1`이나 `fcitx5-hangul` `5.1.12`)을 배포하는 경우까지 막아주지는 않습니다. 그 순간 pacman은 이 빌드들 위에 "업그레이드"를 제안하며, 조용히 순아래 지원을 없애버릴 수 있습니다.

이를 확실히 고정하려면 `/etc/pacman.conf`에 다음을 추가하세요.

```ini
[options]
IgnorePkg = libhangul fcitx5-hangul
```

이렇게 해두면 `pacman -Syu`가 업스트림 릴리스마다 두 패키지를 교체하는 대신 "ignoring package upgrade"로 보고합니다. 이는 해당 `patches/*.patch`를 새 버전에 맞게 리베이스(대개 `patch -p1`로 깨끗하게 적용됨)하고 다시 빌드해야 한다는 신호로 받아들이면 됩니다.

## 제거 / stock 상태로 되돌리기

```sh
sudo pacman -S libhangul fcitx5-hangul   # 실제 extra/ 패키지로 재설치
```

(`IgnorePkg` 줄을 추가했다면 먼저 제거하고, `~/.config/fcitx5/conf/hangul.conf`의 `Keyboard=` 줄을 `Dubeolsik`으로 되돌리거나 삭제하세요.)
