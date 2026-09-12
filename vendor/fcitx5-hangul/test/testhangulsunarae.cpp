/*
 * dubeolsik-sunarae project: end-to-end check that the "Dubeolsik
 * Sun-arae" keyboard, once selected via fcitx5-hangul's own config
 * option (not just at the libhangul API level), reproduces the worked
 * examples from docs/ALGORITHM.md through a real fcitx5 Instance +
 * the hangul input method engine.
 *
 * Modeled on testhangul.cpp, with a RawConfig switching
 * HangulConfig::keyboard to Dubeolsik_Sunarae before sending keys.
 */
#include "testdir.h"
#include "testfrontend_public.h"
#include <fcitx-config/rawconfig.h>
#include <fcitx-utils/eventdispatcher.h>
#include <fcitx-utils/key.h>
#include <fcitx-utils/keysym.h>
#include <fcitx-utils/log.h>
#include <fcitx-utils/macros.h>
#include <fcitx-utils/testing.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputmethodgroup.h>
#include <fcitx/inputmethodmanager.h>
#include <fcitx/inputpanel.h>
#include <fcitx/instance.h>

using namespace fcitx;

void scheduleEvent(Instance *instance) {
    instance->eventDispatcher().schedule([instance]() {
        auto *hangul = instance->addonManager().addon("hangul", true);
        FCITX_ASSERT(hangul);

        RawConfig config;
        config["Keyboard"] = "Dubeolsik Sun-arae";
        hangul->setConfig(config);

        auto defaultGroup = instance->inputMethodManager().currentGroup();
        defaultGroup.inputMethodList().clear();
        defaultGroup.inputMethodList().push_back(
            InputMethodGroupItem("keyboard-us"));
        defaultGroup.inputMethodList().push_back(
            InputMethodGroupItem("hangul"));
        defaultGroup.setDefaultInputMethod("");
        instance->inputMethodManager().setGroup(defaultGroup);
        auto *testfrontend = instance->addonManager().addon("testfrontend");
        auto uuid =
            testfrontend->call<ITestFrontend::createInputContext>("testapp");
        auto *ic = instance->inputContextManager().findByUUID(uuid);
        FCITX_ASSERT(testfrontend->call<ITestFrontend::sendKeyEvent>(
            uuid, Key("Control+space"), false));
        FCITX_ASSERT(instance->inputMethod(ic) == "hangul");

        // 뜻 꽥 옛 걲 꺾, per docs/ALGORITHM.md - the same keys already
        // verified against raw libhangul in tests/test_sunarae.c.
        testfrontend->call<ITestFrontend::pushCommitExpectation>("뜻");
        testfrontend->call<ITestFrontend::pushCommitExpectation>("꽥");
        testfrontend->call<ITestFrontend::pushCommitExpectation>("옛");
        testfrontend->call<ITestFrontend::pushCommitExpectation>("걲");
        testfrontend->call<ITestFrontend::pushCommitExpectation>("꺾");

        const char *keys = "emmtrhhordultrjrrrjjrr";
        for (const char *p = keys; *p; ++p) {
            std::string k(1, *p);
            FCITX_ASSERT(testfrontend->call<ITestFrontend::sendKeyEvent>(
                uuid, Key(k), false));
        }

        instance->deactivate();
    });

    instance->eventDispatcher().schedule([instance]() { instance->exit(); });
}

int main() {
    setupTestingEnvironmentPath(TESTING_BINARY_DIR, {"bin"},
                                {TESTING_BINARY_DIR "/test"});
    char arg0[] = "testhangulsunarae";
    char arg1[] = "--disable=all";
    char arg2[] = "--enable=testim,testfrontend,hangul";
    char *argv[] = {arg0, arg1, arg2};
    fcitx::Log::setLogRule("default=5,hangul=5");
    Instance instance(FCITX_ARRAY_SIZE(argv), argv);
    instance.addonManager().registerDefaultLoader(nullptr);
    scheduleEvent(&instance);
    instance.exec();

    return 0;
}
