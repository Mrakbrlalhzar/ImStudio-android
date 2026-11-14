package com.raais.imstudio.util
import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
fun copyToClipboard(context: Context, text: String, label: String = "ImStudio") {
    val cm = context.getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
    cm.setPrimaryClip(ClipData.newPlainText(label, text))
}
