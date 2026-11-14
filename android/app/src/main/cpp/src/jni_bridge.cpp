#include <jni.h>
#include "jni_bridge.h"
#include "engine/buffer_accessor.h"
#include "engine/code_generator.h"
#include "util/jstring_util.h"

using namespace imstudio;
using namespace imstudio::engine;
using imstudio::util::from_jstring;
using imstudio::util::to_jstring;

extern "C" {

JNIEXPORT void JNICALL
Java_com_raais_imstudio_NativeBridge_initBuffer(JNIEnv*, jobject) {
    imstudio::engine::init();
}

JNIEXPORT jint JNICALL
Java_com_raais_imstudio_NativeBridge_createWidget(JNIEnv* env, jobject, jstring type) {
    return add_widget(from_jstring(env, type));
}

JNIEXPORT jboolean JNICALL
Java_com_raais_imstudio_NativeBridge_deleteWidget(JNIEnv*, jobject, jint id) {
    return delete_widget(id) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jint JNICALL
Java_com_raais_imstudio_NativeBridge_getWidgetCount(JNIEnv*, jobject) {
    return get_count();
}

JNIEXPORT jint JNICALL
Java_com_raais_imstudio_NativeBridge_getWidgetId(JNIEnv*, jobject, jint i) {
    return get_id_at(i);
}

JNIEXPORT jstring JNICALL
Java_com_raais_imstudio_NativeBridge_getWidgetType(JNIEnv* env, jobject, jint id) {
    return to_jstring(env, get_type(id));
}

JNIEXPORT jstring JNICALL
Java_com_raais_imstudio_NativeBridge_getWidgetLabel(JNIEnv* env, jobject, jint id) {
    return to_jstring(env, get_label(id));
}

JNIEXPORT jstring JNICALL
Java_com_raais_imstudio_NativeBridge_getWidgetValueS(JNIEnv* env, jobject, jint id) {
    return to_jstring(env, get_value_s(id));
}

JNIEXPORT jfloat JNICALL
Java_com_raais_imstudio_NativeBridge_getWidgetX(JNIEnv*, jobject, jint id) { return get_x(id); }

JNIEXPORT jfloat JNICALL
Java_com_raais_imstudio_NativeBridge_getWidgetY(JNIEnv*, jobject, jint id) { return get_y(id); }

JNIEXPORT jfloat JNICALL
Java_com_raais_imstudio_NativeBridge_getWidgetSizeX(JNIEnv*, jobject, jint id) { return get_size_x(id); }

JNIEXPORT jfloat JNICALL
Java_com_raais_imstudio_NativeBridge_getWidgetSizeY(JNIEnv*, jobject, jint id) { return get_size_y(id); }

JNIEXPORT jfloat JNICALL
Java_com_raais_imstudio_NativeBridge_getWidgetWidth(JNIEnv*, jobject, jint id) { return get_width(id); }

JNIEXPORT jboolean JNICALL
Java_com_raais_imstudio_NativeBridge_setWidgetLabel(JNIEnv* env, jobject, jint id, jstring v) {
    return set_label(id, from_jstring(env, v)) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_raais_imstudio_NativeBridge_setWidgetValueS(JNIEnv* env, jobject, jint id, jstring v) {
    return set_value_s(id, from_jstring(env, v)) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_raais_imstudio_NativeBridge_moveWidget(JNIEnv*, jobject, jint id, jfloat x, jfloat y) {
    return move(id, x, y) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_raais_imstudio_NativeBridge_resizeWidget(JNIEnv*, jobject, jint id, jfloat sx, jfloat sy) {
    return resize(id, sx, sy) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_raais_imstudio_NativeBridge_setWidgetWidth(JNIEnv*, jobject, jint id, jfloat w) {
    return set_width(id, w) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jstring JNICALL
Java_com_raais_imstudio_NativeBridge_generateCode(JNIEnv* env, jobject) {
    return to_jstring(env, generate_full_code());
}

} // extern "C"
