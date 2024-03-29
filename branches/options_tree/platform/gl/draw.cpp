/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2013 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../platform.h"
#include "../../ui/ui.h"
#include "../../tools/profiler.h"
#include "../../tools/options.h"
#include "../../palette.h"

#ifdef USE_GL

#ifdef _WINDOWS
#include <windows.h>
#endif//_WINDOWS

#ifndef _MAC
#include <GL/gl.h>
#else//_MAC
#include <OpenGL/gl.h>
#endif//_MAC


PROFILER_DECLARE(draw_p);
PROFILER_DECLARE(draw);

namespace xPlatform
{

static struct eOptionZoom : public xOptions::eOptionInt
{
	virtual const char* Name() const { return "zoom"; }
	virtual void Change(bool next = true)
	{
		eOptionInt::Change(3, next);
	}
	float Zoom() const
	{
		switch(*this)
		{
			case 1: return 300.0f/256.0f;
			case 2: return 320.0f/256.0f;
			default: return 1.0f;
		}
	}
protected:
	virtual const char** Values() const
	{
		static const char* values[] = { "fill screen", "small border", "no border", NULL };
		return values;
	}
} op_zoom;
DECLARE_OPTION_ACCESSOR(eOptionInt, op_zoom);

float OpZoom() { return op_zoom.Zoom(); }

static struct eOptionFiltering : public xOptions::eOptionBool
{
	eOptionFiltering() { Set(true); }
	virtual const char* Name() const { return "filtering"; }
} op_filtering;
DECLARE_OPTION_ACCESSOR(eOptionBool, op_filtering);


static dword tex[512*256];

#ifdef USE_BIG_ENDIAN
#define RGBX(r, g, b) (((r) << 24)|((g) << 16)|((b) << 8))
#else//USE_BIG_ENDIAN
#define RGBX(r, g, b) (((b) << 16)|((g) << 8)|(r))
#endif//USE_BIG_ENDIAN

//=============================================================================
//	DrawGL
//-----------------------------------------------------------------------------

static const GLushort vertices[4 * 2] =
{
	0, 0,
	0, 1,
	1, 1,
	1, 0,
};
static const GLubyte triangles[2 * 3] =
{
	0, 1, 2,
	0, 2, 3,
};
static struct eCachedColors
{
	eCachedColors() : palette(NULL) { memset(items, 0, sizeof(items)); }
	void Update()
	{
		const dword* p = Palette();
		if(p == palette)
			return;
		palette = p;
		for(int c = 0; c < 16; ++c)
		{
			items[c] = RGBX(*p&0xff, (*p >> 8)&0xff, (*p >> 16)&0xff);
			++p;
		}
	}
	dword items[16];
	const dword* palette;
} color_cache;

void DrawGL(int _w, int _h)
{
	PROFILER_BEGIN(draw_p);
	color_cache.Update();
	byte* data = (byte*)Handler()->VideoData();
	dword* p = tex;
#ifdef USE_UI
	byte* data_ui = (byte*)Handler()->VideoDataUI();
	if(data_ui)
	{
		for(int y = 0; y < 240; ++y)
		{
			for(int x = 0; x < 320; ++x)
			{
				xUi::eRGBAColor c_ui = xUi::palette[*data_ui++];
				xUi::eRGBAColor c = color_cache.items[*data++];
				*p++ = RGBX((c.r >> c_ui.a) + c_ui.r, (c.g >> c_ui.a) + c_ui.g, (c.b >> c_ui.a) + c_ui.b);
			}
			p += 512 - 320;
		}
	}
	else
#endif//USE_UI
	{
		for(int y = 0; y < 240; ++y)
		{
			for(int x = 0; x < 320; ++x)
			{
				*p++ = color_cache.items[*data++];
			}
			p += 512 - 320;
		}
	}
	PROFILER_END(draw_p);

	PROFILER_SECTION(draw);

	float sx, sy;
	GetScaleWithAspectRatio43(&sx, &sy, _w, _h);
	int w = sx * _w * OpZoom();
	int h = sy * _h * OpZoom();

	GLint filter = op_filtering ? GL_LINEAR : GL_NEAREST;

	glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glScalef(320.0f/512.0f, 240.0f/256.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

	glColor3f(1.0f, 1.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f);
	glViewport((_w - w)/2, (_h - h)/2, w, h);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_SHORT, 0, vertices);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_SHORT, 0, vertices);
	glDrawElements(GL_TRIANGLES, 2*3, GL_UNSIGNED_BYTE, triangles);

	glFlush();
}

}
//namespace xPlatform

#endif//USE_GL
