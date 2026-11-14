package com.raais.imstudio.ui.screens

import androidx.compose.foundation.layout.*
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Add
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import com.raais.imstudio.engine.EditorViewModel
import com.raais.imstudio.model.WidgetType
import com.raais.imstudio.ui.components.*
import com.raais.imstudio.ui.dialogs.CodeDialog
import com.raais.imstudio.ui.dialogs.ConfirmDeleteDialog
import com.raais.imstudio.util.CrashLogger

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun EditorScreen(vm: EditorViewModel) {
    val state = vm.state
    val ctx = LocalContext.current
    var showCode by remember { mutableStateOf(false) }
    var showPalette by remember { mutableStateOf(false) }
    var pendingDelete by remember { mutableStateOf<Int?>(null) }
    val crashFiles = remember { CrashLogger.listCrashFiles(ctx) }

    LaunchedEffect(Unit) { vm.refresh() }

    Scaffold(
        topBar = { EditorToolbar(onShowCode = { showCode = true }) },
        floatingActionButton = {
            if (state.nativeError.value == null) {
                FloatingActionButton(onClick = { showPalette = true }) {
                    Icon(Icons.Default.Add, contentDescription = "افزودن")
                }
            }
        }
    ) { padding ->
        Column(modifier = Modifier.padding(padding).fillMaxSize()) {
            state.nativeError.value?.let { err ->
                Surface(color = MaterialTheme.colorScheme.errorContainer) {
                    Column(modifier = Modifier.fillMaxWidth().padding(16.dp)) {
                        Text("خطای native", style = MaterialTheme.typography.titleMedium,
                            color = MaterialTheme.colorScheme.onErrorContainer)
                        Text(err, style = MaterialTheme.typography.bodySmall,
                            color = MaterialTheme.colorScheme.onErrorContainer)
                    }
                }
            }
            if (crashFiles.isNotEmpty()) {
                Surface(color = MaterialTheme.colorScheme.tertiaryContainer) {
                    Column(modifier = Modifier.fillMaxWidth().padding(12.dp)) {
                        Text("${crashFiles.size} فایل کرش ذخیره‌شده",
                            style = MaterialTheme.typography.titleSmall,
                            color = MaterialTheme.colorScheme.onTertiaryContainer)
                        Text("مسیر: ${crashFiles.first().parentFile?.absolutePath}",
                            style = MaterialTheme.typography.bodySmall,
                            color = MaterialTheme.colorScheme.onTertiaryContainer)
                    }
                }
            }
            Row(modifier = Modifier.fillMaxSize()) {
                Box(modifier = Modifier.weight(0.62f).fillMaxHeight()) {
                    DesignCanvas(state, onSelect = { vm.select(it) }, onLongPress = { pendingDelete = it })
                }
                Box(modifier = Modifier.weight(0.38f).fillMaxHeight()) {
                    PropertiesPanel(state)
                }
            }
        }
    }
    if (showPalette) ComponentPaletteSheet(
        onSelect = { t: WidgetType -> vm.addWidget(t); showPalette = false },
        onDismiss = { showPalette = false }
    )
    if (showCode) CodeDialog(code = vm.generateCode(), onDismiss = { showCode = false })
    pendingDelete?.let { id ->
        ConfirmDeleteDialog(widgetId = id,
            onConfirm = { state.deleteWidget(id); pendingDelete = null },
            onDismiss = { pendingDelete = null })
    }
}
