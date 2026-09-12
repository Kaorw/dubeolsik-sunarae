/*
 * Exhaustive cross-validation for the 2sunarae keyboard.
 *
 * Strategy: for every Sun-arae "no-shift" key sequence, also type the
 * SAME target syllable(s) via the ordinary Shift-based standard "2"
 * (dubeolsik) keyboard, and assert byte-for-byte that both produce
 * identical output. This avoids hand-typing expected Hangul glyphs
 * (error-prone to transcribe correctly) and avoids reusing any of the
 * new automaton's own logic as the "oracle" - the "2" keyboard's
 * Shift-based composition is separate, pre-existing, untouched code.
 *
 * dubeolsik key map (lowercase = unshifted, uppercase = Shift):
 *   r=g(ㄱ) s=n(ㄴ) e=d(ㄷ) f=l(ㄹ) a=m(ㅁ) q=b(ㅂ) t=s(ㅅ) d=ng(ㅇ)
 *   w=j(ㅈ) c=c(ㅊ) z=k(ㅋ) x=t(ㅌ) v=p(ㅍ) g=h(ㅎ)
 *   R=gg(ㄲ) E=dd(ㄸ) Q=bb(ㅃ) T=ss(ㅆ) W=jj(ㅉ)   (tense consonants, Shift)
 *   k=a(ㅏ) o=ae(ㅐ) i=ya(ㅑ) O=yae(ㅒ) j=eo(ㅓ) p=e(ㅔ) u=yeo(ㅕ) P=ye(ㅖ)
 *   h=o(ㅗ) n=u(ㅜ) y=yo(ㅛ) b=yu(ㅠ) m=eu(ㅡ) l=i(ㅣ)
 */
#include <stdio.h>
#include <string.h>
#include <hangul.h>
#include <wchar.h>
#include <locale.h>

static int total = 0, passed = 0;

/* Runs `seq` through `keyboard` and returns the full committed+flushed
 * string as ucschar (caller-provided buffer). */
static int run(const char *keyboard, const char *seq, ucschar *out, int cap) {
    HangulInputContext *hic = hangul_ic_new(keyboard);
    int n = 0;
    for (const char *p = seq; *p; p++) {
        hangul_ic_process(hic, (int)(unsigned char)*p);
        const ucschar *commit = hangul_ic_get_commit_string(hic);
        if (commit) {
            for (int i = 0; commit[i] && n < cap - 1; i++) out[n++] = commit[i];
        }
    }
    const ucschar *tail = hangul_ic_flush(hic);
    if (tail) {
        for (int i = 0; tail[i] && n < cap - 1; i++) out[n++] = tail[i];
    }
    out[n] = 0;
    hangul_ic_delete(hic);
    return n;
}

static void check(const char *label, const char *sunarae_keys, const char *standard_keys) {
    ucschar a[256], b[256];
    run("2sunarae", sunarae_keys, a, 256);
    run("2", standard_keys, b, 256);

    int len_a = 0, len_b = 0;
    while (a[len_a]) len_a++;
    while (b[len_b]) len_b++;

    int ok = (len_a == len_b) && (memcmp(a, b, len_a * sizeof(ucschar)) == 0);
    total++;
    if (ok) passed++;

    printf("[%s] %-42s sunarae(%-9s)=", ok ? "PASS" : "FAIL", label, sunarae_keys);
    for (int i = 0; i < len_a; i++) printf("%lc", (wint_t)a[i]);
    printf("  standard(%-9s)=", standard_keys);
    for (int i = 0; i < len_b; i++) printf("%lc", (wint_t)b[i]);
    printf("\n");
}

/* For pure regression checks where sunarae and standard keys are the
 * SAME sequence and must produce the SAME output (no doubling involved
 * at all) - proves the new keyboard didn't change unrelated behavior. */
static void check_same_keys(const char *label, const char *keys) {
    check(label, keys, keys);
}

int main() {
    setlocale(LC_ALL, "");

    printf("=== Rule 2.1: tense CHOSEONG via doubled vowel (5 consonants x simple vowel) ===\n");
    check("gg + a  (까)",  "rkk",  "Rk");
    check("dd + a  (따)",  "ekk",  "Ek");
    check("bb + a  (빠)",  "qkk",  "Qk");
    check("ss + a  (싸)",  "tkk",  "Tk");
    check("jj + a  (짜)",  "wkk",  "Wk");

    printf("\n=== Rule 2.1: same, but with vowel eo instead of a ===\n");
    check("gg + eo (꺼)",  "rjj",  "Rj");
    check("dd + eo (떠)",  "ejj",  "Ej");
    check("bb + eo (뻐)",  "qjj",  "Qj");
    check("ss + eo (써)",  "tjj",  "Tj");
    check("jj + eo (쩌)",  "wjj",  "Wj");

    printf("\n=== Rule 2.1: tense CHOSEONG + batchim afterward ===\n");
    check("gg+a+batchim g (깍)", "rkkr", "Rkr");
    check("dd+a+batchim n (딴)", "ekks", "Eks");
    check("bb+a+batchim m (빰)", "qkka", "Qka");
    check("ss+a+batchim t (싿)", "tkke", "Tke");
    check("jj+a+batchim g (짝)", "wkkr", "Wkr");

    printf("\n=== Rule 2.1: doubling the FIRST half of a diphthong (꽈-style) ===\n");
    check("gg + wa  (꽈)", "rhhk", "Rhk");
    check("dd + wa  (똬)", "ehhk", "Ehk");
    check("bb + weo (뿨)", "qnnj", "Qnj");
    check("ss + wi  (쒸)", "tnnl", "Tnl");

    printf("\n=== Rule 2.2: ya+i -> yae, yeo+i -> ye (no Shift) ===\n");
    check("ieung + yae (얘)",        "dil", "dO");
    check("ieung + ye  (예)",        "dul", "dP");
    check("giyeok + ye (계)",        "rul", "rP");
    check("ieung+ye+batchim s (옛)", "dult", "dPt");

    printf("\n=== Rule 2.3: doubled JONGSEONG key tenses the batchim ===\n");
    check("bieup+a+batchim gg (밖)", "qkrr", "qkR");
    check("ieung+i+batchim ss (있)", "dltt", "dlT");
    check("giyeok+a+batchim ss (갔)", "rktt", "rkT");
    check("bieup+eo+batchim gg (벆-like)", "qjrr", "qjR");

    printf("\n=== Rule 2.4: tense-initial and tense-final are interchangeable (깎) ===\n");
    check("both shift",              "RkR",   "RkR");
    check("tensify-init, shift-fin", "rkkR",  "RkR");
    check("shift-init, tensify-fin", "Rkrr",  "RkR");
    check("both tensify (fully shift-free)", "rkkrr", "RkR");

    printf("\n=== Regression: standard diphthongs still work (identical keys, both keyboards) ===\n");
    check_same_keys("hwa (화)", "ghk");
    check_same_keys("wae (왜)", "dho");
    check_same_keys("oe  (외)", "dhl");
    check_same_keys("weo (워)", "dnj");
    check_same_keys("we  (웨)", "dnp");
    check_same_keys("wi  (위)", "dnl");
    check_same_keys("yi  (의)", "dml");

    printf("\n=== Regression: standard compound batchim still work ===\n");
    check_same_keys("gs  (몫)", "ahrt");
    check_same_keys("nj  (앉)", "dksw");
    check_same_keys("nh  (않)", "dksg");
    check_same_keys("lg  (닭)", "ekfr");
    check_same_keys("lm  (삶)", "tkfa");
    check_same_keys("ls  (곬)", "rhft");
    check_same_keys("lt  (핥)", "gkfx");
    check_same_keys("lp  (읊)", "dmfv");
    check_same_keys("lh  (앓)", "dkfg");
    check_same_keys("bs  (값)", "rkqt");

    printf("\n=== Regression: non-tensable consonants doubling a vowel must NOT tensify ===\n");
    /* n(ㄴ) l(ㄹ) m(ㅁ) ng(ㅇ) h(ㅎ) k(ㅋ) t(ㅌ) p(ㅍ) c(ㅊ) have no tense
     * form; doubling the following vowel must behave exactly like the
     * plain "2" keyboard (same keys -> same output on both). */
    check_same_keys("n+a+a  (나+ㅏ)", "skk");
    check_same_keys("l+a+a  (라+ㅏ)", "fkk");
    check_same_keys("m+a+a  (마+ㅏ)", "akk");
    check_same_keys("ng+a+a (아+ㅏ)", "dkk");
    check_same_keys("h+a+a  (하+ㅏ)", "gkk");
    check_same_keys("k+a+a  (카+ㅏ)", "zkk");
    check_same_keys("t+a+a  (타+ㅏ)", "xkk");
    check_same_keys("p+a+a  (파+ㅏ)", "vkk");
    check_same_keys("c+a+a  (차+ㅏ)", "ckk");

    printf("\n=== Broader word list (mixes several rules) ===\n");
    check("kkot (꽃: gg+o+batchim ch)",        "rhhc",  "Rhc");
    check("tteolda (떨다: dd+eo, l, d+a)",     "ejjfek", "Ejfek");
    check("ssada (싸다: ss+a, d+a)",           "tkkek",  "Tkek");
    check("jjatda (짰다: jj+a+batchim ss, d+a)", "wkkttek", "WkTek");
    check("kkaek (깩: gg+ae+batchim g)",        "roor",  "Ror");

    printf("\n%d/%d checks passed\n", passed, total);
    return passed == total ? 0 : 1;
}
