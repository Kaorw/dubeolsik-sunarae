# dubeolsik-sunarae

두벌식 순아래(Dubeolsik Sun-arae)는 표준 두벌식(2벌식) 한글 자판에서 Shift 없이도 입력할 수 있도록 만든 변형 자판입니다. 

이 자판을 **Omarchy**에서 **Fcitx5**(`fcitx5-hangul` + `libhangul`)로 사용할 수 있도록 패키징한 프로젝트입니다. 해당 환경 이외에서는 테스트해보지 못했습니다.

물리적 키 배열은 표준 두벌식과 완전히 동일합니다. 달라지는 것은 오직 반복 입력을 해석하는 방식뿐이며, 이 덕분에 된소리 자음(ㄲㄸㅃㅆㅉ)과 ㅒ/ㅖ를 Shift 없이 입력할 수 있습니다. 

전체 규칙과 예시는 [`docs/ALGORITHM.md`](docs/ALGORITHM.md)를 참고하세요.

## 출처와 감사

이 자판의 조합 규칙은 새로 고안한 것이 아니라, [tinyduck(꼬마집오리)님이 정리한 "두벌식 순아래" 스펙 페이지](https://sites.google.com/site/tinyduckn/dubeolsig-sun-alae)를 그대로 따릅니다. 이 프로젝트가 한 일은 그 규칙을 지금의 Fcitx5와 libhangul 환경에서 실제로 쓸 수 있도록 구현하고 패키징한 것뿐입니다.

tinyduck님의 페이지는 다음 분들의 기여도 함께 밝히고 있습니다.

- 우덜(3beol): libhangul 기반 구현
- 팥알
- 이호석
- 김용묵: Windows용 날개셋 한글 입력기 구현

실제 구현 과정에서는 [3beol의 libhangul 포크](https://gitlab.com/3beol/libhangul)(키보드 id `2noshift`, `2n9256`)를 검증 기준으로 삼았습니다.

## 자판 배열

키 위치는 표준 두벌식(fcitx5의 `Keyboard=Dubeolsik`)과 동일합니다. 바뀌는 것은 조합 규칙입니다.

| 규칙 | 효과 | 예시 |
|---|---|---|
| 2.1 | 평자음 뒤 같은 **모음** 키를 두 번 연속 입력 → 해당 자음이 된소리로 바뀜 | `ㄷㅡㅡㅅ` → 뜻 |
| 2.2 | `ㅑ+ㅣ`/`ㅕ+ㅣ`/`ㅘ+ㅣ`/`ㅝ+ㅣ` → `ㅒ`/`ㅖ`/`ㅙ`/`ㅞ`, Shift 불필요 (추가로 `ㅏ+ㅣ`/`ㅓ+ㅣ` → `ㅐ`/`ㅔ`) | `ㅇㅕㅣㅅ` → 옛 |
| 2.3 | 같은 **받침** 키를 두 번 입력 → 된소리 받침 | `ㄱㅓㄱㄱ` → 걲 |
| 2.4 | 기존 Shift 기반 입력도 그대로 동작하며, 두 방식을 섞어 쓸 수 있음 | `ㄲㅓㄲ` / `ㄲㅓㄱㄱ` / `ㄱㅓㅓㄲ` → 꺾 |

## 저장소 구조

```
docs/                    설계 노트, 알고리즘 명세, 테스트 로그
patches/                 libhangul, fcitx5-hangul 패치 (독립적으로 리뷰 가능한 diff)
vendor/libhangul         패치가 적용된 libhangul 소스 트리 (그대로 빌드 가능)
vendor/fcitx5-hangul     패치가 적용된 fcitx5-hangul 소스 트리 (동일)
packaging/libhangul      패치된 libhangul을 빌드·설치하는 Arch PKGBUILD
packaging/fcitx5-hangul  패치된 fcitx5-hangul을 빌드·설치하는 Arch PKGBUILD
fcitx5/                  새 자판을 선택하는 fcitx5-hangul 설정 스니펫
tests/                   libhangul의 HangulInputContext API를 직접 검증하는 C 테스트 하네스
```

## 구현 방식

`libhangul`에 새 키보드 `2sunarae`("Dubeolsik Sun-arae")를 직접 추가했습니다. 기존 "2"(두벌식) 키 테이블을 그대로 재사용하고, 새 오토마톤 함수 하나와 작은 조합 테이블 두 개만 추가했습니다. 다른 키보드의 동작은 전혀 바뀌지 않습니다.

`fcitx5-hangul`은 `libhangul`을 감싸는 래퍼로, `~/.config/fcitx5/conf/hangul.conf`에서 `Keyboard=` 설정을 읽어옵니다. 다만 이 설정이 `libhangul`이 보고하는 값을 그대로 통과시키는 게 아니라 고정된 enum(`fcitx5-hangul/src/engine.h`의 `HangulKeyboard`)으로 하드코딩되어 있어, `fcitx5-hangul`에도 작은 패치가 하나 더 필요했습니다(리스트 두 곳에 각각 항목을 하나씩 추가하되, 항상 끝에 추가하므로 기존 저장된 설정의 번호는 바뀌지 않습니다). 이 패치로 "Dubeolsik Sun-arae"를 선택할 수 있게 됩니다. [`patches/0002-fcitx5-hangul-add-sunarae-option.patch`](patches/0002-fcitx5-hangul-add-sunarae-option.patch)와, 실제 fcitx5 `Instance`를 통해 end-to-end로 검증하는 `vendor/fcitx5-hangul`의 `test/testhangulsunarae.cpp`를 참고하세요.

## 빌드

```sh
cd vendor/libhangul
autoreconf -fi        # 최초 1회 또는 patches/ 변경 후에만 필요
./configure --prefix=/usr --libdir=/usr/lib
make -j$(nproc)
```

시스템에 영향을 주지 않고 오토마톤만 확인하려면:

```sh
gcc tests/test_sunarae.c -Ivendor/libhangul/hangul -o /tmp/test_sunarae \
    -Lvendor/libhangul/hangul/.libs -lhangul
LD_LIBRARY_PATH=vendor/libhangul/hangul/.libs /tmp/test_sunarae
```

## 설치

시스템 패키지 두 개를 교체하는 작업이므로, 단순 `make`/`cmake --install`(pacman이 추적할 수 없고, 이후 `pacman -Syu`가 조용히 되돌려버릴 수 있음) 대신 정식 pacman 패키지로 패키징했습니다. 전체 빌드/설치/업그레이드 고정 방법은 [`packaging/README.md`](packaging/README.md)를, 실제로 입력기를 순아래로 전환하는 설정 변경은 [`fcitx5/hangul.conf`](fcitx5/hangul.conf)를 참고하세요.

## 한계점

설치·설정 과정에서 자주 헷갈리는 systemd/D-Bus 경합 문제와 아직 검증하지 않은 범위는 [`docs/LIMITATIONS.md`](docs/LIMITATIONS.md)를 참고하세요.

## 라이선스

이 프로젝트 자체 콘텐츠(`patches/`, `docs/`, `tests/`, `packaging/`, `fcitx5/`)는 MIT 라이선스입니다. 자세한 내용은 [`LICENSE`](LICENSE)를 참고하세요. `vendor/libhangul/`와 `vendor/fcitx5-hangul/`은 이 프로젝트의 패치가 적용된 각 업스트림 프로젝트의 사본이며, 원래 라이선스(LGPL-2.1-or-later, `fcitx5-hangul` 테스트 스위트의 일부 GPL-2.0-or-later 파일 포함)를 그대로 따릅니다. 자세한 내용은 `LICENSE`를 참고하세요.

## 구현

이 기능 구현을 위해 Claude code와 Codex를 적극 활용했습니다.