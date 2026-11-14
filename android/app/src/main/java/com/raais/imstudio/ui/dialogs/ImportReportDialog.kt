package com.raais.imstudio.ui.dialogs

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.window.Dialog
import com.raais.imstudio.model.ImportFileReport
import com.raais.imstudio.model.ImportReport

@Composable
fun ImportReportDialog(report: ImportReport, onDismiss: () -> Unit) {
    val s = report.session
    Dialog(onDismissRequest = onDismiss) {
        Surface(
            shape = MaterialTheme.shapes.large,
            color = MaterialTheme.colorScheme.surface
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                Text("گزارش ایمپورت", style = MaterialTheme.typography.titleLarge)

                Spacer(Modifier.height(12.dp))

                // Session summary
                Surface(
                    color = MaterialTheme.colorScheme.surfaceVariant,
                    shape = MaterialTheme.shapes.small
                ) {
                    Column(modifier = Modifier.padding(12.dp).fillMaxWidth()) {
                        Text("خلاصه", style = MaterialTheme.typography.titleSmall)
                        Spacer(Modifier.height(4.dp))
                        Text("کل فایل‌ها: ${s.totalFiles}", fontSize = 12.sp)
                        Text("فایل‌های دارای ویجت: ${s.filesWithWidgets}", fontSize = 12.sp)
                        Text("فایل‌های بدون ImGui: ${s.filesWithoutImGui}", fontSize = 12.sp)
                        Text("کل ویجت‌ها: ${s.totalWidgets}",
                            fontSize = 12.sp, color = Color(0xFF6DB3F2))
                        if (s.totalWarnings > 0)
                            Text("هشدارها: ${s.totalWarnings}",
                                fontSize = 12.sp, color = Color(0xFFFFB74D))
                        if (s.totalGlobalWarnings > 0)
                            Text("هشدارهای سراسری: ${s.totalGlobalWarnings}",
                                fontSize = 12.sp, color = Color(0xFFFFB74D))
                    }
                }

                Spacer(Modifier.height(12.dp))

                // Per-file details
                Box(
                    modifier = Modifier
                        .heightIn(max = 420.dp)
                        .verticalScroll(rememberScrollState())
                ) {
                    Column {
                        report.files.forEach { f ->
                            FileReportCard(f)
                            Spacer(Modifier.height(8.dp))
                        }
                        if (report.globalWarnings.isNotEmpty()) {
                            Text("هشدارهای سراسری",
                                style = MaterialTheme.typography.titleSmall,
                                color = Color(0xFFFFB74D))
                            Spacer(Modifier.height(4.dp))
                            report.globalWarnings.forEach { w ->
                                Text("• $w", fontSize = 11.sp)
                            }
                        }
                    }
                }

                Spacer(Modifier.height(12.dp))
                Row(modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.End) {
                    TextButton(onClick = onDismiss) { Text("بستن") }
                }
            }
        }
    }
}

@Composable
private fun FileReportCard(f: ImportFileReport) {
    val statusColor = when (f.status) {
        "ok" -> Color(0xFF81C784)
        "no_widgets" -> Color(0xFFFFB74D)
        "no_imgui" -> Color(0xFF9E9E9E)
        "parse_error" -> Color(0xFFCF6679)
        else -> MaterialTheme.colorScheme.onSurface
    }
    Surface(color = MaterialTheme.colorScheme.surfaceVariant,
            shape = MaterialTheme.shapes.small) {
        Column(modifier = Modifier.padding(10.dp).fillMaxWidth()) {
            Row(modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween) {
                Text(f.path, style = MaterialTheme.typography.bodyMedium,
                     fontFamily = FontFamily.Monospace, maxLines = 1)
                Text(f.status, fontSize = 11.sp, color = statusColor,
                     fontFamily = FontFamily.Monospace)
            }
            Spacer(Modifier.height(4.dp))
            Text("${f.linesScanned} خط، ${f.sizeBytes} بایت، " +
                 "${f.widgetsFound} ویجت، ${f.linesNonImGui} خط بدون ImGui",
                 fontSize = 11.sp, color = MaterialTheme.colorScheme.onSurfaceVariant)

            if (f.recognizedWidgets.isNotEmpty()) {
                Text("ویجت‌های یافت‌شده: ${f.recognizedWidgets.joinToString(", ")}",
                     fontSize = 11.sp)
            }
            if (f.warnings.isNotEmpty()) {
                Spacer(Modifier.height(4.dp))
                Text("⚠ هشدارها", fontSize = 11.sp, color = Color(0xFFFFB74D))
                f.warnings.take(5).forEach { w ->
                    Text("  • خط ${w.line}: ${w.message}", fontSize = 10.sp,
                         fontFamily = FontFamily.Monospace)
                }
                if (f.warnings.size > 5) {
                    Text("  ... و ${f.warnings.size - 5} مورد دیگر", fontSize = 10.sp)
                }
            }
            if (f.unrecognizedImGui.isNotEmpty()) {
                Spacer(Modifier.height(4.dp))
                Text("ImGui:: غیر-ویجت (${f.unrecognizedImGui.size} مورد)",
                     fontSize = 11.sp, color = MaterialTheme.colorScheme.onSurfaceVariant)
                f.unrecognizedImGui.take(3).forEach { u ->
                    Text("  • خط ${u.line}: ${u.text}", fontSize = 10.sp,
                         fontFamily = FontFamily.Monospace,
                         color = MaterialTheme.colorScheme.onSurfaceVariant)
                }
                if (f.unrecognizedImGui.size > 3) {
                    Text("  ... و ${f.unrecognizedImGui.size - 3} مورد دیگر", fontSize = 10.sp)
                }
            }
        }
    }
}
