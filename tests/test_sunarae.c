#include <stdio.h>
#include <hangul.h>
#include <wchar.h>
#include <locale.h>

static void feed(const char *keyboard, const char *seq, const char *label) {
    HangulInputContext *hic = hangul_ic_new(keyboard);
    printf("%-55s keys=%-8s => ", label, seq);
    for (const char *p = seq; *p; p++) {
        hangul_ic_process(hic, (int)(unsigned char)*p);
        const ucschar *commit = hangul_ic_get_commit_string(hic);
        if (commit) for (int i = 0; commit[i]; i++) printf("%lc", (wint_t)commit[i]);
    }
    const ucschar *tail = hangul_ic_flush(hic);
    if (tail) for (int i = 0; tail[i]; i++) printf("%lc", (wint_t)tail[i]);
    printf("\n");
    hangul_ic_delete(hic);
}

int main() {
    setlocale(LC_ALL, "");

    // dubeolsik qwerty mapping (lowercase, unshifted):
    // r=ㄱ s=ㄴ e=ㄷ f=ㄹ a=ㅁ q=ㅂ t=ㅅ d=ㅇ w=ㅈ c=ㅊ z=ㅋ x=ㅌ v=ㅍ g=ㅎ
    // k=ㅏ o=ㅐ i=ㅑ j=ㅓ p=ㅔ u=ㅕ h=ㅗ n=ㅜ y=ㅛ b=ㅠ m=ㅡ l=ㅣ

    printf("== plain sanity (should match standard dubeolsik) ==\n");
    feed("2sunarae", "rk", "ga = g,a");
    feed("2sunarae", "runsk", "han-geul word = h,a,n,g,eu,l,g,a? (see keys)");
    feed("2",        "rk", "[std 2] ga = g,a (reference)");

    printf("\n== rule 2.1: doubled vowel tenses the choseong ==\n");
    feed("2sunarae", "emmt",  "d,eu,eu,s               (expect tteut)");
    feed("2",        "emmt",  "[std 2] d,eu,eu,s        (reference, no tensify)");
    feed("2sunarae", "rhhor", "g,o,o,ae,g              (expect kkwaek+g)");
    feed("2sunarae", "rhoor", "g,o,ae,ae,g (24-key alt, informational)");

    printf("\n== rule 2.2: ya+i / yeo+i compose without Shift ==\n");
    feed("2sunarae", "il",  "ya,i   (expect yae ㅒ)");
    feed("2sunarae", "ul",  "yeo,i  (expect ye ㅖ)");
    feed("2",        "il",  "[std 2] ya,i (reference, expect NOT combined)");
    feed("2sunarae", "dult", "ieung,yeo,i,s            (expect yet)");

    printf("\n== rule 2.3: doubled jongseong key tenses the batchim ==\n");
    feed("2sunarae", "rjrr", "g,eo,g,g                (expect batchim ssanggiyeok)");
    feed("2",        "rjrr", "[std 2] g,eo,g,g         (reference, no tensify)");
    feed("2sunarae", "rjtt", "g,eo,s,s                (expect batchim ssangsios)");

    printf("\n== rule 2.4: tense initial + tense final, fully shift-free ==\n");
    feed("2sunarae", "rjjrr", "g,eo,eo,g,g            (expect kkeok, both tensed)");

    return 0;
}
