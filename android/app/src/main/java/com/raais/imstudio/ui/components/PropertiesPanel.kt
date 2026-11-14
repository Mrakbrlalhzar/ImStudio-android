package com.raais.imstudio.ui.components
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.raais.imstudio.engine.ImStudioState
import com.raais.imstudio.model.Widget

@Composable
fun PropertiesPanel(state: ImStudioState) {
    val sel: Widget? = state.selected()
    Column(modifier = Modifier.fillMaxHeight()
        .background(MaterialTheme.colorScheme.surface)
        .verticalScroll(rememberScrollState()).padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)) {
        Text("ویژگی‌ها", style = MaterialTheme.typography.titleMedium)
        if (sel != null) {
            OutlinedTextField(value = sel.label,
                onValueChange = { state.setLabel(sel.id, it) },
                label = { Text("Label") }, singleLine = true,
                modifier = Modifier.fillMaxWidth())
            OutlinedTextField(value = sel.valueS,
                onValueChange = { state.setValueS(sel.id, it) },
                label = { Text("Value") }, singleLine = true,
                modifier = Modifier.fillMaxWidth())
            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                Num("X", sel.x) { state.moveWidget(sel.id, it, sel.y) }
                Num("Y", sel.y) { state.moveWidget(sel.id, sel.x, it) }
            }
            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                Num("Size X", sel.sizeX) { state.resizeWidget(sel.id, it, sel.sizeY) }
                Num("Size Y", sel.sizeY) { state.resizeWidget(sel.id, sel.sizeX, it) }
            }
            OutlinedTextField(value = sel.type, onValueChange = {}, enabled = false,
                label = { Text("Type") }, modifier = Modifier.fillMaxWidth())
            HorizontalDivider()
            Button(onClick = { state.deleteWidget(sel.id) },
                colors = ButtonDefaults.buttonColors(
                    containerColor = MaterialTheme.colorScheme.error),
                modifier = Modifier.fillMaxWidth()) { Text("حذف ویجت") }
        } else {
            Text("یک ویجت انتخاب کنید",
                color = MaterialTheme.colorScheme.onSurfaceVariant)
        }
    }
}

@Composable private fun RowScope.Num(label: String, value: Float, onChange: (Float) -> Unit) {
    OutlinedTextField(value = value.toString(),
        onValueChange = { it.toFloatOrNull()?.let(onChange) },
        label = { Text(label) }, singleLine = true, modifier = Modifier.weight(1f))
}
