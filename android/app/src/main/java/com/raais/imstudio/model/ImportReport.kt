package com.raais.imstudio.model

import kotlinx.serialization.SerialName
import kotlinx.serialization.Serializable

@Serializable
data class ImportIssue(
    val line: Int = 0,
    val text: String = "",
    val message: String = ""
)

@Serializable
data class ImportFileReport(
    val path: String = "",
    @SerialName("size_bytes") val sizeBytes: Long = 0,
    @SerialName("lines_scanned") val linesScanned: Int = 0,
    @SerialName("widgets_found") val widgetsFound: Int = 0,
    @SerialName("lines_non_imgui") val linesNonImGui: Int = 0,
    val status: String = "ok",
    @SerialName("recognized_widgets") val recognizedWidgets: List<String> = emptyList(),
    val warnings: List<ImportIssue> = emptyList(),
    @SerialName("unrecognized_imgui") val unrecognizedImGui: List<ImportIssue> = emptyList()
)

@Serializable
data class ImportSession(
    @SerialName("total_files") val totalFiles: Int = 0,
    @SerialName("files_with_widgets") val filesWithWidgets: Int = 0,
    @SerialName("files_without_imgui") val filesWithoutImGui: Int = 0,
    @SerialName("total_widgets") val totalWidgets: Int = 0,
    @SerialName("total_warnings") val totalWarnings: Int = 0,
    @SerialName("total_global_warnings") val totalGlobalWarnings: Int = 0
)

@Serializable
data class ImportReport(
    val session: ImportSession = ImportSession(),
    val files: List<ImportFileReport> = emptyList(),
    @SerialName("global_warnings") val globalWarnings: List<String> = emptyList()
)
