package com.raais.imstudio.ui.screens

import androidx.compose.foundation.layout.*
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Add
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import com.raais.imstudio.engine.EditorViewModel
import com.raais.imstudio.model.WidgetType
import com.raais.imstudio.ui.components.*
import com.raais.imstudio.ui.dialogs.CodeDialog
import com.raais.imstudio.ui.dialogs.ConfirmDeleteDialog

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun EditorScreen(vm: EditorViewModel) {
    val state = vm.state
    var showCode by remember { mutableStateOf(false) }
    var showPalette by remember { mutableStateOf(false) }
    var pendingDelete by remember { mutableStateOf<Int?>(null) }

    LaunchedEffect(Unit) { vm.refresh() }

    Scaffold(
        topBar = { EditorToolbar(onShowCode = { showCode = true }) },
        floatingActionButton = {
            FloatingActionButton(onClick = { showPalette = true }) {
                Icon(Icons.Default.Add, contentDescription = "افزودن ویجت")
            }
        }
    ) { padding ->
        Row(modifier = Modifier.padding(padding).fillMaxSize()) {
            Box(modifier = Modifier.weight(0.62f).fillMaxHeight()) {
                DesignCanvas(
                    state = state,
                    onSelect = { vm.select(it) },
                    onLongPress = { pendingDelete = it }
                )
            }
            Box(modifier = Modifier.weight(0.38f).fillMaxHeight()) {
                PropertiesPanel(state = state)
            }
        }
    }

    if (showPalette) {
        ComponentPaletteSheet(
            onSelect = { type: WidgetType -> vm.addWidget(type); showPalette = false },
            onDismiss = { showPalette = false }
        )
    }
    if (showCode) {
        CodeDialog(code = vm.generateCode(), onDismiss = { showCode = false })
    }
    pendingDelete?.let { id ->
        ConfirmDeleteDialog(
            widgetId = id,
            onConfirm = { state.deleteWidget(id); pendingDelete = null },
            onDismiss = { pendingDelete = null }
        )
    }
}
