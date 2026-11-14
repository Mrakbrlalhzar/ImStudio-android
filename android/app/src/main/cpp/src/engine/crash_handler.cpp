#include <jni.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unwind.h>
#include <dlfcn.h>
#include <cxxabi.h>
#include <android/log.h>

#define LOG_TAG "ImStudioCrash"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static char g_CrashDir[512] = {0};

struct BtState { void** current; void** end; };

static _Unwind_Reason_Code unwind_cb(struct _Unwind_Context* ctx, void* arg) {
    BtState* s = static_cast<BtState*>(arg);
    uintptr_t pc = _Unwind_GetIP(ctx);
    if (pc) {
        if (s->current == s->end) return _URC_END_OF_STACK;
        *s->current++ = reinterpret_cast<void*>(pc);
    }
    return _URC_NO_REASON;
}

static int capture_bt(void** buf, int max) {
    BtState s = { buf, buf + max };
    _Unwind_Backtrace(unwind_cb, &s);
    return (int)(s.current - buf);
}

// Async-signal-safe writer: only open/write/close/snprintf on stack buffers
static void write_crash(int sig, siginfo_t* info, void* /*uc*/) {
    if (g_CrashDir[0] == 0) return;
    char path[700];
    time_t now = time(nullptr);
    snprintf(path, sizeof(path), "%s/native_crash_%ld_sig%d.log",
             g_CrashDir, (long)now, sig);
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) return;

    char buf[512];
    int n = snprintf(buf, sizeof(buf),
        "=== Native crash ===\nSignal: %d (%s)\nInfo addr: %p\nTime: %ld\n\n",
        sig, strsignal(sig), info ? info->si_addr : nullptr, (long)now);
    write(fd, buf, n);

    void* frames[64];
    int count = capture_bt(frames, 64);
    n = snprintf(buf, sizeof(buf), "Backtrace (%d frames):\n", count);
    write(fd, buf, n);

    for (int i = 0; i < count; i++) {
        Dl_info dl;
        if (dladdr(frames[i], &dl) && dl.dli_sname) {
            int status = 0;
            char* demangled = abi::__cxa_demangle(dl.dli_sname, nullptr, nullptr, &status);
            const char* name = (status == 0 && demangled) ? demangled : dl.dli_sname;
            long off = (long)((char*)frames[i] - (char*)dl.dli_saddr);
            n = snprintf(buf, sizeof(buf), "  #%02d %p %s+0x%lx (%s)\n",
                         i, frames[i], name, off, dl.dli_fname ? dl.dli_fname : "?");
            if (demangled) free(demangled);
        } else {
            n = snprintf(buf, sizeof(buf), "  #%02d %p\n", i, frames[i]);
        }
        write(fd, buf, n);
    }
    close(fd);
}

static void handler(int sig, siginfo_t* info, void* uc) {
    write_crash(sig, info, uc);
    signal(sig, SIG_DFL);
    raise(sig);
}

extern "C" JNIEXPORT void JNICALL
Java_com_raais_imstudio_NativeBridge_initCrashHandler(JNIEnv* env, jobject, jstring dir) {
    if (dir == nullptr) { LOGI("initCrashHandler: null dir"); return; }
    const char* cdir = env->GetStringUTFChars(dir, nullptr);
    strncpy(g_CrashDir, cdir, sizeof(g_CrashDir) - 1);
    g_CrashDir[sizeof(g_CrashDir) - 1] = 0;
    env->ReleaseStringUTFChars(dir, cdir);

    // altstack for stack-overflow safety
    static char altstack[64 * 1024];
    stack_t ss; ss.ss_sp = altstack; ss.ss_size = sizeof(altstack); ss.ss_flags = 0;
    sigaltstack(&ss, nullptr);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = handler;
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigemptyset(&sa.sa_mask);

    int sigs[] = { SIGSEGV, SIGABRT, SIGFPE, SIGILL, SIGBUS, SIGTRAP };
    for (int s : sigs) sigaction(s, &sa, nullptr);
    LOGI("Crash handler installed, dir=%s", g_CrashDir);
}
