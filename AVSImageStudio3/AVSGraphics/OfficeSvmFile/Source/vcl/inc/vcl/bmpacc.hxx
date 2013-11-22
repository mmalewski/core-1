/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bmpacc.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SV_BMPACC_HXX
#define _SV_BMPACC_HXX

#ifdef AVS
#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/salbtype.hxx>
#include <vcl/bitmap.hxx>
#endif

#include "vcl/bitmap.hxx"
#include "vcl/salbtype.hxx"

namespace SVMCore{

//#if 0 // _SOLAR__PRIVATE

// --------------------
// - Access defines -
// --------------------

#define DECL_FORMAT_GETPIXEL( Format ) \
static BitmapColor GetPixelFor##Format( ConstScanline pScanline, long nX, const ColorMask& rMask );

#define DECL_FORMAT_SETPIXEL( Format ) \
static void SetPixelFor##Format( Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask );

#define DECL_FORMAT( Format )   \
DECL_FORMAT_GETPIXEL( Format )  \
DECL_FORMAT_SETPIXEL( Format )

#define IMPL_FORMAT_GETPIXEL( Format ) \
BitmapColor BitmapReadAccess::GetPixelFor##Format( ConstScanline pScanline, long nX, const ColorMask& rMask )

#define IMPL_FORMAT_GETPIXEL_NOMASK( Format ) \
BitmapColor BitmapReadAccess::GetPixelFor##Format( ConstScanline pScanline, long nX, const ColorMask& )

#define IMPL_FORMAT_SETPIXEL( Format ) \
void BitmapReadAccess::SetPixelFor##Format( Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask )

#define IMPL_FORMAT_SETPIXEL_NOMASK( Format ) \
void BitmapReadAccess::SetPixelFor##Format( Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& )

#define CASE_FORMAT( Format )           \
case( BMP_FORMAT##Format ):            \
{                                       \
    mFncGetPixel = GetPixelFor##Format;\
    mFncSetPixel = SetPixelFor##Format;\
}                                       \
break;

//#endif // __PRIVATE

// --------------------
// - Access functions -
// --------------------

typedef BitmapColor (*FncGetPixel)( ConstScanline pScanline, long nX, const ColorMask& rMask );
typedef void (*FncSetPixel)( Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask );

// --------------------
// - BitmapReadAccess -
// --------------------

class  BitmapReadAccess
{
	friend class BitmapWriteAccess;

private:

    							BitmapReadAccess() {}
                                BitmapReadAccess( const BitmapReadAccess& ) {}
    BitmapReadAccess&    		operator=( const BitmapReadAccess& ) { return *this; }

protected:
    Bitmap                      maBitmap;
    BitmapBuffer*               mpBuffer;
    Scanline*                   mpScanBuf;
    ColorMask                   maColorMask;
    FncGetPixel                 mFncGetPixel;
    FncSetPixel                 mFncSetPixel;
    BOOL                        mbModify;

//#if 0 // _SOLAR__PRIVATE

  void            ImplCreate( Bitmap& rBitmap );
  void            ImplDestroy();
  BOOL            ImplSetAccessPointers( ULONG nFormat );
#ifdef AVS

public:

  void            ImplZeroInitUnusedBits();
  BitmapBuffer*   ImplGetBitmapBuffer() const { return mpBuffer; }
#endif
                                DECL_FORMAT( _1BIT_MSB_PAL )
                                DECL_FORMAT( _1BIT_LSB_PAL )
                                DECL_FORMAT( _4BIT_MSN_PAL )
                                DECL_FORMAT( _4BIT_LSN_PAL )
                                DECL_FORMAT( _8BIT_PAL )
                                DECL_FORMAT( _8BIT_TC_MASK )
                                DECL_FORMAT( _16BIT_TC_MSB_MASK )
                                DECL_FORMAT( _16BIT_TC_LSB_MASK )
                                DECL_FORMAT( _24BIT_TC_BGR )
                                DECL_FORMAT( _24BIT_TC_RGB )
                                DECL_FORMAT( _24BIT_TC_MASK )
                                DECL_FORMAT( _32BIT_TC_ABGR )
                                DECL_FORMAT( _32BIT_TC_ARGB )
                                DECL_FORMAT( _32BIT_TC_BGRA )
                                DECL_FORMAT( _32BIT_TC_RGBA )
                                DECL_FORMAT( _32BIT_TC_MASK )
//#endif // __PRIVATE
#ifdef AVS
protected:
#endif
                                BitmapReadAccess( Bitmap& rBitmap, BOOL bModify );
#ifdef AVS

    void                        Flush();
    void                        ReAccess( BOOL bModify );
#endif
public:
                                BitmapReadAccess( Bitmap& rBitmap );
    virtual                     ~BitmapReadAccess();

public:
    inline BOOL                 operator!() const;

    inline long                 Width() const;
    inline long                 Height() const;
#ifdef AVS
    inline Point                TopLeft() const;
    inline Point                BottomRight() const;
#endif
    inline BOOL                 IsTopDown() const;
    inline BOOL                 IsBottomUp() const;

    inline ULONG                GetScanlineFormat() const;
    inline ULONG                GetScanlineSize() const;

    inline USHORT               GetBitCount() const;

    inline BitmapColor          GetBestMatchingColor( const BitmapColor& rBitmapColor );

    inline Scanline             GetBuffer() const;
    inline Scanline             GetScanline( long nY ) const;

    inline BOOL                 HasPalette() const;
    inline const BitmapPalette& GetPalette() const;

    inline USHORT               GetPaletteEntryCount() const;

    inline const BitmapColor&   GetPaletteColor( USHORT nColor ) const;

    inline const BitmapColor&   GetBestPaletteColor( const BitmapColor& rBitmapColor ) const;
    USHORT                      GetBestPaletteIndex( const BitmapColor& rBitmapColor ) const;
#ifdef AVS
    inline BOOL                 HasColorMask() const;
    inline ColorMask&           GetColorMask() const;

    inline BitmapColor          GetPixelFromData( const BYTE* pData, long nX ) const;
    inline void                 SetPixelOnData( BYTE* pData, long nX, const BitmapColor& rBitmapColor );
#endif
    inline BitmapColor          GetPixel( long nY, long nX ) const;

	inline BitmapColor			GetColor( long nY, long nX ) const;
	inline BYTE					GetLuminance( long nY, long nX ) const;

};

// ---------------------
// - BitmapWriteAccess -
// ---------------------

class  BitmapWriteAccess : public BitmapReadAccess
{
public:

								BitmapWriteAccess( Bitmap& rBitmap );
    virtual                     ~BitmapWriteAccess();

    void                        CopyScanline( long nY, const BitmapReadAccess& rReadAcc );
#ifdef AVS
    void                        CopyScanline( long nY, ConstScanline aSrcScanline, 
											  ULONG nSrcScanlineFormat, ULONG nSrcScanlineSize );
#endif
    void                        CopyBuffer( const BitmapReadAccess& rReadAcc );

    inline void                 SetPalette( const BitmapPalette& rPalette );

    inline void                 SetPaletteEntryCount( USHORT nCount );
    inline void                 SetPaletteColor( USHORT nColor, const BitmapColor& rBitmapColor );
    inline void                 SetPixel( long nY, long nX, const BitmapColor& rBitmapColor );

    void                 		SetLineColor();
    void                 		SetLineColor( const Color& rColor );
    Color		   				GetLineColor() const;

    void                 		SetFillColor();
    void                 		SetFillColor( const Color& rColor );
#ifdef AVS
    Color				   		GetFillColor() const;

	void						Erase( const Color& rColor );

    void                        DrawLine( const Point& rStart, const Point& rEnd );
#endif
    void                        FillRect( const Rectangle& rRect );
#ifdef AVS
	void                        DrawRect( const Rectangle& rRect );

	void                        FillPolygon( const Polygon& rPoly );
	void                        DrawPolygon( const Polygon& rPoly );

	void                        FillPolyPolygon( const PolyPolygon& rPoly );
	void                        DrawPolyPolygon( const PolyPolygon& rPolyPoly );

private:
#endif
	BitmapColor*                mpLineColor;
    BitmapColor*           		mpFillColor;
#ifdef AVS
								BitmapWriteAccess() {}
								BitmapWriteAccess( const BitmapWriteAccess& ) : BitmapReadAccess() {}
#endif
	BitmapWriteAccess& 			operator=( const BitmapWriteAccess& ) { return *this; } 

};
#ifdef AVS
// -------------------
// - Accessor Helper -
// -------------------

/** This template handles BitmapAccess the RAII way. 

	Please don't use directly, but the ready-made typedefs for
	BitmapReadAccess and BitmapWriteAccess below.
 */
template < class Access > class ScopedBitmapAccess
{
public:
    ScopedBitmapAccess( Access* pAccess,
                        Bitmap& rBitmap ) :
        mpAccess( pAccess ),
        mrBitmap( rBitmap )
    {
    }

    ~ScopedBitmapAccess()
    {
        mrBitmap.ReleaseAccess( mpAccess );
    }

    Access* 		get() { return mpAccess; }
    const Access* 	get() const { return mpAccess; }

    Access* 		operator->() { return mpAccess; }
    const Access* 	operator->() const { return mpAccess; }

    Access& 		operator*() { return *mpAccess; }
    const Access& 	operator*() const { return *mpAccess; }

private:
    Access*		mpAccess;
    Bitmap&		mrBitmap;
};

/** This wrapper handles BitmapReadAccess the RAII way.

	Use as follows: 
    Bitmap aBitmap
	ScopedBitmapReadAccess pReadAccess( aBitmap.AcquireReadAccess(), aBitmap );
    pReadAccess->SetPixel()...

    @attention for practical reasons, ScopedBitmapReadAccess stores a
    reference to the provided bitmap, thus, make sure that the bitmap
    specified at construction time lives at least as long as the
    ScopedBitmapReadAccess.
*/
typedef ScopedBitmapAccess< BitmapReadAccess > ScopedBitmapReadAccess;

/** This wrapper handles BitmapWriteAccess the RAII way.

	Use as follows: 
    Bitmap aBitmap
	ScopedBitmapWriteAccess pWriteAccess( aBitmap.AcquireWriteAccess(), aBitmap );
    pWriteAccess->SetPixel()...

    @attention for practical reasons, ScopedBitmapWriteAccess stores a
    reference to the provided bitmap, thus, make sure that the bitmap
    specified at construction time lives at least as long as the
    ScopedBitmapWriteAccess.
*/
typedef ScopedBitmapAccess< BitmapWriteAccess > ScopedBitmapWriteAccess;
#endif
// -----------
// - Inlines -
// -----------

inline BOOL BitmapReadAccess::operator!() const
{
    return( mpBuffer == NULL );
}

// ------------------------------------------------------------------

/**/inline long BitmapReadAccess::Width() const
{
    return( mpBuffer ? mpBuffer->mnWidth : 0L );
}

// ------------------------------------------------------------------

inline long BitmapReadAccess::Height() const
{
    return( mpBuffer ? mpBuffer->mnHeight : 0L );
}
#ifdef AVS
// ------------------------------------------------------------------

inline Point BitmapReadAccess::TopLeft() const
{
    return Point();
}

// ------------------------------------------------------------------

inline Point BitmapReadAccess::BottomRight() const
{
    return Point( Width() - 1L, Height() - 1L );
}
#endif
// ------------------------------------------------------------------

/**/inline BOOL BitmapReadAccess::IsTopDown() const
{
    ////////DBG_ASSERT( mpBuffer, "Access is not valid!" );
    return( mpBuffer ? static_cast<BOOL>( BMP_SCANLINE_ADJUSTMENT( mpBuffer->mnFormat ) == BMP_FORMAT_TOP_DOWN ) : FALSE );
}

// ------------------------------------------------------------------

/**/inline BOOL BitmapReadAccess::IsBottomUp() const
{
    return !IsTopDown();
}

// ------------------------------------------------------------------

/**/inline ULONG BitmapReadAccess::GetScanlineFormat() const
{
    ////////DBG_ASSERT( mpBuffer, "Access is not valid!" );
    return( mpBuffer ? BMP_SCANLINE_FORMAT( mpBuffer->mnFormat ) : 0UL );
}

// ------------------------------------------------------------------

/**/inline ULONG BitmapReadAccess::GetScanlineSize() const
{
    ////////DBG_ASSERT( mpBuffer, "Access is not valid!" );
    return( mpBuffer ? mpBuffer->mnScanlineSize : 0UL );
}

// ------------------------------------------------------------------

inline USHORT  BitmapReadAccess::GetBitCount() const
{
    ////////DBG_ASSERT( mpBuffer, "Access is not valid!" );
    return( mpBuffer ? mpBuffer->mnBitCount : 0 );
}

// ------------------------------------------------------------------

inline BitmapColor BitmapReadAccess::GetBestMatchingColor( const BitmapColor& rBitmapColor )
{
    if( HasPalette() )
        return BitmapColor( (BYTE) GetBestPaletteIndex( rBitmapColor ) );
    else
        return rBitmapColor;
}

// ------------------------------------------------------------------

/**/inline Scanline BitmapReadAccess::GetBuffer() const
{
    ////////DBG_ASSERT( mpBuffer, "Access is not valid!" );
    return( mpBuffer ? mpBuffer->mpBits : NULL );
}

// ------------------------------------------------------------------

inline Scanline BitmapReadAccess::GetScanline( long nY ) const
{
    ////////DBG_ASSERT( mpBuffer, "Access is not valid!" );
    ////////DBG_ASSERT( nY < mpBuffer->mnHeight, "y-coordinate out of range!" );
    return( mpBuffer ? mpScanBuf[ nY ] : NULL );
}

// ------------------------------------------------------------------

inline BOOL BitmapReadAccess::HasPalette() const
{
    ////////DBG_ASSERT( mpBuffer, "Access is not valid!" );
    return( mpBuffer && !!mpBuffer->maPalette );
}

// ------------------------------------------------------------------

inline const BitmapPalette& BitmapReadAccess::GetPalette() const
{
    ////////DBG_ASSERT( mpBuffer, "Access is not valid!" );
    return mpBuffer->maPalette;
}

// ------------------------------------------------------------------

/**/inline USHORT BitmapReadAccess::GetPaletteEntryCount() const
{
    ////////DBG_ASSERT( HasPalette(), "Bitmap has no palette!" );
    return( HasPalette() ? mpBuffer->maPalette.GetEntryCount() : 0 );
}

// ------------------------------------------------------------------

inline const BitmapColor& BitmapReadAccess::GetPaletteColor( USHORT nColor ) const
{
    ////////DBG_ASSERT( mpBuffer, "Access is not valid!" );
    ////////DBG_ASSERT( HasPalette(), "Bitmap has no palette!" );
    return mpBuffer->maPalette[ nColor ];
}

// ------------------------------------------------------------------

inline const BitmapColor& BitmapReadAccess::GetBestPaletteColor( const BitmapColor& rBitmapColor ) const
{
    return GetPaletteColor( GetBestPaletteIndex( rBitmapColor ) );
}
#ifdef AVS
// ------------------------------------------------------------------

inline BOOL BitmapReadAccess::HasColorMask() const
{
    ////////DBG_ASSERT( mpBuffer, "Access is not valid!" );
    const ULONG nFormat = BMP_SCANLINE_FORMAT( mpBuffer->mnFormat );

    return( nFormat == BMP_FORMAT_8BIT_TC_MASK  ||
            nFormat == BMP_FORMAT_16BIT_TC_MSB_MASK ||
            nFormat == BMP_FORMAT_16BIT_TC_LSB_MASK ||
            nFormat == BMP_FORMAT_24BIT_TC_MASK ||
            nFormat == BMP_FORMAT_32BIT_TC_MASK );
}

// ------------------------------------------------------------------

inline ColorMask& BitmapReadAccess::GetColorMask() const
{
    ////////DBG_ASSERT( mpBuffer, "Access is not valid!" );
    return mpBuffer->maColorMask;
}
#endif
// ------------------------------------------------------------------

inline BitmapColor BitmapReadAccess::GetPixel( long nY, long nX ) const
{
    ////////DBG_ASSERT( mpBuffer, "Access is not valid!" );
    ////////DBG_ASSERT( nX < mpBuffer->mnWidth, "x-coordinate out of range!" );
    ////////DBG_ASSERT( nY < mpBuffer->mnHeight, "y-coordinate out of range!" );
    return mFncGetPixel( mpScanBuf[ nY ], nX, maColorMask );
}
#ifdef AVS
// ------------------------------------------------------------------

inline BitmapColor BitmapReadAccess::GetPixelFromData( const BYTE* pData, long nX ) const
{
    ////////DBG_ASSERT( pData, "Access is not valid!" );
    return mFncGetPixel( pData, nX, maColorMask );
}

// ------------------------------------------------------------------

inline void BitmapReadAccess::SetPixelOnData( BYTE* pData, long nX, const BitmapColor& rBitmapColor )
{
    ////////DBG_ASSERT( pData, "Access is not valid!" );
    mFncSetPixel( pData, nX, rBitmapColor, maColorMask );
}
#endif
// ------------------------------------------------------------------

inline BitmapColor BitmapReadAccess::GetColor( long nY, long nX ) const
{
	if( !!mpBuffer->maPalette )
	{
	    ////////DBG_ASSERT( mpBuffer, "Access is not valid!" );
		return mpBuffer->maPalette[ GetPixel( nY, nX ).GetIndex() ];
	}
	else
		return GetPixel( nY, nX );
}

// ------------------------------------------------------------------

inline BYTE BitmapReadAccess::GetLuminance( long nY, long nX ) const
{
	return GetColor( nY, nX ).GetLuminance();
}

// ------------------------------------------------------------------

inline void BitmapWriteAccess::SetPalette( const BitmapPalette& rPalette )
{
    ////////DBG_ASSERT( mpBuffer, "Access is not valid!" );
    mpBuffer->maPalette = rPalette;
}

// ------------------------------------------------------------------

/**/inline void BitmapWriteAccess::SetPaletteEntryCount( USHORT nCount )
{
    ////////DBG_ASSERT( mpBuffer, "Access is not valid!" );
    mpBuffer->maPalette.SetEntryCount( nCount );
}

// ------------------------------------------------------------------

/**/inline void BitmapWriteAccess::SetPaletteColor( USHORT nColor, const BitmapColor& rBitmapColor )
{
    ////////DBG_ASSERT( mpBuffer, "Access is not valid!" );
    ////////DBG_ASSERT( HasPalette(), "Bitmap has no palette!" );
    mpBuffer->maPalette[ nColor ] = rBitmapColor;
}

// ------------------------------------------------------------------

/**/inline void BitmapWriteAccess::SetPixel( long nY, long nX, const BitmapColor& rBitmapColor )
{
    ////////DBG_ASSERT( mpBuffer, "Access is not valid!" );
    ////////DBG_ASSERT( nX < mpBuffer->mnWidth, "x-coordinate out of range!" );
    ////////DBG_ASSERT( nY < mpBuffer->mnHeight, "y-coordinate out of range!" );
    mFncSetPixel( mpScanBuf[ nY ], nX, rBitmapColor, maColorMask );
}
}//SVMCore
#endif // _SV_BMPACC_HXX
