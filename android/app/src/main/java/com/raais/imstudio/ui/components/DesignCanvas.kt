package com.raais.imstudio.ui.components
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.gestures.detectDragGestures
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.layout.*
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.drawscope.DrawScope
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import com.raais.imstudio.engine.ImStudioState
import com.raais.imstudio.model.Widget
@Composable fun DesignCanvas(state: ImStudioState, onSelect: (Int?) -> Unit, onLongPress: (Int) -> Unit) {
    Box(modifier = Modifier.fillMaxSize().background(Color(0xFF121212))
        .pointerInput(Unit) { detectTapGestures(onTap = { onSelect(null) }) }) {
        Canvas(modifier = Modifier.fillMaxSize()) { drawGrid() }
        state.widgets.forEach { w -> key(w.id) {
            WidgetItem(w, state.selectedId.value == w.id,
                onSelect = { onSelect(w.id) }, onLongPress = { onLongPress(w.id) },
                onMove = { dx, dy -> state.moveWidget(w.id, w.x + dx, w.y + dy) })
        } }
    }
}
private fun DrawScope.drawGrid() {
    val c = Color(0xFF1F1F1F); val s = 24f
    var x = 0f; while (x < size.width) { drawLine(c, Offset(x, 0f), Offset(x, size.height), 1f); x += s }
    var y = 0f; while (y < size.height) { drawLine(c, Offset(0f, y), Offset(size.width, y), 1f); y += s }
}
@Composable private fun WidgetItem(w: Widget, isSel: Boolean, onSelect: () -> Unit,
                       onLongPress: () -> Unit, onMove: (Float, Float) -> Unit) {
    var drag by remember(w.id) { mutableStateOf(Offset.Zero) }
    val dw = if (w.sizeX > 0f) w.sizeX.dp else w.width.dp
    val dh = if (w.sizeY > 0f) w.sizeY.dp else 36.dp
    Box(modifier = Modifier.offset(x = (w.x + drag.x).dp, y = (w.y + drag.y).dp)
        .size(width = dw, height = dh)
        .background(if (isSel) Color(0xFF4A90D9) else Color(0xFF2C2C2C))
        .pointerInput(w.id) { detectDragGestures(
            onDragStart = { onSelect() },
            onDrag = { ch, d -> ch.consume(); drag += d },
            onDragEnd = { onMove(drag.x, drag.y); drag = Offset.Zero }) }
        .pointerInput(w.id) { detectTapGestures(onLongPress = { onLongPress() }) },
        contentAlignment = Alignment.Center) {
        Text(w.displayText, color = Color(0xFFE0E0E0), maxLines = 1, overflow = TextOverflow.Ellipsis)
    }
}
