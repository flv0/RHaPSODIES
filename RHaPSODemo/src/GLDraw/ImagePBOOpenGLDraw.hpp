/*============================================================================*/
/*                  Copyright (c) 2014 RWTH Aachen University                 */
/*============================================================================*/
/*                                  License                                   */
/*                                                                            */
/*  This program is free software: you can redistribute it and/or modify      */
/*  it under the terms of the GNU Lesser General Public License as published  */
/*  by the Free Software Foundation, either version 3 of the License, or      */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU Lesser General Public License for more details.                       */
/*                                                                            */
/*  You should have received a copy of the GNU Lesser General Public License  */
/*  along with this program.  If not, see <http://www.gnu.org/licenses/>.     */
/*============================================================================*/
/*                                Contributors                                */
/*                                                                            */
/*============================================================================*/
// $Id: $

#ifndef _RHAPSODIES_IMAGEPBOOPENGLDRAW
#define _RHAPSODIES_IMAGEPBOOPENGLDRAW

#include <GL/gl.h>

#include <GLDraw/TexturedQuadGLDraw.hpp>

namespace rhapsodies {
	class ShaderRegistry;

	class ImagePBOOpenGLDraw : public TexturedQuadGLDraw {
		GLuint m_pboIds[2];
		unsigned char m_pboIndex;
		void *m_pPBO;

		int m_texWidth;
		int m_texHeight;
		bool m_texUpdate;

		void UpdateTexture();

	public:
		ImagePBOOpenGLDraw(int width, int height,
						   ShaderRegistry *pShaderReg);
		virtual ~ImagePBOOpenGLDraw();

		bool Do();

		bool FillPBOFromBuffer(const unsigned char*,
							   int width, int height);
	};
}

#endif // _RHAPSODIES_IMAGEPBOOPENGLDRAW
