package com.raais.imstudio.ui.components

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.raais.imstudio.model.WidgetCategory
import com.raais.imstudio.model.WidgetType

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ComponentPaletteSheet(
    onSelect: (WidgetType) -> Unit,
    onDismiss: () -> Unit
) {
    ModalBottomSheet(onDismissRequest = onDismiss) {
        LazyColumn(
            modifier = Modifier.fillMaxWidth().padding(horizontal = 16.dp),
            verticalArrangement = Arrangement.spacedBy(4.dp)
        ) {
            WidgetCategory.entries.forEach { cat ->
                item {
                    Text(cat.displayName,
                        style = MaterialTheme.typography.titleSmall,
                        modifier = Modifier.padding(vertical = 8.dp))
                }
                items(WidgetType.entries.filter { it.category == cat }) { type ->
                    ListItem(
                        headlineContent = { Text(type.displayName) },
                        supportingContent = { Text(type.nativeName) },
                        trailingContent = {
                            TextButton(onClick = { onSelect(type) }) { Text("افزودن") }
                        }
                    )
                }
            }
        }
    }
}
