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
        val def = Thread.getDefaultUncaughtExceptionHandler()
        Thread.setDefaultUncaughtExceptionHandler { thread, throwable ->
            try {
                val ts = SimpleDateFormat("yyyyMMdd_HHmmss", Locale.US).format(Date())
                val f = File(dir, "java_crash_${ts}.log")
                val sw = StringWriter(); throwable.printStackTrace(PrintWriter(sw))
                f.writeText("Thread: ${thread.name}\nTime: ${Date()}\n\n$sw")
            } catch (_: Throwable) {}
            def?.uncaughtException(thread, throwable)
        }
        try { NativeBridge.initCrashHandler(dir.absolutePath) } catch (_: Throwable) {}
    }
    fun listCrashFiles(context: Context): List<File> =
        crashDir(context).listFiles()?.sortedByDescending { it.lastModified() } ?: emptyList()
}
