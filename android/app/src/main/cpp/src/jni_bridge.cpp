#include <jni.h>
#include "jni_bridge.h"
#include "engine/buffer_accessor.h"
#include "engine/code_generator.h"
#include "util/jstring_util.h"
#include <android/log.h>
#include <exception>
#include <stdexcept>

#define LOG_TAG "ImStudioJNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using namespace imstudio::engine;
using imstudio::util::from_jstring;
using imstudio::util::to_jstring;

static void throw_runtime(JNIEnv* env, const char* msg) {
    if (env->ExceptionCheck()) return;
    jclass cls = env->FindClass("java/lang/RuntimeException");
    if (cls) env->ThrowNew(cls, msg);
}

#define TRY  try {
#define CATCH(env) } catch (const std::exception& e) { \
    LOGE("C++ exception: %s", e.what()); throw_runtime((env), e.what()); \
} catch (...) { LOGE("Unknown C++ exception"); \
    throw_runtime((env), "Unknown C++ exception"); }

extern "C" {

JNIEXPORT jint JNI_OnLoad(JavaVM* /*vm*/, void* /*reserved*/) {
    LOGI("========== JNI_OnLoad ==========");
    try {
        imstudio::engine::init();
        LOGI("========== JNI_OnLoad init OK ==========");
    } catch (const std::exception& e) {
        LOGE("JNI_OnLoad init FAILED: %s", e.what());
    } catch (...) {
        LOGE("JNI_OnLoad init FAILED (unknown)");
    }
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNI_OnUnload(JavaVM* /*vm*/, void* /*reserved*/) {
    try { imstudio::engine::shutdown(); } catch (...) {}
}

JNIEXPORT void JNICALL
Java_com_raais_imstudio_NativeBridge_initBuffer(JNIEnv* env, jobject) {
    LOGI("[JNI] initBuffer"); TRY
        imstudio::engine::init();
    CATCH(env)
}

JNIEXPORT void JNICALL
Java_com_raais_imstudio_NativeBridge_shutdown(JNIEnv* env, jobject) {
    TRY imstudio::engine::shutdown(); CATCH(env)
}

JNIEXPORT jint JNICALL
Java_com_raais_imstudio_NativeBridge_createWidget(JNIEnv* env, jobject, jstring type) {
    TRY return add_widget(from_jstring(env, type)); CATCH(env) return -1;
}
JNIEXPORT jboolean JNICALL
Java_com_raais_imstudio_NativeBridge_deleteWidget(JNIEnv* env, jobject, jint id) {
    TRY return delete_widget(id) ? JNI_TRUE : JNI_FALSE; CATCH(env) return JNI_FALSE;
}
JNIEXPORT jint JNICALL
Java_com_raais_imstudio_NativeBridge_getWidgetCount(JNIEnv* env, jobject) {
    TRY return get_count(); CATCH(env) return 0;
}
JNIEXPORT jint JNICALL
Java_com_raais_imstudio_NativeBridge_getWidgetId(JNIEnv* env, jobject, jint i) {
    TRY return get_id_at(i); CATCH(env) return -1;
}
JNIEXPORT jstring JNICALL
Java_com_raais_imstudio_NativeBridge_getWidgetType(JNIEnv* env, jobject, jint id) {
    TRY return to_jstring(env, get_type(id)); CATCH(env) return env->NewStringUTF("");
}
JNIEXPORT jstring JNICALL
Java_com_raais_imstudio_NativeBridge_getWidgetLabel(JNIEnv* env, jobject, jint id) {
    TRY return to_jstring(env, get_label(id)); CATCH(env) return env->NewStringUTF("");
}
JNIEXPORT jstring JNICALL
Java_com_raais_imstudio_NativeBridge_getWidgetValueS(JNIEnv* env, jobject, jint id) {
    TRY return to_jstring(env, get_value_s(id)); CATCH(env) return env->NewStringUTF("");
}
JNIEXPORT jfloat JNICALL
Java_com_raais_imstudio_NativeBridge_getWidgetX(JNIEnv* env, jobject, jint id) {
    TRY return get_x(id); CATCH(env) return 0.f;
}
JNIEXPORT jfloat JNICALL
Java_com_raais_imstudio_NativeBridge_getWidgetY(JNIEnv* env, jobject, jint id) {
    TRY return get_y(id); CATCH(env) return 0.f;
}
JNIEXPORT jfloat JNICALL
Java_com_raais_imstudio_NativeBridge_getWidgetSizeX(JNIEnv* env, jobject, jint id) {
    TRY return get_size_x(id); CATCH(env) return 0.f;
}
JNIEXPORT jfloat JNICALL
Java_com_raais_imstudio_NativeBridge_getWidgetSizeY(JNIEnv* env, jobject, jint id) {
    TRY return get_size_y(id); CATCH(env) return 0.f;
}
JNIEXPORT jfloat JNICALL
Java_com_raais_imstudio_NativeBridge_getWidgetWidth(JNIEnv* env, jobject, jint id) {
    TRY return get_width(id); CATCH(env) return 0.f;
}
JNIEXPORT jboolean JNICALL
Java_com_raais_imstudio_NativeBridge_setWidgetLabel(JNIEnv* env, jobject, jint id, jstring v) {
    TRY return set_label(id, from_jstring(env, v)) ? JNI_TRUE : JNI_FALSE; CATCH(env) return JNI_FALSE;
}
JNIEXPORT jboolean JNICALL
Java_com_raais_imstudio_NativeBridge_setWidgetValueS(JNIEnv* env, jobject, jint id, jstring v) {
    TRY return set_value_s(id, from_jstring(env, v)) ? JNI_TRUE : JNI_FALSE; CATCH(env) return JNI_FALSE;
}
JNIEXPORT jboolean JNICALL
Java_com_raais_imstudio_NativeBridge_moveWidget(JNIEnv* env, jobject, jint id, jfloat x, jfloat y) {
    TRY return move(id, x, y) ? JNI_TRUE : JNI_FALSE; CATCH(env) return JNI_FALSE;
}
JNIEXPORT jboolean JNICALL
Java_com_raais_imstudio_NativeBridge_resizeWidget(JNIEnv* env, jobject, jint id, jfloat sx, jfloat sy) {
    TRY return resize(id, sx, sy) ? JNI_TRUE : JNI_FALSE; CATCH(env) return JNI_FALSE;
}
JNIEXPORT jboolean JNICALL
Java_com_raais_imstudio_NativeBridge_setWidgetWidth(JNIEnv* env, jobject, jint id, jfloat w) {
    TRY return set_width(id, w) ? JNI_TRUE : JNI_FALSE; CATCH(env) return JNI_FALSE;
}
JNIEXPORT jstring JNICALL
Java_com_raais_imstudio_NativeBridge_generateCode(JNIEnv* env, jobject) {
    TRY return to_jstring(env, generate_full_code()); CATCH(env) return env->NewStringUTF("// error");
}
} // extern "C"
