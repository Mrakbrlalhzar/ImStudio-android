# Build

Prereqs: JDK 17, Android SDK, NDK 26.1.10909125.

```bash
cd android
./gradlew assembleDebug
```

Output: `android/app/build/outputs/apk/debug/app-debug.apk`.

CI builds via `.github/workflows/build-android.yml`.
