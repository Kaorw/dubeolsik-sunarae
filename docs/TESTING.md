# 테스트 로그

## 레벨 1: 패치되지 않은 설치본 대비 가정 확인

코드를 작성하기 전, 표준 `"2"` 키보드가 **시스템에 설치된** `libhangul 0.2.0-1`에서 실제로 다음 동작을 하지 않는지 (`tests/test_hangul.c`로) 먼저 확인했습니다.

- `gg`(모음 없이)가 `ㄲ`으로 조합되지 않음: `option_combi_on_double_stroke`가 기본적으로 꺼져 있음을 확인.
- `ya`,`i`가 `ㅒ`로 조합되지 않음: `hangul_combination_table_default`에 해당 항목이 없음을 확인.
- 받침 중복 입력(`g,a` 다음 `g,g`)도 같은 옵션 게이트로 인해 된소리 받침으로 조합되지 않음.

이를 통해 "혹시 stock libhangul이 이미 이 기능을 지원하지 않을까"라는 가능성을 배제하고, `tests/test_sunarae.c`가 재사용하는 동일 프로세스 기준선을 확보했습니다(모든 순아래 검사에는 `"2"` 키보드를 이용한 대조군 실행이 함께 포함됩니다).

## 레벨 2: 새 오토마톤 (libhangul 공개 API 직접 검증)

`tests/test_sunarae.c`는 `HangulInputContext`(`hangul_ic_new`, `hangul_ic_process`, `hangul_ic_get_commit_string`, `hangul_ic_flush`)를 키보드 id `2sunarae`로 직접 구동해, `docs/ALGORITHM.md`의 모든 예시를 재현합니다.

| 입력 키 | 규칙 | 기대값 | 결과 |
|---|---|---|---|
| `emmt` (d,eu,eu,s) | 2.1 | 뜻 | 뜻 ✅ |
| `rhhor` (g,o,o,ae,g) | 2.1 | 꽥 | 꽥 ✅ |
| `rhoor` (g,o,ae,ae,g) | 2.1, 24키 대응 변형 | 꽥 | 괘ㅐㄱ ❌ (의도된 결과, 아래 참고) |
| `il` (ya,i) | 2.2 | ㅒ | ㅒ ✅ |
| `ul` (yeo,i) | 2.2 | ㅖ | ㅖ ✅ |
| `dult` (ieung,yeo,i,s) | 2.2 | 옛 | 옛 ✅ |
| `rjrr` (g,eo,g,g) | 2.3 | 걲 | 걲 ✅ |
| `rjtt` (g,eo,s,s) | 2.3 | 겄 | 겄 ✅ |
| `rjjrr` (g,eo,eo,g,g) | 2.1 + 2.3 | 꺾 | 꺾 ✅ |
| `rk` (g,a) | 기본 확인 | 가 | 가 ✅ |

의도적으로 불일치하는 `rhoor`(이중모음의 *두 번째* 절반을 중복 입력하는 방식)는 스펙에서 말하는 Windows "24키 대응" 변형이며 기본 형태가 아닙니다. 이 구현이 왜 이 방식을 (아직) 지원하지 않는지는 `docs/ALGORITHM.md` 규칙 2.1을 참고하세요. (레벨 8에서 별도 로직으로 지원이 추가되었습니다.)

위 모든 검사는 같은 프로세스 안에서 표준 `"2"` 키보드로도 함께 실행되어, 표준 키보드의 동작이 레벨 1의 기준선과 바이트 단위로 동일하게 유지됨을 함께 확인합니다.

직접 실행하려면:

```sh
gcc tests/test_sunarae.c -Ivendor/libhangul/hangul -o /path/under/HOME/test_sunarae \
    -Lvendor/libhangul/hangul/.libs -lhangul
LD_LIBRARY_PATH=vendor/libhangul/hangul/.libs /path/under/HOME/test_sunarae
```

**알아두면 좋은 점:** 반드시 홈 디렉터리 등 `/tmp`가 아닌 곳에서 빌드·실행하세요. `-lhangul`로 링크한 바이너리를 `LD_LIBRARY_PATH`가 `/tmp/...` 경로를 가리키는 상태로 실행하면, 이 환경에서는 아무 오류 없이 시스템 `libhangul.so.1`로 조용히 폴백됩니다(`hangul_keyboard_list_get_count()`가 그냥 stock 값을 반환). 같은 `.so` 파일을 `md5sum`으로 비교해 바이트가 완전히 동일함에도 불구하고 로드되는 디렉터리에 따라 동작이 달라짐을 확인했습니다. 월드-라이터블 경로에서 라이브러리를 로드하는 것에 대한 시스템 차원의 방어 조치로 보이며, `/tmp`를 쓰지 않으면 문제되지 않습니다.

## 레벨 3: 빌드된 패키지

```sh
cd packaging && makepkg -f
```

를 실행하면 실제 Arch `libhangul` 패키지와 동일한 21개 파일 구성(`pacman -Ql libhangul`로 비교), 동일한 soname(`libhangul.so.1.1.0`)을 가진 `libhangul-0.2.0-103-x86_64.pkg.tar.zst`가 만들어집니다. (`~/...` 경로에서, 레벨 2의 `/tmp` 관련 사항에 유의하며) 다음을 확인했습니다.

- 빌드된 패키지의 `usr/bin/hangul --list`가 stock 9개 키보드와 함께 `2sunarae Dubeolsik Sun-arae`를 나열함.
- `usr/bin/hangul -k 2sunarae -i "emmtrhhordultrjrrrjjrr"`가 `뜻꽥옛걲꺾`(다섯 예시를 이어붙인 값)를 출력하며, 레벨 2와 정확히 일치함: 빌드 디렉터리뿐 아니라 *패키징된* 바이너리도 올바르게 동작함을 확인.

## 레벨 4: 실제 fcitx5 `Instance`를 통한 end-to-end 검증

`vendor/fcitx5-hangul/test/testhangulsunarae.cpp`는 (fcitx5-hangul 자체 업스트림 테스트와 동일한 방식으로) 전체 `fcitx::Instance`를 구성하고, `hangul` 입력기를 등록한 뒤, 실제 `~/.config/fcitx5/conf/hangul.conf`가 사용하는 것과 동일한 메커니즘인 `RawConfig`를 통해 `Keyboard=Dubeolsik Sun-arae`를 선택하고, `testfrontend`로 키 이벤트를 흘려보내 다섯 가지 예시가 정확히 커밋되는지 검증합니다. `vendor/fcitx5-hangul/build`에서 `ctest`로 실행되며, `packaging/fcitx5-hangul/PKGBUILD`의 `check()` 단계로도 자동 실행됩니다. 통과.

## 레벨 5: 실제 데스크톱 환경에 설치 후 검증

패치된 두 패키지(`libhangul 0.2.0-103`, `fcitx5-hangul 5.1.11-100`)를 모두 설치하고, 실제 사용 중인 `~/.config/fcitx5/conf/hangul.conf`에 `Keyboard=Dubeolsik Sun-arae`를 설정한 뒤 fcitx5를 재시작한 상태에서, Shift 없이 `emmt`를 실제 텍스트 필드에 입력하면 뜻이 출력됨을 확인했습니다.

**디버깅 노트:** 처음 재시작을 시도했을 때(`fcitx5 -r &`을 수동 실행) 설정이 조용히 `# Keyboard=Dubeolsik`(주석 처리됨)로 되돌아간 것처럼 보였습니다. 원인은 패치와 무관했습니다. 이 환경의 `omarchy-fcitx5.service`(systemd 사용자 유닛)와 D-Bus 서비스 활성화(`/usr/share/dbus-1/services/org.fcitx.Fcitx5.service`)가 둘 다 `org.fcitx.Fcitx5` 버스 이름을 점유하려 시도합니다. 수동으로 `fcitx5 -r &`을 실행하면 systemd 유닛과 경합하게 되고, 진 쪽은 크래시 루프에 빠집니다(`Failed to create addon: dbus ... Is there another fcitx already running?`). 그리고 실제로 설정을 메모리에 들고 있는 인스턴스는 입력기가 활성화될 때마다 주기적으로 `updateAction()` → `safeSaveAsIni()`(`fcitx5-hangul/src/engine.cpp` 참고)를 호출해 **현재 메모리 상의 설정**을 디스크에 다시 씁니다. 즉, 파일이 수정되기 전에 시작된 인스턴스가 자신의 오래된 기본값으로 방금 한 수정을 덮어써 버릴 수 있습니다. `pkill -9 fcitx5` 후 `systemctl --user restart omarchy-fcitx5.service`로 인스턴스를 정확히 하나만 남기고 설정을 다시 적용해 해결했습니다. 이 systemd/D-Bus 경합 자체(이 프로젝트와 무관한 별도 이슈)는 `docs/LIMITATIONS.md` 참고.

## 레벨 6: 전수 조합 매트릭스 (`tests/test_matrix.c`)

가능한 한 많은 키 조합을 검증하기 위한 것으로, Hangul 글자를 손으로 직접 타이핑해 기대값을 적는 대신(사람이 옮겨 적다 실수하기 쉬움, 이 로그 초반에도 그런 실수를 발견해 고친 적이 있습니다) 교차 검증 방식을 사용합니다: 순아래의 Shift-free 키 시퀀스마다, 같은 목표 음절을 기존 Shift 기반 표준 `"2"` 키보드로도 입력해 두 결과가 바이트 단위로 동일한지 검사합니다. 표준 키보드의 Shift 기반 조합은 별도의, 수정되지 않은 기존 코드이므로 일치한다는 것은 같은 로직끼리의 자기 비교가 아니라 실제 정확성의 근거가 됩니다.

개발 빌드(`vendor/libhangul`)와 설치된 시스템 `libhangul` 양쪽 모두에서 **62/62개 검사 통과**. 범위:

| 그룹 | 개수 | 검증 내용 |
|---|---|---|
| 규칙 2.1, 단순 모음 | 10 | 된소리 자음 5개(ㄲㄸㅃㅆㅉ) × 모음 2개(ㅏ, ㅓ) |
| 규칙 2.1 + 받침 | 5 | 된소리 자음 음절 뒤 평받침이 오는 경우 |
| 규칙 2.1, 이중모음 중복 | 4 | 이중모음의 *첫 번째* 모음을 중복 입력해도 (ㅘ/ㅝ/ㅟ 등) 자음이 정상적으로 된소리화됨 |
| 규칙 2.2 | 4 | ㅑ+ㅣ=ㅒ, ㅕ+ㅣ=ㅖ, 뒤에 초성/받침이 오는 경우 포함 |
| 규칙 2.3 | 4 | 받침 키 중복 입력 → ㄲ/ㅆ 받침, 여러 음절에서 |
| 규칙 2.4 | 4 | 깎을 입력하는 네 가지 방법(shift/shift, 중복/shift, shift/중복, 중복/중복)이 모두 동일한 결과 |
| 이중모음 회귀 | 7 | ㅘㅙㅚㅝㅞㅟㅢ가 표준 두벌식과 동일하게 조합됨 |
| 겹받침 회귀 | 10 | ㄳㄵㄶㄺㄻㄽㄾㄿㅀㅄ이 표준 두벌식과 동일하게 조합됨 |
| 된소리 없는 자음 회귀 | 9 | ㄴㄹㅁㅇㅎㅋㅌㅍㅊ(된소리 형태 없음)의 모음 중복 입력이 표준 두벌식과 완전히 동일하게 동작: 새 된소리화 로직이 해당되지 않는 경우 아무것도 바꾸지 않음을 증명 |
| 확장 단어 | 5 | 꽃, 떨다, 싸다, 짰다, 깩: 여러 음절, 규칙 혼합 |

직접 실행하려면:

```sh
gcc tests/test_matrix.c $(pkg-config --cflags --libs libhangul) -o /path/under/HOME/test_matrix
/path/under/HOME/test_matrix
```

(아직 설치하지 않은 빌드를 테스트하려면 시스템 `pkg-config` 출력 대신 `vendor/libhangul/hangul/.libs`를 링크하세요. 레벨 2의 `/tmp` 관련 사항이 동일하게 적용됩니다.)

## 레벨 7: 스펙 페이지의 모든 예시 검증

<https://sites.google.com/site/tinyduckn/dubeolsig-sun-alae> 페이지에 나온 17개 예시 전체를 순서대로 검증했으며(`docs/ALGORITHM.md`에 이미 뽑아둔 예시만이 아니라), 레벨 6과 같은 방식으로 교차 검증했습니다. 그 결과 실제 공백 두 가지를 발견했습니다.

- 규칙 2.2는 문자 그대로는 `ㅑ+ㅣ`/`ㅕ+ㅣ`뿐 아니라 `ㅘ+ㅣ`→`ㅙ`, `ㅝ+ㅣ`→`ㅞ`도 포함합니다(예: "왜"를 `ㅇㅗㅏㅣ`로 입력). 이 프로젝트가 참고한 3beol 구현의 실제 조합 테이블(변경 이력 설명이 아니라 실제 테이블)에도 이 두 조합은 없어서 누락되어 있었습니다.
- 이 공백을 고친 뒤, 페이지에서 별도 Windows 전용 변형으로 소개하는 "24키 대응" 확장(`ㅏ+ㅣ`→`ㅐ`, `ㅓ+ㅣ`→`ㅔ`)도 무해한 추가 기능으로 함께 지원하도록 추가했습니다.

두 수정 모두 레벨 2/6에서 추가한 동일한 조합 테이블에 두 줄을 더하는 것으로 끝났습니다. 오토마톤 로직 변경은 없었습니다. `libhangul` 패키지는 이를 위해 두 차례 더 빌드되었습니다(`0.2.0-101`: wae/we, `0.2.0-102`: ae/e), 각각 재설치·재검증 완료. `tests/test_matrix.c`는 62개에서 68개로 늘었습니다(여기서 발견한 조합 6개 추가).

**디버깅 노트:** `0.2.0-101`을 설치한 뒤에도 실제 fcitx5에서는 한동안 이전(수정 전) 동작이 그대로 나타났습니다. 패치의 버그가 아니라, 실행 중인 프로세스가 디스크의 파일이 교체된 뒤에도 예전 공유 라이브러리를 메모리에 그대로 매핑해두고 있기 때문입니다. 설정 변경 후와 마찬가지로, `libhangul`을 재설치할 때마다 `pkill -9 fcitx5` + `systemctl --user restart omarchy-fcitx5.service`가 필요합니다(레벨 5의 노트 참고).

`0.2.0-102` 설치 후 실제 텍스트 필드에서 왜(`dhkl`), ㅐ(`kl`), ㅔ(`jl`) 모두 정상 동작함을 확인했습니다.

## 아직 테스트하지 않은 것

- 실제 키보드 *하드웨어* 동작(Hyprland 환경): 키를 길게 눌렀을 때의 자동 반복(key repeat)이 이중 입력 규칙과 어떻게 상호작용하는지, IME 후보 창과의 상호작용, 그리고 레벨 5에서 확인한 한 곳 외의 다른 입력 대상(브라우저, 터미널, GTK/Qt 텍스트 필드 등).

레벨 8에서 "24키 대응" 변형의 마지막 남은 부분까지 구현되어, 스펙 원본 페이지의 모든 예시가 (서로 상호 배타적이라 동시에 성립할 수 없는 부분을 제외하고, `docs/ALGORITHM.md` 참고) 동작함을 확인했습니다.

## 레벨 8: "24키 대응" 두 번째 절반 중복 입력

레벨 7에서 남겨둔 마지막 한 가지: 이중모음의 된소리화 신호를 중복 입력하는 또 다른 방식으로, *두 번째* 절반을 중복 입력하는 방식입니다(꽥을 위해 이미 지원하던 `ㄱㅗㅗㅐㄱ` 대신 `ㄱㅗㅐㅐㄱ`).

이번에는 테이블에 행을 추가하는 것만으로는 부족한, 실제(작은) 로직 변경이 필요했습니다. 기존 검사는 들어오는 키를 `hangul_ic_peek()`(내부 스택 최상단)와 비교하는데, 이는 이중모음이 아직 조합되기 *전*의 원시 자모에만 적용됩니다. `ㅗ+ㅐ`가 `ㅙ`로 조합되고 나면 스택 최상단은 `ㅐ`가 아니라 `ㅙ`가 되어, 같은 검사로는 "두 번째 절반이 중복 입력되었다"는 것을 인식할 수 없습니다. 이를 위해 `hangul_ic_sunarae_is_second_half(jung, ch)`를 추가했습니다. `ch`가 *전용 키가 없는* 이중모음(ㅘㅙㅚㅝㅞㅟㅒㅖㅢ)의 알려진 두 번째 구성 요소인지 확인하는 작은 switch 문입니다. 이런 이중모음들은 항상 두 번의 키 입력 조합 결과일 수밖에 없으므로 모호함이 없습니다.

전용 키가 있는 ㅐ/ㅔ는 의도적으로 제외했습니다: 버퍼에 담긴 `ㅐ`는 단일 키 입력일 수도, `ㅏ+ㅣ`(레벨 7에서 추가)의 결과일 수도 있어 `hic->buffer.jungseong`만으로는 구분할 수 없습니다. 이 제외가 실제로 필요하고 올바른지는 전용 회귀 테스트(`tests/test_matrix.c`의 "ae/e are NOT treated as doubleable diphthong halves": `g,ae,i` 입력이 된소리화로 오작동하지 않고 개+ㅣ 그대로 유지되는지 확인)로 검증했습니다.

`tests/test_matrix.c`는 68개에서 77개로 늘었습니다(두 번째 절반 케이스 7개 + ㅐ/ㅔ 제외 회귀 케이스 2개 추가). 개발 빌드와 패키징 후 빌드 양쪽 모두 통과.
