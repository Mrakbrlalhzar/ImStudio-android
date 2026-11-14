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
    jclass c = env->FindClass("java/lang/RuntimeException");
    if (c) env->ThrowNew(c, msg);
}
#define TRY try {
#define CATCH(env) } catch (const std::exception& e) { \
    LOGE("C++ ex: %s", e.what()); throw_runtime((env), e.what()); \
} catch (...) { throw_runtime((env), "Unknown C++ exception"); }

extern "C" {
JNIEXPORT jint JNI_OnLoad(JavaVM*, void*) {
    try { imstudio::engine::init(); } catch (...) {}
    return JNI_VERSION_1_6;
}
JNIEXPORT void JNI_OnUnload(JavaVM*, void*) {
    try { imstudio::engine::shutdown(); } catch (...) {}
}
JNIEXPORT void JNICALL Java_com_raais_imstudio_NativeBridge_initBuffer(JNIEnv* env, jobject) {
    TRY imstudio::engine::init(); CATCH(env)
}
JNIEXPORT void JNICALL Java_com_raais_imstudio_NativeBridge_shutdown(JNIEnv* env, jobject) {
    TRY imstudio::engine::shutdown(); CATCH(env)
}
JNIEXPORT void JNICALL Java_com_raais_imstudio_NativeBridge_clearAll(JNIEnv* env, jobject) {
    TRY clear_all(); CATCH(env)
}
JNIEXPORT jboolean JNICALL Java_com_raais_imstudio_NativeBridge_loadFromText(JNIEnv* env, jobject, jstring text) {
    TRY return load_from_text(from_jstring(env, text)) ? JNI_TRUE : JNI_FALSE; CATCH(env) return JNI_FALSE;
}
JNIEXPORT jboolean JNICALL Java_com_raais_imstudio_NativeBridge_importFromCpp(JNIEnv* env, jobject, jstring code) {
    TRY return import_from_cpp(from_jstring(env, code)) ? JNI_TRUE : JNI_FALSE; CATCH(env) return JNI_FALSE;
}
// ★ NEW: multi-file import
JNIEXPORT void JNICALL Java_com_raais_imstudio_NativeBridge_importBegin(JNIEnv* env, jobject) {
    TRY imstudio::engine::import_begin(); CATCH(env)
}
JNIEXPORT void JNICALL Java_com_raais_imstudio_NativeBridge_importFile(JNIEnv* env, jobject, jstring path, jstring content) {
    TRY {
        std::string p = from_jstring(env, path);
        std::string c = from_jstring(env, content);
        imstudio::engine::import_file(p, c);
    } CATCH(env)
}
JNIEXPORT jstring JNICALL Java_com_raais_imstudio_NativeBridge_importEnd(JNIEnv* env, jobject) {
    TRY return to_jstring(env, imstudio::engine::import_end());
    CATCH(env) return env->NewStringUTF("{}");
}
JNIEXPORT jstring JNICALL Java_com_raais_imstudio_NativeBridge_dumpToJson(JNIEnv* env, jobject) {
    TRY return to_jstring(env, dump_to_json()); CATCH(env) return env->NewStringUTF("{\"version\":1,\"objects\":[]}");
}
JNIEXPORT jint JNICALL Java_com_raais_imstudio_NativeBridge_createWidget(JNIEnv* env, jobject, jstring t) {
    TRY return add_widget(from_jstring(env, t)); CATCH(env) return -1;
}
JNIEXPORT jboolean JNICALL Java_com_raais_imstudio_NativeBridge_deleteWidget(JNIEnv* env, jobject, jint id) {
    TRY return delete_widget(id) ? JNI_TRUE : JNI_FALSE; CATCH(env) return JNI_FALSE;
}
JNIEXPORT jint JNICALL Java_com_raais_imstudio_NativeBridge_getWidgetCount(JNIEnv* env, jobject) {
    TRY return get_count(); CATCH(env) return 0;
}
JNIEXPORT jint JNICALL Java_com_raais_imstudio_NativeBridge_getWidgetId(JNIEnv* env, jobject, jint i) {
    TRY return get_id_at(i); CATCH(env) return -1;
}
JNIEXPORT jstring JNICALL Java_com_raais_imstudio_NativeBridge_getWidgetType(JNIEnv* env, jobject, jint id) {
    TRY return to_jstring(env, get_type(id)); CATCH(env) return env->NewStringUTF("");
}
JNIEXPORT jstring JNICALL Java_com_raais_imstudio_NativeBridge_getWidgetLabel(JNIEnv* env, jobject, jint id) {
    TRY return to_jstring(env, get_label(id)); CATCH(env) return env->NewStringUTF("");
}
JNIEXPORT jstring JNICALL Java_com_raais_imstudio_NativeBridge_getWidgetValueS(JNIEnv* env, jobject, jint id) {
    TRY return to_jstring(env, get_value_s(id)); CATCH(env) return env->NewStringUTF("");
}
JNIEXPORT jfloat JNICALL Java_com_raais_imstudio_NativeBridge_getWidgetX(JNIEnv* env, jobject, jint id) {
    TRY return get_x(id); CATCH(env) return 0.f;
}
JNIEXPORT jfloat JNICALL Java_com_raais_imstudio_NativeBridge_getWidgetY(JNIEnv* env, jobject, jint id) {
    TRY return get_y(id); CATCH(env) return 0.f;
}
JNIEXPORT jfloat JNICALL Java_com_raais_imstudio_NativeBridge_getWidgetSizeX(JNIEnv* env, jobject, jint id) {
    TRY return get_size_x(id); CATCH(env) return 0.f;
}
JNIEXPORT jfloat JNICALL Java_com_raais_imstudio_NativeBridge_getWidgetSizeY(JNIEnv* env, jobject, jint id) {
    TRY return get_size_y(id); CATCH(env) return 0.f;
}
JNIEXPORT jfloat JNICALL Java_com_raais_imstudio_NativeBridge_getWidgetWidth(JNIEnv* env, jobject, jint id) {
    TRY return get_width(id); CATCH(env) return 0.f;
}
JNIEXPORT jboolean JNICALL Java_com_raais_imstudio_NativeBridge_setWidgetLabel(JNIEnv* env, jobject, jint id, jstring v) {
    TRY return set_label(id, from_jstring(env, v)) ? JNI_TRUE : JNI_FALSE; CATCH(env) return JNI_FALSE;
}
JNIEXPORT jboolean JNICALL Java_com_raais_imstudio_NativeBridge_setWidgetValueS(JNIEnv* env, jobject, jint id, jstring v) {
    TRY return set_value_s(id, from_jstring(env, v)) ? JNI_TRUE : JNI_FALSE; CATCH(env) return JNI_FALSE;
}
JNIEXPORT jboolean JNICALL Java_com_raais_imstudio_NativeBridge_moveWidget(JNIEnv* env, jobject, jint id, jfloat x, jfloat y) {
    TRY return move(id, x, y) ? JNI_TRUE : JNI_FALSE; CATCH(env) return JNI_FALSE;
}
JNIEXPORT jboolean JNICALL Java_com_raais_imstudio_NativeBridge_resizeWidget(JNIEnv* env, jobject, jint id, jfloat sx, jfloat sy) {
    TRY return resize(id, sx, sy) ? JNI_TRUE : JNI_FALSE; CATCH(env) return JNI_FALSE;
}
JNIEXPORT jboolean JNICALL Java_com_raais_imstudio_NativeBridge_setWidgetWidth(JNIEnv* env, jobject, jint id, jfloat w) {
    TRY return set_width(id, w) ? JNI_TRUE : JNI_FALSE; CATCH(env) return JNI_FALSE;
}
JNIEXPORT jstring JNICALL Java_com_raais_imstudio_NativeBridge_generateCode(JNIEnv* env, jobject) {
    TRY return to_jstring(env, generate_full_code()); CATCH(env) return env->NewStringUTF("// error");
}
}
