package com.raais.imstudio.ui.components

import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Code
import androidx.compose.material3.*
import androidx.compose.runtime.Composable

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun EditorToolbar(onShowCode: () -> Unit) {
    TopAppBar(
        title = { Text("ImStudio") },
        actions = {
            IconButton(onClick = onShowCode) {
                Icon(Icons.Default.Code, contentDescription = "تولید کد")
            }
        }
    )
}
