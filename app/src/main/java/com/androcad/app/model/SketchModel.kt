// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.model

import androidx.compose.ui.geometry.Offset
import kotlin.math.PI
import kotlin.math.abs
import kotlin.math.hypot
import kotlin.math.round

sealed class SketchEntity {
    abstract val id: String

    data class Rectangle(
        override val id: String,
        val minX: Float,
        val minY: Float,
        val width: Float,
        val height: Float
    ) : SketchEntity() {
        val area: Float get() = abs(width * height)
    }

    data class Circle(
        override val id: String,
        val centerX: Float,
        val centerY: Float,
        val radius: Float
    ) : SketchEntity() {
        val area: Float get() = (PI * radius * radius).toFloat()
    }

    data class Line(
        override val id: String,
        val startX: Float,
        val startY: Float,
        val endX: Float,
        val endY: Float
    ) : SketchEntity() {
        val length: Float get() = hypot(endX - startX, endY - startY)
    }
}

data class SketchClosedProfile(
    val isClosed: Boolean,
    val area: Double,
    val description: String,
    val entityCount: Int
)

object SketchMath {
    fun snapToGrid(value: Float, step: Float): Float {
        if (step <= 0f) return value
        return round(value / step) * step
    }

    fun evaluateProfile(entities: List<SketchEntity>): SketchClosedProfile? {
        if (entities.isEmpty()) return null

        val rects = entities.filterIsInstance<SketchEntity.Rectangle>()
        if (rects.isNotEmpty()) {
            val lastRect = rects.last()
            return SketchClosedProfile(
                isClosed = true,
                area = lastRect.area.toDouble(),
                description = "Rectangle (${lastRect.width.toInt()} × ${lastRect.height.toInt()} mm)",
                entityCount = entities.size
            )
        }

        val circles = entities.filterIsInstance<SketchEntity.Circle>()
        if (circles.isNotEmpty()) {
            val lastCirc = circles.last()
            return SketchClosedProfile(
                isClosed = true,
                area = lastCirc.area.toDouble(),
                description = "Circle (Ø ${(lastCirc.radius * 2).toInt()} mm)",
                entityCount = entities.size
            )
        }

        val lines = entities.filterIsInstance<SketchEntity.Line>()
        if (lines.size >= 3) {
            // Check if any subset or full sequence of lines forms a closed loop
            val endpoints = mutableListOf<Pair<Float, Float>>()
            for (l in lines) {
                endpoints.add(Pair(l.startX, l.startY))
                endpoints.add(Pair(l.endX, l.endY))
            }
            var unmatchedCount = 0
            for (i in endpoints.indices) {
                val p = endpoints[i]
                var hasNeighbor = false
                for (j in endpoints.indices) {
                    if (i != j && hypot(p.first - endpoints[j].first, p.second - endpoints[j].second) <= 10f) {
                        hasNeighbor = true
                        break
                    }
                }
                if (!hasNeighbor) unmatchedCount++
            }
            val first = lines.first()
            val last = lines.last()
            val directDist = hypot(first.startX - last.endX, first.startY - last.endY)
            if (unmatchedCount == 0 || directDist <= 10f) {
                var sum = 0.0
                for (i in lines.indices) {
                    val l = lines[i]
                    sum += (l.startX * l.endY - l.endX * l.startY)
                }
                val polygonArea = abs(sum) * 0.5
                return SketchClosedProfile(
                    isClosed = true,
                    area = if (polygonArea > 1.0) polygonArea else 1000.0,
                    description = "Closed Polygon (${lines.size} segments)",
                    entityCount = lines.size
                )
            }
        }

        return SketchClosedProfile(
            isClosed = false,
            area = 0.0,
            description = "Open Wire (${entities.size} entities)",
            entityCount = entities.size
        )
    }
}
