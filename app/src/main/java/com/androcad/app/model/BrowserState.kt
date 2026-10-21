// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.model

data class CadBodyItem(
    val id: String,
    var name: String,
    var isVisible: Boolean = true,
    var isSelected: Boolean = false,
    val faceCount: Int = 6,
    val volume: Double = 100000.0
)

data class CadSketchItem(
    val id: String,
    var name: String,
    var isVisible: Boolean = true,
    var isSelected: Boolean = false,
    val profileArea: Double = 5000.0
)

data class BrowserState(
    var isExpanded: Boolean = true,
    var isDocSettingsExpanded: Boolean = false,
    var isNamedViewsExpanded: Boolean = false,
    var isOriginExpanded: Boolean = false,
    var isBodiesExpanded: Boolean = true,
    var isSketchesExpanded: Boolean = true,
    var isConstructionExpanded: Boolean = false,
    var originXyVisible: Boolean = true,
    var originXzVisible: Boolean = true,
    var originYzVisible: Boolean = true,
    val bodies: MutableList<CadBodyItem> = mutableListOf(
        CadBodyItem("body_1", "Solid Box 1", isVisible = true, isSelected = false)
    ),
    val sketches: MutableList<CadSketchItem> = mutableListOf(),
    var selectedEntityId: String? = null
) {
    fun toggleVisibility(bodyId: String) {
        bodies.find { it.id == bodyId }?.let {
            it.isVisible = !it.isVisible
        }
    }

    fun selectItem(id: String) {
        selectedEntityId = id
        bodies.forEach { it.isSelected = (it.id == id) }
        sketches.forEach { it.isSelected = (it.id == id) }
    }

    fun renameBody(bodyId: String, newName: String) {
        bodies.find { it.id == bodyId }?.let {
            it.name = newName
        }
    }

    fun deleteBody(bodyId: String) {
        bodies.removeAll { it.id == bodyId }
        if (selectedEntityId == bodyId) {
            selectedEntityId = null
        }
    }
}
