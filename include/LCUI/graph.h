﻿/* ***************************************************************************
 * LCUI_Graph.h -- The base graphics handling module for LCUI
 *
 * Copyright (C) 2012-2015 by
 * Liu Chao
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
 * ****************************************************************************/

/* ****************************************************************************
 * LCUI_Graph.h -- LCUI的基本图形处理模块
 *
 * 版权所有 (C) 2012-2015 归属于
 * 刘超
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
 * 您应已收到附随于本文件的GPLv2许可协议的副本，它通常在LICENSE.TXT文件中，如果s
 * 没有，请查看：<http://www.gnu.org/licenses/>.
 * ***************************************************************************/
#ifndef __LCUI_GRAPH_H__
#define __LCUI_GRAPH_H__

LCUI_BEGIN_HEADER

#define COLOR_TURQUOISE		RGB(26, 188, 156)
#define COLOR_EMERALD		RGB(46, 204, 113)
#define COLOR_PETER_RIVER	RGB(52, 152, 219)
#define COLOR_AMETHYST		RGB(155, 89, 182)
#define COLOR_WET_ASPHALT	RGB(52, 73, 94)
#define COLOR_GREEN_SEA		RGB(22, 160, 133)
#define COLOR_NEPHRITIS		RGB(39, 174, 96)
#define COLOR_BELIZE_HOLE	RGB(41, 128, 185)
#define COLOR_WISTERIA		RGB(142, 68, 173)
#define COLOR_MIDNIGHT_BLUE	RGB(44, 62, 80)
#define COLOR_SUN_FLOWER	RGB(241, 196, 15)
#define COLOR_CARROT		RGB(230, 126, 34)
#define COLOR_ALIZARIN		RGB(231, 76, 60)
#define COLOR_CLOUNDS		RGB(236, 240, 241)
#define COLOR_CONCRETE		RGB(149, 165, 166)
#define COLOR_ORANGE		RGB(243, 156, 18)
#define COLOR_PUMPKIN		RGB(211, 84, 0)
#define COLOR_POMEGRANATE	RGB(192, 57, 43)
#define COLOR_SILVER		RGB(189, 195, 199)
#define COLOR_ASBESTOS		RGB(127, 140, 141)

/** 色彩模式 */
enum GraphColorType {
	COLOR_TYPE_INDEX8,	/**< 8位索引 */
	COLOR_TYPE_GRAY8,	/**< 8位灰度 */
	COLOR_TYPE_RGB323,	/**< RGB323 */
	COLOR_TYPE_ARGB2222,	/**< ARGB2222 */
	COLOR_TYPE_RGB555,	/**< RGB555 */
	COLOR_TYPE_RGB565,	/**< RGB565 */
	COLOR_TYPE_RGB888,	/**< RGB888 */
	COLOR_TYPE_ARGB8888	/**< RGB8888 */
};

#define COLOR_TYPE_RGB COLOR_TYPE_RGB888
#define COLOR_TYPE_ARGB COLOR_TYPE_ARGB8888

/* 将两个像素点的颜色值进行alpha混合 */
#define _ALPHA_BLEND(__back__ , __fore__, __alpha__)	\
    ((((__fore__-__back__)*(__alpha__))>>8)+__back__)

#define ALPHA_BLEND(__back__ , __fore__, __alpha__)		\
{								\
    __back__ =_ALPHA_BLEND(__back__,__fore__,__alpha__);	\
}

/* 获取像素的RGB值 */
#define RGB_FROM_RGB565(pixel, r, g, b)	\
{\
	r = (((pixel&0xF800)>>11)<<3);	\
	g = (((pixel&0x07E0)>>5)<<2);	\
	b = ((pixel&0x001F)<<3);	\
}

#define RGB_FROM_RGB555(pixel, r, g, b)	\
{\
	r = (((pixel&0x7C00)>>10)<<3);	\
	g = (((pixel&0x03E0)>>5)<<3);	\
	b = ((pixel&0x001F)<<3);	\
}

#define RGB_FROM_RGB888(pixel, r, g, b)	\
{\
	r = ((pixel&0xFF0000)>>16);	\
	g = ((pixel&0xFF00)>>8);	\
	b = (pixel&0xFF);		\
}

/* 混合像素的RGB值 */
#define RGB565_FROM_RGB(pixel, r, g, b)			\
{							\
    pixel = ((r>>3)<<11)|((g>>2)<<5)|(b>>3);		\
}

#define RGB555_FROM_RGB(pixel, r, g, b)			\
{							\
	pixel = ((r>>3)<<10)|((g>>3)<<5)|(b>>3);	\
}

#define RGB888_FROM_RGB(pixel, r, g, b)			\
{							\
	pixel = (r<<16)|(g<<8)|b;			\
}

/* 解除RGB宏 */
#ifdef RGB
#undef RGB
#endif

#define Graph_GetQuote(g) (g)->quote.is_valid ? (g)->quote.source:(g)

static inline LCUI_Color RGB( uchar_t r, uchar_t g, uchar_t b)
{
	LCUI_Color color;
	color.red = r;
	color.green = g;
	color.blue = b;
	color.alpha = 255;
	return color;
}

static inline LCUI_Color ARGB( uchar_t a, uchar_t r, uchar_t g, uchar_t b )
{
	LCUI_Color color;
	color.alpha = a;
	color.red = r;
	color.green = g;
	color.blue = b;
	return color;
}

static inline void Graph_SetPixel( LCUI_Graph *graph, int x, int y, LCUI_Color color )
{
	if( graph->color_type == COLOR_TYPE_ARGB ) {
		graph->argb[graph->w*y+x] = color;
	} else {
		/* 右移8位是为了去除ARGB中的alpha值 */
		graph->bytes[(graph->w*y+x)*3] = color.value>>8;
	}
}

static inline void Graph_SetPixelAlpha( LCUI_Graph *graph, int x, int y, uchar_t alpha )
{
	graph->argb[graph->w*y+x].alpha = alpha;
}

LCUI_API void Graph_PrintInfo( LCUI_Graph *graph );

LCUI_API void Graph_Init( LCUI_Graph *graph );

/** 改变色彩类型 */
LCUI_API int Graph_ChangeColorType( LCUI_Graph *graph, int color_type );

LCUI_API int Graph_Create( LCUI_Graph *graph, int w, int h );

LCUI_API void Graph_Copy( LCUI_Graph *des, const LCUI_Graph *src );

LCUI_API void Graph_Free( LCUI_Graph *graph );

/**
* 为图像创建一个引用
* @param self	用于存放图像引用的缓存区
* @param source	引用的源图像
* &param rect	引用的区域，若为NULL，则引用整个图像
*/
LCUI_API int Graph_Quote( LCUI_Graph *self, LCUI_Graph *source, const LCUI_Rect *rect );

/** 判断图像是否有Alpha透明通道 */
static inline LCUI_BOOL Graph_HaveAlpha( const LCUI_Graph *graph )
{
	return graph->quote.is_valid ? (
		graph->quote.source->color_type == COLOR_TYPE_ARGB
	) : graph->color_type == COLOR_TYPE_ARGB;
}

/** 判断图像是否有效 */
static inline LCUI_BOOL Graph_IsValid( const LCUI_Graph *graph )
{
	return graph->quote.is_valid ? (graph->quote.source
	 && graph->quote.source->w > 0 && graph->quote.source->h > 0
	) : (graph && graph->bytes && graph->h > 0 && graph->w > 0);
}

static inline void Graph_GetSize( const LCUI_Graph *graph, LCUI_Size *size )
{
	size->w = graph->w;
	size->h = graph->h;
}

LCUI_API void Graph_GetValidRect( const LCUI_Graph *graph, LCUI_Rect *rect );

LCUI_API int Graph_SetAlphaBits( LCUI_Graph *graph, uchar_t *a, size_t size );

LCUI_API int Graph_SetRedBits( LCUI_Graph *graph, uchar_t *r, size_t size );

LCUI_API int Graph_SetGreenBits( LCUI_Graph *graph, uchar_t *g, size_t size );

LCUI_API int Graph_SetBlueBits( LCUI_Graph *graph, uchar_t *b, size_t size );

LCUI_API int Graph_Zoom( const LCUI_Graph *graph, LCUI_Graph *buff,
			 LCUI_BOOL keep_scale, LCUI_Size size );

LCUI_API int Graph_Cut( const LCUI_Graph *graph, LCUI_Rect rect,
		        LCUI_Graph *buff );

LCUI_API int Graph_HorizFlip( const LCUI_Graph *graph, LCUI_Graph *buff );

LCUI_API int Graph_VertiFlip( const LCUI_Graph *graph, LCUI_Graph *buff );

LCUI_API int Graph_FillRect( LCUI_Graph *graph, LCUI_Color color,
				LCUI_Rect rect );

LCUI_API int Graph_FillColor( LCUI_Graph *graph, LCUI_Color color );

LCUI_API int Graph_FillAlpha( LCUI_Graph *graph, uchar_t alpha );

LCUI_API int Graph_Tile( LCUI_Graph *buff,  const LCUI_Graph *graph,
			 LCUI_BOOL replace );

LCUI_API int Graph_Mix(	LCUI_Graph *bg, const LCUI_Graph *fg, LCUI_Pos pos );

LCUI_API int Graph_Replace( LCUI_Graph *bg,  const LCUI_Graph *fg,
			    LCUI_Pos pos );

LCUI_API int Graph_PutImage( LCUI_Graph *graph, LCUI_Graph *image, int align, LCUI_BOOL replace );


/** 填充图像
 * @param graph		目标图像
 * @param backimg	要填充的背景图
 * @param layout	背景图的布局
 * @param area		需要绘制的区域
 */
LCUI_API int Graph_FillImageEx( LCUI_Graph *graph, const LCUI_Graph *backimg,
				int layout, LCUI_Rect area );

/** 填充图像和背景色
 * @param graph		目标图像
 * @param backimg	背景图
 * @param layout	背景图的布局
 * @param color		背景色
 * @param area		需要绘制的区域
 */
LCUI_API int Graph_FillImageWithColorEx( LCUI_Graph *graph,
					const LCUI_Graph *backimg, int layout,
					LCUI_Color color, LCUI_Rect area );

LCUI_API int Graph_FillImage( LCUI_Graph *graph, const LCUI_Graph *backimg,
				int layout );

LCUI_API int Graph_FillImageWithColor( LCUI_Graph *graph,
		const LCUI_Graph *backimg, int layout, LCUI_Color color );

LCUI_END_HEADER

#include <LCUI/draw.h>
#include <LCUI/graphlayer.h>

#endif /* __LCUI_GRAPH_H__ */
