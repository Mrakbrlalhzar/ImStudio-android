package com.raais.imstudio.ui.dialogs

import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ContentCopy
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.window.Dialog
import com.raais.imstudio.util.copyToClipboard

@Composable
fun CodeDialog(code: String, onDismiss: () -> Unit) {
    val ctx = LocalContext.current
    Dialog(onDismissRequest = onDismiss) {
        Surface(shape = MaterialTheme.shapes.large,
                color = MaterialTheme.colorScheme.surface) {
            Column(modifier = Modifier.padding(16.dp)) {
                Text("کد تولیدشده", style = MaterialTheme.typography.titleLarge)
                Spacer(Modifier.height(12.dp))
                Box(modifier = Modifier
                    .heightIn(max = 420.dp)
                    .horizontalScroll(rememberScrollState())
                    .verticalScroll(rememberScrollState())
                ) {
                    Text(code, fontFamily = FontFamily.Monospace,
                         fontSize = 12.sp)
                }
                Spacer(Modifier.height(12.dp))
                Row(modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.End) {
                    TextButton(onClick = { copyToClipboard(ctx, code, "ImStudio code") }) {
                        Icon(Icons.Default.ContentCopy, contentDescription = null)
                        Spacer(Modifier.width(4.dp)); Text("کپی")
                    }
                    Spacer(Modifier.width(8.dp))
                    TextButton(onClick = onDismiss) { Text("بستن") }
                }
            }
        }
    }
}
