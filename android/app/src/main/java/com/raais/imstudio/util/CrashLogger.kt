package com.raais.imstudio.util

import android.content.Context
import android.util.Log
import com.raais.imstudio.NativeBridge
import java.io.File
import java.io.PrintWriter
import java.io.StringWriter
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

object CrashLogger {
    private const val TAG = "ImStudioCrash"

    fun crashDir(context: Context): File {
        val ext = context.getExternalFilesDir(null) ?: context.filesDir
        return File(ext, "crashes").apply { mkdirs() }
    }

    fun install(context: Context) {
        val dir = crashDir(context)

        // 1) Java/Kotlin uncaught exceptions
        val def = Thread.getDefaultUncaughtExceptionHandler()
        Thread.setDefaultUncaughtExceptionHandler { thread, throwable ->
            try {
                val ts = SimpleDateFormat("yyyyMMdd_HHmmss", Locale.US).format(Date())
                val f = File(dir, "java_crash_${ts}.log")
                val sw = StringWriter()
                throwable.printStackTrace(PrintWriter(sw))
                f.writeText("Thread: ${thread.name}\nTime: ${Date()}\n\n$sw")
                Log.e(TAG, "Java crash written: ${f.absolutePath}", throwable)
            } catch (t: Throwable) {
                Log.e(TAG, "Failed to write Java crash", t)
            }
            def?.uncaughtException(thread, throwable)
        }

        // 2) Native crashes (SIGSEGV, SIGABRT, ...) via JNI
        try {
            NativeBridge.initCrashHandler(dir.absolutePath)
            Log.i(TAG, "Native crash handler installed. dir=${dir.absolutePath}")
        } catch (t: Throwable) {
            Log.e(TAG, "Failed to install native crash handler", t)
        }
    }

    fun listCrashFiles(context: Context): List<File> {
        return crashDir(context).listFiles()?.sortedByDescending { it.lastModified() } ?: emptyList()
    }
}
