# 업스트림 기여

이 프로젝트의 두 패치는 각각 독립적인 PR로 바로 제출할 수 있도록 작성되어 있습니다.

## libhangul

`patches/0001-add-dubeolsik-sunarae-keyboard.patch`는 [libhangul/libhangul](https://github.com/libhangul/libhangul)에 새 키보드 `2sunarae`를 추가합니다. 정확히 이 기능을 요청하는 [이슈 #31 "두벌식 순아래 자판 추가"](https://github.com/libhangul/libhangul/issues/31)가 2019년부터 열려 있고, 아직 해결되지 않았습니다. 구현 방식과 검증 과정은 [`ALGORITHM.md`](ALGORITHM.md)와 [`TESTING.md`](TESTING.md)를 참고하세요.

## fcitx5-hangul

`patches/0002-fcitx5-hangul-add-sunarae-option.patch`는 [fcitx/fcitx5-hangul](https://github.com/fcitx/fcitx5-hangul)에 "Dubeolsik Sun-arae"를 선택 가능한 `Keyboard` 옵션으로 추가합니다. libhangul 패치가 먼저 머지된 뒤에나 의미가 있으므로, libhangul PR이 받아들여진 뒤 순서대로 제출할 계획입니다.

## 진행 상황

아직 두 저장소 어디에도 PR을 올리지 않았습니다. libhangul 이슈 #31에 이 구현을 알리는 댓글을 먼저 남기고, 메인테이너 반응을 본 뒤 PR로 이어갈 예정입니다.
