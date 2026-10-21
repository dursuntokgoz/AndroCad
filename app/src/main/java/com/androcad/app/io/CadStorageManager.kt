// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.io

import android.content.Context
import android.net.Uri
import com.androcad.kernel.BoxMeshResult
import com.androcad.kernel.NativeKernel
import com.androcad.kernel.NativeTimeline
import java.io.File
import java.io.FileOutputStream
import java.io.InputStream
import java.io.OutputStream

object CadStorageManager {

    fun importStepFromUri(context: Context, uri: Uri): BoxMeshResult {
        val tempFile = File(context.cacheDir, "import_step_${System.currentTimeMillis()}.step")
        return try {
            copyUriToFile(context, uri, tempFile)
            NativeKernel.importStep(tempFile.absolutePath)
        } catch (t: Throwable) {
            BoxMeshResult(
                success = false,
                volume = 0.0,
                faceCount = 0,
                edgeCount = 0,
                triangleCount = 0,
                vertexBuffer = null,
                indexBuffer = null,
                edgeVertexBuffer = null,
                boundingBox = null,
                errorMessage = "Failed to copy content URI to cache: ${t.message}",
                nativeHandle = 0L
            )
        } finally {
            if (tempFile.exists()) {
                tempFile.delete()
            }
        }
    }

    fun importStlFromUri(context: Context, uri: Uri): BoxMeshResult {
        val tempFile = File(context.cacheDir, "import_stl_${System.currentTimeMillis()}.stl")
        return try {
            copyUriToFile(context, uri, tempFile)
            NativeKernel.importStl(tempFile.absolutePath)
        } catch (t: Throwable) {
            BoxMeshResult(
                success = false,
                volume = 0.0,
                faceCount = 0,
                edgeCount = 0,
                triangleCount = 0,
                vertexBuffer = null,
                indexBuffer = null,
                edgeVertexBuffer = null,
                boundingBox = null,
                errorMessage = "Failed to copy content URI to cache: ${t.message}",
                nativeHandle = 0L
            )
        } finally {
            if (tempFile.exists()) {
                tempFile.delete()
            }
        }
    }

    fun exportStepToUri(context: Context, solid: BoxMeshResult, uri: Uri): Boolean {
        val tempFile = File(context.cacheDir, "export_step_${System.currentTimeMillis()}.step")
        return try {
            val res = NativeKernel.exportStep(solid, tempFile.absolutePath)
            if (!res.success || !tempFile.exists() || tempFile.length() == 0L) {
                return false
            }
            copyFileToUri(context, tempFile, uri)
            true
        } catch (t: Throwable) {
            false
        } finally {
            if (tempFile.exists()) {
                tempFile.delete()
            }
        }
    }

    fun exportStlToUri(context: Context, solid: BoxMeshResult, uri: Uri): Boolean {
        val tempFile = File(context.cacheDir, "export_stl_${System.currentTimeMillis()}.stl")
        return try {
            val res = NativeKernel.exportStl(solid, tempFile.absolutePath)
            if (!res.success || !tempFile.exists() || tempFile.length() == 0L) {
                return false
            }
            copyFileToUri(context, tempFile, uri)
            true
        } catch (t: Throwable) {
            false
        } finally {
            if (tempFile.exists()) {
                tempFile.delete()
            }
        }
    }

    fun saveProjectToUri(context: Context, timeline: NativeTimeline, uri: Uri): Boolean {
        val tempFile = File(context.cacheDir, "save_proj_${System.currentTimeMillis()}.androcad")
        return try {
            val res = timeline.saveProject(tempFile.absolutePath)
            if (!res.success || !tempFile.exists()) {
                return false
            }
            copyFileToUri(context, tempFile, uri)
            true
        } catch (t: Throwable) {
            false
        } finally {
            if (tempFile.exists()) {
                tempFile.delete()
            }
        }
    }

    fun loadProjectFromUri(context: Context, timeline: NativeTimeline, uri: Uri): Boolean {
        val tempFile = File(context.cacheDir, "load_proj_${System.currentTimeMillis()}.androcad")
        return try {
            copyUriToFile(context, uri, tempFile)
            val res = timeline.loadProject(tempFile.absolutePath)
            res.success
        } catch (t: Throwable) {
            false
        } finally {
            if (tempFile.exists()) {
                tempFile.delete()
            }
        }
    }

    private fun copyUriToFile(context: Context, uri: Uri, destFile: File) {
        val inputStream: InputStream = context.contentResolver.openInputStream(uri)
            ?: throw IllegalStateException("Cannot open input stream for URI: $uri")
        inputStream.use { input ->
            FileOutputStream(destFile).use { output ->
                input.copyTo(output)
            }
        }
    }

    private fun copyFileToUri(context: Context, srcFile: File, uri: Uri) {
        val outputStream: OutputStream = context.contentResolver.openOutputStream(uri)
            ?: throw IllegalStateException("Cannot open output stream for URI: $uri")
        outputStream.use { output ->
            srcFile.inputStream().use { input ->
                input.copyTo(output)
            }
        }
    }
}
