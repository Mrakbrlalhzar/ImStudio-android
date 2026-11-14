#pragma once
#include <jni.h>
#include <string>
namespace imstudio::util {
    inline std::string from_jstring(JNIEnv* env, jstring s) {
        if (!s) return {};
        const char* c = env->GetStringUTFChars(s, nullptr);
        std::string out(c ? c : "");
        env->ReleaseStringUTFChars(s, c);
        return out;
    }
    inline jstring to_jstring(JNIEnv* env, const std::string& s) {
        return env->NewStringUTF(s.c_str());
    }
}
