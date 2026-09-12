#include <stdio.h>
#include <hangul.h>
#include <wchar.h>
#include <locale.h>

static void feed(const char *keyboard, const char *seq, const char *label) {
    HangulInputContext *hic = hangul_ic_new(keyboard);
    printf("== %-45s (%-9s) keys=%-8s => ", label, keyboard, seq);
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
    // r=g(ㄱ) s=n(ㄴ) e=d(ㄷ) f=l(ㄹ) a=m(ㅁ) q=b(ㅂ) t=s(ㅅ) d=ng(ㅇ)
    // w=j(ㅈ) c=c(ㅊ) z=k(ㅋ) x=t(ㅌ) v=p(ㅍ) g=h(ㅎ)
    // k=a(ㅏ) o=ae(ㅐ) i=ya(ㅑ) O=yae(ㅒ,shift-o) j=eo(ㅓ) p=e(ㅔ) u=yeo(ㅕ) P=ye(ㅖ,shift-p)
    // h=o(ㅗ) n=u(ㅜ) y=yo(ㅛ) b=yu(ㅠ) m=eu(ㅡ) l=i(ㅣ)
    feed("2", "rr", "choseong g,g before any vowel");
    feed("2", "rrk", "choseong g,g then a  (rr + k)");
    feed("2", "emm", "d, eu, eu (tteut attempt without final s)");
    feed("2", "emmt", "d, eu, eu, s (tteut 뜻 attempt)");
    feed("2", "iy", "ya, i  (yae attempt via i+y? fix keys)");
    feed("2", "il", "ya(i), i(l)  => attempt yae");
    feed("2", "ul", "yeo(u), i(l) => attempt ye");
    feed("2", "kl", "a(k), i(l) => ai (no compose expected)");
    feed("2", "rkrr", "ga(rk) + g,g as jongseong (rr) => attempt 각+g");
    feed("2", "rktt", "ga(rk) + s,s as jongseong (tt) => attempt 갓+s => ㅆ?");
    feed("3-91", "aaa", "smoke test other layout exists");
    return 0;
}
