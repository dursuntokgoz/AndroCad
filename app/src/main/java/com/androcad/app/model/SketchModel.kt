// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.model

import androidx.compose.ui.geometry.Offset
import kotlin.math.PI
import kotlin.math.abs
import kotlin.math.atan2
import kotlin.math.cos
import kotlin.math.hypot
import kotlin.math.round
import kotlin.math.sin

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

    data class Polygon(
        override val id: String,
        val centerX: Float,
        val centerY: Float,
        val radius: Float,
        val sides: Int = 6
    ) : SketchEntity() {
        val area: Float
            get() {
                val n = sides.coerceAtLeast(3)
                return (0.5 * n * radius * radius * sin(2.0 * PI / n)).toFloat()
            }

        fun toOutlinePoints(): List<Pair<Float, Float>> {
            val n = sides.coerceAtLeast(3)
            return (0 until n).map { i ->
                val angle = 2.0 * PI * i / n
                Pair(
                    centerX + (radius * cos(angle)).toFloat(),
                    centerY + (radius * sin(angle)).toFloat()
                )
            }
        }
    }

    data class Slot(
        override val id: String,
        val startX: Float,
        val startY: Float,
        val endX: Float,
        val endY: Float,
        val radius: Float
    ) : SketchEntity() {
        val centerLength: Float get() = hypot(endX - startX, endY - startY)
        val area: Float get() = (centerLength * 2f * radius + PI * radius * radius).toFloat()

        fun toOutlinePoints(capSegments: Int = 6): List<Pair<Float, Float>> {
            val dx = endX - startX
            val dy = endY - startY
            val baseAngle = atan2(dy.toDouble(), dx.toDouble())
            val pts = mutableListOf<Pair<Float, Float>>()
            // Cap around end (baseAngle - PI/2 .. baseAngle + PI/2)
            for (i in 0..capSegments) {
                val a = baseAngle - PI / 2.0 + (PI * i / capSegments)
                pts.add(Pair(endX + (radius * cos(a)).toFloat(), endY + (radius * sin(a)).toFloat()))
            }
            // Cap around start (baseAngle + PI/2 .. baseAngle + 3*PI/2)
            for (i in 0..capSegments) {
                val a = baseAngle + PI / 2.0 + (PI * i / capSegments)
                pts.add(Pair(startX + (radius * cos(a)).toFloat(), startY + (radius * sin(a)).toFloat()))
            }
            return pts
        }
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

    fun offsetLastEntity(entities: List<SketchEntity>, offsetMm: Float = 5f): SketchEntity? {
        val last = entities.lastOrNull() ?: return null
        val ts = System.currentTimeMillis()
        return when (last) {
            is SketchEntity.Rectangle -> {
                val nw = (last.width + 2f * offsetMm).coerceAtLeast(4f)
                val nh = (last.height + 2f * offsetMm).coerceAtLeast(4f)
                SketchEntity.Rectangle(
                    id = "rect_offset_$ts",
                    minX = last.minX - offsetMm,
                    minY = last.minY - offsetMm,
                    width = nw,
                    height = nh
                )
            }
            is SketchEntity.Circle -> {
                SketchEntity.Circle(
                    id = "circ_offset_$ts",
                    centerX = last.centerX,
                    centerY = last.centerY,
                    radius = (last.radius + offsetMm).coerceAtLeast(2f)
                )
            }
            is SketchEntity.Polygon -> {
                SketchEntity.Polygon(
                    id = "poly_offset_$ts",
                    centerX = last.centerX,
                    centerY = last.centerY,
                    radius = (last.radius + offsetMm).coerceAtLeast(3f),
                    sides = last.sides
                )
            }
            is SketchEntity.Slot -> {
                SketchEntity.Slot(
                    id = "slot_offset_$ts",
                    startX = last.startX,
                    startY = last.startY,
                    endX = last.endX,
                    endY = last.endY,
                    radius = (last.radius + offsetMm).coerceAtLeast(2f)
                )
            }
            is SketchEntity.Line -> null
        }
    }

    fun evaluateProfile(entities: List<SketchEntity>): SketchClosedProfile? {
        if (entities.isEmpty()) return null

        val slots = entities.filterIsInstance<SketchEntity.Slot>()
        if (slots.isNotEmpty()) {
            val lastSlot = slots.last()
            return SketchClosedProfile(
                isClosed = true,
                area = lastSlot.area.toDouble(),
                description = "Slot (${lastSlot.centerLength.toInt()} × ${(lastSlot.radius * 2).toInt()} mm)",
                entityCount = entities.size
            )
        }

        val polys = entities.filterIsInstance<SketchEntity.Polygon>()
        if (polys.isNotEmpty()) {
            val lastPoly = polys.last()
            return SketchClosedProfile(
                isClosed = true,
                area = lastPoly.area.toDouble(),
                description = "Hexagon (R ${lastPoly.radius.toInt()} mm)",
                entityCount = entities.size
            )
        }

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
