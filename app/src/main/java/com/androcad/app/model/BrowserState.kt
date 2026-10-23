// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.model

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue

class CadBodyItem(
    val id: String,
    name: String,
    isVisible: Boolean = true,
    isSelected: Boolean = false,
    val faceCount: Int = 6,
    val volume: Double = 100000.0,
    var solid: com.androcad.kernel.BoxMeshResult? = null
) {
    var name by mutableStateOf(name)
    var isVisible by mutableStateOf(isVisible)
    var isSelected by mutableStateOf(isSelected)
}

class CadSketchItem(
    val id: String,
    name: String,
    isVisible: Boolean = true,
    isSelected: Boolean = false,
    val profileArea: Double = 5000.0
) {
    var name by mutableStateOf(name)
    var isVisible by mutableStateOf(isVisible)
    var isSelected by mutableStateOf(isSelected)
}

class BrowserState(
    isExpanded: Boolean = true,
    isDocSettingsExpanded: Boolean = false,
    isNamedViewsExpanded: Boolean = false,
    isOriginExpanded: Boolean = false,
    isBodiesExpanded: Boolean = true,
    isSketchesExpanded: Boolean = true,
    isConstructionExpanded: Boolean = false,
    originXyVisible: Boolean = true,
    originXzVisible: Boolean = true,
    originYzVisible: Boolean = true,
    bodies: List<CadBodyItem> = listOf(
        CadBodyItem("body_1", "Solid Box 1", isVisible = true, isSelected = false)
    ),
    sketches: List<CadSketchItem> = emptyList(),
    selectedEntityId: String? = null
) {
    var isExpanded by mutableStateOf(isExpanded)
    var isDocSettingsExpanded by mutableStateOf(isDocSettingsExpanded)
    var isNamedViewsExpanded by mutableStateOf(isNamedViewsExpanded)
    var isOriginExpanded by mutableStateOf(isOriginExpanded)
    var isBodiesExpanded by mutableStateOf(isBodiesExpanded)
    var isSketchesExpanded by mutableStateOf(isSketchesExpanded)
    var isConstructionExpanded by mutableStateOf(isConstructionExpanded)
    var originXyVisible by mutableStateOf(originXyVisible)
    var originXzVisible by mutableStateOf(originXzVisible)
    var originYzVisible by mutableStateOf(originYzVisible)
    val bodies = mutableStateListOf<CadBodyItem>().apply { addAll(bodies) }
    val sketches = mutableStateListOf<CadSketchItem>().apply { addAll(sketches) }
    var selectedEntityId by mutableStateOf(selectedEntityId)

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

    fun addBody(body: CadBodyItem) {
        bodies.add(body)
    }

    fun addSketch(sketch: CadSketchItem) {
        sketches.add(sketch)
    }
}
