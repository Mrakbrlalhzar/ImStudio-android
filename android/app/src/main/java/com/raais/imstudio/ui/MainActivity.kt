package com.raais.imstudio.ui
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.viewModels
import com.raais.imstudio.engine.EditorViewModel
import com.raais.imstudio.ui.screens.EditorScreen
import com.raais.imstudio.ui.theme.ImStudioTheme

class MainActivity : ComponentActivity() {
    private val vm: EditorViewModel by viewModels()
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent { ImStudioTheme { EditorScreen(vm) } }
    }
}
