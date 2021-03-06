﻿/* ***************************************************************************
 * win32_surface.c -- surface support for win32 platform.
 *
 * Copyright (C) 2014-2015 by Liu Chao <lc-soft@live.cn>
 *
 * This file is part of the LCUI project, and may only be used, modified, and
 * distributed under the terms of the GPLv2.
 *
 * (GPLv2 is abbreviation of GNU General Public License Version 2)
 *
 * By continuing to use, modify, or distribute this file you indicate that you
 * have read the license and understand and accept it fully.
 *
 * The LCUI project is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GPL v2 for more details.
 *
 * You should have received a copy of the GPLv2 along with this file. It is
 * usually in the LICENSE.TXT file, If not, see <http://www.gnu.org/licenses/>.
 * ***************************************************************************/

/* ****************************************************************************
 * win32_surface.c -- win32平台的 surface 功能支持。
 *
 * 版权所有 (C) 2014-2015 归属于 刘超 <lc-soft@live.cn>
 *
 * 这个文件是LCUI项目的一部分，并且只可以根据GPLv2许可协议来使用、更改和发布。
 *
 * (GPLv2 是 GNU通用公共许可证第二版 的英文缩写)
 *
 * 继续使用、修改或发布本文件，表明您已经阅读并完全理解和接受这个许可协议。
 *
 * LCUI 项目是基于使用目的而加以散布的，但不负任何担保责任，甚至没有适销性或特
 * 定用途的隐含担保，详情请参照GPLv2许可协议。
 *
 * 您应已收到附随于本文件的GPLv2许可协议的副本，它通常在LICENSE.TXT文件中，如果
 * 没有，请查看：<http://www.gnu.org/licenses/>.
 * ***************************************************************************/

#include <LCUI_Build.h>
#define __IN_SURFACE_SOURCE_FILE__
#ifdef LCUI_BUILD_IN_WIN32
#include <LCUI/LCUI.h>
#include <LCUI/graph.h>
#include <LCUI/thread.h>
#include <LCUI/widget_build.h>
#include <LCUI/surface.h>
#include "resource.h"

#define WIN32_WINDOW_STYLE	(WS_OVERLAPPEDWINDOW &~WS_THICKFRAME &~WS_MAXIMIZEBOX)
#define MSG_SURFACE_CREATE	WM_USER+100

enum SurfaceTaskType {
	TASK_MOVE,
	TASK_RESIZE,
	TASK_SHOW,
	TASK_SET_CAPTION,
	TASK_TOTAL_NUM
};

typedef struct LCUI_SurfaceTask {
	LCUI_BOOL is_valid;
	union {
		struct {
			int x, y;
		};
		struct {
			int width, height;
		};
		LCUI_BOOL show;
		wchar_t *caption;
	};
} LCUI_SurfaceTask;

struct LCUI_SurfaceRec_ {
	HWND hwnd;
	int mode;
	int w, h;
	HDC fb_hdc;
	HBITMAP fb_bmp;
	LCUI_Graph fb;
	LCUI_SurfaceTask task_buffer[TASK_TOTAL_NUM];
};

static struct {
	HINSTANCE main_instance;	/**< 主程序的资源句柄 */
	HINSTANCE dll_instance;		/**< 动态库中的资源句柄 */
	LCUI_Thread loop_thread;	/**< 消息循环线程 */
	LCUI_Cond cond;			/**< 条件，当消息循环已创建时成立 */
	LCUI_BOOL is_ready;		/**< 消息循环线程是否已准备好 */
} win32;

void Win32_LCUI_Init( HINSTANCE hInstance )
{
	win32.main_instance = hInstance;
}

/** win32的动态库的入口函数 */
BOOL APIENTRY DllMain( HMODULE hModule, DWORD reason, LPVOID unused )
{
	switch (reason) {
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	win32.dll_instance = hModule;
	return TRUE;
}

/** 根据 hwnd 获取 Surface */
static LCUI_Surface GetSurfaceByHWND( HWND hwnd )
{
	return NULL;
}

static LCUI_Surface GetSurfaceByWidget( LCUI_Widget widget )
{
	return NULL;
}

static LRESULT CALLBACK
WndProc( HWND hwnd, UINT msg, WPARAM arg1, LPARAM arg2 )
{
	LCUI_Surface surface;
	surface = GetSurfaceByHWND( hwnd);
	if( !surface ) {
		return DefWindowProc( hwnd, msg, arg1, arg2 );
	}
	switch( msg ) {
	case WM_SETFOCUS:
		break;
	case WM_KILLFOCUS:
		break;
	case WM_CLOSE:
		Surface_Delete( surface );
		break;
	case WM_SHOWWINDOW:
		if( arg1 ) {
			//Widget_Show( surface->target );
		} else {
			//Widget_Hide( surface->target );
		}
		break;
	case WM_ACTIVATE:
		break;
	default:break;
	}
	return DefWindowProc( hwnd, msg, arg1, arg2 );
}

/** 删除 Surface */
void Surface_Delete( LCUI_Surface surface )
{
	surface->w = 0;
	surface->h = 0;
	Graph_Free( &surface->fb );
}

/** 新建一个 Surface */
LCUI_Surface Surface_New(void)
{
	int i;
	LCUI_Surface surface;

	surface = (LCUI_Surface)malloc(sizeof(struct LCUI_SurfaceRec_));
	surface->mode = RENDER_MODE_BIT_BLT;
	surface->fb_hdc = NULL;
	surface->fb_bmp = NULL;
	Graph_Init( &surface->fb );
	surface->fb.color_type = COLOR_TYPE_ARGB;
	surface->hwnd = NULL;
	for( i=0; i<TASK_TOTAL_NUM; ++i ) {
		surface->task_buffer[i].is_valid = FALSE;
	}
	if( !win32.is_ready ) {
		/* 等待 Surface 线程创建完 windows 消息队列 */
		LCUICond_Wait( &win32.cond );
	}
	/* 让 Surface 线程去完成 windows 窗口的创建 */
	PostThreadMessage( win32.loop_thread, MSG_SURFACE_CREATE, 0, (LPARAM)surface );
	return surface;
}

static void Surface_ExecMove( LCUI_Surface surface, int x, int y )
{
	x += GetSystemMetrics( SM_CXFIXEDFRAME );
	y += GetSystemMetrics( SM_CYFIXEDFRAME );
	SetWindowPos(
		surface->hwnd, HWND_NOTOPMOST,
		x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER
		);
	return;
}

void Surface_Move( LCUI_Surface surface, int x, int y )
{
	/* 如果已经得到窗口句柄，则直接改变窗口位置 */
	if( surface->hwnd ) {
		Surface_ExecMove( surface, x, y );
		return;
	}
	/* 缓存任务，等获得窗口句柄后处理 */
	surface->task_buffer[TASK_MOVE].x = x;
	surface->task_buffer[TASK_MOVE].y = y;
	surface->task_buffer[TASK_MOVE].is_valid = TRUE;
}

void Surface_ExecResize( LCUI_Surface surface, int w, int h )
{
	HDC hdc_client;
	HBITMAP old_bmp;

	surface->w = w;
	surface->h = h;
	Graph_Create( &surface->fb, w, h );
	hdc_client = GetDC( surface->hwnd );
	surface->fb_bmp = CreateCompatibleBitmap( hdc_client, w, h );
	old_bmp = (HBITMAP)SelectObject( surface->fb_hdc, surface->fb_bmp );
	if( old_bmp ) {
		DeleteObject( old_bmp );
	}
	/* 加上窗口边框的尺寸 */
	w += GetSystemMetrics( SM_CXFIXEDFRAME ) * 2;
	h += GetSystemMetrics( SM_CYFIXEDFRAME ) * 2;
	h += GetSystemMetrics( SM_CYCAPTION );
	SetWindowLong( surface->hwnd, GWL_STYLE, WIN32_WINDOW_STYLE );
	SetWindowPos(
		surface->hwnd, HWND_NOTOPMOST,
		0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER
	);
}

void Surface_Resize( LCUI_Surface surface, int w, int h )
{
	if( surface->hwnd ) {
		Surface_ExecResize( surface, w, h );
		return;
	}
	surface->task_buffer[TASK_RESIZE].width = w;
	surface->task_buffer[TASK_RESIZE].height = h;
	surface->task_buffer[TASK_RESIZE].is_valid = TRUE;
}

void Surface_Show( LCUI_Surface surface )
{
	if( surface->hwnd ) {
		DEBUG_MSG("surface: %p, direct show.\n", surface);
		ShowWindow( surface->hwnd, SW_SHOWNORMAL );
		return;
	}
	DEBUG_MSG("surface: %p, buffer show.\n", surface);
	surface->task_buffer[TASK_SHOW].show = TRUE;
	surface->task_buffer[TASK_SHOW].is_valid = TRUE;
}

void Surface_Hide( LCUI_Surface surface )
{
	if( surface->hwnd ) {
		ShowWindow( surface->hwnd, SW_HIDE );
		return;
	}
	surface->task_buffer[TASK_SHOW].show = FALSE;
	surface->task_buffer[TASK_SHOW].is_valid = TRUE;
}

void Surface_SetCaptionW( LCUI_Surface surface, const wchar_t *str )
{
	int len;
	wchar_t *caption;

	if( surface->hwnd ) {
		SetWindowText( surface->hwnd, str );
		return;
	}
	len = wcslen(str) + 1;
	caption = (wchar_t*)malloc(sizeof(wchar_t)*len);
	wcsncpy( caption, str, len );

	if( surface->task_buffer[TASK_SET_CAPTION].is_valid
	 && surface->task_buffer[TASK_SET_CAPTION].caption ) {
		free( surface->task_buffer[TASK_SET_CAPTION].caption );
	}
	surface->task_buffer[TASK_SET_CAPTION].caption = caption;
	surface->task_buffer[TASK_SET_CAPTION].is_valid = TRUE;
}

void Surface_SetOpacity( LCUI_Surface surface, float opacity )
{

}

static void
OnWidgetShow( LCUI_Widget widget, LCUI_WidgetEvent *unused )
{
	LCUI_Surface surface;
	surface = GetSurfaceByWidget( widget );
	if( !surface ) {
		return;
	}
	Surface_Show( surface );
}

static void
OnWidgetHide( LCUI_Widget widget, LCUI_WidgetEvent *unused )
{
	LCUI_Surface surface;
	surface = GetSurfaceByWidget( widget );
	if( !surface ) {
		return;
	}
	Surface_Hide( surface );
}

static void
OnWidgetDestroy( LCUI_Widget widget, LCUI_WidgetEvent *unused )
{
	LCUI_Surface surface;
	surface = GetSurfaceByWidget( widget );
	if( !surface ) {
		return;
	}
	Surface_Delete( surface );
}

static void
OnWidgetResize( LCUI_Widget widget, LCUI_WidgetEvent *e )
{
	LCUI_Surface surface;
	surface = GetSurfaceByWidget( widget );
	if( !surface ) {
		return;
	}
	//...
}

static void
OnWidgetOpacityChange( LCUI_Widget widget, LCUI_WidgetEvent *e )
{
	LCUI_Surface surface;
	surface = GetSurfaceByWidget( widget );
	if( !surface ) {
		return;
	}
	Surface_SetOpacity( surface, 1.0 );
}

/** 设置 Surface 的渲染模式 */
void Surface_SetRenderMode( LCUI_Surface surface, int mode )
{
	surface->mode = mode;
}

/**
 * 准备绘制 Surface 中的内容
 * @param[in] surface	目标 surface
 * @param[in] rect	需进行绘制的区域，若为NULL，则绘制整个 surface
 * @return		返回绘制上下文句柄
 */
LCUI_PaintContext Surface_BeginPaint( LCUI_Surface surface, LCUI_Rect *rect )
{
	LCUI_PaintContext paint;
	paint = (LCUI_PaintContext)malloc(sizeof(LCUI_PaintContextRec_));
	Graph_Quote( &paint->canvas, &surface->fb, rect );
	paint->rect = *rect;
	return paint;
}

/**
 * 结束对 Surface 的绘制操作
 * @param[in] surface	目标 surface
 * @param[in] paint_ctx	绘制上下文句柄
 */
void Surface_EndPaint( LCUI_Surface surface, LCUI_PaintContext paint_ctx )
{
	free( paint_ctx );
}

/** 将帧缓存中的数据呈现至Surface的窗口内 */
void Surface_Present( LCUI_Surface surface )
{
	HDC hdc_client;
	RECT client_rect;

	hdc_client = GetDC( surface->hwnd );
	Graph_WritePNG( "fb.png", &surface->fb );
	SetBitmapBits( surface->fb_bmp, surface->fb.mem_size, surface->fb.bytes );
	switch(surface->mode) {
	case RENDER_MODE_STRETCH_BLT:
		GetClientRect( surface->hwnd, &client_rect );
		StretchBlt( hdc_client, 0, 0,
			client_rect.right, client_rect.bottom,
			surface->fb_hdc, 0, 0,
			surface->w, surface->h, SRCCOPY );
		break;
       case RENDER_MODE_BIT_BLT:
       default:
		BitBlt( hdc_client, 0, 0, surface->w, surface->h,
			surface->fb_hdc, 0, 0, SRCCOPY );
		break;
	}
	ValidateRect( surface->hwnd, NULL );
}

/** 处理当前缓存的任务 */
static void Surface_ProcTaskBuffer( LCUI_Surface surface )
{
	LCUI_SurfaceTask *t;
	DEBUG_MSG("surface: %p\n", surface);
	t = &surface->task_buffer[TASK_MOVE];
	if( t->is_valid ) {
		Surface_ExecMove( surface, t->x, t->y );
		t->is_valid = FALSE;
	}

	t = &surface->task_buffer[TASK_RESIZE];
	if( t->is_valid ) {
		Surface_ExecResize( surface, t->width, t->height );
		t->is_valid = FALSE;
	}

	t = &surface->task_buffer[TASK_SET_CAPTION];
	if( t->is_valid ) {
		SetWindowText( surface->hwnd, t->caption );
		if( t->caption ) {
			free( t->caption );
			t->caption = NULL;
		}
	}
	t->is_valid = FALSE;

	t = &surface->task_buffer[TASK_SHOW];
	DEBUG_MSG("surface: %p, hwnd: %p, is_valid: %d, show: %d\n",
	 surface, surface->hwnd, t->is_valid, t->show);
	if( t->is_valid ) {
		if( t->show ) {
			ShowWindow( surface->hwnd, SW_SHOWNORMAL );
		} else {
			ShowWindow( surface->hwnd, SW_HIDE );
		}
	}
	t->is_valid = FALSE;
}

static void LCUISurface_Loop( void *unused )
{
	MSG msg;
	LCUI_Surface surface;
	DEBUG_MSG("start\n");
	/* 创建消息队列 */
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	win32.is_ready = TRUE;
	LCUICond_Broadcast( &win32.cond );
	while( GetMessage( &msg, NULL, 0, 0 ) ) {
		if( msg.message == MSG_SURFACE_CREATE ) {
			HDC hdc_client;
			surface = (LCUI_Surface)msg.lParam;
			/* 为 Surface 创建窗口 */
			surface->hwnd = CreateWindow(
				TEXT("LCUI"), TEXT("LCUI Surface"),
				WIN32_WINDOW_STYLE,
				CW_USEDEFAULT, CW_USEDEFAULT,
				0, 0, NULL, NULL,
				win32.main_instance, NULL
			);
			hdc_client = GetDC( surface->hwnd );
			surface->fb_hdc = CreateCompatibleDC( hdc_client );
			DEBUG_MSG("surface: %p, surface->hwnd: %p\n", surface, surface->hwnd);
			Surface_ProcTaskBuffer( surface );
			continue;
		}
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
	DEBUG_MSG("quit\n");
	LCUIThread_Exit(NULL);
}

int LCUISurface_Init(void)
{
	WNDCLASS wndclass;
	TCHAR szAppName[] = TEXT ("LCUI");

	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = WndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = win32.main_instance;
	/* 载入动态库里的图标 */
	wndclass.hIcon         = LoadIcon( win32.dll_instance, MAKEINTRESOURCE(IDI_MAIN_ICON) );
	wndclass.hCursor       = LoadCursor( NULL, IDC_ARROW );
	wndclass.hbrBackground = (HBRUSH)GetStockObject( WHITE_BRUSH );
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = szAppName;

	if( !RegisterClass(&wndclass) ) {
		wchar_t str[256];
		wsprintf(str, L"LCUISurface_Init(): error code: %d\n", GetLastError());
		MessageBox( NULL, str, szAppName, MB_ICONERROR );
		return -1;
	}
	win32.is_ready = FALSE;
	LCUICond_Init( &win32.cond );
	LCUIThread_Create( &win32.loop_thread, LCUISurface_Loop, NULL );
	return 0;
}

#endif
