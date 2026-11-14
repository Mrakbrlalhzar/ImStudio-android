package com.raais.imstudio.ui.dialogs
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
@Composable fun ConfirmDeleteDialog(widgetId: Int, onConfirm: () -> Unit, onDismiss: () -> Unit) {
    AlertDialog(onDismissRequest = onDismiss, title = { Text("حذف ویجت") },
        text = { Text("حذف ویجت #$widgetId؟") },
        confirmButton = { TextButton(onClick = onConfirm) { Text("حذف") } },
        dismissButton = { TextButton(onClick = onDismiss) { Text("لغو") } })
}
