#pragma once

#include <windowsx.h>
#include <windows.h>
#include <winuser.h>

#include <Messages.h>
#include <XmlUtils.h>

#include "Tracking.h"

/*

#include <ImageColors.h>
#include <Utilities.h>
#include <ImageIO.h>
#include <ImagePaintState.h>
#include <ImageTransformsCore.h>

CAtlArray<Gdiplus::Bitmap*>     ImageStudio::Paint::Effects::OldPhoto::Masks;

*/

namespace Tracking
{
	struct StringContext
	{
		StringContext ( long BeginInd , long EndInd ) : m_nBeginInd ( BeginInd ), m_nEndInd ( EndInd )
		{

		}

		inline bool IsConsists ( long Index )
		{
			return ( Index >= m_nBeginInd && m_nEndInd >= Index );
		}

		inline bool IsEndString ( long Index )
		{
			return Index == m_nEndInd;
		}

		long	m_nBeginInd;
		long	m_nEndInd;
	};

	//------------------------------------------------------------------------------------------------------------------
	// TextRotatable Xml 
	//------------------------------------------------------------------------------------------------------------------
	class CTrackTextRotatable : public CTrackObjectText
	{
	public:
		CTrackTextRotatable ()
		{
			m_pFontFamily			=	NULL;
			m_bEditingModeEnabled	=	false;
			m_fLineSpacing			=	0.0;
			m_chDeadKey				=	L'\0';
		}

		virtual ~CTrackTextRotatable ()
		{
			Clear ( );
		}


		inline void Create ( double Left, double Top, double Right, double Bottom, double Angle, double StickAngle, WCHAR* string, WCHAR* FontName, int FontSize, int FontType, int FontColor, bool bBlockEditing = false, bool bBlockResize = false )
		{
			m_bNotAutoFitText				=	true;		
			m_bResizeShapeToFitText			=	false;

			//m_bNotAutoFitText				=	false;		
			//m_bResizeShapeToFitText		=	true;

			// Set Text Settings Old Style

			// Set Font Settings
			m_oFont.SetName ( CString ( W2BSTR ( FontName ) ) );
			m_oFont.SetFontSize ( FontSize );
			m_oFont.SetBold ( FontType & FONTSTYLE_BOLD );
			m_oFont.SetStrikeout ( FontType & FONTSTYLE_STRIKEOUT );
			m_oFont.SetUnderline ( FontType & FONTSTYLE_UNDERLINE );
			m_oFont.SetItalic ( FontType & FONTSTYLE_ITALIC );

			// Set Font Format
			m_oFormat.SetStringAlignmentHorizontal ( GetFontAlignedW ( FontType ) );
			m_oFormat.SetStringAlignmentVertical ( 0 );

			m_nStringAlignmentVertical	=	GetFontAlignedH ( FontType );

			// Set Brush
			m_oBrush.SetAlpha1 ( 255 );
			m_oBrush.SetColor1 ( FontColor );
			m_oBrush.SetAlpha2 ( 255 );
			m_oBrush.SetColor2 ( FontColor );

			m_String   		= CString( string );
			m_FontName 		= CString( FontName );
			m_CurrPos  		= m_String.GetLength();
			m_FontSize 		= FontSize;
			m_FontType		= FontType;
			m_FontColor		= FontColor;

			m_bEditingModeEnabled		= false;
			m_IsSelect		= false;
			m_IsTransfom	= true;
			m_SelectStart	= 0;
			m_SelectEnd		= 0;
			m_hDC			= NULL;
			m_dZoom			= 1.0;
			m_StickAngle	=	( StickAngle < 0 || StickAngle > 360 ) ? 0.0 : rad ( StickAngle );
			m_bBlockEditing	=	bBlockEditing;
			m_bBlockResize	=	bBlockResize;

			if ( Right < 0 && Bottom < 0 ) 
				m_AutoSize = true;
			else					
				m_AutoSize = false;

			m_ScaleText = false;

			m_Scale.x = 1.0;
			m_Scale.y = 1.0;

			// if create by mouse
			if( abs ( Right -  Left ) < 1 || abs (  Bottom -  Top ) < 1 || m_AutoSize )	
				m_IsFirstResize = true;
			else																		
				m_IsFirstResize = false;

			SetType ( Constants::sc_nTrackTypeTextRotatable );

			Clear();

			if( m_AutoSize )
			{
				Right	=	Left	+	FontSize;
				Bottom	=	Top		+	FontSize;
			}

			Add ( Left,  Top );																//	0
			Add ( Right, Top );																//	1
			Add ( Right, Bottom );															//	2
			Add ( Left,  Bottom );															//	3

			Add ( 0.5 * ( Left + Right ), Top );											//	4
			Add ( Right, 0.5 * ( Top + Bottom ) );											//	5
			Add ( 0.5 * ( Left + Right ), Bottom );											//	6
			Add ( Left, 0.5 * ( Top + Bottom ) );											//	7

			Add( 0.5 * ( Left + Right ), Top - 30 );										//	rotate selector		//	8
			Add( 0.5 * ( Left + Right ), 0.5 * ( Top + Bottom ) );							//	center				//	9
			Add( 0.5 * ( Left + Right ), 0.5 * ( Top + Bottom ) );							//	catch				//	10

			SetCurrentPointIndex ( -1 );

			RotateToAlpha ( rad ( Angle ) );

			m_bFirstPaint			=	false;
			m_bTextAllSelect		=	false;
			m_bScrollingText		=	false;

			m_nScrollLine			=	0;
			m_nCanScrollLines		=	0;
			m_nVisibleLines			=	0;

			m_bCaretAtHome			=	false;
			m_nCurrentString		=	-1;
			m_bSetCaretAtHome		=	false;
		}

	private:

		inline void		Clear						( )
		{
			m_oTextureManager.Clear ();

			RELEASEOBJECT ( m_pFontFamily );
		}

	protected:

		inline int		GetPointUnderCursor			( int nX, int nY )
		{
			int nTrackIndex = FindByInteger( nX, nY, c_nTrackPointSizeTouch, FALSE );

			if( nTrackIndex < 0 )
			{
				Structures::RECTI rectD;

				double dX = MapToDataX( nX );
				double dY = MapToDataY( nY );

				double dAngle = Geometry::GetAngle( GetAt(9).dX, GetAt(9).dY, GetAt(8).dX, GetAt(8).dY );

				double tX, tY;
				double cosA = cos( -(dAngle + M_PI_2) );
				double sinA = sin( -(dAngle + M_PI_2) );

				tX = dX - GetAt(9).dX;
				tY = dY - GetAt(9).dY;				
				dX = GetAt(9).dX + tX * cosA - tY * sinA;
				dY = GetAt(9).dY + tX * sinA + tY * cosA;

				Structures::POINTI pointTrack ( (int)dX, (int)dY );

				rectD.left   = (int)GetAt(0).dX;
				rectD.top    = (int)GetAt(0).dY;
				rectD.right  = (int)GetAt(2).dX;
				rectD.bottom = (int)GetAt(2).dY;

				tX = GetAt(0).dX - GetAt(9).dX;
				tY = GetAt(0).dY - GetAt(9).dY;				
				rectD.left  = (int) ( GetAt(9).dX + tX * cosA - tY * sinA );
				rectD.top   = (int) ( GetAt(9).dY + tX * sinA + tY * cosA );	

				tX = GetAt(2).dX - GetAt(9).dX;
				tY = GetAt(2).dY - GetAt(9).dY;
				rectD.right  = (int) ( GetAt(9).dX + tX * cosA - tY * sinA );
				rectD.bottom = (int) ( GetAt(9).dY + tX * sinA + tY * cosA );

				if( rectD.right < rectD.left ) 
				{ 
					double tmpRight		= rectD.right; 
					rectD.right			= rectD.left;
					rectD.left			= (int)tmpRight;
				}
				if( rectD.top > rectD.bottom ) 
				{ 
					double tmpBottom	= rectD.bottom; 
					rectD.bottom		= rectD.top;
					rectD.top			= (int)tmpBottom;
				}

				if( rectD.IsPointInside( pointTrack ) )
					nTrackIndex = GetCount() - 1;
				else
					return -1;
			}

			return nTrackIndex;
		}

		inline void		RotateToAlpha				( double alpha )
		{
			for( int index = 0; index < 9; ++index )
				Geometry::RotatePoint( GetAt(index).dX, GetAt(index).dY, GetAt(9).dX, GetAt(9).dY, alpha );
		}
		inline int		GetStringByPos				( int &caretPos )
		{
			int arrSize = m_arrStrings.GetSize();
			for ( int index = 0; index < arrSize; ++index )
			{
				if ( caretPos < m_arrStrings[index].GetLength() || ( index == arrSize - 1 ) )
					return index;

				caretPos -= m_arrStrings[index].GetLength();
			}

			return -1;
		}
		inline int		PosOnMouse					( int dX, int dY )
		{
			if ( NULL == m_hDC )
				return -1;
			
			int CountLines = m_arrStrings.GetSize();
			
			if ( 0 == CountLines )
				return -1;

			Gdiplus::Graphics graphics( m_hDC );

			Gdiplus::RectF			orig, currStrRect;

			int FontStyle							=	Gdiplus::FontStyleRegular;
			Gdiplus::Font*			pFont			=	NULL;
			Gdiplus::FontFamily*	pFontFamily		=	NULL;
			Gdiplus::StringFormat*	pStringFormat	=	NULL;

			if ( false == GetFontObjects ( &pFont, &pFontFamily, &pStringFormat, FontStyle ) )
				return -1;

			Structures::RECTD rectD( GetAt(0).dX, GetAt(0).dY, GetAt(2).dX, GetAt(2).dY );
			rectD.Normalize();

			if( dY > rectD.bottom ) return -1;

			double dWidth	= rectD.GetWidth();
			double dWidth_2 = dWidth / 2.0;

			Structures::POINTI pointTrack( dX, dY );
		
			int SymbolsOffset	=	0;

			float FontHeight	=	(float)Rendering::Utils::GetFontHeight ( pFont );

			double displaceByY	=	CountLines * FontHeight;

			for ( int i = 0; i < m_nScrollLine; ++i )
			{
				SymbolsOffset += m_arrStrings[i].GetLength();
			}

			for ( int i = m_nScrollLine; i < CountLines; ++i )
			{
				graphics.MeasureString ( m_arrStrings [ i ], -1, pFont, orig, pStringFormat, &currStrRect );	
				
				currStrRect.X		= (Gdiplus::REAL)rectD.left;
				currStrRect.Width	= (Gdiplus::REAL)dWidth;

				if ( m_ScaleText )
				{
					currStrRect.Height = FontHeight * (Gdiplus::REAL)m_Scale.y * (Gdiplus::REAL)CountLines;
					currStrRect.Offset ( 0.0f, (Gdiplus::REAL) i * (Gdiplus::REAL)currStrRect.Height + (Gdiplus::REAL)rectD.top );
				}
				else
				{
					switch ( m_nStringAlignmentVertical )
					{
					case byTop:
						{
							currStrRect.Offset	( 0.0f, (float) ( i - m_nScrollLine ) * FontHeight + (Gdiplus::REAL)rectD.top );
							break;
						}
					case byMiddle:
						{
							currStrRect.Offset	( 0.0f, (float) i * FontHeight + (Gdiplus::REAL)rectD.GetCenter().y - (Gdiplus::REAL)displaceByY / 2.0f );
							break;	
						}
					case byBottom:
						{
							currStrRect.Offset	( 0.0f, (float) i * FontHeight + (Gdiplus::REAL)rectD.bottom - (Gdiplus::REAL)displaceByY );
							break;	
						}
					default:
						{
							currStrRect.Offset	( 0.0f, (float) i * FontHeight + (Gdiplus::REAL)rectD.top );
						}
					}
				}

				if ( currStrRect.Contains ( (Gdiplus::REAL)dX, (Gdiplus::REAL)dY ) )
				{
					m_nCurrentString	=	i;

					//BSTR line = m_arrStrings[i].AllocSysString();
					graphics.MeasureString ( m_arrStrings[i], -1, pFont, orig, pStringFormat, &currStrRect );
				//	SysFreeString( line );
					currStrRect.Height = (Gdiplus::REAL)FontHeight;

					currStrRect.Width  *= (Gdiplus::REAL)m_Scale.x;
					currStrRect.Height *= (Gdiplus::REAL)m_Scale.y;

					if ( pStringFormat->GetAlignment() == StringAlignmentNear )
					{
						currStrRect.Offset( (Gdiplus::REAL)rectD.left, (Gdiplus::REAL)i * currStrRect.Height + (Gdiplus::REAL)rectD.top );
					}
					else if( pStringFormat->GetAlignment() == StringAlignmentCenter )
					{
						currStrRect.Offset( (Gdiplus::REAL)rectD.left + (Gdiplus::REAL)dWidth_2, (Gdiplus::REAL)i * currStrRect.Height + (Gdiplus::REAL)rectD.top );
					}
					else if( pStringFormat->GetAlignment() == StringAlignmentFar )
					{
						currStrRect.Offset( (Gdiplus::REAL)rectD.right, (Gdiplus::REAL)i * currStrRect.Height + (Gdiplus::REAL)rectD.top );
					}

					double partOfChar = pFont->GetSize() * m_Scale.x / 5.0;

					StringFormat localStringFormat;
					localStringFormat.SetAlignment(StringAlignmentNear);
					//BSTR lineStringAll = m_arrStrings[i].AllocSysString();
					graphics.MeasureString( m_arrStrings[i], -1, pFont, orig, &localStringFormat, &currStrRect );
					//SysFreeString( lineStringAll );

					CStringW lineString = m_arrStrings[i];
					lineString.Replace( L"\r\n", L"" );

					int SymbolsCount	=	m_arrStrings[i].GetLength();

					if ( pStringFormat->GetAlignment() == StringAlignmentNear )
					{
						for( int j = 0; j < SymbolsCount; ++j )
						{
							RectF currCharRect;
							BSTR line = m_arrStrings[i].AllocSysString();
							Painter::GetRectCurrentCharacter( &graphics, &localStringFormat, pFont, line, -1, j, currStrRect, currCharRect );
							SysFreeString( line );

							if ( ( currCharRect.X + currCharRect.Width * 0.5f ) * m_Scale.x >= dX - rectD.left )
							{
								if ( 0 == j && m_bSetCaretAtHome )
									m_bCaretAtHome = true;

								return SymbolsOffset + j;
							}
							
							if ( j == lineString.GetLength() - 1 )
							{
								return SymbolsOffset + j + 1;
							}

						}	
					}
					else if( pStringFormat->GetAlignment() == StringAlignmentCenter )
					{
						for ( int j = 0; j < SymbolsCount; ++j )
						{
							RectF currCharRect;
							BSTR line = m_arrStrings[i].AllocSysString();
							Painter::GetRectCurrentCharacter( &graphics, &localStringFormat, pFont, line, -1, j, currStrRect, currCharRect );
							SysFreeString( line );

							if ( ( currCharRect.X + currCharRect.Width * 0.5f ) * m_Scale.x >= ( dX - (rectD.left + rectD.GetWidth() / 2 ) ) + currStrRect.Width / 2 )
							{
								if ( 0 == j && m_bSetCaretAtHome )
									m_bCaretAtHome = true;

								return SymbolsOffset + j;
							}
							else if ( dX < currStrRect.X ) 
							{
								return SymbolsOffset;
							}
							
							if ( j == lineString.GetLength() - 1 )
								return SymbolsOffset + j + 1;
						}
					}
					else if ( pStringFormat->GetAlignment() == StringAlignmentFar )
					{
						for( int j = 0; j < SymbolsCount; ++j )
						{
							RectF currCharRect;
							BSTR line = m_arrStrings[i].AllocSysString();
							Painter::GetRectCurrentCharacter( &graphics, &localStringFormat, pFont, line, -1, j, currStrRect, currCharRect );
							SysFreeString( line );

							if( ( currCharRect.X + currCharRect.Width * 0.5f ) * m_Scale.x >= dX - rectD.right + currStrRect.Width )
							{
								if ( 0 == j && m_bSetCaretAtHome )
									m_bCaretAtHome = true;

								return SymbolsOffset + j;
							}

							if ( j == lineString.GetLength() - 1 )
							{
								return SymbolsOffset + j + 1;
							}

						}
					}

					return SymbolsOffset + m_arrStrings[i].GetLength();// -2 for skeep "\r\n";
				}

				SymbolsOffset += m_arrStrings[i].GetLength();
			}

			return -1;				
		}

		inline void		SplitString					( )
		{
			m_arrStrings.RemoveAll();
			m_bCaretAtHome	=	false;

			if( 0 == m_String.GetLength() || NULL == m_hDC ) 
				return;

			Graphics graphics ( m_hDC );

			// GraphicsContainer  graphicsContainer;
			// graphicsContainer = graphics.BeginContainer();

			double dAngle = Geometry::GetAngle( GetAt(9).dX, GetAt(9).dY, GetAt(8).dX, GetAt(8).dY ) + M_PI_2;
			Structures::RECTD rectD = GetUnrotatableRect ( dAngle );

			double dWidth  = rectD.GetWidth();

			RectF	orig, rectF, strBB;

			Gdiplus::Font*			pFont			=	NULL;
			Gdiplus::FontFamily*	pFontFamily		=	NULL;
			Gdiplus::StringFormat*	pStringFormat	=	NULL;
			int FontStyle							=	Gdiplus::FontStyleRegular;

			if ( false == GetFontObjects ( &pFont, &pFontFamily, &pStringFormat, FontStyle, false ) )
				return;

			double maxWidth		=	0;
			double maxHeight	=	0;

			Gdiplus::REAL nFontSize = pFont->GetSize();
			if ( dWidth + 0.5 < nFontSize && !m_ScaleText )
				return;

			float fFontHeight		=	pFontFamily->GetEmHeight ( FontStyle );
			float fFontSpacing		=	pFontFamily->GetLineSpacing ( FontStyle );

			m_fLineSpacing			=	fFontSpacing * nFontSize / fFontHeight;

			int strLen = m_String.GetLength();

			int		strLine		= 0;
			int		countChar	= 1;
			double  displaceX	= 0.0;
			double  displaceY	= 0.0;
			bool	isEnd		= false;

			//float BoundWidth		=	rectD.GetWidth();
			//long CountChars			=	m_String.GetLength ();
			//Gdiplus::RectF BoundingBox;

			//CStringW CurrWord;
			//CStringW CurrString;

			//long CurrInd			=	0;
			//long CurrWordStart		=	0;
			//long CurrWordLength		=	0;
			//long CurrStringStart	=	0;
			//long CurrStringLength	=	0;

			//CAtlArray <CStringW>	m_Words;

			//WCHAR* pBuffer = m_String.GetBuffer ();

			//for ( long i = 0; i < CountChars; ++i )
			//{
			//	WCHAR Char = *(pBuffer + i);

			//	if ( '\r' == Char )
			//	{
			//		CurrString += CurrWord;
			//		
			//		OutputDebugStringW ( CurrString + CStringW ( L"\n" ) );
			//		
			//		m_arrStrings.Add ( CurrString + CStringW ( L"\r\n" ) );

			//		m_Words.Add ( CurrWord );

			//		CurrInd++;
			//		
			//		CurrWordStart		=	0;
			//		CurrWordLength		=	0;
			//		CurrStringStart		=	0;
			//		CurrStringLength	=	0;

			//		CurrString = "";
			//		
			//		++i;

			//		continue;
			//	}
			//
			//	CurrStringLength++;
			//	
			//	graphics.MeasureString  ( (pBuffer + i), CurrStringLength, pFont, orig, pStringFormat, &BoundingBox );

			//	if ( BoundWidth <= BoundingBox.Width )
			//	{
			//		if ( ' ' == Char )
			//		{
			//			OutputDebugStringW ( CurrString + CStringW ( L"\n" ) );

			//			m_arrStrings.Add ( CurrString );

			//			CurrString = "";
			//			CurrStringLength = 0;
			//		}
			//		else
			//		{
			//			CurrString.Delete ( CurrString.GetLength () - 1 - CurrWord.GetLength () - 1, CurrWord.GetLength () - 1 );

			//			m_arrStrings.Add ( CurrString );

			//			CurrString = "";
			//			CurrStringLength = 0;
			//		}
			//	}

			//	if ( ' ' == Char )
			//	{
			//		CurrWord += Char;
			//		
			//		m_Words.Add ( CurrWord );
			//		m_Words.Add ( CStringW ( Char ) );
			//		
			//		CurrString += CurrWord;
			//		CurrWord			=	"";
			//		// CurrStringLength	=	0;
			//		
			//		//OutputDebugStringW ( CurrString );

			//		continue;
			//	}

			//	CurrWord += m_String [i];
			//}

			//m_Words.RemoveAll ();

			//return;

			BSTR fullString = m_String.AllocSysString();

			for ( int i = 0; i < strLen; ++i )
			{
				if ( *( fullString + strLine + countChar - 1 ) == '\r' )
				{
					CStringW tmpStr = CStringW ( fullString + strLine );
					i++;
					tmpStr.Delete( countChar + 1, strLen - i + 2);
					m_arrStrings.Add( tmpStr  /*+ L"\r\n"*/ ); /////////////
					strLine		= i + 1;
					countChar	= 1;
					isEnd = true;

					// graphics.EndContainer ( graphicsContainer );

					continue;
				}

				graphics.MeasureString  ( fullString + strLine, countChar, pFont, orig, pStringFormat, &strBB );

				displaceX	=	strBB.Width;
				maxWidth	=	max( maxWidth, displaceX );

				if ( displaceX > dWidth && !m_AutoSize && !m_ScaleText )
				{
					CStringW tmpStr	=	CStringW ( fullString + strLine );					
					tmpStr.Delete( countChar - 1, strLen - i );
					BSTR subString	=	tmpStr.AllocSysString();
					BSTR rSpace		=	wcsrchr( subString, ' ' );
					if ( NULL != rSpace )
					{						
						int strLenSpace = (int)wcslen( rSpace );
						i -= strLenSpace - 1;
						tmpStr.Delete ( countChar - strLenSpace, strLenSpace );
					}
					m_arrStrings.Add( tmpStr /*+ L"\r\n"*/ ); /////////////
					strLine		= i;
					countChar	= 1;
					isEnd = true;
					SysFreeString( subString );
				}	
				countChar++;
				isEnd = false;
			}

			if( !isEnd )
			{
				m_arrStrings.Add ( CStringW ( fullString + strLine ) + L"\r\n" );
			}

			m_String.FreeExtra();

			double fontHeight = Rendering::Utils::GetFontHeight ( pFont );

			if( m_ScaleText )
			{
				double dHeight = rectD.GetHeight();

				maxHeight = fontHeight * m_arrStrings.GetSize();

				if ( m_arrStrings.GetSize() > 0 && maxHeight > 0)
				{
					m_Scale.x = dWidth / max( 0.001, maxWidth );
					m_Scale.y = dHeight / maxHeight / m_arrStrings.GetSize();
				}
			}
			else if( m_AutoSize )
			{
				maxHeight = fontHeight * m_arrStrings.GetSize();

				double eX1, eX2, eY1, eY2;

				eX1 = 1.0; eY1 = 0.0;
				eX2 = 0.0; eY2 = 1.0;

				double mAngle = Geometry::GetAngle( GetAt(9).dX, GetAt(9).dY, GetAt(8).dX, GetAt(8).dY ) + M_PI_2;
				Geometry::RotatePoint( eX1, eY1, 0.0, 0.0, mAngle );
				Geometry::RotatePoint( eX2, eY2, 0.0, 0.0, mAngle );

				GetAt(2).dX = GetAt(0).dX + eX1 * maxWidth + eX2 * maxHeight;
				GetAt(2).dY = GetAt(0).dY + eY1 * maxWidth + eY2 * maxHeight;
				GetAt(1).dX = GetAt(0).dX + eX1 * maxWidth;
				GetAt(1).dY = GetAt(0).dY + eY1 * maxWidth;
				GetAt(3).dX = GetAt(0).dX + eX2 * maxHeight;
				GetAt(3).dY = GetAt(0).dY + eY2 * maxHeight;

				GetAt(4).Create( 0.5*( GetAt(0).dX + GetAt(1).dX ), 0.5*( GetAt(1).dY + GetAt(0).dY ) );
				GetAt(5).Create( 0.5*( GetAt(1).dX + GetAt(2).dX ), 0.5*( GetAt(1).dY + GetAt(2).dY ) );
				GetAt(6).Create( 0.5*( GetAt(3).dX + GetAt(2).dX ), 0.5*( GetAt(3).dY + GetAt(2).dY ) );
				GetAt(7).Create( 0.5*( GetAt(3).dX + GetAt(0).dX ), 0.5*( GetAt(3).dY + GetAt(0).dY ) );

				double vX = GetAt(4).dX - GetAt(6).dX;
				double vY = GetAt(4).dY - GetAt(6).dY;

				double length = Geometry::GetLength( GetAt(4).dX, GetAt(4).dY,GetAt(6).dX, GetAt(6).dY );
				if( length < 1 )
				{
					vX = -eX2; 
					vY = -eY2;
				}
				else
				{
					vX /= length;
					vY /= length;
				}

				GetAt(8).dX = GetAt(4).dX + vX * 30;
				GetAt(8).dY = GetAt(4).dY + vY * 30;
				GetAt(9).Create(0.5*(GetAt(0).dX + GetAt(2).dX), 0.5*( GetAt(2).dY + GetAt(0).dY ) );
			}

			SysFreeString ( fullString );

			m_aStringsInfo.RemoveAll ();

			long SizeSymbols	=	0;
			long BeginSymbols	=	0;

			for ( int i = 0; i < m_arrStrings.GetSize (); ++i )
			{
				int Length = m_arrStrings [i].GetLength (); 

				SizeSymbols	+=	Length; 

				if ( '\n' == m_arrStrings [i] [Length - 1 ] )
				{
					m_aStringsInfo.Add ( StringContext ( BeginSymbols, SizeSymbols - 2 ) );
					BeginSymbols = SizeSymbols;
				}
				else
				{
					m_aStringsInfo.Add ( StringContext ( BeginSymbols, SizeSymbols ) );
					BeginSymbols = SizeSymbols;
				}
			}

			// graphics.EndContainer ( graphicsContainer );
		}

		inline void		StickAngle					( double &dAngle, double angleStick )
		{
			int		countStick	 = 0;
			double  tmpBaseAngle = dAngle;
			double  rangeStick	 = rad(2);
			if( 0 == angleStick || rangeStick > angleStick ) angleStick = M_PI_2;
			if( dAngle < 0 ) angleStick = -angleStick;
			while( (dAngle < 0) ? (tmpBaseAngle < angleStick + rangeStick) : (tmpBaseAngle > angleStick - rangeStick) )
			{ 
				tmpBaseAngle -= angleStick;
				countStick++;
			}
			if( abs( tmpBaseAngle ) < rangeStick ) 
			{
				tmpBaseAngle = countStick * angleStick;
				dAngle = tmpBaseAngle;
			}
		}
		inline void		UpdateMinimizeRegion		( int CapturePoint )
		{
			double CharSize		=	(double)m_FontSize * (double)GetDeviceCaps ( GetWindowDC (NULL), LOGPIXELSY ) / 72.0;
			double CharSizeRotX	=	CharSize;
			double CharSizeRotY	=	CharSize;

			double RotateAngle = Geometry::GetAngle ( GetAt(9).dX, GetAt(9).dY, GetAt(8).dX, GetAt(8).dY ) + M_PI / 2.0;
			Geometry::RotatePoint ( CharSizeRotX, CharSizeRotY, 0.0, 0.0, RotateAngle );

			double BoundWidth		=	Geometry::GetLength ( GetAt(0).dX, GetAt(0).dY, GetAt(1).dX, GetAt(1).dY );
			double BoundHeight		=	Geometry::GetLength ( GetAt(0).dX, GetAt(0).dY, GetAt(3).dX, GetAt(3).dY );

			double BoundWidthRot	=	BoundWidth;
			double BoundHeightRot	=	BoundHeight;

			//-------------------------------------------------------------------------------------------

			if ( CapturePoint == 0 )
			{
				bool RegionMinimize	=	false;

				if ( BoundWidth <= CharSize )
				{
					double OffsetX	=	cos ( RotateAngle ) * CharSize;
					double OffsetY	=	sin ( RotateAngle ) * CharSize;

					GetAt(0).dX		=	GetAt(1).dX - OffsetX;
					GetAt(7).dX		=	GetAt(5).dX - OffsetX;
					GetAt(3).dX		=	GetAt(2).dX - OffsetX;

					GetAt(0).dY		=	GetAt(1).dY - OffsetY;
					GetAt(7).dY		=	GetAt(1).dY - OffsetY - ( GetAt(1).dY - GetAt(5).dY );
					GetAt(3).dY		=	GetAt(1).dY - OffsetY - ( GetAt(1).dY - GetAt(2).dY );

					RegionMinimize	=	true;
				}

				if ( BoundHeight <= CharSize )
				{
					double OffsetX	=	cos ( RotateAngle + M_PI / 2.0 ) * CharSize;
					double OffsetY	=	sin ( RotateAngle + M_PI / 2.0 ) * CharSize;

					GetAt(0).dY		=	GetAt(3).dY - OffsetY;
					GetAt(4).dY		=	GetAt(6).dY - OffsetY;
					GetAt(1).dY		=	GetAt(2).dY - OffsetY;

					GetAt(0).dX		=	GetAt(3).dX - OffsetX;
					GetAt(4).dX		=	GetAt(3).dX - OffsetX - ( GetAt(3).dX - GetAt(6).dX );
					GetAt(1).dX		=	GetAt(3).dX - OffsetX - ( GetAt(3).dX - GetAt(2).dX );

					RegionMinimize	=	true;
				}
			}

			//-------------------------------------------------------------------------------------------

			// TODO_

			if ( CapturePoint == 1 )
			{
				bool RegionMinimize	=	false;

				if ( BoundWidth <= CharSize )
				{
					double OffsetX	=	cos ( RotateAngle ) * CharSize;
					double OffsetY	=	sin ( RotateAngle ) * CharSize;

					GetAt(1).dX		=	GetAt(0).dX + OffsetX;
					GetAt(5).dX		=	GetAt(7).dX + OffsetX;
					GetAt(2).dX		=	GetAt(3).dX + OffsetX;

					GetAt(1).dY		=	GetAt(0).dY + OffsetY;
					GetAt(5).dY		=	GetAt(0).dY + OffsetY - ( GetAt(0).dY - GetAt(7).dY );
					GetAt(2).dY		=	GetAt(0).dY + OffsetY - ( GetAt(0).dY - GetAt(3).dY );

					RegionMinimize	=	true;
				}

				if ( BoundHeight <= CharSize )
				{
					double OffsetX	=	cos ( RotateAngle + M_PI / 2.0 ) * CharSize;
					double OffsetY	=	sin ( RotateAngle + M_PI / 2.0 ) * CharSize;

					GetAt(0).dY		=	GetAt(3).dY - OffsetY;
					GetAt(4).dY		=	GetAt(6).dY - OffsetY;
					GetAt(1).dY		=	GetAt(2).dY - OffsetY;

					GetAt(0).dX		=	GetAt(3).dX - OffsetX;
					GetAt(4).dX		=	GetAt(3).dX - OffsetX - ( GetAt(3).dX - GetAt(6).dX );
					GetAt(1).dX		=	GetAt(3).dX - OffsetX - ( GetAt(3).dX - GetAt(2).dX );

					RegionMinimize	=	true;
				}
			}

			//-------------------------------------------------------------------------------------------

			if ( CapturePoint == 2 || CapturePoint == 5 )
			{
				bool RegionMinimize	=	false;

				if ( BoundWidth <= CharSize )
				{
					double OffsetX	=	cos ( RotateAngle ) * CharSize;
					double OffsetY	=	sin ( RotateAngle ) * CharSize;

					GetAt(1).dX		=	GetAt(0).dX + OffsetX;
					GetAt(5).dX		=	GetAt(7).dX + OffsetX;
					GetAt(2).dX		=	GetAt(3).dX + OffsetX;

					GetAt(1).dY		=	GetAt(0).dY + OffsetY;
					GetAt(5).dY		=	GetAt(0).dY + OffsetY - ( GetAt(0).dY - GetAt(7).dY );
					GetAt(2).dY		=	GetAt(0).dY + OffsetY - ( GetAt(0).dY - GetAt(3).dY );

					RegionMinimize	=	true;
				}

				if ( BoundHeight <= CharSize )
				{
					double OffsetX	=	cos ( RotateAngle + M_PI / 2.0 ) * CharSize;
					double OffsetY	=	sin ( RotateAngle + M_PI / 2.0 ) * CharSize;

					GetAt(3).dY		=	GetAt(0).dY + OffsetY;
					GetAt(6).dY		=	GetAt(4).dY + OffsetY;
					GetAt(2).dY		=	GetAt(1).dY + OffsetY;

					GetAt(3).dX		=	GetAt(0).dX + OffsetX;
					GetAt(6).dX		=	GetAt(0).dX + OffsetX - ( GetAt(0).dX - GetAt(4).dX );
					GetAt(2).dX		=	GetAt(0).dX + OffsetX - ( GetAt(0).dX - GetAt(1).dX );

					RegionMinimize	=	true;
				}
			}

			//-------------------------------------------------------------------------------------------

			if ( CapturePoint == 3 || CapturePoint == 6 )
			{
				bool RegionMinimize	=	false;

				if ( BoundWidth <= CharSize )
				{
					double OffsetX	=	cos ( RotateAngle ) * CharSize;
					double OffsetY	=	sin ( RotateAngle ) * CharSize;

					GetAt(0).dX		=	GetAt(1).dX - OffsetX;
					GetAt(7).dX		=	GetAt(5).dX - OffsetX;
					GetAt(3).dX		=	GetAt(2).dX - OffsetX;

					GetAt(3).dY		=	GetAt(2).dY - OffsetY;
					GetAt(7).dY		=	GetAt(2).dY - OffsetY - ( GetAt(2).dY - GetAt(5).dY );
					GetAt(0).dY		=	GetAt(2).dY - OffsetY - ( GetAt(2).dY - GetAt(1).dY );

					RegionMinimize	=	true;
				}

				if ( BoundHeight <= CharSize )
				{
					double OffsetX	=	cos ( RotateAngle + M_PI / 2.0 ) * CharSize;
					double OffsetY	=	sin ( RotateAngle + M_PI / 2.0 ) * CharSize;

					GetAt(3).dY		=	GetAt(0).dY + OffsetY;
					GetAt(6).dY		=	GetAt(4).dY + OffsetY;
					GetAt(2).dY		=	GetAt(1).dY + OffsetY;

					GetAt(3).dX		=	GetAt(0).dX + OffsetX;
					GetAt(6).dX		=	GetAt(0).dX + OffsetX - ( GetAt(0).dX - GetAt(4).dX );
					GetAt(2).dX		=	GetAt(0).dX + OffsetX - ( GetAt(0).dX - GetAt(1).dX );

					RegionMinimize	=	true;
				}
			}

			//-------------------------------------------------------------------------------------------

			if ( CapturePoint == 4 || CapturePoint == 7 )
			{
				bool RegionMinimize	=	false;

				if ( BoundWidth <= CharSize )
				{
					double OffsetX	=	cos ( RotateAngle ) * CharSize;
					double OffsetY	=	sin ( RotateAngle ) * CharSize;

					GetAt(0).dX		=	GetAt(1).dX - OffsetX;
					GetAt(7).dX		=	GetAt(5).dX - OffsetX;
					GetAt(3).dX		=	GetAt(2).dX - OffsetX;

					GetAt(0).dY		=	GetAt(1).dY - OffsetY;
					GetAt(7).dY		=	GetAt(1).dY - OffsetY - ( GetAt(1).dY - GetAt(5).dY );
					GetAt(3).dY		=	GetAt(1).dY - OffsetY - ( GetAt(1).dY - GetAt(2).dY );

					RegionMinimize	=	true;
				}

				if ( BoundHeight <= CharSize )
				{
					double OffsetX	=	cos ( RotateAngle + M_PI / 2.0 ) * CharSize;
					double OffsetY	=	sin ( RotateAngle + M_PI / 2.0 ) * CharSize;

					GetAt(0).dY		=	GetAt(3).dY - OffsetY;
					GetAt(4).dY		=	GetAt(6).dY - OffsetY;
					GetAt(1).dY		=	GetAt(2).dY - OffsetY;

					GetAt(0).dX		=	GetAt(3).dX - OffsetX;
					GetAt(4).dX		=	GetAt(3).dX - OffsetX - ( GetAt(3).dX - GetAt(6).dX );
					GetAt(1).dX		=	GetAt(3).dX - OffsetX - ( GetAt(3).dX - GetAt(2).dX );

					RegionMinimize	=	true;
				}
			}

			//-------------------------------------------------------------------------------------------

		}


		inline bool		GetFontObjects				( Gdiplus::Font** ppFont, Gdiplus::FontFamily** ppFontFamily, Gdiplus::StringFormat** ppStringFormat, int& FontStyle, bool Zoom = true )
		{
			// int FontStyle		=	Gdiplus::FontStyleRegular;

			Gdiplus::Font*			pFont			=	NULL;
			Gdiplus::FontFamily*	pFontFamily		=	NULL;
			Gdiplus::StringFormat*	pStringFormat	=	NULL;

			if ( NULL == m_pFontFamily )
			{
#ifdef UNICODE
				m_pFontFamily	=	new Gdiplus::FontFamily ( m_oFont.GetName () );
#else
				m_pFontFamily	=	new Gdiplus::FontFamily ( A2BSTR ( m_oFont.GetName () ) );
#endif
			}

			//Gdiplus::FontFamily	fontFamily( fontName );
			//Gdiplus::Font			font ( &fontFamily, (Gdiplus::REAL)fontSize * (Gdiplus::REAL)Zoom, fontStyle, Gdiplus::UnitPixel );
			//Gdiplus::StringFormat	stringFormat;

			if ( Zoom )
				m_oFont.SetFontSize ( (Gdiplus::REAL)m_FontSize * (Gdiplus::REAL)m_dZoom );
			else
				m_oFont.SetFontSize ( (Gdiplus::REAL)m_FontSize );

			pFont			=	m_oFont.GetFont ();		//	Font Set By Xml String
			pStringFormat	=	m_oFormat.GetStringFormat();
			pStringFormat->SetFormatFlags ( StringFormatFlagsMeasureTrailingSpaces );
			FontStyle		=	pFont->GetStyle ();

			*ppFontFamily	=	m_pFontFamily;	
			*ppFont			=	pFont;
			*ppStringFormat	=	pStringFormat;

			return true;
		}


		inline long		GetStringIndex				( long SymbolIndex )
		{
			for ( long i = 0; i < (long)m_aStringsInfo.GetSize (); ++i )
			{
				if ( m_aStringsInfo [i].IsConsists ( SymbolIndex ) )
					return i;
			}

			return -1;
		}

		inline long		GetCurrentString			( )
		{
			if ( m_bCaretAtHome )
			{
				return GetStringIndex ( __min ( m_CurrPos + 1, m_String.GetLength () - 1 ) );
			}

			return GetStringIndex ( m_CurrPos );
		}

	public:

		virtual BOOL	OnMessages					( UINT Message, WPARAM wParam, LPARAM lParam )
		{
			switch ( Message )
			{
			case WM_LBUTTONDOWN: 
				{
					OnLButtonDown ( GET_X_LPARAM ( lParam ), GET_Y_LPARAM ( lParam ) );
				}
				break;
			case WM_LBUTTONUP: 
				{
					OnLButtonUp ( GET_X_LPARAM ( lParam ), GET_Y_LPARAM ( lParam ) );
				}
				break;
			case WM_MOUSEMOVE: 
				{
					OnMouseMove ( GET_X_LPARAM ( lParam ), GET_Y_LPARAM ( lParam ) );
				}
				break;
			
			case WM_MOUSEWHEEL:
				{
					int zDelta = GET_WHEEL_DELTA_WPARAM ( wParam );

					if ( m_bEditingModeEnabled && m_bScrollingText )
					{
						if ( zDelta > 0 )
						{
							m_nScrollLine--;
							m_nScrollLine = __max ( 0, m_nScrollLine );
						}
						else 
						if ( zDelta < 0 )
						{
							m_nScrollLine++;
							
							if ( m_nScrollLine > m_nCanScrollLines )
								m_nScrollLine = m_nCanScrollLines;
						}
			
						SendMessage ( Constants::sc_nMessageTrackTextChange, GetCurrentPointIndex() );
					}
				}

				break;

			case WM_KEYDOWN: 
				{
					OnKeyDown ( (UINT) wParam );
				}
				break;
			case WM_KEYUP: 
				{
					OnKeyUp ( (UINT)wParam );
				}
				break;
			case WM_DESTROY: 
				{
					m_arrStrings.RemoveAll();
				}
				break;
			case WM_KILLFOCUS: 
				{
					/////////////////////EndEditText();

					//m_arrStrings.RemoveAll();
				}
				break;
			case WM_MOUSELEAVE: 
				{
					m_IsSelect	=	false;
				}
				break;
			case WM_LBUTTONDBLCLK: 
				{
					if ( m_bBlockEditing )
						return FALSE;

					if ( m_bEditingModeEnabled ) 
					{
						SelectByDblClick ( ); 
						OnMouseMove ( GET_X_LPARAM ( lParam ), GET_Y_LPARAM( lParam ) );

						return FALSE;
					}
					else
					{
						m_bEditingModeEnabled	=	true;	

						SendMessage ( Constants::sc_nMessageTrackTextBegin, GetCurrentPointIndex () );

						SplitString();

						m_SelectStart = m_SelectEnd = m_CurrPos;

						OnLButtonDown ( GET_X_LPARAM ( lParam ), GET_Y_LPARAM ( lParam ) );
					}
				}
				break;

			default:
				break;
			}

			return FALSE;
		}
		virtual BOOL	OnSetCursor					( int nX, int nY, int& nPointStatus, int& nChangeStatus )
		{
			nPointStatus = c_nTrackPointStatusNone;
			nChangeStatus = c_nTrackChangeStatusNone;

			int nTrackIndex = GetCurrentPointIndex();

			if (nTrackIndex < 0)
				nTrackIndex = GetPointUnderCursor(nX, nY);

			if ( nTrackIndex == 9 || (nTrackIndex == GetCount() - 1) || (nTrackIndex >= 0 && GetAsyncKeyState(VK_CONTROL) < 0))
			{
				nPointStatus = c_nTrackPointStatusSizeAll;
				nChangeStatus = c_nTrackChangeStatusMove;
			}
			else if ( nTrackIndex >= 0 && nTrackIndex < 8 && !m_AutoSize )
			{
				nPointStatus = c_nTrackPointStatusPoint;
				nChangeStatus = c_nTrackChangeStatusSize;
			}
			else if (nTrackIndex == 8)
			{
				nPointStatus = c_nTrackPointStatusRotation;
				nChangeStatus = c_nTrackChangeStatusRotation;
			}
			else
			{
				nPointStatus = c_nTrackPointStatusNone;
				nChangeStatus = c_nTrackChangeStatusNone;
			}

			if ( m_bBlockResize )
			{
				if ( nTrackIndex >= 0 && nTrackIndex < 8 )
				{
					nPointStatus = c_nTrackPointStatusNone;
					nChangeStatus = c_nTrackChangeStatusNone;
				}
			}

			if( PosOnMouse( (int)MapToDataX ( nX ), (int)MapToDataY( nY ) ) != -1 && m_bEditingModeEnabled && !m_IsTransfom )
				nPointStatus = c_nTrackPointStatusIBeam;
			return FALSE;
		}
		virtual void	OnPaintLines				( Tracking::CTrackPainter& oPainter )
		{
			double Angle	=	Geometry::GetAngle( GetAt(9).dX, GetAt(9).dY, GetAt(8).dX, GetAt(8).dY );

			GetZoom ( this );

			if ( m_hDC != oPainter.GetDC() )
				m_hDC = oPainter.GetDC();

			if ( m_bFirstPaint )
			{
				SplitString ( );

				UpdateResizeShapeToFitText	( );
				UpdateScrollingText			( );

				m_bFirstPaint	=	false;
			}

			if ( m_bScrollingText )
			{
				DrawScrollingText ();
			}

			if ( m_bNotAutoFitText )
			{
				DrawNotAutoFitText ( );
			}

			if ( m_bResizeShapeToFitText )
			{
				DrawResizeShapeToFitText ( );
			}

			if ( m_AutoSize && m_IsFirstResize )
			{
				SplitString ( );
				m_IsFirstResize = false;
			}

#ifdef _DEBUG
			Gdiplus::Graphics graphics ( m_hDC );

			if ( Gdiplus::Ok == graphics.GetLastStatus () )
			{
				CStringW DebugMessage;

				Gdiplus::SolidBrush		oBlackSolidBrush ( Gdiplus::Color::White );
				Gdiplus::StringFormat	oStringFormat;
				oStringFormat.SetFormatFlags ( StringFormatFlagsNoWrap );	

				if ( m_bNotAutoFitText )		
				{
					DebugMessage	=	CStringW ( L"AutoFit : NotAutoFitText" );
					graphics.FillRectangle ( new SolidBrush ( 0x9F000000 ), Gdiplus::RectF ( 20, 20, 200, 20 ) );
					graphics.DrawString ( DebugMessage, -1, new Gdiplus::Font ( L"Tahoma", 10 ), Gdiplus::RectF ( 20, 20, 500, 200 ), &oStringFormat, &oBlackSolidBrush );
				}

				if ( m_bResizeShapeToFitText )	
				{
					DebugMessage	=	CStringW ( L"AutoFit : ResizeShapeToFitText" );
					graphics.FillRectangle ( new SolidBrush ( 0x9F000000 ), Gdiplus::RectF ( 20, 20, 200, 20 ) );
					graphics.DrawString ( DebugMessage, -1, new Gdiplus::Font ( L"Tahoma", 10 ), Gdiplus::RectF ( 20, 20, 500, 200 ), &oStringFormat, &oBlackSolidBrush );
				}

				//if ( m_bScrollingText )		
				{
					DebugMessage.Format (	
						L"ScrollingText \n"
						L"\n Cursor String Index : %d"
						L"\n CaretAtHome :%s"
						L"\n Lines :__________%d "
						L"\n VisibleLines :___%d "
						L"\n CaretString : ____%d "
						L"\n CanScrollLines :___%d "
						L"\n ScrollLine :_______%d ", 

						m_nCurrentString,
						(( m_bCaretAtHome ) ? L"TRUE" : L"FALSE"),
						m_arrStrings.GetSize(), 
						m_nVisibleLines, 
						GetCurrentString(),
						m_nCanScrollLines, 
						m_nScrollLine );

					CStringW LineInfo;	LineInfo.Format ( L"\nCURR POS : %d\n", m_CurrPos );
					DebugMessage += LineInfo;

					//for ( int i = 0; i < m_aStringsInfo.GetSize (); ++i )
					//{
					//	CStringW LineInfo;	LineInfo.Format ( L"Line : %d, Begin : %d, End : %d\n", i, m_aStringsInfo[i].m_nBeginInd, m_aStringsInfo[i].m_nEndInd );
					//	//DebugMessage += LineInfo;
					//}

					graphics.FillRectangle ( new SolidBrush ( 0x9F000000 ), Gdiplus::RectF ( 20, 20, 200, 920 ) );
					graphics.DrawString ( DebugMessage, -1, new Gdiplus::Font ( L"Tahoma", 10 ), Gdiplus::RectF ( 20, 20, 500, 900 ), &oStringFormat, &oBlackSolidBrush );
				}																		

			}
#endif
		}

		virtual void	OnPaintPoints				( Tracking::CTrackPainter& oPainter )
		{
			if ( m_bBlockResize )
				return;

			if( !m_AutoSize ) 
				oPainter.DrawPoints(this, GetCount() - 3, c_nTrackPointSizePaint);
		}


		inline void		OnKeyDown					( unsigned int Key )
		{
			if ( !m_bEditingModeEnabled )
				return;

			bool isShift			=	false;
			bool isCtrl				=	false;

			bool bSwitchKey			=	false;

			switch ( Key )
			{
			case VK_ESCAPE:
				{
					EndEditText ( );

					m_CurrPos		=	m_SelectEnd;
					m_SelectStart	=	m_SelectEnd;

					bSwitchKey		=	true;
				}
				break;

			case VK_CONTROL:
				{
					isCtrl			=	true;
					bSwitchKey		=	true;
				}
				break;

			case VK_LEFT:
				{
					if ( GetAsyncKeyState ( VK_SHIFT ) < 0 ) 
						m_IsSelect	=	true;
					
					if ( m_CurrPos > 0 )
					{
						if ( false == m_bCaretAtHome  )
						{
							if ( m_CurrPos - 2 > 0 )
							{
								if ( m_String [ m_CurrPos - 2 ] != '\r' )
								{
									if ( GetStringIndex ( m_CurrPos ) != GetStringIndex ( m_CurrPos - 1 ) )
									{
										m_bCaretAtHome	=	true;
										bSwitchKey		=	true;

										--m_CurrPos;

										break;
									}
								}
							}
						}
						else
						{
							m_bCaretAtHome	=	false;
							break;
						}
					}

					if ( m_CurrPos > 0 )
						--m_CurrPos;

					if ( m_CurrPos > 0 )
					{
						if ( m_String [ m_CurrPos - 1 ] == '\r' ) 
							m_CurrPos--;
					}
					
					bSwitchKey		=	true;
					m_bCaretAtHome	=	false;
				}
				break;

			case VK_RIGHT:
				{
					if ( GetAsyncKeyState( VK_SHIFT ) < 0 ) 
						m_IsSelect = true;

					if ( m_CurrPos < m_String.GetLength() ) 
					{
						if ( false == m_bCaretAtHome )
						{
							if ( m_String [ m_CurrPos ] != '\r' )
							{
								if ( GetStringIndex ( m_CurrPos ) != GetStringIndex ( m_CurrPos + 1 ) )
								{
									m_bCaretAtHome	=	true;
									bSwitchKey		=	true;

									break;
								}
							}
						}
					}

					if ( m_CurrPos < m_String.GetLength() ) 
						++m_CurrPos;

					if ( m_CurrPos > 0 )
					{
						if ( m_String [ m_CurrPos - 1 ] == '\r' ) 
							++m_CurrPos;
					}

					bSwitchKey		=	true;
					m_bCaretAtHome	=	false;
				}
				break;

			case VK_UP:
				{
					if ( GetAsyncKeyState( VK_SHIFT ) < 0 ) 
						m_IsSelect		=	true;

					int tmpCaretPos		=	m_CurrPos;

					int currIdxString	=	GetStringByPos ( tmpCaretPos );
					if ( currIdxString < 1 ) 
						return;

					int sumLineLenght	=	0;
					for( int i = 0; i <= currIdxString; ++i )
						sumLineLenght += m_arrStrings[i].GetLength();

					int posInLine		=	m_arrStrings[currIdxString].GetLength() - sumLineLenght + m_CurrPos;

					if ( 2 != m_oFormat.GetStringAlignmentHorizontal () )	//	aligned right
					{
						if ( m_arrStrings [ currIdxString - 1 ].GetLength() - 2 < posInLine ) // - 2 for "\r\n"
							m_CurrPos	-=	posInLine + 2; // + 2 for "\r\n"
						else
							m_CurrPos	-=	m_arrStrings[currIdxString - 1].GetLength();	
					}
					else
					{
						if ( m_arrStrings[currIdxString - 1].GetLength() < m_arrStrings[currIdxString].GetLength() - posInLine ) // - 2 for "\r\n"	
							m_CurrPos	-=	posInLine + m_arrStrings[currIdxString - 1].GetLength();
						else
							m_CurrPos	-=	m_arrStrings[currIdxString].GetLength();
					}

					bSwitchKey			=	true;
					
					int StringWork		=	GetCurrentString ( );

					if ( StringWork < ( m_nScrollLine ) )
					{
						m_nScrollLine--;
						m_nScrollLine	=	__max ( 0, m_nScrollLine );
					}
				}
				break;

			case VK_DOWN:
				{
					if ( GetAsyncKeyState( VK_SHIFT ) < 0 )
						m_IsSelect = true;

					int tmpCaretPos		=	m_CurrPos;
					int currIdxString	=	GetStringByPos( tmpCaretPos );
					if( -1 == currIdxString || currIdxString == m_arrStrings.GetSize() - 1 )
						return;

					int sumLineLenght	=	0;
					for( int i = 0; i <= currIdxString; ++i )
						sumLineLenght	+=	m_arrStrings[i].GetLength();

					int posInLine		=	m_arrStrings[currIdxString].GetLength() - sumLineLenght + m_CurrPos;

					if ( 2 != m_oFormat.GetStringAlignmentHorizontal () )	//	aligned right
					{
						if ( m_arrStrings[currIdxString + 1].GetLength() - 2 < posInLine ) 
							m_CurrPos	+=	sumLineLenght - m_CurrPos + m_arrStrings[currIdxString + 1].GetLength() - 2;
						else
							m_CurrPos	+=	m_arrStrings[currIdxString].GetLength();
					}
					else
					{
						if ( m_arrStrings[currIdxString+1].GetLength() < m_arrStrings[currIdxString].GetLength() - posInLine )
							m_CurrPos	+=	m_arrStrings[currIdxString].GetLength() - posInLine;
						else
							m_CurrPos	+=	m_arrStrings[currIdxString].GetLength() - posInLine + (m_arrStrings[currIdxString+1].GetLength() - m_arrStrings[currIdxString].GetLength() + posInLine );
					}

					bSwitchKey			=	true;

					int StringWork		=	GetCurrentString ( );

					if ( StringWork >= ( m_nScrollLine + m_nVisibleLines ) )
					{
						m_nScrollLine++;
						m_nScrollLine	=	__min ( m_nCanScrollLines, m_nScrollLine );
					}
				}
				break;

			case VK_SHIFT:
				{
					//if( !m_IsSelect ) m_SelectStart = m_SelectEnd = m_CurrPos;
					isShift				=	true;

					bSwitchKey			=	true;
				}

				break;

			case VK_DELETE:
				{
					if ( m_CurrPos == m_String.GetLength() && m_SelectStart == m_SelectEnd ) 
						return;

					if ( m_CurrPos > 0 )
					{
						if ( m_String[ m_CurrPos - 1 ] == '\r' ) 
							--m_CurrPos;
					}

					if( m_SelectStart != m_SelectEnd )
					{
						m_CurrPos		=	min( m_SelectEnd, m_SelectStart );
						m_String.Delete( m_CurrPos, abs( m_SelectEnd - m_SelectStart ) );
					}
					else
					{
						bool deleteRN	=	false;

						if ( m_CurrPos + 2 <= m_String.GetLength() )
						{
							if( m_String[ m_CurrPos + 1 ] == '\n' )
							{
								m_String.Delete( m_CurrPos, 2 );
								deleteRN = true;
							}
						}

						if ( !deleteRN )
							m_String.Delete( m_CurrPos );
					}

					SplitString();

					bSwitchKey		=	true;
				}
				break;

			case VK_BACK:
				{
					if ( m_CurrPos == 0 && m_SelectStart == m_SelectEnd ) 
						return;

					if ( m_CurrPos > 0 )
					{
						if ( m_String[ m_CurrPos - 1 ] == '\r' )
							--m_CurrPos;
					}

					if ( m_SelectStart != m_SelectEnd  )	
					{
						m_CurrPos		=	min( m_SelectEnd, m_SelectStart );
						m_String.Delete( m_CurrPos, abs( m_SelectEnd - m_SelectStart ) );
					}
					else
					{
						bool deleteRN	=	false;

						if( m_CurrPos > 0 )
						{
							if( m_String[ m_CurrPos - 1 ] == '\n' )
							{
								m_String.Delete( m_CurrPos - 2, 2 );
								m_CurrPos -= 2;
								deleteRN = true;
							}
						}

						if( !deleteRN )
						{
							m_String.Delete( m_CurrPos - 1 );
							--m_CurrPos;
						}
					}

					SplitString();

					bSwitchKey		=	true;
				}
				break;

			case VK_SPACE:
				{
					if ( m_SelectStart != m_SelectEnd  )	
					{
						m_CurrPos	=	min ( m_SelectEnd, m_SelectStart );
						m_String.Delete ( m_CurrPos, abs( m_SelectEnd - m_SelectStart ) );
					}

					if ( m_CurrPos == m_String.GetLength() )
					{
						m_String	+=	L" ";
						m_CurrPos	=	m_String.GetLength();
					}
					else
					{
						m_String.Insert( m_CurrPos, L" " );
						++m_CurrPos;
					}

					SplitString();

					bSwitchKey		=	true;
				}
				break;

			case VK_RETURN:
				{
					if ( m_CurrPos == m_String.GetLength() )
					{
						m_String	+=	L"\r\n";
						m_CurrPos	=	m_String.GetLength();
					}
					else
					{
						m_String.Insert ( m_CurrPos, L"\r\n" );
						m_CurrPos	+=	2;
					}

					if ( m_bScrollingText )
						m_nScrollLine++;

					SplitString ();

					bSwitchKey		=	true;
				}
				break;
				//case VK_TAB:
				//	{
				//	}
				//	break;

			case VK_HOME:
				{
					if ( GetAsyncKeyState ( VK_CONTROL ) < 0 )
					{
						m_CurrPos		=	0;
						m_nScrollLine	=	0;
						break;
					}

					if ( GetAsyncKeyState ( VK_SHIFT ) < 0 )
						m_IsSelect		=	true;

					if ( m_bCaretAtHome )	// каретка в начале строки
						break;

					// m_CurrPos		=	0;

					int StringWork		=	GetStringIndex ( m_CurrPos );
					if ( 0 < StringWork )
					{
						if ( m_aStringsInfo [ StringWork ].m_nEndInd - m_aStringsInfo [ StringWork ].m_nBeginInd == 0 )	// пропуск на след. строку
							return;

						m_CurrPos		=	m_aStringsInfo [ StringWork ].m_nBeginInd;
					}
					else
					{
						m_CurrPos		=	0;
					}

					m_bCaretAtHome		=	true;	//

					bSwitchKey			=	true;
				}
				break;

			case VK_END:
				{
					if ( GetAsyncKeyState ( VK_CONTROL ) < 0 )
					{
						m_CurrPos			=	m_String.GetLength();
						
						if ( m_bScrollingText )
							m_nScrollLine	=	m_nCanScrollLines;

						break;
					}

					if ( GetAsyncKeyState( VK_SHIFT ) < 0 )
						m_IsSelect			=	true;

					// m_CurrPos			=	m_String.GetLength();

					int StringWork			=	GetCurrentString ( );// ( m_CurrPos );
					if ( 0 <= StringWork && StringWork < m_aStringsInfo.GetSize () - 1 )
					{
						m_CurrPos			=	m_aStringsInfo [ StringWork ].m_nEndInd;
					}
					else
					{
						m_CurrPos			=	m_String.GetLength();
					}

					m_bCaretAtHome			=	false;	// 

					bSwitchKey				=	true;
				}
				break;

			default :
				{

				}
				break;
			}

			if ( false == bSwitchKey )
			{
				InsertKey ( Key );
			}

			if( !isShift && !isCtrl )
			{
				if ( m_IsSelect )
					m_SelectEnd		=	m_CurrPos;
				else				
					m_SelectStart	=	m_SelectEnd = m_CurrPos;
			}

			UpdateResizeShapeToFitText	( );
			UpdateScrollingText			( );

			SendMessage ( Constants::sc_nMessageTrackTextChange, GetCurrentPointIndex() );
		}

		inline void		InsertKey					( unsigned int Key )
		{
			if (( Key >= 'A'		&& Key <= 'Z'		) || 
				( Key >= '0'		&& Key <= '9'		) || 
				( Key >= VK_OEM_1	&& Key <= VK_OEM_3	) || 
				( Key >= VK_OEM_4	&& Key <= VK_OEM_8	) ||
				( Key >= VK_NUMPAD0 && Key <= VK_DIVIDE ) )
			{

				if ( ProcessControlKeyState ( Key ) )
					return;

				if ( m_SelectStart != m_SelectEnd  )	
				{
					m_CurrPos			=	min( m_SelectEnd, m_SelectStart );
					m_String.Delete( m_CurrPos, abs( m_SelectEnd - m_SelectStart ) );
					m_IsSelect			=	false;
				}
				
				if ( m_CurrPos > 0 )
				{
					if( m_String [ m_CurrPos - 1 ] == L'\r' ) 
					{
						m_CurrPos--;
					}
				}
		
				wchar_t wKey[3]			=	{ L'\0', L'\0', L'\0' };

				BYTE kbdState[256];
				ZeroMemory( kbdState, 256 );
				
				GetKeyboardState ( kbdState );
				
				wchar_t KeyboardState	=	wKey [ 0 ];
				
    			if ( 1 == ToUnicode ( Key, 0, kbdState, wKey, 3, 0 ) )
				{
					//if ( m_chDeadKey != L'\0' ) 
					//{
					//	wchar_t wcs_in	[3];
					//	wchar_t wcs_out	[3];

					//	wcs_in[0]	=	wKey[0];
					//	wcs_in[1]	=	m_chDeadKey;
					//	wcs_in[2]	=	L'\0';

					//	/* from accent char to unicode */
					//	if ( FoldStringW ( MAP_PRECOMPOSED, wcs_in, 3, wcs_out, 3 ) )
					//	{
					//		wKey[0]	=	wcs_out[0];
					//		wKey[1]	=	wcs_out[1];
					//		wKey[2]	=	wcs_out[2];
					//	}
					//}
					//else
					//{

					//}

					//m_chDeadKey		=	L'\0';
				}
				else
				{
					//m_chDeadKey		=	L'\0';

					//switch(wKey[0]) 
					//{
					//case 0x5e:          /* circumflex */
					//	m_chDeadKey	=	0x302;  break;
					//case 0x60:          /* grave accent */
					//	m_chDeadKey	=	0x300;  break;
					//case 0xa8:          /* diaeresis */
					//	m_chDeadKey	=	0x308;  break;
					//case 0xb4:          /* acute accent */
					//	m_chDeadKey	=	0x301;  break;
					//case 0xb8:          /* cedilla */
					//	m_chDeadKey	=	0x327;  break;
					//default:
					//	m_chDeadKey	=	wKey[0];
					//	break;
					//}

					return;
				}					

				if ( m_CurrPos == m_String.GetLength() )
				{
					m_String		+=	CStringW ( wKey );
					m_CurrPos		=	m_String.GetLength ();
				}
				else
				{
					m_String.Insert ( m_CurrPos, CStringW ( wKey ) );
					++m_CurrPos;
				}

				SplitString ();
			}
		}

		inline bool		ProcessControlKeyState		( unsigned int Key )
		{
			if ( GetAsyncKeyState ( VK_CONTROL ) < 0 )
			{
				if ( Key == 'X' )
				{
					CopyToClipboard ( true );
				}

				if ( Key == 'C' )
				{
					CopyToClipboard ( );
				}

				if ( Key == 'V' )
				{
					PastToClipboard ( );
				}

				if ( Key == 'A' )
				{
					SelectTextAll ( );

					m_bTextAllSelect	=	true;
				}

				UpdateResizeShapeToFitText	( );
				UpdateScrollingText			( );

				SendMessage ( Constants::sc_nMessageTrackTextChange, GetCurrentPointIndex() );

				return true;
			}

			return false;
		}


		inline void		GetZoom						( Tracking::CTrackPoints* pTrack )
		{
			if (!pTrack || pTrack->GetCount() < 1 ) return;

			POINT* pPoints = new POINT[10];
			pTrack->GetPointArray( pPoints, 10 );

			Structures::RECTD rectD;

			double dAngle = Geometry::GetAngle( GetAt(9).dX, GetAt(9).dY, GetAt(8).dX, GetAt(8).dY ) + M_PI_2;

			double tX, tY;
			double cosA = cos( -dAngle );
			double sinA = sin( -dAngle );

			rectD.left   = (pPoints + 0)->x;
			rectD.top    = (pPoints + 0)->y;
			rectD.right  = (pPoints + 2)->x;
			rectD.bottom = (pPoints + 2)->y;

			tX = (pPoints + 0)->x - (pPoints + 9)->x;
			tY = (pPoints + 0)->y - (pPoints + 9)->y;				
			rectD.left  = (pPoints + 9)->x + tX * cosA - tY * sinA;
			rectD.top   = (pPoints + 9)->y + tX * sinA + tY * cosA;	

			tX = (pPoints + 2)->x - (pPoints + 9)->x;
			tY = (pPoints + 2)->y - (pPoints + 9)->y;
			rectD.right  = (pPoints + 9)->x + tX * cosA - tY * sinA;
			rectD.bottom = (pPoints + 9)->y + tX * sinA + tY * cosA;

			if( rectD.right < rectD.left ) 
			{ 
				double tmpRight		= rectD.right; 
				rectD.right			= rectD.left;
				rectD.left			= tmpRight;
			}
			if( rectD.top > rectD.bottom ) 
			{ 
				double tmpBottom	= rectD.bottom; 
				rectD.bottom		= rectD.top;
				rectD.top			= tmpBottom;
			}		

			Structures::RECTD rectDTracking = GetUnrotatableRect( dAngle );

			if( rectDTracking.GetWidth() > 0 )
				m_dZoom = rectD.GetWidth() / rectDTracking.GetWidth();

			if( pPoints )
			{
				delete []pPoints;
				pPoints = NULL;
			}
		}

		inline void		EnableBlockEditing			( bool Value )
		{
			m_bBlockEditing		=	Value;
		}

		inline void		EnableBlockResize			( bool Value )
		{
			m_bBlockResize		=	Value;
		}


		inline double	GetStickAngle				( )
		{ 
			return m_StickAngle;
		}


	public:

		inline void		Update						( )
		{
			SplitString ( );
		}

		// управляющие методы операции
		inline void		SelectTextAll				( )
		{
			m_IsSelect		=	true;
			m_CurrPos		=	m_String.GetLength ();

			m_SelectStart	=	0;
			m_SelectEnd		=	m_CurrPos;
		}

		inline void		BeginEditText				( )
		{
			if ( m_bBlockEditing )
				return;

			if ( false == m_bEditingModeEnabled ) 
			{
				m_bEditingModeEnabled	=	true;	

				SendMessage ( Constants::sc_nMessageTrackTextBegin, GetCurrentPointIndex () );

				SplitString ( );

				m_SelectStart = m_SelectEnd	=	m_CurrPos	=	0;
			}
		}

		inline void		EndEditText					( )
		{
			if ( m_bEditingModeEnabled )
			{
				m_bEditingModeEnabled	=	false;	
				m_bEditingModeEnabled	=	false;

				SendMessage( Constants::sc_nMessageTrackTextEnd, GetCurrentPointIndex() );
			}
		}


		
		inline void		SetBrush					( CString Xml )
		{
			/*
			XmlUtils::CXmlNode oXmlNode;
			if ( oXmlNode.FromXmlString ( Xml ) )
			{
				ImageStudio::Serialize::Paint::Structures::Brush oBrush;
				oBrush.FromXmlNode ( oXmlNode );

				m_oPaintState.SetBrush ( oBrush );

				m_oTextureManager.Clear ();

				m_oBrush.Update();
				m_oBrush = m_oPaintState.GetBrush ( );
			}
			*/
		}

		inline void		SetFont						( CString Xml )
		{
			/*
			XmlUtils::CXmlNode oXmlNode;
			if ( oXmlNode.FromXmlString ( Xml ) )
			{
				ImageStudio::Serialize::Paint::Structures::Font oFont;
				oFont.FromXmlNode ( oXmlNode );

				m_oPaintState.SetFont ( oFont );

				m_oFont.Update();
				m_oFont		=	m_oPaintState.GetFont ( );

				m_oFormat.Update();
				m_oFormat	=	m_oPaintState.GetFormat ( );

				m_nStringAlignmentVertical	=	Painter::CFormat::GetAlignment ( m_oFormat.GetStringAlignmentVertical () );
				m_oFormat.SetStringAlignmentVertical ( 0 );

				RELEASEOBJECT ( m_pFontFamily );
				
				if ( NULL == m_pFontFamily )
				{
#ifdef UNICODE
					m_pFontFamily	=	new Gdiplus::FontFamily ( m_oFont.GetName () );
#else
					m_pFontFamily	=	new Gdiplus::FontFamily ( A2BSTR ( m_oFont.GetName () ) );
#endif
				}

				if ( m_bResizeShapeToFitText )
				{
					SplitString					( );
					UpdateResizeShapeToFitText	( );
				}
			}
			*/
		}

		inline void		SetEdgeText					( CString Xml )
		{
			/*
			XmlUtils::CXmlNode oXmlNode;
			if ( oXmlNode.FromXmlString ( Xml ) )
			{
				ImageStudio::Serialize::Paint::Structures::Edge oEdge;
				oEdge.FromXmlNode ( oXmlNode );

				m_oPaintState.SetEdge ( oEdge );

				m_oEdgeText.Update();
				m_oEdgeText = m_oPaintState.GetEdge ( );
			}
			*/
		}

		inline void		SetShadow					( CString Xml )
		{
			/*
			XmlUtils::CXmlNode oXmlNode;
			if ( oXmlNode.FromXmlString ( Xml ) )
			{
				ImageStudio::Serialize::Paint::Structures::Shadow oShadow;
				oShadow.FromXmlNode ( oXmlNode );

				m_oPaintState.SetShadow ( oShadow );

				m_oShadow.Update();
				m_oShadow = m_oPaintState.GetShadow ( );
			}
			*/
		}


		inline void		SetFontSize					( int FontSize )		
		{
			m_oFont.SetFontSize ( FontSize );

			RELEASEOBJECT ( m_pFontFamily );
			if ( NULL == m_pFontFamily )
			{
#ifdef UNICODE
				m_pFontFamily	=	new Gdiplus::FontFamily ( m_oFont.GetName () );
#else
				m_pFontFamily	=	new Gdiplus::FontFamily ( A2BSTR ( m_oFont.GetName () ) );
#endif
			}

			if ( m_bResizeShapeToFitText )
			{
				SplitString					( );
				UpdateResizeShapeToFitText	( );
			}
		}
		inline void		SetFontName					( CString FontName )
		{ 
			m_oFont.SetName ( FontName );

			RELEASEOBJECT ( m_pFontFamily );
		
			if ( NULL == m_pFontFamily )
			{
#ifdef UNICODE
				m_pFontFamily	=	new Gdiplus::FontFamily ( m_oFont.GetName () );
#else
				m_pFontFamily	=	new Gdiplus::FontFamily ( A2BSTR ( m_oFont.GetName () ) );
#endif
			}

			if ( m_bResizeShapeToFitText )
			{
				SplitString					( );
				UpdateResizeShapeToFitText	( );
			}
		}

		inline void		SetNotAutoFitText			( bool Value )
		{
			m_bNotAutoFitText			=	Value;
			m_bResizeShapeToFitText		=	false;
			m_bScrollingText			=	false;
			m_bFirstPaint				=	true;
			m_nScrollLine				=	0;
		}

		inline void		SetResizeShapeToFitText		( bool Value )
		{
			m_bNotAutoFitText			=	false;
			m_bResizeShapeToFitText		=	Value;
			m_bScrollingText			=	false;
			m_bFirstPaint				=	true;
			m_nScrollLine				=	0;
		}

		inline void		SetScrollingText			( bool Value )
		{
			m_bNotAutoFitText			=	false;
			m_bResizeShapeToFitText		=	false;
			m_bScrollingText			=	Value;
			m_bFirstPaint				=	true;
			m_nScrollLine				=	0;
		}

		inline void		UpdateResizeShapeToFitText	( )
		{
			if ( m_bResizeShapeToFitText )
			{
				double FontHeight = Rendering::Utils::GetFontHeight ( m_oFont.GetFont () );

				double AngleRot		=	Geometry::GetAngle ( GetAt(9).dX, GetAt(9).dY, GetAt(8).dX, GetAt(8).dY ) + M_PI / 2.0;
				double SizeRotX		=	0.0;
				double SizeRotY		=	0.0;

				double BoundWidth	=	Geometry::GetLength ( (double)GetAt(0).nX, (double)GetAt(0).nY, (double)GetAt(1).nX, (double)GetAt(1).nY );
				double BoundHeight	=	__max ( 1.0, (double) ( m_arrStrings.GetSize () ) * FontHeight);

				double AX			=	( GetAt(4).nX - GetAt(0).nX ) / BoundWidth	* 0.5;
				double AY 			=	( GetAt(4).nY - GetAt(0).nY ) / BoundHeight * 0.5;
				double BX 			=	( GetAt(4).nX - GetAt(9).nX ) / BoundWidth	* 0.5;
				double BY 			=	( GetAt(4).nY - GetAt(9).nY ) / BoundHeight * 0.5;

				if ( AX * BY - AY * BX <= 0 )
				{
					{
						SizeRotX	=	0.0;
						SizeRotY	=	BoundHeight;

						Geometry::RotatePoint ( SizeRotX, SizeRotY, 0.0, 0.0, AngleRot );

						GetAt(3).Create ( GetAt(0).dX + SizeRotX, GetAt(0).dY + SizeRotY );
					}

					{
						SizeRotX	=	Geometry::GetLength ( GetAt(0).dX, GetAt(0).dY, GetAt(1).dX, GetAt(1).dY );
						SizeRotY	=	BoundHeight;

						Geometry::RotatePoint ( SizeRotX, SizeRotY, 0.0, 0.0, AngleRot );

						GetAt(2).Create ( GetAt(0).dX + SizeRotX, GetAt(0).dY + SizeRotY );
					}

					GetAt(7).Create ( ( GetAt(0).dX + GetAt(3).dX ) * 0.5, ( GetAt(0).dY + GetAt(3).dY ) * 0.5 );
					GetAt(5).Create ( ( GetAt(1).dX + GetAt(2).dX ) * 0.5, ( GetAt(1).dY + GetAt(2).dY ) * 0.5 );
					GetAt(6).Create ( ( GetAt(3).dX + GetAt(2).dX ) * 0.5, ( GetAt(3).dY + GetAt(2).dY ) * 0.5 );

					GetAt(9).Create ( ( GetAt(0).dX + GetAt(2).dX ) * 0.5, ( GetAt(0).dY + GetAt(2).dY ) * 0.5 );
				}
				else
				{
					{
						SizeRotX	=	0.0;
						SizeRotY	=	BoundHeight;

						Geometry::RotatePoint ( SizeRotX, SizeRotY, 0.0, 0.0, AngleRot );

						GetAt(2).Create ( GetAt(1).dX + SizeRotX, GetAt(1).dY + SizeRotY );
					}

					{
						SizeRotX	=	Geometry::GetLength ( GetAt(0).dX, GetAt(0).dY, GetAt(1).dX, GetAt(1).dY );
						SizeRotY	=	BoundHeight;

						Geometry::RotatePoint ( SizeRotX, SizeRotY, 0.0, 0.0, AngleRot );

						GetAt(3).Create ( GetAt(1).dX + SizeRotX, GetAt(1).dY + SizeRotY );
					}

					GetAt(7).Create ( ( GetAt(0).dX + GetAt(3).dX ) * 0.5, ( GetAt(0).dY + GetAt(3).dY ) * 0.5 );
					GetAt(5).Create ( ( GetAt(1).dX + GetAt(2).dX ) * 0.5, ( GetAt(1).dY + GetAt(2).dY ) * 0.5 );
					GetAt(6).Create ( ( GetAt(3).dX + GetAt(2).dX ) * 0.5, ( GetAt(3).dY + GetAt(2).dY ) * 0.5 );

					GetAt(9).Create ( ( GetAt(0).dX + GetAt(2).dX ) * 0.5, ( GetAt(0).dY + GetAt(2).dY ) * 0.5 );
				}
			}
		}

		inline void		UpdateScrollingText			( )
		{
			if ( m_bScrollingText )
			{
				double FontHeight	=	Rendering::Utils::GetFontHeight ( m_oFont.GetFont () );

				float BoundWidth	=	(float) ( Geometry::GetLength ( (double)GetAt(0).nX, (double)GetAt(0).nY, (double)GetAt(1).nX, (double)GetAt(1).nY ) );
				float BoundHeight	=	(float) ( Geometry::GetLength ( (double)GetAt(0).nX, (double)GetAt(0).nY, (double)GetAt(3).nX, (double)GetAt(3).nY ) );

				m_nVisibleLines		=	__min ( (int)(  BoundHeight / FontHeight )/* + 1*/, m_arrStrings.GetSize () );
				m_nCanScrollLines	=	__max ( 0, (int)( ( (float) ( (m_arrStrings.GetSize ()) * FontHeight ) - BoundHeight ) / FontHeight ) + 1 );

				if ( 0 == m_nCanScrollLines )
				{
					m_nScrollLine	=	0;
				}

				if ( m_nScrollLine > m_nCanScrollLines )
				{
					m_nScrollLine	=	m_nCanScrollLines;
				}
			}
		}

	public:

		bool IMouseButtonHandlers::OnMouseMove		( int MouseX, int MouseY )				
		{
			double dX = MapToDataX ( MouseX );
			double dY = MapToDataY ( MouseY );

			if ( false == m_IsTransfom )
			{
				if ( m_bTextAllSelect )	//	CTRL + A - выделение всего текста
				{
					return false;
				}

				int tmpPos = PosOnMouse ( (int)dX, (int)dY );
				if( tmpPos != -1 && m_bEditingModeEnabled )
				{
					if( m_IsSelect && !(GetAsyncKeyState( VK_SHIFT ) < 0) ) 
					{
						m_CurrPos	= tmpPos;
						m_SelectEnd = tmpPos;
					}

					if ( false == IsTrackChild () )
						SendMessage( Constants::sc_nMessageTrackInvalidate, GetCurrentPointIndex() );

					return false;
				}
			}

			if (!IsCurrentPointValid())
				return false;

			int nCurrentPoint = m_IsFirstResize ? 2 : GetCurrentPointIndex();

			if ( nCurrentPoint == 9 || nCurrentPoint == GetCount() - 1 || GetAsyncKeyState ( VK_CONTROL ) )
			{
				OffsetByDouble(dX - GetCurrentPoint().dX, dY - GetCurrentPoint().dY);

				if ( false == IsTrackChild () )
					SendMessage(Constants::sc_nMessageTrackPointProcessAll, GetCurrentPointIndex());
			}
			else
			{
				// correct rectangle points
				double dx, dy;
				double eX1, eX2, eY1, eY2;

				double CharSize		=	(double)m_FontSize * (double)GetDeviceCaps( GetWindowDC (NULL), LOGPIXELSY ) / 72.0;
				double CharSizeRotX	=	CharSize;
				double CharSizeRotY	=	CharSize;

				if (nCurrentPoint <= 8)
				{	
					eX1 = 1.0; eY1 = 0.0;
					eX2 = 0.0; eY2 = 1.0;

					double mAngle = Geometry::GetAngle( GetAt(9).dX, GetAt(9).dY, GetAt(8).dX, GetAt(8).dY ) + M_PI / 2.0;
					Geometry::RotatePoint( eX1, eY1, 0.0, 0.0, mAngle );
					Geometry::RotatePoint( eX2, eY2, 0.0, 0.0, mAngle );

					Geometry::RotatePoint ( CharSizeRotX, CharSizeRotY, 0.0, 0.0, mAngle );
				}

				if (nCurrentPoint == 0)
				{
					dX -= GetAt(0).dX;
					dY -= GetAt(0).dY;

					dx = dX*eX1 + dY*eY1;
					dy = dX*eX2 + dY*eY2;

					GetAt(0).dX += dX;
					GetAt(0).dY += dY;
					GetAt(1).dX += eX2*dy;
					GetAt(1).dY += eY2*dy;
					GetAt(3).dX += eX1*dx;
					GetAt(3).dY += eY1*dx;
				}
				else if (nCurrentPoint == 1)
				{
					dX -= GetAt(1).dX;
					dY -= GetAt(1).dY;

					dx = dX*eX1 + dY*eY1;
					dy = dX*eX2 + dY*eY2;

					GetAt(1).dX += dX;
					GetAt(1).dY += dY;
					GetAt(0).dX += eX2*dy;
					GetAt(0).dY += eY2*dy;
					GetAt(2).dX += eX1*dx;
					GetAt(2).dY += eY1*dx;

				}
				else if (nCurrentPoint == 2)
				{
					dX -= GetAt(2).dX;
					dY -= GetAt(2).dY;

					dx = dX*eX1 + dY*eY1;
					dy = dX*eX2 + dY*eY2;

					GetAt(2).dX += dX;
					GetAt(2).dY += dY;
					GetAt(1).dX += eX1*dx;
					GetAt(1).dY += eY1*dx;
					GetAt(3).dX += eX2*dy;
					GetAt(3).dY += eY2*dy;
				}
				else if (nCurrentPoint == 3)
				{
					dX -= GetAt(3).dX;
					dY -= GetAt(3).dY;

					dx = dX*eX1 + dY*eY1;
					dy = dX*eX2 + dY*eY2;

					GetAt(3).dX += dX;
					GetAt(3).dY += dY;
					GetAt(0).dX += eX1*dx;
					GetAt(0).dY += eY1*dx;
					GetAt(2).dX += eX2*dy;
					GetAt(2).dY += eY2*dy;
				}
				else if (nCurrentPoint == 4)
				{
					dX -= GetAt(4).dX;
					dY -= GetAt(4).dY;

					dx = dX*eX1 + dY*eY1;
					dy = dX*eX2 + dY*eY2;

					GetAt(4).dX += dX;
					GetAt(4).dY += dY;
					GetAt(0).dX += eX2*dy;
					GetAt(0).dY += eY2*dy; 
					GetAt(1).dX += eX2*dy;
					GetAt(1).dY += eY2*dy;
				}
				else if (nCurrentPoint == 5)
				{
					dX -= GetAt(5).dX;
					dY -= GetAt(5).dY;

					dx = dX*eX1 + dY*eY1;
					dy = dX*eX2 + dY*eY2;

					GetAt(5).dX += dX;
					GetAt(5).dY += dY;
					GetAt(1).dX += eX1*dx;
					GetAt(1).dY += eY1*dx;
					GetAt(2).dX += eX1*dx;
					GetAt(2).dY += eY1*dx;					
				}
				else if (nCurrentPoint == 6)
				{
					dX -= GetAt(6).dX;
					dY -= GetAt(6).dY;

					dx = dX*eX1 + dY*eY1;
					dy = dX*eX2 + dY*eY2;

					GetAt(6).dX += dX;
					GetAt(6).dY += dY;
					GetAt(2).dX += eX2*dy;
					GetAt(2).dY += eY2*dy; 
					GetAt(3).dX += eX2*dy;
					GetAt(3).dY += eY2*dy;
				}
				else if (nCurrentPoint == 7)
				{
					dX -= GetAt(7).dX;
					dY -= GetAt(7).dY;

					dx = dX*eX1 + dY*eY1;
					dy = dX*eX2 + dY*eY2;

					GetAt(7).dX += dX;
					GetAt(7).dY += dY;
					GetAt(0).dX += eX1*dx;
					GetAt(0).dY += eY1*dx;
					GetAt(3).dX += eX1*dx;
					GetAt(3).dY += eY1*dx;	
				}				
				else if (nCurrentPoint == 8)
				{
					double baseAngle = Geometry::GetAngle( GetAt(9).dX, GetAt(9).dY, dX, dY );
					StickAngle( baseAngle, m_StickAngle );
					double mAngle	 = baseAngle - Geometry::GetAngle( GetAt(9).dX, GetAt(9).dY, GetAt(8).dX, GetAt(8).dY );

					RotateToAlpha ( mAngle );
				}

				UpdateMinimizeRegion	( nCurrentPoint );
				UpdateScrollingText		( );

				// recompute centers
				if (nCurrentPoint >= 0 && nCurrentPoint < 8)
				{
					GetAt(4).Create( 0.5*( GetAt(0).dX + GetAt(1).dX ), 0.5*( GetAt(1).dY + GetAt(0).dY ) );
					GetAt(5).Create( 0.5*( GetAt(1).dX + GetAt(2).dX ), 0.5*( GetAt(1).dY + GetAt(2).dY ) );
					GetAt(6).Create( 0.5*( GetAt(3).dX + GetAt(2).dX ), 0.5*( GetAt(3).dY + GetAt(2).dY ) );
					GetAt(7).Create( 0.5*( GetAt(3).dX + GetAt(0).dX ), 0.5*( GetAt(3).dY + GetAt(0).dY ) );

					double vX = GetAt(4).dX - GetAt(6).dX;
					double vY = GetAt(4).dY - GetAt(6).dY;

					double length = Geometry::GetLength( GetAt(4).dX, GetAt(4).dY,GetAt(6).dX, GetAt(6).dY );
					if( length < 1 )
					{
						vX = -eX2; 
						vY = -eY2;
					}
					else
					{
						vX /= length;
						vY /= length;
					}

					GetAt(8).dX = GetAt(4).dX + vX * 30;
					GetAt(8).dY = GetAt(4).dY + vY * 30;
					GetAt(9).Create(0.5*(GetAt(0).dX + GetAt(2).dX), 0.5*( GetAt(2).dY + GetAt(0).dY ) );

					SplitString();				
				}

				if ( false == IsTrackChild () )
					SendMessage(Constants::sc_nMessageTrackPointProcess, GetCurrentPointIndex());
			}

			return true;
		}

		bool IMouseButtonHandlers::OnLButtonDown	( int MouseX, int MouseY )	
		{ 
			m_bSetCaretAtHome	=	true;

			int CaretPosition = PosOnMouse ( (int)MapToDataX( MouseX ), (int)MapToDataY( MouseY ) );

			m_bSetCaretAtHome	=	false;

			if ( CaretPosition != -1 && m_bEditingModeEnabled ) 
			{
				m_bTextAllSelect	=	false;		//	CTRL + A - выделение всего текста

				m_bCaretAtHome		=	false;

				if ( GetAsyncKeyState ( VK_SHIFT ) < 0 )
				{
					m_SelectEnd		=	CaretPosition;
				}
				else
				{
					m_SelectStart	=	CaretPosition;
				}

				m_CurrPos			=	CaretPosition;
				m_IsSelect			=	true;
				m_IsTransfom		=	false;

				// it's for repaint
				OnMouseMove( MouseX, MouseY );

				return true;
			}

			int nTrackIndex = GetPointUnderCursor( MouseX, MouseY );
			if ( -1 == nTrackIndex )
				return false;

			if( nTrackIndex >= 0 && nTrackIndex <= 9 )
				m_IsTransfom = true;

			if( nTrackIndex == GetCount() - 1 )
				GetAt(nTrackIndex).Create( MapToDataX( MouseX ), MapToDataY( MouseY ) );

			if( -1 == GetPointUnderCursor ( MouseX, MouseY ) )
				EndEditText();

			if ( m_bBlockResize && nTrackIndex >=0 && nTrackIndex < 8 )
				nTrackIndex = -1;

			if( m_AutoSize && nTrackIndex >=0 && nTrackIndex < 8 )
				nTrackIndex = -1;

			SetCurrentPointIndex( nTrackIndex );

			if ( false == IsTrackChild () )
				SendMessage( Constants::sc_nMessageTrackPointBegin, nTrackIndex );

			return true;
		}

		bool IMouseButtonHandlers::OnLButtonUp		( int MouseX, int MouseY )		
		{
			m_IsFirstResize = false;

			if( GetAsyncKeyState( VK_SHIFT ) >= 0 ) 
			{
				if( m_IsSelect )
				{
					int tmpPos = PosOnMouse( (int)MapToDataX( MouseX ), (int)MapToDataY( MouseY ) );
					if( tmpPos != -1 && m_bEditingModeEnabled ) 
					{
						m_CurrPos		= tmpPos;
						m_SelectEnd 	= tmpPos;
					}
					m_IsSelect		= false;
				}
			}
			if (!IsCurrentPointValid())
				return false;

			UpdateResizeShapeToFitText	( );
			UpdateScrollingText			( );

			if ( false == IsTrackChild () )
				SendMessage( Constants::sc_nMessageTrackPointEnd, GetCurrentPointIndex() );

			SetCurrentPointIndex(-1);

			m_IsTransfom = false;

			return true;
		}

		bool IMouseButtonHandlers::OnRButtonDown	( int MouseX, int MouseY )		
		{
			return false; 
		}

		bool IMouseButtonHandlers::OnRButtonUp		( int MouseX, int MouseY )		
		{
			return false; 
		}

		bool IMouseButtonHandlers::OnMButtonDown	( int MouseX, int MouseY )	
		{
			return false; 
		}

		bool IMouseButtonHandlers::OnMButtonUp		( int MouseX, int MouseY )			
		{
			return false; 
		}


	
	private:

		inline bool DrawNotAutoFitText ()	// текст автоматически выравнивается по ширине
		{
			long TranslateInd	=	0;
			Structures::RECTD TextRect;

			if ( DrawTrackLines ( TranslateInd, TextRect ) )
			{
				if ( m_bEditingModeEnabled )
				{
					if ( m_arrStrings.GetSize() )
					{
						Gdiplus::Font*			pFont			=	NULL;
						Gdiplus::FontFamily*	pFontFamily		=	NULL;
						Gdiplus::StringFormat*	pStringFormat	=	NULL;
						int FontStyle							=	Gdiplus::FontStyleRegular;

						if ( false == GetFontObjects ( &pFont, &pFontFamily, &pStringFormat, FontStyle ) )
							return FALSE;

						Gdiplus::PointF Bounding;
						Bounding.X	=	(float) ( Geometry::GetLength ( (double)GetAt(0).nX, (double)GetAt(0).nY, (double)GetAt(1).nX, (double)GetAt(1).nY ) );
						Bounding.Y	=	(float) ( Geometry::GetLength ( (double)GetAt(0).nX, (double)GetAt(0).nY, (double)GetAt(3).nX, (double)GetAt(3).nY ) );

						double BoundAngle	=	Geometry::GetAngle ( GetAt(9).dX, GetAt(9).dY, GetAt(8).dX, GetAt(8).dY );

						float Angle			=	(float) grad  ( BoundAngle + M_PI_2 );

						Gdiplus::PointF Move;
						Move.X				=	(float) GetAt ( TranslateInd ).nX;
						Move.Y				=	(float) GetAt ( TranslateInd ).nY;

						Gdiplus::Graphics graphics ( m_hDC );

						if ( Gdiplus::Ok == graphics.GetLastStatus () )
						{
							DrawEditRotateText ( &graphics, m_nStringAlignmentVertical, Angle, Move, Bounding );

							DrawRotateTextSelect ( graphics, m_hDC, m_arrStrings, (WCHAR*)&m_String, m_SelectStart, m_SelectEnd, *m_oFont.GetFont(),
								*m_oFormat.GetStringFormat (), m_nStringAlignmentVertical, BoundAngle, TextRect, Move.X, Move.Y, Bounding.X, Bounding.Y );

							DrawCaret ( graphics, m_arrStrings, *m_oFormat.GetStringFormat (), m_nStringAlignmentVertical, m_CurrPos, BoundAngle, TextRect );
						}
					}
				}

				return true;
			}

			return false;
		}


		inline bool DrawResizeShapeToFitText ()		// текст автоматически выравнивает высоту по текущей ширине (форматирование для текста только по горизонтали)
		{
			long TranslateInd	=	0;
			Structures::RECTD TextRect;

			if ( DrawTrackLines ( TranslateInd, TextRect ) )
			{
				if ( m_bEditingModeEnabled )
				{
					if ( m_arrStrings.GetSize() )
					{
						Gdiplus::Font*			pFont			=	NULL;
						Gdiplus::FontFamily*	pFontFamily		=	NULL;
						Gdiplus::StringFormat*	pStringFormat	=	NULL;
						int FontStyle							=	Gdiplus::FontStyleRegular;

						if ( false == GetFontObjects ( &pFont, &pFontFamily, &pStringFormat, FontStyle ) )
							return FALSE;

						Gdiplus::PointF Bounding;
						Bounding.X	=	(float) ( Geometry::GetLength ( (double)GetAt(0).nX, (double)GetAt(0).nY, (double)GetAt(1).nX, (double)GetAt(1).nY ) );
						Bounding.Y	=	(float) ( Geometry::GetLength ( (double)GetAt(0).nX, (double)GetAt(0).nY, (double)GetAt(3).nX, (double)GetAt(3).nY ) );

						double BoundAngle	=	Geometry::GetAngle ( GetAt(9).dX, GetAt(9).dY, GetAt(8).dX, GetAt(8).dY );

						float Angle			=	(float) grad  ( BoundAngle + M_PI_2 );

						Gdiplus::PointF Move;
						Move.X				=	(float) GetAt ( TranslateInd ).nX;
						Move.Y				=	(float) GetAt ( TranslateInd ).nY;

						Gdiplus::Graphics graphics ( m_hDC );

						if ( Gdiplus::Ok == graphics.GetLastStatus () )
						{
							DrawEditRotateText ( &graphics, 0/*m_nStringAlignmentVertical*/, Angle, Move, Bounding );

							DrawRotateTextSelect ( graphics, m_hDC, m_arrStrings, (WCHAR*)&m_String, m_SelectStart, m_SelectEnd, *m_oFont.GetFont(),
								*m_oFormat.GetStringFormat (), 0 /*m_nStringAlignmentVertical*/, BoundAngle, TextRect, Move.X, Move.Y, Bounding.X, Bounding.Y );

							DrawCaret ( graphics, m_arrStrings, *m_oFormat.GetStringFormat (), 0 /*m_nStringAlignmentVertical*/, m_CurrPos, BoundAngle, TextRect );
						}
					}
				}

				return true;
			}

			return false;
		}

		inline bool DrawScrollingText ()
		{
			long TranslateInd	=	0;
			Structures::RECTD TextRect;

			if ( DrawTrackLines ( TranslateInd, TextRect ) )
			{
				if ( m_bEditingModeEnabled )
				{
					if ( m_arrStrings.GetSize() )
					{
						Gdiplus::Font*			pFont			=	NULL;
						Gdiplus::FontFamily*	pFontFamily		=	NULL;
						Gdiplus::StringFormat*	pStringFormat	=	NULL;
						int FontStyle							=	Gdiplus::FontStyleRegular;

						if ( false == GetFontObjects ( &pFont, &pFontFamily, &pStringFormat, FontStyle ) )
							return FALSE;

						Gdiplus::PointF Bounding;
						Bounding.X	=	(float) ( Geometry::GetLength ( (double)GetAt(0).nX, (double)GetAt(0).nY, (double)GetAt(1).nX, (double)GetAt(1).nY ) );
						Bounding.Y	=	(float) ( Geometry::GetLength ( (double)GetAt(0).nX, (double)GetAt(0).nY, (double)GetAt(3).nX, (double)GetAt(3).nY ) );

						double BoundAngle	=	Geometry::GetAngle ( GetAt(9).dX, GetAt(9).dY, GetAt(8).dX, GetAt(8).dY );

						float Angle			=	(float) grad  ( BoundAngle + M_PI_2 );

						Gdiplus::PointF Move;
						Move.X				=	(float) GetAt ( TranslateInd ).nX;
						Move.Y				=	(float) GetAt ( TranslateInd ).nY;

						Gdiplus::Graphics graphics ( m_hDC );

						if ( Gdiplus::Ok == graphics.GetLastStatus () )
						{
							DrawScrollEditRotateText ( &graphics, 0/*m_nStringAlignmentVertical*/, Angle, Move, Bounding ); 

							DrawRotateTextSelect ( graphics, m_hDC, m_arrStrings, (WCHAR*)&m_String, m_SelectStart, m_SelectEnd, *m_oFont.GetFont(),
								*m_oFormat.GetStringFormat (), 0 /*m_nStringAlignmentVertical*/, BoundAngle, TextRect, Move.X, Move.Y, Bounding.X, Bounding.Y );

							DrawCaret ( graphics, m_arrStrings, *m_oFormat.GetStringFormat (), 0 /*m_nStringAlignmentVertical*/, m_CurrPos, BoundAngle, TextRect );
						}
					}
				}

				return true;
			}

			return false;
		}


		inline bool DrawTrackLines ( long& TranslateInd, Structures::RECTD& rectD )
		{
			if ( GetCount () < 10 || NULL == m_hDC )
				return false;

			POINT* pPoints = new POINT[10];
			if ( NULL == pPoints )
				return FALSE;

			GetPointArray ( pPoints, 10 );

			double PlaneX	=	0.0;
			double PlaneY	=	0.0;

			double BoundAngle	=	Geometry::GetAngle ( GetAt(9).dX, GetAt(9).dY, GetAt(8).dX, GetAt(8).dY );

			double CoS			=	cos ( - BoundAngle + M_PI_2 );
			double SiN			=	sin ( - BoundAngle + M_PI_2 );

			rectD.left		=	GetAt(0).nX;
			rectD.top		=	GetAt(0).nY;
			rectD.right		=	GetAt(2).nX;
			rectD.bottom	=	GetAt(2).nY;

			PlaneX			=	GetAt(0).nX - GetAt(9).nX;
			PlaneY			=	GetAt(0).nY - GetAt(9).nY;				
			rectD.left		=	GetAt(9).nX + PlaneX * CoS - PlaneY * SiN;
			rectD.top		=	GetAt(9).nY + PlaneX * SiN + PlaneY * CoS;	

			PlaneX			=	GetAt(2).nX - GetAt(9).nX;
			PlaneY			=	GetAt(2).nY - GetAt(9).nY;
			rectD.right		=	GetAt(9).nX + PlaneX * CoS - PlaneY * SiN;
			rectD.bottom	=	GetAt(9).nY + PlaneX * SiN + PlaneY * CoS;

			float BoundWidth	=	(float) ( Geometry::GetLength ( (double)GetAt(0).nX, (double)GetAt(0).nY, (double)GetAt(1).nX, (double)GetAt(1).nY ) );
			float BoundHeight	=	(float) ( Geometry::GetLength ( (double)GetAt(0).nX, (double)GetAt(0).nY, (double)GetAt(3).nX, (double)GetAt(3).nY ) );

			double AX			=	( GetAt(4).nX - GetAt(0).nX ) / BoundWidth	* 0.5;
			double AY 			=	( GetAt(4).nY - GetAt(0).nY ) / BoundHeight * 0.5;
			double BX 			=	( GetAt(4).nX - GetAt(9).nX ) / BoundWidth	* 0.5;
			double BY 			=	( GetAt(4).nY - GetAt(9).nY ) / BoundHeight * 0.5;

			if ( AX * BY - AY * BX <= 0 )
			{
				TranslateInd	=	0;
			}
			else
			{
				TranslateInd	=	1;
			}

			if ( rectD.right < rectD.left ) 
			{ 
				double tmpRight		= rectD.right; 
				rectD.right			= rectD.left;
				rectD.left			= tmpRight;
			}

			if ( rectD.top > rectD.bottom ) 
			{ 
				double tmpBottom	= rectD.bottom; 
				rectD.bottom		= rectD.top;
				rectD.top			= tmpBottom;
			}	

			int Ind = (	Geometry::GetLength(GetAt(8).nX, GetAt(8).nY, GetAt(4).nX, GetAt(4).nY ) > 
				Geometry::GetLength(GetAt(8).nX, GetAt(8).nY, GetAt(6).nX, GetAt(6).nY ) ) ? 6 : 4;

			Rendering::Gdi::DrawPolyline ( m_hDC, pPoints, 4, TRUE );
			Rendering::Gdi::DrawLine ( m_hDC, GetAt(8).nX, GetAt(8).nY, GetAt(Ind).nX, GetAt(Ind).nY );
			Rendering::Gdi::DrawRotatableSelector ( m_hDC, GetAt(8).nX, GetAt(8).nY, 5 );

			if ( pPoints )
			{
				delete [] pPoints; 
				pPoints = NULL;
			}

			return true;
		}

		inline void DrawEditRotateText ( Gdiplus::Graphics* pGraphics, int StringAlignmentVertical, float Angle, Gdiplus::PointF Move, Gdiplus::PointF Bounding )
		{
			if ( 0 == m_arrStrings.GetSize() || Gdiplus::Ok != pGraphics->GetLastStatus () || false == m_bEnableDrawText )
				return;

			Gdiplus::Font*			pFont	=	m_oFont.GetFont();
			Gdiplus::Brush*			pBrush	=	m_oBrush.GetBrush ( &m_oTextureManager, &Gdiplus::RectF ( 0.0f, 0.0f, Bounding.X, Bounding.Y ) );
			Gdiplus::StringFormat*	pFormat	=	m_oFormat.GetStringFormat ();

			RectF BoundClipZone ( 0.0f, 0.0f, Bounding.X, Bounding.Y );

			Gdiplus::StringFormat* pStringFormat = pFormat->Clone ( );
			if ( NULL == pStringFormat || NULL == pFont || NULL == pBrush )
				return;

			pStringFormat->SetFormatFlags ( StringFormatFlagsNoWrap );

			GraphicsContainer  graphicsContainer;

			graphicsContainer	=	pGraphics->BeginContainer();

			pGraphics->TranslateTransform	( Move.X, Move.Y );
			pGraphics->RotateTransform		( Angle );

			pGraphics->SetClip				( BoundClipZone );
			pGraphics->SetTextRenderingHint	( TextRenderingHintAntiAlias );

			pGraphics->SetTextRenderingHint	( Gdiplus::TextRenderingHintAntiAlias );
			pGraphics->SetSmoothingMode		( SmoothingModeAntiAlias );
			//pGraphics->SetInterpolationMode	( Gdiplus::InterpolationModeBilinear );
			pGraphics->SetInterpolationMode	( Gdiplus::InterpolationModeDefault );

#ifdef _DEBUG
			// pGraphics->DrawRectangle ( new Gdiplus::Pen ( Gdiplus::Color ( 192, 255, 255, 0 ), 2 ), BoundClipZone );
#endif

			float FontHeight = (float)Rendering::Utils::GetFontHeight ( pFont );

			switch ( StringAlignmentVertical )
			{
			case 0: // Vertical Aligment Top
				{
					Gdiplus::RectF DrawRgn = Gdiplus::RectF ( 0.0f, 0.0f, Bounding.X, FontHeight );

					for ( int i = 0; i < m_arrStrings.GetSize(); ++i )
					{
						DrawRgn.Y =  i * FontHeight;
						
						if ( DrawRgn.Y > Bounding.Y  )
							break;

						m_oEdgeText.Draw ( pGraphics, pFont, pStringFormat, _bstr_t ( m_arrStrings [i] ), -1, DrawRgn );
						pGraphics->DrawString ( Rendering::TextRendering::NormNonBreakingSpaces ( m_arrStrings[i] ), -1, pFont, DrawRgn, pStringFormat, pBrush );
					}
					break;
				}
			case 1: // Vertical Aligment Middle
				{
					float OffSetVertical = static_cast<float> ( m_arrStrings.GetSize() * FontHeight - Bounding.Y ) * 0.5f;

					Gdiplus::RectF DrawRgn = Gdiplus::RectF ( 0.0f, 0.0f, Bounding.X, FontHeight );

					for ( int i = 0; i < m_arrStrings.GetSize(); ++i )
					{
						DrawRgn.Y =  i * FontHeight - OffSetVertical;

						if ( DrawRgn.Y > Bounding.Y )
							break;

						m_oEdgeText.Draw ( pGraphics, pFont, pStringFormat, _bstr_t ( m_arrStrings [i] ), -1, DrawRgn );
						pGraphics->DrawString ( Rendering::TextRendering::NormNonBreakingSpaces ( m_arrStrings [i] ), -1, pFont, DrawRgn, pStringFormat, pBrush );
					}
					break;		
				}

			case 2: // Vertical Aligment Bottom
				{
					float OffSetVertical = static_cast<float> ( m_arrStrings.GetSize() * FontHeight - Bounding.Y );

					Gdiplus::RectF DrawRgn = Gdiplus::RectF ( 0.0f, 0.0f, Bounding.X, FontHeight );
					for ( int i = 0; i < m_arrStrings.GetSize(); ++i )
					{
						DrawRgn.Y =  i * FontHeight - OffSetVertical;

						if ( DrawRgn.Y > Bounding.Y )
							break;

						m_oEdgeText.Draw ( pGraphics, pFont, pStringFormat, _bstr_t ( m_arrStrings [i] ), -1, DrawRgn );
						pGraphics->DrawString ( Rendering::TextRendering::NormNonBreakingSpaces ( m_arrStrings [i] ), -1, pFont, DrawRgn, pStringFormat, pBrush );
					}
					break;		
				}
			default:
				{
					Gdiplus::RectF DrawRgn = Gdiplus::RectF ( 0.0f, 0.0f, Bounding.X, FontHeight );

					for ( int i = 0; i < m_arrStrings.GetSize(); ++i )
					{
						DrawRgn.Y =  i * FontHeight;

						if ( DrawRgn.Y > Bounding.Y )
							break;

						m_oEdgeText.Draw ( pGraphics, pFont, pStringFormat, _bstr_t ( m_arrStrings [i] ), -1, DrawRgn );
						pGraphics->DrawString ( Rendering::TextRendering::NormNonBreakingSpaces ( m_arrStrings[i] ), -1, pFont, DrawRgn, pStringFormat, pBrush );
					}
					break;
				}
			}

			pGraphics->EndContainer ( graphicsContainer );

			RELEASEOBJECT ( pStringFormat );
		}

		inline void DrawRotateTextSelect ( Graphics& graphics, HDC hDC, CSimpleArray<CStringW> &lines, WCHAR* string, int selectStart, int selectEnd, Gdiplus::Font& font, Gdiplus::StringFormat &stringFormat, int fontAlignW, double Angle, Structures::RECTD trackingRect, float TranslateX, float TranslateY,	float BoundWidth, float BoundHeight )
		{
			if ( selectStart == selectEnd || Gdiplus::Ok != graphics.GetLastStatus () ) 
				return;

			struct Select
			{
				double  posStart;
				double  posEnd;
				int		strIndex;
			};

			if( selectStart > selectEnd )
			{
				int tmpSelect	= selectStart;	
				selectStart		= selectEnd;
				selectEnd		= tmpSelect;
			}

			CSimpleArray<Select> selectLines;

			double centerX = trackingRect.left + ( trackingRect.right - trackingRect.left ) / 2.0;
			double centerY = trackingRect.top  + ( trackingRect.bottom - trackingRect.top ) / 2.0;

			double dWidth		= trackingRect.GetWidth();
			double dHeight		= trackingRect.GetHeight();
			double dWidth_2		= dWidth  / 2.0;
			double dHeight_2	= dHeight / 2.0;

			Gdiplus::RectF	rectF, selectRectStart, selectRectEnd, selectRect;

			if( dWidth < font.GetSize() / 2.0f )
				return;

			stringFormat.SetFormatFlags ( StringFormatFlagsMeasureTrailingSpaces );

			double realHeight = Rendering::Utils::GetFontHeight( &font );

			int		lineStart	= 0;
			int		idx			= 0;
			bool	stop		= false;
			Select	selectPos;
			while( selectEnd > 0 )
			{
				if( idx > lines.GetSize() - 1 || stop ) break;
				if( selectStart > lines[idx].GetLength() ) 
				{
					selectStart -= lines[idx].GetLength();
					selectEnd   -= lines[idx].GetLength();
					lineStart++;
					idx++;
					continue;
				}
				if( selectStart > 0 )
				{
					graphics.MeasureString( lines[idx], selectStart, &font, rectF, &stringFormat, &selectRect );
					selectPos.posStart = selectRect.Width;
				}
				else
				{
					selectPos.posStart = font.GetSize() / 8.0;
				}

				if( selectEnd <= lines[idx].GetLength() )
				{
					graphics.MeasureString( lines[idx], selectEnd, &font, rectF, &stringFormat, &selectRect );
					selectPos.posEnd = selectRect.Width;
					stop = true;
				}
				else
				{
					graphics.MeasureString( lines[idx], -1, &font, rectF, &stringFormat, &selectRect );
					selectPos.posEnd = selectRect.Width;
					selectEnd  -= lines[idx].GetLength();
					selectStart = 0;
				}

				selectPos.strIndex = idx;
				selectLines.Add( selectPos ); 
				idx++;
			}

			int countSpace = 1;
			for( int i = 0; i < lines.GetSize(); ++i )
			{
				BSTR line = lines[i].AllocSysString();
				WCHAR* currString = line;
				while( *currString++ )
				{
					if( *currString == '\n' ) countSpace++;
				}
				SysFreeString( line );
			}

			RectF trackingRectF( (Gdiplus::REAL)-dWidth_2, (Gdiplus::REAL)-dHeight_2, (Gdiplus::REAL)trackingRect.GetWidth(), (Gdiplus::REAL)trackingRect.GetHeight() );

			LOGBRUSH brushSolid	=	{ BS_SOLID, RGB ( 0, 0, 0 ), 0 };
			HPEN selectPen		=	ExtCreatePen( PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_FLAT, (DWORD)realHeight, &brushSolid, 0, NULL );

			GraphicsContainer  graphicsContainer;

			graphicsContainer	=	graphics.BeginContainer();
			//graphics.TranslateTransform	( (Gdiplus::REAL) centerX, (Gdiplus::REAL) centerY );
			graphics.TranslateTransform	( TranslateX, TranslateY );

			graphics.RotateTransform	( (Gdiplus::REAL) grad ( Angle + M_PI_2 ) );
			graphics.SetClip			( trackingRectF );
			graphics.SetSmoothingMode	( SmoothingModeHighQuality );

			Structures::POINTD center	=	Structures::POINTD( centerX, centerY );
			double angle				=	Angle + M_PI_2;

			for( int i = 0; i < selectLines.GetSize(); ++i )
			{
				if ( stringFormat.GetAlignment() == StringAlignmentCenter )
				{
					CStringW trimString = lines [ selectLines[i].strIndex ];
					trimString.TrimRight();
					//BSTR str = trimString.AllocSysString();
					graphics.MeasureString( trimString, -1, &font, rectF, &stringFormat, &selectRect );
					selectLines[i].posStart += selectRect.X + dWidth_2;
					selectLines[i].posEnd	+= selectRect.X + dWidth_2;
					//SysFreeString( str );
				}
				else if( stringFormat.GetAlignment() == StringAlignmentFar )
				{
					CStringW trimString = lines [ selectLines[i].strIndex ];
					trimString.TrimRight();
					//BSTR str = trimString.AllocSysString();
					graphics.MeasureString( trimString, -1, &font, rectF, &stringFormat, &selectRect );
					selectLines[i].posStart += selectRect.X + dWidth;
					selectLines[i].posEnd	+= selectRect.X + dWidth;
					//SysFreeString( str );
				}				

				double displaceByY = lines.GetSize() * realHeight - dHeight;

				Structures::POINTD startSelection;
				Structures::POINTD endSelection;

				switch( fontAlignW )
				{
				case byTop: // FontAlign
					{
						startSelection.x = trackingRect.left + selectLines[i].posStart - font.GetSize() / 8.0;
						startSelection.y = trackingRect.top  + realHeight / 2.0 + realHeight * (i+lineStart)  - realHeight * m_nScrollLine;
						endSelection.x	 = trackingRect.left + selectLines[i].posEnd - font.GetSize() / 8.0;
						endSelection.y	 = trackingRect.top  + realHeight / 2.0 + realHeight * (i+lineStart)  - realHeight * m_nScrollLine;
						//if( endSelection.y > trackingRect.bottom ) return;//old clamp
						break;
					}
				case byMiddle: // FontAlign
					{							
						startSelection.x = trackingRect.left + selectLines[i].posStart - font.GetSize() / 8.0;
						startSelection.y = trackingRect.top  + ( -displaceByY + realHeight ) / 2.0 + realHeight * (REAL)(i+lineStart)  - realHeight * m_nScrollLine;
						endSelection.x	 = trackingRect.left + selectLines[i].posEnd - font.GetSize() / 8.0;
						endSelection.y	 = trackingRect.top  + ( -displaceByY + realHeight ) / 2.0 + realHeight * (REAL)(i+lineStart)  - realHeight * m_nScrollLine;
						//if( endSelection.y < trackingRect.top )		continue;//old clamp
						//if( endSelection.y > trackingRect.bottom )	return;//old clamp
						break;
					}
				case byBottom: // FontAlign
					{
						startSelection.x = trackingRect.left + selectLines[i].posStart - font.GetSize() / 8.0;
						startSelection.y = trackingRect.top - displaceByY + realHeight / 2.0 + realHeight * (REAL)(i+lineStart)  - realHeight * m_nScrollLine;
						endSelection.x	 = trackingRect.left + selectLines[i].posEnd - font.GetSize() / 8.0;
						endSelection.y	 = trackingRect.top - displaceByY + realHeight / 2.0 + realHeight * (REAL)(i+lineStart)  - realHeight * m_nScrollLine;
						//if( endSelection.y < trackingRect.top )		continue;//old clamp
						break;
					}
				default:
					{
						startSelection.x = trackingRect.left + selectLines[i].posStart - font.GetSize() / 8.0;
						startSelection.y = trackingRect.top  + realHeight / 2.0 + realHeight * (i+lineStart)  - realHeight * m_nScrollLine;
						endSelection.x	 = trackingRect.left + selectLines[i].posEnd - font.GetSize() / 8.0;
						endSelection.y	 = trackingRect.top  + realHeight / 2.0 + realHeight * (i+lineStart)  - realHeight * m_nScrollLine;
						//if( endSelection.y > trackingRect.bottom ) return; //old clamp
						break;
					}
				}

				Geometry::RotatePoint( startSelection,	center, angle );
				Geometry::RotatePoint( endSelection,	center, angle );

				Structures::POINTD leftTop		( trackingRect.left,	trackingRect.top );
				Structures::POINTD rightTop		( trackingRect.right,	trackingRect.top );
				Structures::POINTD rightBottom	( trackingRect.right,	trackingRect.bottom );
				Structures::POINTD leftBottom	( trackingRect.left,	trackingRect.bottom );

				Geometry::RotatePoint( leftTop,		center, angle );
				Geometry::RotatePoint( rightTop,	center, angle );
				Geometry::RotatePoint( rightBottom, center, angle );
				Geometry::RotatePoint( leftBottom,	center, angle );

				POINT points[4];
				points[0].x = (long)leftTop.x;
				points[0].y = (long)leftTop.y;
				points[1].x = (long)rightTop.x;
				points[1].y = (long)rightTop.y;
				points[2].x = (long)rightBottom.x;
				points[2].y = (long)rightBottom.y;
				points[3].x = (long)leftBottom.x;
				points[3].y = (long)leftBottom.y;

				HRGN clipRegn = CreatePolygonRgn ( points, 4, ALTERNATE );
				SelectClipRgn ( hDC, clipRegn );

				SelectObject( hDC, selectPen );
				int oldMode = SetROP2( hDC, R2_NOTXORPEN );
				MoveToEx( hDC, (int)startSelection.x, (int)startSelection.y , (LPPOINT) NULL );
				LineTo( hDC, (int)endSelection.x, (int)endSelection.y );
				SetROP2( hDC, oldMode );
				DeleteObject( selectPen );
				DeleteObject( clipRegn );
				SelectClipRgn( hDC, NULL );
			}
			graphics.EndContainer(graphicsContainer);

			selectLines.RemoveAll();
		}

		inline void DrawCaret ( Gdiplus::Graphics& graphics, CSimpleArray<CStringW> &lines, Gdiplus::StringFormat &stringFormat, int fontAlignW, int posCaret, double fontAngle, Structures::RECTD trackingRect )
		{
			RectF	orig, strBB, chBB, caretRect, allStringRect;
			double	displaceByY		= 0.0;
			double	displaceByX		= 0.0;

			double	centerX			= trackingRect.left + ( trackingRect.right - trackingRect.left ) / 2.0;
			double	centerY			= trackingRect.top  + ( trackingRect.bottom - trackingRect.top ) / 2.0;

			double	dWidth			= trackingRect.GetWidth();
			double	dHeight			= trackingRect.GetHeight();
			double	dWidth_2		= dWidth  / 2.0;
			double	dHeight_2		= dHeight / 2.0;

			bool	bEmtyString		= false;
			bool	isLastString	= false;

			//if( dWidth < font.GetSize() + font.GetSize() / 8.0 ) return;

			if( 0 == lines.GetSize() ) bEmtyString = true;

			Gdiplus::RectF	rectF, fontRect;
			Gdiplus::Pen	fillCaretColor	 ( Gdiplus::Color( 255,   0,   0,   0 ), 2.0 );
			Gdiplus::Pen	borderCaretColor ( Gdiplus::Color( 255, 255, 255, 255 ), 4.0 );

			int		internCaretPos	= posCaret;
			int		index			= 0;
			double	fontHeight		= Rendering::Utils::GetFontHeight( m_oFont.GetFont() );

			if( !bEmtyString )
			{
				for( index = 0; index < lines.GetSize(); index++ )
				{
					if ( internCaretPos <= lines[index].GetLength() )
						break;
					internCaretPos -= lines[index].GetLength();
				}

				int LineLength	=	lines[index].GetLength();
				WCHAR SymbolEnd	=	lines[index][ LineLength - 1];

				if ( ( internCaretPos == LineLength || internCaretPos == LineLength - 1 ) && (index + 1) < lines.GetSize() && SymbolEnd == '\n' ) 
				{
					internCaretPos = 0;
					index++;
				}
				else if( ( internCaretPos == LineLength || internCaretPos == LineLength - 1 ) && (index + 1) == lines.GetSize() && SymbolEnd == '\n' )
				{
					isLastString = true;	
					internCaretPos = 0;
				}

				//if ( '\n' != SymbolEnd )
				{
					if ( ( internCaretPos == LineLength && m_bCaretAtHome )  )
					{
						++index;
						internCaretPos = 0;
					}
					else if ( 1 == internCaretPos && m_bCaretAtHome )
					{
					//	internCaretPos = 0;
					}
				}

				stringFormat.SetFormatFlags( StringFormatFlagsMeasureTrailingSpaces );

				//char buff[32];
				//sprintf( buff, "line num %d caret pos %d\n", index, internCaretPos );
				//::OutputDebugStringA( buff );

				CStringW trimString = lines[index];
				//trimString.TrimRight();
				trimString.Delete ( trimString.GetLength () - 2, 2 );
				//!!!!!!!!!!!
		
				graphics.MeasureString ( trimString, -1, m_oFont.GetFont(), orig, &stringFormat, &allStringRect );

				if ( internCaretPos == 0 )
				{
					if ( 0 == fontHeight )
					{
						graphics.MeasureString( Rendering::TextRendering::NormNonBreakingSpaces ( lines[0] ), -1, m_oFont.GetFont(), orig, &stringFormat, &allStringRect );	
					}

					caretRect = RectF ( 0.0f, 0.0f, m_oFont.GetFont()->GetSize() / 5.0f, (Gdiplus::REAL)fontHeight );
				}
				else
				{
					graphics.MeasureString ( trimString, internCaretPos, m_oFont.GetFont(), orig, &stringFormat, &caretRect );

					caretRect.Height = (Gdiplus::REAL) ( fontHeight );
				}
			}
			else
			{
				caretRect.X		 = 0;
				caretRect.Y		 = 0;
				caretRect.Width  = 5;
				caretRect.Height = (Gdiplus::REAL) ( fontHeight );
			}

			GraphicsContainer  graphicsContainer;
			graphicsContainer	=	graphics.BeginContainer();

			graphics.TranslateTransform ( (Gdiplus::REAL)centerX, (Gdiplus::REAL)centerY );
			RectF trackingRectF( (Gdiplus::REAL)-dWidth_2, (Gdiplus::REAL)-dHeight_2, 
				(Gdiplus::REAL)trackingRect.GetWidth(), (Gdiplus::REAL)trackingRect.GetHeight() );
			
			if ( stringFormat.GetAlignment() == StringAlignmentNear )
			{
				caretRect.Width += (Gdiplus::REAL) ( -dWidth_2 );
			}
			else if ( stringFormat.GetAlignment() == StringAlignmentCenter )
			{
				caretRect.Offset ( ( caretRect.Width - allStringRect.Width ) / 2.0f, 0.0f );
				if( !bEmtyString ) caretRect.Width += ( lines[index][0] == '\r' ) ? 0 : caretRect.X;
				if( isLastString ) caretRect.Width =	m_oFont.GetFont()->GetSize() / 5;
			}
			else if ( stringFormat.GetAlignment() == StringAlignmentFar )
			{
				if( bEmtyString )
					caretRect.Width += (Gdiplus::REAL) ( dWidth_2 - 5.0 );
				else
				{
					caretRect.Offset( caretRect.Width - allStringRect.Width, 0.0 );
					caretRect.Width += ( lines[index][0] == '\r' ) ? 
						(Gdiplus::REAL) ( dWidth_2 - m_oFont.GetFont()->GetSize() / 5 ) :
					(Gdiplus::REAL) ( caretRect.X + dWidth_2 );
				}

				if( isLastString ) 
					caretRect.Width = (Gdiplus::REAL) ( dWidth_2 );
			}

			displaceByY = lines.GetSize() * fontHeight;

			graphics.RotateTransform ( (Gdiplus::REAL)grad( fontAngle + M_PI_2 ) );
			graphics.SetClip( trackingRectF );

			switch( fontAlignW )
			{
			case byTop:
				{
					caretRect.Y			=	(Gdiplus::REAL) ( -dHeight_2 + ( isLastString ? index + 1 : index ) * fontHeight - fontHeight * m_nScrollLine );
					break;
				}
			case byMiddle:
				{	
					caretRect.Y			=	(Gdiplus::REAL) ( -displaceByY / 2.0 + ( isLastString ? index + 1 : index ) * fontHeight - fontHeight * m_nScrollLine );
					
					if ( bEmtyString ) 
						caretRect.Y		-=	(Gdiplus::REAL) ( fontHeight / 2.0 );

					break;
				}
			case byBottom:
				{
					caretRect.Y			=	(Gdiplus::REAL) ( dHeight_2 - displaceByY + ( isLastString ? index + 1 : index ) * fontHeight - fontHeight * m_nScrollLine );
					
					if ( bEmtyString ) 
						caretRect.Y		-=	(Gdiplus::REAL) ( fontHeight );

					break;
				}
			default:
				{
					caretRect.Y			=	(Gdiplus::REAL) ( -dHeight_2 + ( isLastString ? index + 1 : index ) * fontHeight - fontHeight * m_nScrollLine );
				}
			}

			graphics.DrawLine( &borderCaretColor, caretRect.Width - m_oFont.GetFont()->GetSize() / 8, caretRect.Y - 1, caretRect.Width - m_oFont.GetFont()->GetSize() / 8, caretRect.Y + caretRect.Height + 1);
			graphics.DrawLine( &fillCaretColor,   caretRect.Width - m_oFont.GetFont()->GetSize() / 8, caretRect.Y, caretRect.Width - m_oFont.GetFont()->GetSize() / 8, caretRect.Y + caretRect.Height );

			graphics.EndContainer( graphicsContainer );
		}

		inline void DrawScrollEditRotateText ( Gdiplus::Graphics* pGraphics, int StringAlignmentVertical, float Angle, Gdiplus::PointF Move, Gdiplus::PointF Bounding )
		{
			if ( 0 == m_arrStrings.GetSize() || Gdiplus::Ok != pGraphics->GetLastStatus () )
				return;

			Gdiplus::Font*			pFont	=	m_oFont.GetFont();
			Gdiplus::Brush*			pBrush	=	m_oBrush.GetBrush ( &m_oTextureManager, &Gdiplus::RectF ( 0.0f, 0.0f, Bounding.X, Bounding.Y ) );
			Gdiplus::StringFormat*	pFormat	=	m_oFormat.GetStringFormat ();

			RectF BoundClipZone ( 0.0f, 0.0f, Bounding.X, Bounding.Y );

			Gdiplus::StringFormat* pStringFormat = pFormat->Clone ( );
			if ( NULL == pStringFormat || NULL == pFont || NULL == pBrush )
				return;

			pStringFormat->SetFormatFlags ( StringFormatFlagsNoWrap );
		
			float FontHeight = (float)Rendering::Utils::GetFontHeight ( pFont );

			GraphicsContainer  graphicsContainer;

			graphicsContainer	=	pGraphics->BeginContainer();

			pGraphics->TranslateTransform	( Move.X, Move.Y );
			pGraphics->RotateTransform		( Angle );

			pGraphics->SetClip				( BoundClipZone );
			pGraphics->SetTextRenderingHint	( TextRenderingHintAntiAlias );

			pGraphics->SetTextRenderingHint	( Gdiplus::TextRenderingHintAntiAlias );
			pGraphics->SetSmoothingMode		( SmoothingModeAntiAlias );
			//pGraphics->SetInterpolationMode	( Gdiplus::InterpolationModeBilinear );
			pGraphics->SetInterpolationMode	( Gdiplus::InterpolationModeDefault );

#ifdef _DEBUG
			pGraphics->DrawRectangle ( new Gdiplus::Pen ( Gdiplus::Color ( 192, 255, 255, 0 ), 2 ), BoundClipZone );
#endif

			switch ( StringAlignmentVertical )
			{
			case 0: // Vertical Aligment Top
				{
					Gdiplus::RectF DrawRgn = Gdiplus::RectF ( 0.0f, 0.0f, Bounding.X, FontHeight );

					int Count = m_arrStrings.GetSize();

					for ( int i = m_nScrollLine; i < m_arrStrings.GetSize(); ++i )
					{
						DrawRgn.Y =  ( i - m_nScrollLine ) * FontHeight;

						if ( DrawRgn.Y > Bounding.Y )
							break;

						m_oEdgeText.Draw ( pGraphics, pFont, pStringFormat, _bstr_t ( m_arrStrings [ i ] ), -1, DrawRgn );
						pGraphics->DrawString ( Rendering::TextRendering::NormNonBreakingSpaces ( m_arrStrings[ i ] ), -1, pFont, DrawRgn, pStringFormat, pBrush );
					}
					break;
				}
			case 1: // Vertical Aligment Middle
				{
					float OffSetVertical = static_cast<float> ( m_arrStrings.GetSize() * FontHeight - Bounding.Y ) * 0.5f;

					Gdiplus::RectF DrawRgn = Gdiplus::RectF ( 0.0f, 0.0f, Bounding.X, FontHeight );

					for ( int i = 0; i < m_arrStrings.GetSize(); ++i )
					{
						DrawRgn.Y =  i * FontHeight - OffSetVertical;

						if ( DrawRgn.Y > Bounding.Y )
							break;

						m_oEdgeText.Draw ( pGraphics, pFont, pStringFormat, _bstr_t ( m_arrStrings [i] ), -1, DrawRgn );
						pGraphics->DrawString ( Rendering::TextRendering::NormNonBreakingSpaces ( m_arrStrings [i] ), -1, pFont, DrawRgn, pStringFormat, pBrush );
					}
					break;		
				}

			case 2: // Vertical Aligment Bottom
				{
					float OffSetVertical = static_cast<float> ( m_arrStrings.GetSize() * FontHeight - Bounding.Y );

					Gdiplus::RectF DrawRgn = Gdiplus::RectF ( 0.0f, 0.0f, Bounding.X, FontHeight );
					for ( int i = 0; i < m_arrStrings.GetSize(); ++i )
					{
						DrawRgn.Y =  i * FontHeight - OffSetVertical;

						if ( DrawRgn.Y > Bounding.Y )
							break;

						m_oEdgeText.Draw ( pGraphics, pFont, pStringFormat, _bstr_t ( m_arrStrings [i] ), -1, DrawRgn );
						pGraphics->DrawString ( Rendering::TextRendering::NormNonBreakingSpaces ( m_arrStrings [i] ), -1, pFont, DrawRgn, pStringFormat, pBrush );
					}
					break;		
				}
			default:
				{
					Gdiplus::RectF DrawRgn = Gdiplus::RectF ( 0.0f, 0.0f, Bounding.X, FontHeight );

					for ( int i = 0; i < m_arrStrings.GetSize(); ++i )
					{
						DrawRgn.Y =  i * FontHeight;

						if ( DrawRgn.Y > Bounding.Y )
							break;

						m_oEdgeText.Draw ( pGraphics, pFont, pStringFormat, _bstr_t ( m_arrStrings [i] ), -1, DrawRgn );
						pGraphics->DrawString ( Rendering::TextRendering::NormNonBreakingSpaces ( m_arrStrings[i] ), -1, pFont, DrawRgn, pStringFormat, pBrush );
					}
					break;
				}
			}

			pGraphics->EndContainer ( graphicsContainer );

			RELEASEOBJECT ( pStringFormat );
		}

	private:

		bool				m_bTextAllSelect;

		HDC					m_hDC;
		double				m_dZoom;
		bool				m_IsFirstResize;
		bool				m_AutoSize;
		bool				m_ScaleText;

		Structures::POINTD	m_Scale;
		bool				m_bBlockEditing;
		bool				m_bBlockResize;
		double				m_StickAngle;

	public:

		CAtlArray < Gdiplus::RectF > 		m_arrBoundingLines;
		
		//ImageStudio::Paint::CPaintState	m_oPaintState;

		Painter::CBrush						m_oBrush;
		Painter::CFont						m_oFont;
		Painter::CFormat					m_oFormat;
		Painter::CEdgeText					m_oEdgeText;
		Painter::CShadow					m_oShadow;
		Painter::CTextureManager			m_oTextureManager;
		
		float								m_fLineSpacing;

		int									m_nStringAlignmentVertical;

		Gdiplus::FontFamily*				m_pFontFamily;

		bool								m_bFirstPaint;

		bool								m_bNotAutoFitText;			//	обычный режим отрисовки текста, доступно 9 форматирований текста, трекинг не изменяется
		bool								m_bResizeShapeToFitText;	//	весь текст в трекинге рисуется, форматирование по горизонтали, трек подстраивает свою высоту по ширине текста
		bool								m_bScrollingText;			//	текст прокручивается, форматирование по горизонтали, трек не изменяется по тексту

		long								m_nCanScrollLines;			// сколько строк допустимо пролистывать
		long								m_nScrollLine;				// сколько строк пролистали
		long								m_nVisibleLines;			// сколько линий отображается

		CSimpleArray <StringContext>		m_aStringsInfo;
		bool								m_bCaretAtHome;				// каретка находится в конце строки (или в начале строки, для правильного переноса каретки при разнесенных строках )
		int									m_nCurrentString;
		bool								m_bSetCaretAtHome;

		WCHAR								m_chDeadKey;
	};
}