// LICENSE HERE.

//
// client/rmlui/Interfaces/RenderInterface.cpp
//
//
// RmlUI N&C System Interface implementation.
// 
// Uses the refresh API and is render agnostic.
//

#include "../librmlui.h"

// RmlUI includes.
#include "../rmlui.h"

// QGL.
#include "../../src/refresh/gl/gl.h"

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif


RmlUiRenderInterface::RmlUiRenderInterface() : m_width(0), m_height(0), m_transform_enabled(false)
{

}

// Called to initialize the renderer, enable certain OpenGL states.
void RmlUiRenderInterface::Initialize() {
	m_width = r_config.width;
	m_height = r_config.height;
}

//
//=============================================================================
// CompileGeometry
// 
// Called by RmlUi when it wants to render geometry that it does not wish to 
// optimise.	
//=============================================================================
//
void RmlUiRenderInterface::RenderGeometry(Rml::Vertex* vertices, int RMLUI_UNUSED_PARAMETER(num_vertices), int* indices, int num_indices, const Rml::TextureHandle texture, const Rml::Vector2f& translation)
{
	RMLUI_UNUSED(num_vertices);

	//qglLoadIdentity();
	//// Setup 2D rendering mode.
	//qglViewport(0, 0, r_config.width, r_config.height);
	//GL_Ortho(0, r_config.width, r_config.height, 0, -1, 1);
	//qglMatrixMode(GL_MODELVIEW);
	//qglLoadIdentity();

	//qglTranslatef(translation.x, translation.y, 0);

	//qglVertexPointer(2, GL_FLOAT, sizeof(Rml::Vertex), &vertices[0].position);
	//qglEnableClientState(GL_COLOR_ARRAY);
	//qglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Rml::Vertex), &vertices[0].colour);

	//if (!texture)
	//{
	//	qglDisable(GL_TEXTURE_2D);
	//	qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
	//}
	//else
	//{
	//	qglEnable(GL_TEXTURE_2D);
	//	qglBindTexture(GL_TEXTURE_2D, (GLuint)texture);
	//	qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	//	qglTexCoordPointer(2, GL_FLOAT, sizeof(Rml::Vertex), &vertices[0].tex_coord);
	//}

	//qglDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, indices);

	//qglPopMatrix();
}

//
//=============================================================================
// CompileGeometry
// 
// Called by RmlUi when it wants to compile geometry it believes will be static 
// for the forseeable future.		
//=============================================================================
//
Rml::CompiledGeometryHandle RmlUiRenderInterface::CompileGeometry(Rml::Vertex* RMLUI_UNUSED_PARAMETER(vertices), int RMLUI_UNUSED_PARAMETER(num_vertices), int* RMLUI_UNUSED_PARAMETER(indices), int RMLUI_UNUSED_PARAMETER(num_indices), const Rml::TextureHandle RMLUI_UNUSED_PARAMETER(texture))
{
	RMLUI_UNUSED(vertices);
	RMLUI_UNUSED(num_vertices);
	RMLUI_UNUSED(indices);
	RMLUI_UNUSED(num_indices);
	RMLUI_UNUSED(texture);

	return (Rml::CompiledGeometryHandle) nullptr;
}

//
//=============================================================================
// RenderCompiledGeometry
// 
// Called by RmlUi when it wants to render application-compiled geometry.	
//=============================================================================
//
void RmlUiRenderInterface::RenderCompiledGeometry(Rml::CompiledGeometryHandle RMLUI_UNUSED_PARAMETER(geometry), const Rml::Vector2f& RMLUI_UNUSED_PARAMETER(translation))
{
	RMLUI_UNUSED(geometry);
	RMLUI_UNUSED(translation);
}

//
//=============================================================================
// ReleaseCompiledGeometry
// 
// Called by RmlUi when it wants to release application-compiled geometry.	
//=============================================================================
//
void RmlUiRenderInterface::ReleaseCompiledGeometry(Rml::CompiledGeometryHandle RMLUI_UNUSED_PARAMETER(geometry))
{
	RMLUI_UNUSED(geometry);
}

//
//=============================================================================
// EnableScissorRegion
// 
// Called by RmlUi when it wants to enable or disable scissoring to clip content.	
//=============================================================================
//
void RmlUiRenderInterface::EnableScissorRegion(bool enable)
{
	// If enabled, generate cliprect and apply.
	if (scissorEnabled) {
		clipRect_t clipRect;
		clipRect.left = scissorRect[0];
		clipRect.top = scissorRect[1];
		clipRect.right = scissorRect[2];
		clipRect.bottom = scissorRect[3];
		
		R_SetClipRect(&clipRect);
	} else {
		R_SetClipRect(nullptr);
	}

	scissorEnabled = enable;
}

//
//=============================================================================
// SetScissorRegion
// 
// Called by RmlUi when it wants to change the scissor region.		
//=============================================================================
//
void RmlUiRenderInterface::SetScissorRegion(int x, int y, int width, int height)
{
	// Store scissor rect.
	scissorRect[0] = x;
	scissorRect[1] = y;
	scissorRect[2] = width;
	scissorRect[3] = height;

	// If enabled, generate cliprect and apply.
	if (scissorEnabled) {
		clipRect_t clipRect;
		clipRect.left = scissorRect[0];
		clipRect.top = scissorRect[1];
		clipRect.right = scissorRect[2];
		clipRect.bottom = scissorRect[3];
				
		R_SetClipRect(&clipRect);
	}
}

//
//=============================================================================
// LoadTexture
// 
// Called by RmlUi when a texture is required by the library.		
//=============================================================================
//
bool RmlUiRenderInterface::LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source)
{
	// Load pic and fetch width+height.
	qhandle_t picHandle = R_RegisterPic2(source.c_str());

	// If no handle was returned, return false.
	if (!picHandle) {
		Rml::Log::Message(Rml::Log::LT_ERROR, "Couildn't find pic: \"%s\"", source.c_str());
		return false;
	}


	R_GetPicSize(picWidth, picHeight, picHandle);
	
	// Pass over information to RmlUI.
	texture_dimensions.x = width;
	texture_dimensions.y = height;
	texture_handle = picHandle;

	// Success.
	return true;
}

//
//=============================================================================
// GenerateTexture
// 
// Called by RmlUi when a texture is required to be built from an internally-
// generated sequence of pixels.
//=============================================================================
//
bool RmlUiRenderInterface::GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions)
{
	// I know using this is a cheap method to generate names with but it'll have to do for now.
	static int32_t imageID = 0;
	Rml::String imageName = "rmlui_image_" + imageID;

	// Register image data, use nearest filtering.
	R_RegisterRawImage(imageName.c_str(), source_dimensions.x, source_dimensions.y, source, IF_SRGB, IF_NEAREST);

	// Increase imageID.
	imageID++;

	return true;
}

//
//=============================================================================
// ReleaseTexture
// 
// Called by RmlUi when a loaded texture is no longer required.		
//=============================================================================
//
void RmlUiRenderInterface::ReleaseTexture(Rml::TextureHandle texture_handle)
{
	// Unregister handle, simple as that.
	R_UnregisterImage(texture_handle);
}

//poly_t* RmlUiRenderInterface::GenerateRefreshGeometry(bool temp, Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture) {
//	poly_t* poly;
//	int i;
//
//	if (temp) {
//		poly = polyAlloc.get_temp(num_vertices, num_indices);
//	}
//	else {
//		poly = polyAlloc.alloc(num_vertices, num_indices);
//	}
//
//	// copy stuff over
//	for (i = 0; i < num_vertices; i++) {
//		poly->verts[i][0] = vertices[i].position.x;
//		poly->verts[i][1] = vertices[i].position.y;
//		poly->verts[i][2] = 0;
//		poly->verts[i][3] = 1;
//
//		poly->normals[i][0] = 0;
//		poly->normals[i][1] = 0;
//		poly->normals[i][2] = 0;
//		poly->normals[i][3] = 0;
//
//		poly->stcoords[i][0] = vertices[i].tex_coord.x;
//		poly->stcoords[i][1] = vertices[i].tex_coord.y;
//
//		poly->colors[i][0] = vertices[i].colour.red;
//		poly->colors[i][1] = vertices[i].colour.green;
//		poly->colors[i][2] = vertices[i].colour.blue;
//		poly->colors[i][3] = vertices[i].colour.alpha;
//	}
//
//	for (i = 0; i < num_indices; i++)
//		poly->elems[i] = indices[i];
//
//	poly->shader = (texture == 0 ? whiteShader : (shader_t*)texture);
//
//	return poly;
//}

//
//=============================================================================
// SetTransform
// 
// Called by RmlUi when it wants to set the current transform matrix to a new matrix.	
//=============================================================================
//
void RmlUiRenderInterface::SetTransform(const Rml::Matrix4f* transform)
{
	// Enable/Disable transform.
	m_transform_enabled = (bool)transform;

	if (transform) {
		// Transpose matrix in case it is a row major mat.
		if (std::is_same<Rml::Matrix4f, Rml::ColumnMajorMatrix4f>::value) {
			R_Set2DTransform(transform->data());
		} else if (std::is_same<Rml::Matrix4f, Rml::RowMajorMatrix4f>::value) {
			R_Set2DTransform(transform->Transpose().data());
		}
	} else {
		R_Set2DTransform(mat4_identity());
	}
}

//
//=============================================================================
// CaptureScreen
// 
// Called by RmlUi when it wants to capture the screen output.
//=============================================================================
//
RmlUiRenderInterface::Image RmlUiRenderInterface::CaptureScreen()
{
	Image image;

	return image;
}
