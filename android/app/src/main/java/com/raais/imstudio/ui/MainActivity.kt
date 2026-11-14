package com.raais.imstudio.ui

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.lifecycle.ViewModelProvider
import com.raais.imstudio.engine.EditorViewModel
import com.raais.imstudio.ui.screens.EditorScreen
import com.raais.imstudio.ui.theme.ImStudioTheme
import com.raais.imstudio.util.CrashLogger

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        CrashLogger.install(applicationContext)

        // ★ CRITICAL FIX: Create ViewModel BEFORE setContent.
        // Doing this during composition modifies state (nativeError.value)
        // mid-composition, which corrupts Compose's slot table.
        val vm = ViewModelProvider(this)[EditorViewModel::class.java]

        setContent { ImStudioTheme { EditorScreen(vm) } }
    }
}
