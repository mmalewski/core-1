

// Здесь будут считываться метеданные

#pragma once

#include <XmlUtils.h>
#include <atlfile.h>

//#include <nmmintrin.h>

#pragma pack(push, 1)
struct TRational
{
	int Fraction;
	int Denominator;
};
struct TRationalU
{
	unsigned int Fraction;
	unsigned int Denominator;
};
#pragma pack(pop)

		class CMetaData
		{

		public:

			CMetaData()
			{
				m_bBitmap = FALSE;
				Clear();
			}

			~CMetaData()
			{
			}


			BOOL FromFile(const CString& strFilePath)
			{
				if (strFilePath.GetLength()<1)return FALSE;

				m_bMultiPageImage = FALSE;
				m_bBitmap = FALSE;

				// сначала пробуем открыть как Raw
				if ( LoadRawMetaData(strFilePath) )
					return TRUE;
				
				BSTR bsFilePath = strFilePath.AllocSysString();

				// cначала пробуем открыть как bitmap
				
				// Gdi+ Bitmap object
				Bitmap* bitmap = 0;
				
				bitmap = new Bitmap(bsFilePath);

				// check for valid bitmap
				if ( !( !bitmap || bitmap->GetLastStatus() != Ok ) )
				{
					LoadBitmapMetaData(bitmap);
					m_bBitmap = TRUE;
				}
				if (bitmap)
					delete bitmap;
				bitmap = 0;

				::SysFreeString(bsFilePath);

				// открываем файл
				HANDLE hFile = CreateFile(strFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (INVALID_HANDLE_VALUE == hFile)
					return FALSE; // Невозможно открыть файл

				// мапим этот файл в память - так быстрее читаются данные из файла
				DWORD  nFileSize = GetFileSize(hFile, NULL);
				HANDLE hMapFile  = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, nFileSize, NULL);
				if (NULL == hMapFile)
				{
					CloseHandle( hFile );
					return FALSE; // Невозможно создать отображение файла
				}

				// создаем view of file
				DWORD nMaxBytesRead	= nFileSize;
				VOID* pBaseAddress	= MapViewOfFile( hMapFile, FILE_MAP_READ, 0, 0, 0 );
				if ( !pBaseAddress )
				{
					CloseHandle( hMapFile );
					CloseHandle( hFile );
					return FALSE;
				}
				BYTE* pnFile		= (BYTE*)pBaseAddress;
				BOOL  bRetValue		= FALSE;

				if ( LoadTiffMetaData(pnFile, nFileSize) )
				{
					UnmapViewOfFile(pBaseAddress);
					CloseHandle(hMapFile);
					CloseHandle( hFile );
					return TRUE;
				}

				if ( LoadPNGMetaData(pnFile, nFileSize) )
				{
					UnmapViewOfFile(pBaseAddress);
					CloseHandle(hMapFile);
					CloseHandle( hFile );	
					return TRUE;
				}
				
				if ( LoadTGAMetaData(pnFile, nFileSize) )
				{
					UnmapViewOfFile(pBaseAddress);
					CloseHandle(hMapFile);
					CloseHandle( hFile );	
					return TRUE;
				}

				if ( LoadFoveonMetaData(pnFile, nFileSize) )
				{
					UnmapViewOfFile(pBaseAddress);
					CloseHandle(hMapFile);
					CloseHandle( hFile );
					return TRUE;
				}

				if ( LoadPSDMetaData(pnFile, nFileSize) )
				{
					UnmapViewOfFile(pBaseAddress);
					CloseHandle(hMapFile);
					CloseHandle( hFile );
					return TRUE;
				}

				if ( LoadPCXMetaData(pnFile, nFileSize) )
				{
					UnmapViewOfFile(pBaseAddress);
					CloseHandle(hMapFile);
					CloseHandle( hFile );
					return TRUE;
				}

				if ( LoadRasMetaData(pnFile, nFileSize) )
				{
					UnmapViewOfFile(pBaseAddress);
					CloseHandle(hMapFile);
					CloseHandle( hFile );
					return TRUE;
				}

				if ( LoadJ2kMetaData(pnFile, nFileSize) )
				{
					UnmapViewOfFile(pBaseAddress);
					CloseHandle(hMapFile);
					CloseHandle( hFile );
					return TRUE;
				}

				UnmapViewOfFile(pBaseAddress);
				CloseHandle(hMapFile);
				CloseHandle( hFile );

				return TRUE;
			}
			BOOL SaveToBMP(Bitmap* bitmap)
			{
				SetPropertyStr(bitmap, PropertyTagImageTitle,       m_sTitle);
				SetPropertyStr(bitmap, PropertyTagArtist,           m_sAuthor);
				SetPropertyStr(bitmap, PropertyTagImageDescription, m_sDescription);
				
				CString sDateTime = _T("");
				CString sTemp = _T("");
				char sValue[33];
				_itoa(m_oDateTimeCreation.nYear, sValue, 10);

				sTemp = sValue;
				while ( sTemp.GetLength() < 4 )
					sTemp = _T("0") + sTemp;

				sDateTime += sTemp + _T(":");

				_itoa(m_oDateTimeCreation.nMonth, sValue, 10);

				sTemp = sValue;
				while ( sTemp.GetLength() < 2 )
					sTemp = _T("0") + sTemp;

				sDateTime += sTemp + _T(":");

				_itoa(m_oDateTimeCreation.nDay, sValue, 10);

				sTemp = sValue;
				while ( sTemp.GetLength() < 2 )
					sTemp = _T("0") + sTemp;

				sDateTime += sTemp + _T(" ");

				_itoa(m_oDateTimeCreation.nHour, sValue, 10);

				sTemp = sValue;
				while ( sTemp.GetLength() < 2 )
					sTemp = _T("0") + sTemp;

				sDateTime += sTemp + _T(":");

				_itoa(m_oDateTimeCreation.nMinute, sValue, 10);

				sTemp = sValue;
				while ( sTemp.GetLength() < 2 )
					sTemp = _T("0") + sTemp;

				sDateTime += sTemp + _T(":");

				_itoa(m_oDateTimeCreation.nSecond, sValue, 10);

				sTemp = sValue;
				while ( sTemp.GetLength() < 2 )
					sTemp = _T("0") + sTemp;

				sDateTime += sTemp;

				SetPropertyStr(bitmap, PropertyTagDateTime,         sDateTime);
				SetPropertyStr(bitmap, PropertyTagExifUserComment,	m_sComment);
				SetPropertyStr(bitmap, PropertyTagCopyright,        m_sCopyright);
				SetPropertyStr(bitmap, PropertyTagHostComputer,     m_sHostComputer);
				SetPropertyStr(bitmap, PropertyTagEquipMake,        m_sEquipmentType);
				SetPropertyStr(bitmap, PropertyTagEquipModel,       m_sEquipmentModel);
				SetPropertyStr(bitmap, PropertyTagSoftwareUsed,     m_sSoftwareID);
				SetPropertyStr(bitmap, PropertyTagDocumentName,     m_sDocumentName);
				SetPropertyStr(bitmap, PropertyTagPageName,         m_sPageName);

				short nPageNumber = BYTE(m_nPagesCount) << 8 | BYTE(m_nPageNumber);
				SetPropertyShort(bitmap, PropertyTagPageNumber, nPageNumber);
				
				SetPropertyStr(bitmap, PropertyTagPageName,         m_sDisclaimer);
				//SetPropertyStr(bitmap, PropertyTagPageName,         m_sJobName);
				//SetPropertyUndefined(bitmap,PropertyTagExifVer,         m_sVersionLetter);
				SetPropertyShort(bitmap, PropertyTagExifISOSpeed,   m_nISOSpeed);
				//SetPropertyShort(bitmap, PropertyTagPageName,       m_nBitDepth);
				SetPropertyShort(bitmap, PropertyTagExifColorSpace,       m_nColorSpace);

				SetPropertyShort(bitmap, PropertyTagOrientation,    m_nOrientation);
				SetPropertyShort(bitmap, PropertyTagExifExposureProg,m_nExposureProgram);
				SetPropertyShort(bitmap, PropertyTagExifFlash,       m_nFlashMode);
				SetPropertyShort(bitmap, PropertyTagExifMeteringMode, m_nMeteringMode);

				SetPropertyRational(bitmap,PropertyTagExifFNumber,m_oFNumber);
				SetPropertyRational(bitmap,PropertyTagExifExposureTime,m_oExposureTime);
				SetPropertyRational(bitmap,PropertyTagExifAperture,m_oLensAperture);
				SetPropertyRational(bitmap,PropertyTagExifMaxAperture,m_oMaxApertureValue);
				SetPropertySRational(bitmap,PropertyTagExifShutterSpeed,m_oShutterSpeed);
				SetPropertySRational(bitmap,PropertyTagExifExposureBias,m_oExposureCompensation);
				SetPropertySRational(bitmap,PropertyTagExifBrightness,m_oBrightness);
				SetPropertyRational(bitmap,PropertyTagExifFocalLength,m_oFocalLength);

				//exif 2.2 
				SetPropertyShort(bitmap,PropertyTagExifExposureMode,m_nExposureMode );
				SetPropertyShort(bitmap,PropertyTagExifWhiteBalance,m_nWhiteBalance);
				SetPropertyShort(bitmap,PropertyTagExifSensingMethod , m_nSensingMethod);
				SetPropertyShort(bitmap,PropertyTagExifFocalLengthIn35mmFilm,m_nFocalLengthIn35mmFilm);


				if (bitmap->GetLastStatus() != Ok)
					return FALSE;

				//дополнительные данные

				return TRUE;
			}

			BOOL SaveToTGA(const CString& strFilePath)
			{
				CAtlFile file;
				if (S_OK != file.Create(strFilePath, GENERIC_WRITE, FILE_SHARE_WRITE, OPEN_EXISTING))
					return FALSE; // Не могу создать файл

				ULONGLONG nFileSize;
				file.GetSize((ULONGLONG&)nFileSize);

				file.Seek(nFileSize);
				SaveAsTGA(file);
				file.Close();
				return TRUE;
			}

			BOOL SaveToPNG(const CString& strFilePath)
			{
				// сначала найдем указатель на последний кусочек файла
				// в принципе, он должен быть равен (nFileSize - 12)

				ULONGLONG nOffset = GetPNGLastPoint(strFilePath);
				if ( nOffset < 0 )
					return FALSE;
				
				CAtlFile file;
				if (S_OK != file.Create(strFilePath, GENERIC_WRITE, FILE_SHARE_WRITE, OPEN_EXISTING))
					return FALSE; // Не могу создать файл

				

				ULONGLONG nFileSize;
				file.GetSize((ULONGLONG&)nFileSize);

				//file.Seek(nFileSize - 12);
				file.Seek(nOffset);

				SaveAsPNG(file);
				file.Close();
				return TRUE;
			}

			void Clear()
			{
				m_sAuthor      = _T("");
				m_sTitle       = _T("");
				m_sDescription = _T("");
				m_oDateTimeCreation.nYear    = 0;
				m_oDateTimeCreation.nMonth   = 0;
				m_oDateTimeCreation.nDay     = 0;
				m_oDateTimeCreation.nHour    = 0;
				m_oDateTimeCreation.nMinute  = 0;
				m_oDateTimeCreation.nSecond  = 0;
				m_oDateTimeCreation.sSummary = _T("");
				m_sCopyright      = _T("");
				m_sDisclaimer     = _T("");
				m_sComment        = _T("");
				m_sEquipmentType  = _T("");
				m_sEquipmentModel = _T("");
				m_oJobTime.nHour   = 0;
				m_oJobTime.nMinute = 0;
				m_oJobTime.nSecond = 0;
				m_sJobName       = _T("");
				m_sSoftwareID    = _T("");
				m_nVersionNumber = 0;
				m_sVersionLetter = _T("");
				m_sHostComputer  = _T("");
				m_sWarning       = _T("");
				m_sDocumentName  = _T("");
				m_sPageName      = _T("");
				m_nPageNumber    = 0;
				m_nPagesCount    = 0;

				// дополнительные данные для photoeditor

				m_nExposureProgram                  = 0;
				m_oExposureTime.Fraction            = 0;
				m_oExposureTime.Denominator         = 0;
				m_oExposureCompensation.Fraction    = 0;
				m_oExposureCompensation.Denominator = 0;
				m_oShutterSpeed.Fraction            = 0;
				m_oShutterSpeed.Denominator         = 0;
				m_oLensAperture.Fraction            = 0;
				m_oLensAperture.Denominator         = 0;
				m_oFocalLength.Fraction             = 0;
				m_oFocalLength.Denominator          = 0;
				m_oFNumber.Fraction                 = 0;
				m_oFNumber.Denominator              = 0;
				m_oBrightness.Denominator			= 0;
				m_oBrightness.Fraction				= 0;

				m_nISOSpeed                         = 0;
				m_nMeteringMode                     = 0;
				m_nFlashMode                        = 0;
				m_nColorSpace                       = 0;
		
				m_oMaxApertureValue.Fraction        = 0;
				m_oMaxApertureValue.Denominator     = 0;
				
				m_nSensingMethod                    = 0;
				m_nWhiteBalance                     = 0;
				m_nExposureMode                     = 0;
				m_nOrientation                      = 1;
				m_nFocalLengthIn35mmFilm			= 0;
				
				m_nBitDepth                         = 1;

				m_sMetaDataXML = _T("");


			}
			CString GetXML()
			{
				MakeXML();
				return m_sMetaDataXML;
			}

			void SetXML(CString sXML)
			{
				ParseXML(sXML);
				return;
			}

			short GetOrientation()
			{
				return m_nOrientation;
			}
		protected:

			//common functions
			int  IntFromBytes2( const BYTE* pBytes, int nType)
			{
				int nValue = 0;

				if ( 0 == nType )
				{
					nValue = pBytes[0];
					nValue = ( nValue << 8 ) | pBytes[1];
				}
				else
				{
					nValue = pBytes[1];
					nValue = ( nValue << 8 ) | pBytes[0];
				}
				return nValue;
			}

			int  IntFromBytes4( const BYTE* pBytes, int nType)
			{
				int nValue = 0;

				if ( 0 == nType )
				{
					nValue = pBytes[0];
					nValue = ( nValue << 8 ) | pBytes[1];
					nValue = ( nValue << 8 ) | pBytes[2];
					nValue = ( nValue << 8 ) | pBytes[3];
				}
				else
				{
					nValue = pBytes[3];
					nValue = ( nValue << 8 ) | pBytes[2];
					nValue = ( nValue << 8 ) | pBytes[1];
					nValue = ( nValue << 8 ) | pBytes[0];
				}
				return nValue;
			}







			void IntToBytes4( int nNumber, BYTE* pBytes )
			{
				pBytes[3] = (BYTE)nNumber; nNumber >>= 8;
				pBytes[2] = (BYTE)nNumber; nNumber >>= 8;
				pBytes[1] = (BYTE)nNumber; nNumber >>= 8;
				pBytes[0] = (BYTE)nNumber;
			}
			// PNG functions
			BOOL CheckPNGSignature(BYTE* pnFile)
			{
				// Сигнатура PNG, первые 8 байт:
				// 137 80 78 71 13 10 26 10
				// ЙPNG♪◙☻◙

				if ( 137 == pnFile[0] && 80 == pnFile[1] && 78 == pnFile[2] && 71 == pnFile[3] &&
					 13  == pnFile[4] && 10 == pnFile[5] && 26 == pnFile[6] && 10 == pnFile[7] )
					 return TRUE;

				return FALSE;
			}

			CString ChunkType(const BYTE* pCurPoint)
			{
				CString sType = _T("");
				sType += pCurPoint[0];
				sType += pCurPoint[1];
				sType += pCurPoint[2];
				sType += pCurPoint[3];
				return sType;
			}




			BOOL SaveChunk(ATL::CAtlFile& oFile, CString sKeyword, CString sValue)
			{
				TChunk oChunk;
				DWORD nLenght = sKeyword.GetLength() + sValue.GetLength() + 1;
				BYTE *pCheckSum = new BYTE[4 + nLenght];

				IntToBytes4( nLenght, oChunk.nLenght);
				oChunk.nType[0] = 't';
				oChunk.nType[1] = 'E';
				oChunk.nType[2] = 'X';
				oChunk.nType[3] = 't';

				for (int nIndex = 0; nIndex < 4; nIndex++)
					pCheckSum[nIndex] = oChunk.nType[nIndex];

				if( S_OK != oFile.Write( &oChunk, 8))
					return FALSE;

				CString sData = sKeyword.GetBuffer();
				char *aData = (char*)(LPSTR)sData.GetBuffer(sData.GetLength());

				for (int nIndex = 4; nIndex < 4 + sKeyword.GetLength(); nIndex++)
					pCheckSum[nIndex] = aData[nIndex - 4];

				//if( S_OK != oFile.Write( aData, sData.GetLength()))
				//	return FALSE;

				pCheckSum[4 + sKeyword.GetLength()] = 0;

				sData = sValue.GetBuffer();
				aData = (char*)(LPSTR)sData.GetBuffer(sData.GetLength());
				for (int nIndex = 4 + sKeyword.GetLength() + 1; nIndex < 4 + nLenght; nIndex++)
					pCheckSum[nIndex] = aData[nIndex - ( 4 + sKeyword.GetLength() + 1 )];

				//BYTE nNullSeparator = 0;

				//if( S_OK != oFile.Write( &nNullSeparator, 1))
				//	return FALSE;


				if( S_OK != oFile.Write( pCheckSum + 4, nLenght))
					return FALSE;
				
				crc32 oCRC;
				oCRC.ProcessCRC(pCheckSum, nLenght + 4);
				delete []pCheckSum;
				
				BYTE pCRC[4] = {0, 0, 0, 0};
				IntToBytes4( oCRC.m_crc32, pCRC);

				if( S_OK != oFile.Write( pCRC, 4))
					return FALSE;

				return TRUE;
			}
			DWORD GetPNGLastPoint(const CString& strFilePath)
			{
				// открываем файл
				HANDLE hFile = CreateFile(strFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (INVALID_HANDLE_VALUE == hFile)
					return FALSE; // Невозможно открыть файл

				// мапим этот файл в память - так быстрее читаются данные из файла
				DWORD  nFileSize = GetFileSize(hFile, NULL);
				HANDLE hMapFile  = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, nFileSize, NULL);
				if (NULL == hMapFile)
				{
					CloseHandle( hFile );
					return FALSE; // Невозможно создать отображение файла
				}

				// создаем view of file
				DWORD nMaxBytesRead	= nFileSize;
				VOID* pBaseAddress	= MapViewOfFile( hMapFile, FILE_MAP_READ, 0, 0, 0 );
				BYTE* pnFile		= (BYTE*)pBaseAddress;
				BOOL  bRetValue		= FALSE;

				if ( !CheckPNGSignature(pnFile) )
					return -1;

				BYTE* pCurPoint = pnFile + 8;

				const TChunk& oFirstChunk = *(TChunk*)pCurPoint;
				CString sType = ChunkType(oFirstChunk.nType);

				if ( -1 == sType.Find(_T("IHDR")) )
					return -1;

				DWORD nBytesAlreadyRead = 8;
				int nLenght = IntFromBytes4(oFirstChunk.nLenght, 0);
				int nChunkSize = nLenght + 12;

				while ( -1 == sType.Find(_T("IEND")) &&  nBytesAlreadyRead < nFileSize )
				{
					pCurPoint += nChunkSize;
					nBytesAlreadyRead += nChunkSize;
					const TChunk& oCurChunk = *(TChunk*)pCurPoint;
					sType = ChunkType(oCurChunk.nType);
					nLenght = IntFromBytes4(oCurChunk.nLenght, 0);
					nChunkSize = nLenght + 12;
				}
				CloseHandle( hFile );
				return nBytesAlreadyRead; 
			}
			// TGA functions
			CString TGAASKIIValue(const BYTE* pCurPoint, int nLenght)
			{
				CString sReturn = _T("");
				for ( int nIndex = 0; nIndex < nLenght; nIndex++ )
					sReturn += pCurPoint[nIndex];
				
				return sReturn;
			}



			void BytesFromString(CString sSource, BYTE *pResult, int nCount)
			{
				int nLen = min( (int)sSource.GetLength(), nCount);
				for (int nIndex = 0; nIndex < nLen; nIndex++ )
					pResult[nIndex] = sSource[nIndex];
			}

			// BMP functions
			CString GetPropertyStr(Bitmap*& pImage, PROPID nId)
			{
				// reset value
				CString sValue = _T("");

				// retrieve property size
				UINT nSize = pImage->GetPropertyItemSize(nId);

				// check for valid size
				if (nSize > 0)
				{
					// allocate memory for tag
					PropertyItem* pProp = (PropertyItem*)(malloc(nSize));
					if ( NULL == pProp )
						return sValue;

					// read tag to buffer
					pImage->GetPropertyItem(nId, nSize, pProp);

					// copy data to buffer
					if ( NULL != pProp->value )
						sValue = (const char*)pProp->value;
					
					// release buffer
					free(pProp);
				}
				return sValue;
			}

			int GetPropertyInt(Bitmap*& pImage, PROPID nId)
			{
				// reset value
				int nValue = 0;

				// retrieve property size
				UINT nSize = pImage->GetPropertyItemSize(nId);

				// check for valid size
				if (nSize > 0)
				{
					// allocate memory for tag
					PropertyItem* pProp = (PropertyItem*)(malloc(nSize));
					if ( NULL == pProp )
						return nValue;

					// read tag to buffer
					pImage->GetPropertyItem(nId, nSize, pProp);

					// copy data to buffer
					if ( NULL != pProp->value )
						nValue = *(int*)pProp->value;
					
					// release buffer
					free(pProp);
				}
				return nValue;
			}


			short GetPropertyShort(Bitmap*& pImage, PROPID nId)
			{
				// reset value
				short nValue = 0;

				// retrieve property size
				UINT nSize = pImage->GetPropertyItemSize(nId);

				// check for valid size
				if (nSize > 0)
				{
					// allocate memory for tag
					PropertyItem* pProp = (PropertyItem*)(malloc(nSize));
					if ( NULL == pProp )
						return nValue;

					// read tag to buffer
					pImage->GetPropertyItem(nId, nSize, pProp);

					// copy data to buffer
					if ( NULL != pProp->value )
						nValue = *(short*)pProp->value;
					
					// release buffer
					free(pProp);
				}
				return nValue;
			}


			double GetPropertyDouble(Bitmap*& pImage, PROPID nId)
			{
				// reset value
				double dValue = 0;

				// retrieve property size
				UINT nSize = pImage->GetPropertyItemSize(nId);

				// check for valid size
				if (nSize > 0)
				{
					// allocate memory for tag
					PropertyItem* pProp = (PropertyItem*)(malloc(nSize));
					if ( NULL == pProp )
						return dValue;

					// read tag to buffer
					pImage->GetPropertyItem(nId, nSize, pProp);

					// copy data to buffer
					if ( NULL != pProp->value )
						dValue = *(double*)pProp->value;
					
					// release buffer
					free(pProp);
				}
				return dValue;
			}

			void GetPropertyRationalU(Bitmap*& pImage, PROPID nId, TRationalU *pResult)
			{
				//rational - пара двух чисел (числитель и знаменатель)
				(*pResult).Denominator = 1;
				(*pResult).Fraction    = 0;
				// retrieve property size
				UINT nSize = pImage->GetPropertyItemSize(nId);

				// check for valid size
				if (nSize > 0)
				{
					// allocate memory for tag
					PropertyItem* pProp = (PropertyItem*)(malloc(nSize));
					if ( NULL == pProp )
						return;

					// read tag to buffer
					pImage->GetPropertyItem(nId, nSize, pProp);

					// copy data to buffer
					if ( NULL != pProp->value )
						*pResult = *(TRationalU*)pProp->value;
					
					// release buffer
					free(pProp);
				}
			}


			void GetPropertyRational(Bitmap*& pImage, PROPID nId, TRational *pResult)
			{
				//rational - пара двух чисел (числитель и знаменатель)
				(*pResult).Denominator = 1;
				(*pResult).Fraction    = 0;
				// retrieve property size
				UINT nSize = pImage->GetPropertyItemSize(nId);

				// check for valid size
				if (nSize > 0)
				{
					// allocate memory for tag
					PropertyItem* pProp = (PropertyItem*)(malloc(nSize));
					if ( NULL == pProp )
						return;

					// read tag to buffer
					pImage->GetPropertyItem(nId, nSize, pProp);

					// copy data to buffer
					if ( NULL != pProp->value )
						*pResult = *(TRational*)pProp->value;
					
					// release buffer
					free(pProp);
				}
			}


			void SetPropertyStr(Bitmap*& pImage, PROPID nId, CString& sValue)
			{
				// check for valid string
				if (sValue.GetLength() < 1)
					return;

				// create new property item
				PropertyItem* pProp = new PropertyItem;
				if ( NULL == pProp )
					return;

				// compose property item
				pProp->id = nId;
				pProp->length = (nId == PropertyTagDateTime) ? 20 : sValue.GetLength() + 1;
				pProp->type = PropertyTagTypeASCII; 

				CT2A convert(sValue);
				pProp->value =convert;

				// sset property item
				pImage->SetPropertyItem(pProp);

				// delete property item
				delete pProp;
			}
			void SetPropertyShort(Bitmap*& pImage, PROPID nId, short& nValue)
			{
				// create new property item
				PropertyItem* pProp = new PropertyItem;
				if ( NULL == pProp )
					return;

				// compose property item
				pProp->id = nId;
				pProp->length = sizeof(short);
				pProp->type = PropertyTagTypeShort; 
				pProp->value = (short*)(&nValue);

				// sset property item
				pImage->SetPropertyItem(pProp);

				// delete property item
				delete pProp;
			}
			void SetPropertyRational(Bitmap*& pImage, PROPID nId, TRationalU& nValue)
			{//unsignes
				// create new property item
				PropertyItem* pProp = new PropertyItem;
				if ( NULL == pProp )
					return;

				// compose property item
				pProp->id = nId;
				pProp->length = 2 * sizeof(long);
				pProp->type = PropertyTagTypeRational; 
				pProp->value = (unsigned long*)(&nValue);

				// sset property item
				pImage->SetPropertyItem(pProp);

				// delete property item
				delete pProp;
			}
			void SetPropertySRational(Bitmap*& pImage, PROPID nId, TRational& nValue)
			{//signed
				// create new property item
				PropertyItem* pProp = new PropertyItem;
				if ( NULL == pProp )
					return;

				// compose property item
				pProp->id = nId;
				pProp->length = 2 * sizeof(long);
				pProp->type = PropertyTagTypeSRational; 
				pProp->value = (long*)(&nValue);

				// sset property item
				pImage->SetPropertyItem(pProp);

				// delete property item
				delete pProp;
			}
			CString RationalUToString(TRationalU oRat)
			{
				CString sResult = _T("");
				char str[32];
				_itoa(oRat.Fraction, str, 10);
				sResult += str;
				if ( 0 != oRat.Denominator )
				{
					sResult += _T("/");
					_itoa(oRat.Denominator, str, 10);
					sResult += str;
				}
				return sResult;
			}
			CString RationalToString(TRational oRat)
			{
				CString sResult = _T("");
				char str[32];
				_itoa(oRat.Fraction, str, 10);
				sResult += str;
				if ( 0 != oRat.Denominator )
				{
					sResult += _T("/");
					_itoa(oRat.Denominator, str, 10);
					sResult += str;
				}
				return sResult;
			}
			// Raw functions
			BOOL ReadMakerNoteSignature(BYTE *pnMakerNote, int *pByteOrder, DWORD *pOffset)
			{
				int nByteOrder = -1;

				// изначально читаем 10 символов для проверки сигнатуры
				DWORD nOffset = 10;

				TCHAR cSignature[10];
				for (int nIndex = 0; nIndex < 10; nIndex++ )
					cSignature[nIndex] = pnMakerNote[nIndex];

				// не Tiff формат

				if (!_tcsnccmp (cSignature,_T("KDK") ,3) || !_tcsnccmp (cSignature,_T("VER") ,3) ||
					!_tcsnccmp (cSignature,_T("IIII"),4) || !_tcsnccmp (cSignature,_T("MMMM"),4) ) 
					return false;

				/* Konica KD-400Z, KD-510Z */	
				/* Minolta DiMAGE G series */

				if (!_tcsnccmp (cSignature,_T("KC"),2) || !_tcsnccmp (cSignature,_T("MLY") ,3)) 
					return false;

				if (!_tcscmp (cSignature,_T("Nikon"))) 
				{
					const TTiffFileHeader& oHeader = *(TTiffFileHeader*)(pnMakerNote + 10);
					CString sByteOrder(oHeader.nByteOrder);
					
					if ( -1 != sByteOrder.Find(_T("II"), 0) )
						nByteOrder = 1;
					else if ( -1 != sByteOrder.Find(_T("MM"), 0) )
						nByteOrder = 0;
					else
						return false;

					int nSignature = IntFromBytes2(oHeader.nSignature, nByteOrder);
					if ( 42 != nSignature )
						return false;

					nOffset += IntFromBytes4( oHeader.nIDFoffset, nByteOrder);
				} 
				else if (!_tcscmp (cSignature,_T("OLYMPUS"))) 
				{
					const TTiffFileHeader& oHeader = *(TTiffFileHeader*)(pnMakerNote + 8);
					CString sByteOrder(oHeader.nByteOrder);
					
					if ( -1 != sByteOrder.Find(_T("II"), 0) )
						nByteOrder = 1;
					else if ( -1 != sByteOrder.Find(_T("MM"), 0) )
						nByteOrder = 0;
					else
						return false;

					nOffset += 2;
					
				} 
				else if (!_tcsnccmp (cSignature,_T("FUJIFILM"),8) || !_tcsnccmp (cSignature,_T("SONY"),4) || 
						 !_tcscmp  (cSignature,_T("Panasonic")) ) 
				{
					nByteOrder = 1;// 0x4949
					nOffset += 2;
				} 
				else if (!_tcscmp (cSignature,_T("OLYMP")) ||	!_tcscmp (cSignature,_T("LEICA")) ||
						 !_tcscmp (cSignature,_T("Ricoh")) ||	!_tcscmp (cSignature,_T("EPSON")) )
					nOffset -= 2;
				else if (!_tcscmp (cSignature,_T("AOC")) || !_tcscmp (cSignature,_T("QVC")))
					nOffset -= 4;
				else 
					nOffset -= 10;

				// проверям менялся ли порядок байт
				if ( nByteOrder >= 0 )
					pByteOrder[0] = nByteOrder;
				pOffset[0] = nOffset;

				return TRUE;
			}

			void ReadMakerNote(BYTE *pnMakerNote, int nByteOrder, BYTE *pnFile, DWORD nBaseOffset)
			{
				// В MakerNote может быть свой Tiff Header ( возможно,
				// со своим порядком байт), а возможно тут хранится таблица
				BYTE* pCurPoint = pnMakerNote;

				DWORD nOffset = 0;

				if ( !ReadMakerNoteSignature(pnMakerNote, &nByteOrder, &nOffset) )
					return;

				pCurPoint += nOffset;

				int nDirectoryEntryCount = IntFromBytes2(pCurPoint, nByteOrder);

				if ( nDirectoryEntryCount > 1000 )
					return;
				
				int nIndex = 0;

				while ( nDirectoryEntryCount-- ) 
				{
					
					BYTE* pCurDirectory = pCurPoint + 2 + 12 * nIndex;
					nIndex++;

					const TDirectoryEntry& oDirEntry = *(TDirectoryEntry*)(pCurDirectory);
					int nTag   = IntFromBytes2(oDirEntry.nTag, nByteOrder);
					int nType  = IntFromBytes2(oDirEntry.nType, nByteOrder);
					int nCount = IntFromBytes4(oDirEntry.nCount, nByteOrder);
					int nValueOffset = IntFromBytes4(oDirEntry.nValueOffset, nByteOrder);

					nValueOffset += nBaseOffset;
					
					if ( 2 == nType ) // значение типa ASCII
					{
		
						CString sCur = _T("");
						if (nCount <= 4)
						{
							sCur = TGAASKIIValue(oDirEntry.nValueOffset, nCount);
						}
						else
						{
							if ( -1 != m_sEquipmentType.Find(_T("FUJIFILM")) )
								for (int nI = 0; nI < nCount; nI++ )
									sCur += (pnMakerNote + nValueOffset)[nI];
							else if ( -1 != m_sEquipmentType.Find(_T("NIKON")) )
								for (int nI = 0; nI < nCount; nI++ )
									sCur += (pnMakerNote + nOffset - 8 + nValueOffset)[nI];
							else
								for (int nI = 0; nI < nCount; nI++ )
									sCur += (pnFile + nValueOffset)[nI];

						}

						if ( -1 != m_sEquipmentType.Find(_T("OLYMP")) ||  -1 != m_sEquipmentType.Find(_T("EPSON")) ||
							 -1 != m_sEquipmentType.Find(_T("Agfa")) )
						{
							if ( 0x0207 == nTag )
							{
								m_sComment += sCur;
								m_sComment += _T("\n");
							}
							else if ( 0x0208 == nTag )
							{
								m_sComment += _T("Picture Info: ") + sCur;
								m_sComment += _T("\n");
							}
							else if ( 0x0209 == nTag )
							{
								m_sComment += _T("Camera ID: ") + sCur;
								m_sComment += _T("\n");
							}
							else if ( 0x02D  == nTag )
							{
								m_sComment += _T("Original Manufacturer Model: ") + sCur;
								m_sComment += _T("\n");
							}
								
						}

						if ( -1 != m_sEquipmentType.Find(_T("Canon")) )
						{
							if ( 0x0006 == nTag )
							{
								m_sComment += _T("CanonImageType: ") + sCur;
								m_sComment += _T("\n");
							}
							else if ( 0x0007 == nTag )
							{
								m_sComment += sCur;
								m_sComment += _T("\n");
							}
							else if ( 0x0009 == nTag )
								m_sAuthor = sCur;

						}
						if ( -1 != m_sEquipmentType.Find(_T("Casio")) )
						{
							if ( 0x3006 == nTag )
							{
								m_sComment += _T("HometownCity: ") + sCur;
								m_sComment += _T("\n");
							}

						}
							
						if ( -1 != m_sEquipmentType.Find(_T("FUJIFILM")) )
						{
							if ( 0x8000 == nTag ) // fileSource
								m_sDocumentName = sCur;
						}
						if ( -1 != m_sEquipmentType.Find(_T("Kodak")) )
						{
							if ( 8 == nTag && m_sEquipmentType.GetLength() < 1 ) 
								m_sEquipmentType = sCur;
							else if ( 40 == nTag && m_sEquipmentModel.GetLength() < 1 ) 
								m_sEquipmentModel = sCur;
							else if ( 32 == nType ) // originalFileName
								m_sDocumentName = sCur;
						}
						if ( -1 != m_sEquipmentType.Find(_T("Sigma")) || -1 != m_sEquipmentType.Find(_T("Foevon")) )
						{
							if ( 0x0017 == nTag ) 
							{
								m_sComment += sCur.GetBuffer();
								m_sComment += "\n";
							}
							else if ( 0x0018 == nTag ) 
								m_sSoftwareID = sCur;
						}
					}

				}


			}

			void ReadRawIFD(BYTE* pnFile, unsigned short nIFDTag, DWORD nIFDOffset, unsigned long nByteOrder, int nIndex, DWORD nBaseOffset)
			{
				BYTE *nStartPoint = pnFile + nIFDOffset;

				// проверяем  makernote или нет
				if ( 37500 == nIFDTag )  
				{
					ReadMakerNote(nStartPoint, nByteOrder, pnFile, nBaseOffset);
					return;
				}

				else if ( 0 == nIFDTag || //
							34665 == nIFDTag )//exif 
				{
					int nBitPerSample = 1, nSamplePerPixel = 1;

					int nDirectoryEntryCount = IntFromBytes2(nStartPoint, nByteOrder);
					if ( nDirectoryEntryCount > 512)
						return;
					for ( int nIndex = 0; nIndex < nDirectoryEntryCount; nIndex++ )
					{
						BYTE *pCurDirectory = nStartPoint + 2 + 12 * nIndex;

						//int r1 = IntFromBytes2(nStartPoint + 12 * nIndex,0);

						const TDirectoryEntry& oDirEntry = *(TDirectoryEntry*)(pCurDirectory);
						int nTag   = IntFromBytes2(oDirEntry.nTag, nByteOrder);
						int nType  = IntFromBytes2(oDirEntry.nType, nByteOrder);
						int nCount = IntFromBytes4(oDirEntry.nCount, nByteOrder);
						int nValueOffset = IntFromBytes4(oDirEntry.nValueOffset, nByteOrder);

						nValueOffset += nBaseOffset;
						if ( 34665 == nTag ) // ExifIDFTag
							ReadRawIFD( pnFile, nTag, nValueOffset, nByteOrder, nIndex, nBaseOffset);
						// если тип Value меньше 4 байт, тогда в переменной ValueOffset 
						// указан не сдвиг, а самое значение Value (см., например, таг = 297)

						if (PropertyTagTypeASCII == nType ) 
						{
							CString sCur = _T("");
							if (nCount <= 4)
							{
								sCur = TGAASKIIValue(oDirEntry.nValueOffset, nCount);
							}
							else
							{
								for (int nI = 0; nI < nCount; nI++ )
									sCur += (pnFile + nValueOffset)[nI];
							}
							switch ( nTag )
							{
							case 269: m_sDocumentName = sCur; break;
							case 270: m_sDescription = sCur; break;
							case 271: m_sEquipmentType = sCur; break;
							case 272: m_sEquipmentModel = sCur; break;
							case 305: m_sSoftwareID = sCur; break;
							case 306:
								{
									// формат строки "YYYY:MM:DD HH:MM:SS"
									m_oDateTimeCreation.nYear   = _ttoi( TGAASKIIValue(pnFile + nValueOffset, 4) );
									m_oDateTimeCreation.nMonth  = _ttoi( TGAASKIIValue(pnFile + nValueOffset + 5, 2) );
									m_oDateTimeCreation.nDay    = _ttoi( TGAASKIIValue(pnFile + nValueOffset + 8, 2) );
									m_oDateTimeCreation.nHour   = _ttoi( TGAASKIIValue(pnFile + nValueOffset + 11, 2) );
									m_oDateTimeCreation.nMinute = _ttoi( TGAASKIIValue(pnFile + nValueOffset + 14, 2) );
									m_oDateTimeCreation.nSecond = _ttoi( TGAASKIIValue(pnFile + nValueOffset + 17, 2) );
									break;
								}
							case 315: m_sAuthor = sCur; break;
							case 316: m_sHostComputer = sCur; break;
							case 330: 
								// тут лежит SubIFD надо бы ее разобрать
								break;
							case 800: m_sTitle = sCur; break;
							default:
								AtlTrace ("id = 0x%x\tvalue = %s\n",nTag ,sCur);

							}
						}
						else if ( PropertyTagTypeShort == nType ||	PropertyTagTypeLong == nType)
						{
							int nValue = 0;
							if (PropertyTagTypeShort == nType)nValue = IntFromBytes2( oDirEntry.nValueOffset, nByteOrder );
							if (PropertyTagTypeLong == nType) nValue = IntFromBytes4( oDirEntry.nValueOffset, nByteOrder );
							switch(nTag)
							{
								case 0x0102:
								{
									nBitPerSample = nValue;
								}break; 
								case  0x0115:
								{
									nSamplePerPixel = nValue;
								}break; 
								case  297:
								{
									m_nPageNumber = (BYTE)oDirEntry.nValueOffset[0];
									m_nPagesCount = (BYTE)oDirEntry.nValueOffset[1];
								}break; 
								case  274:
								{
									m_nOrientation = max( 1, min( 8, nValue ));
								}break;
								case  0x8822: 
								{
									m_nExposureProgram = nValue;
								}break; 
								case  0x8827: 
								{
									m_nISOSpeed = nValue;
								}break; 
								case  0x9207:
								{
									m_nMeteringMode = nValue;
								}break; 
								case  0x9209:
								{
									m_nFlashMode = nValue;
								}
								break; 
								case  0xA001:
								{
									m_nColorSpace = nValue;
								}break;
								case  0xa217:
								{
									m_nSensingMethod = nValue;
								}break;
								case  0xa402: 
								{
									m_nExposureMode = nValue;
								}break;
								case  0xa403: 
								{
									m_nWhiteBalance = nValue;
								}break;
								case  0xa405: 
								{
									m_nFocalLengthIn35mmFilm = nValue;
								}break;								
								default:
								{
									AtlTrace ("id = 0x%x\tvalue = %d\n",nTag,nValue);
								}
							}
						}
						else if ( PropertyTagTypeRational == nType || PropertyTagTypeSRational == nType)
						{
							int nFraction    = (int)IntFromBytes4( pnFile + nValueOffset, nByteOrder );
							int nDenominator = (int)IntFromBytes4( pnFile + nValueOffset + 4, nByteOrder );
							switch(nTag)
							{
								case 0x829A:
								{
									m_oExposureTime.Fraction    = (unsigned int)nFraction;
									m_oExposureTime.Denominator = (unsigned int)nDenominator;
								}break;
								case 0x9201:
								{
									m_oShutterSpeed.Fraction    = nFraction;
									m_oShutterSpeed.Denominator = nDenominator;
								}break;
								case 0x9202:
								{
									m_oLensAperture.Fraction    = (unsigned int)nFraction;
									m_oLensAperture.Denominator = (unsigned int)nDenominator;
								}break;
								case 0x9203:
								{
									m_oBrightness.Fraction    = (int)nFraction;
									m_oBrightness.Denominator = (int)nDenominator;
								}break;
								case 0x9204:
								{
									m_oExposureCompensation.Fraction    = (int)nFraction;
									m_oExposureCompensation.Denominator = (int)nDenominator;
								}break;
								case 0x920A:
								{
									m_oFocalLength.Fraction    = (unsigned int)nFraction;
									m_oFocalLength.Denominator = (unsigned int)nDenominator;
								}break;
								case  0x829D:
								{
									m_oFNumber.Fraction    = (unsigned int)nFraction;
									m_oFNumber.Denominator = (unsigned int)nDenominator;

								}break;
								case  0x9205:
								{
									m_oMaxApertureValue.Fraction    = (unsigned int)nFraction;
									m_oMaxApertureValue.Denominator = (unsigned int)nDenominator;

								}break;							
								default:
								{
									AtlTrace ("id = 0x%x\tvalue = %d/%d\n",nTag ,nFraction,nDenominator);
								}
							}
						}

					}

					if ( m_nBitDepth <= 1 )
						m_nBitDepth = nBitPerSample * nSamplePerPixel;
				}

			}


			// Raw-foveon functions
			BOOL CheckFoveonSignature(BYTE* pnFile)
			{
				// первые четыре байта должны быть FOVb
				
				CString sSignature = TGAASKIIValue(pnFile, 4);
				if ( -1 != sSignature.Find(_T("FOVb")) )
					return TRUE;

				return FALSE;
			}
			void UTF16ToASKII(BYTE** pResult, BYTE* pSource, DWORD nLenght)
			{
				for ( int nIndex = 0; nIndex < nLenght; nIndex++ )
					(*pResult)[nIndex] = pSource[nIndex * 2];
			}
			CString FoveonASKIIValue(const BYTE* pCurPoint, DWORD nTotalLenght)
			{
				CString sReturn = _T("");
				int nIndex = 0;
				while( 0 != pCurPoint[nIndex] && nIndex < nTotalLenght)
				{
					sReturn += pCurPoint[nIndex];
					nIndex++;
				}
				
				return sReturn;
			}
			// Ras functions
			DWORD ReverseDword(DWORD dwValue)
			{
				DWORD dwResult;

				((BYTE*)&dwResult)[0] = ((BYTE*)&dwValue)[3];
				((BYTE*)&dwResult)[1] = ((BYTE*)&dwValue)[2];
				((BYTE*)&dwResult)[2] = ((BYTE*)&dwValue)[1];
				((BYTE*)&dwResult)[3] = ((BYTE*)&dwValue)[0];

				return dwResult;
			}

			// J2k functions
			BOOL ReadJpxBoxHeader   (BYTE **ppPointer, DWORD *pnMaxBytesRead, unsigned int *punBoxType, unsigned int *punBoxLen, unsigned int *punDataLen) 
			{
				unsigned int unLenH;

				if ( 8 > (*pnMaxBytesRead) )
					return FALSE;
				
				(*pnMaxBytesRead) -= 8;

				unsigned int unLen = (unsigned int)IntFromBytes4( (*ppPointer), 0 ); (*ppPointer) += 4;
				*punBoxType        = (unsigned int)IntFromBytes4( (*ppPointer), 0 ); (*ppPointer) += 4;

				if ( 1 == unLen ) 
				{
					if ( 0 > (*pnMaxBytesRead) )
						return FALSE;
					
					(*pnMaxBytesRead) -= 8;

					unLenH = (unsigned int)IntFromBytes4( (*ppPointer), 0 ); (*ppPointer) += 4;
					unLen  = (unsigned int)IntFromBytes4( (*ppPointer), 0 ); (*ppPointer) += 4;

					if ( unLenH ) 
						return FALSE;

					*punBoxLen  = unLen;
					*punDataLen = unLen - 16;
				} 
				else if ( 0 == unLen ) 
				{
					*punBoxLen  = 0;
					*punDataLen = 0;
				} 
				else 
				{
					*punBoxLen  = unLen;
					*punDataLen = unLen - 8;
				}

				return TRUE;
			}

			BOOL ReadJpxMarkerHeader(BYTE **ppPointer, DWORD *pnMaxBytesRead, int *pnSegmentType, unsigned int *pnSegmentLen) 
			{
				int nChar = 0;

				do 
				{
					do 
					{
						if ( 1 > (*pnMaxBytesRead) )
							return FALSE;

						(*pnMaxBytesRead)--;

						nChar = (*ppPointer)[0]; (*ppPointer)++;

						if ( nChar == EOF ) 
							return FALSE;

					} while ( nChar != 0xff );
					do 
					{
						if ( 1 > (*pnMaxBytesRead) )
							return FALSE;

						(*pnMaxBytesRead)--;
	
						nChar = (*ppPointer)[0]; (*ppPointer)++;

						if ( nChar == EOF)  
							return FALSE;
					} while ( nChar == 0xff );
				} while ( nChar == 0x00 );

				*pnSegmentType = nChar;
				if ( ( nChar >= 0x30 && nChar <= 0x3f ) || nChar == 0x4f || nChar == 0x92 || nChar == 0x93 || nChar == 0xd9 ) 
				{
					*pnSegmentLen = 0;
					return TRUE;
				}

				if ( 2 > (*pnMaxBytesRead) )
					return FALSE;

				(*pnMaxBytesRead) -= 2;

				*pnSegmentLen = IntFromBytes2( (*ppPointer), 0 ); (*ppPointer) += 2;

				return TRUE;
			}

			BOOL ReadJpxCodestream  (BYTE **ppPointer, DWORD *pnMaxBytesRead, unsigned int *pnBitsPerComponent, unsigned int *pnWidth, unsigned int *pnHeight, unsigned int *pnComponentCount)
			{
				int nSegmentType;
				unsigned int nSegmentLen;


				while ( ReadJpxMarkerHeader( ppPointer, pnMaxBytesRead, &nSegmentType, &nSegmentLen ) )
				{
					if ( 0x51 == nSegmentType )
					{
						unsigned int nComponentsCount, nBPC, nXsiz, nYsiz, nXOsiz, nYOsiz;

						if ( 37 > (*pnMaxBytesRead) )
							return FALSE;

						(*pnMaxBytesRead) -= 37;

						(*ppPointer) += 2;
						nXsiz  = IntFromBytes4( (*ppPointer), 0 ); (*ppPointer) += 4;
						nYsiz  = IntFromBytes4( (*ppPointer), 0 ); (*ppPointer) += 4;
						nXOsiz = IntFromBytes4( (*ppPointer), 0 ); (*ppPointer) += 4;
						nYOsiz = IntFromBytes4( (*ppPointer), 0 ); (*ppPointer) += 4;
						(*ppPointer) += 16;
						nComponentsCount = IntFromBytes2( (*ppPointer), 0 ); (*ppPointer) += 2;
						nBPC = (*ppPointer)[0]; (*ppPointer)++;

						*pnBitsPerComponent = (nBPC & 0x7f) + 1;
						*pnWidth  = nXsiz - nXOsiz;
						*pnHeight = nYsiz - nYOsiz;

						*pnComponentCount = nComponentsCount;

						break;
					}
				}

				return TRUE;
			}
			BOOL ReadJpxHeader(BYTE *pnFile, DWORD nFileSize)
			{
				BYTE *pPointer = pnFile;
				DWORD nBytesLeft = nFileSize;

				if ( 1 > nBytesLeft )
					return FALSE;

				// Считываем первый символ
				int nChar = pPointer[0];

				unsigned int nBitsPerComponent = 1;
				unsigned int nCSPrec = 0;
				unsigned int nWidth = 0, nHeight = 0;
				unsigned int nComponentsCount = 1;

				if ( 0xff == nChar )
				{
					if ( !ReadJpxCodestream( &pPointer, &nBytesLeft, &nBitsPerComponent, &nWidth, &nHeight, &nComponentsCount ) )
						return FALSE;
				}
				else
				{
					BOOL bHaveJp2Header = FALSE;
					unsigned int nBoxType, nBoxLen, nDataLen;

					while( ReadJpxBoxHeader( &pPointer, &nBytesLeft, &nBoxType, &nBoxLen, &nDataLen  ) )
					{
						if ( nBoxType == 0x6a703268 ) // JP2 header
						{
							bHaveJp2Header = TRUE;
						} 
						else if ( nBoxType == 0x69686472 ) // Image header
						{
							if ( 14 > nBytesLeft )
								return FALSE;

							nBytesLeft -= 14;

							nBitsPerComponent = pPointer[10] + 1;

							pPointer += 14;
						} 
						else if ( nBoxType == 0x6A703263 ) // Codestream
						{
							if ( !ReadJpxCodestream( &pPointer, &nBytesLeft, &nBitsPerComponent, &nWidth, &nHeight, &nComponentsCount ) )
								return FALSE;
						} 
						else 
						{
							if ( nDataLen > nBytesLeft )
								break;

							nBytesLeft -= nDataLen;

							pPointer += nDataLen;
						}
					}

					if ( !bHaveJp2Header )
						return FALSE;
				}

				m_nBitDepth = nBitsPerComponent * nComponentsCount;

				return TRUE;
			}

			// Load MetaData functions
			BOOL LoadTiffMetaData(BYTE* pnFile, DWORD nFileSize)
			{
				if ( !m_bBitmap )
					return FALSE;

				BYTE *pLastPointer = pnFile + nFileSize;
				int nTiffType = -1;
				
				const TTiffFileHeader& oHeader = *(TTiffFileHeader*)pnFile;
				CString sByteOrder(oHeader.nByteOrder);
				
				if ( -1 != sByteOrder.Find(_T("II"), 0) )
					nTiffType = 1;
				else if ( -1 != sByteOrder.Find(_T("MM"), 0) )
					nTiffType = 0;
				else
					return FALSE;

				int nSignature = IntFromBytes2(oHeader.nSignature, nTiffType);
				if ( 42 != nSignature )
					return FALSE;

				int nOffset = IntFromBytes4( oHeader.nIDFoffset, nTiffType);

				BYTE* pIDF = pnFile + nOffset;
				if ( pIDF > pLastPointer )
					return FALSE;

				int nBitPerSample = 1, nSamplePerPixel = 1;

				BYTE* pCurDirectory;
				int nDirectoryEntryCount = IntFromBytes2(pIDF, nTiffType);
				for ( int nIndex = 0; nIndex < nDirectoryEntryCount; nIndex++ )
				{
					pCurDirectory = pIDF + 2 + 12 * nIndex;
					if ( pCurDirectory > pLastPointer )
						return FALSE;
					const TDirectoryEntry& oDirEntry = *(TDirectoryEntry*)(pCurDirectory);
					int nTag   = IntFromBytes2(oDirEntry.nTag, nTiffType);
					int nType  = IntFromBytes2(oDirEntry.nType, nTiffType);
					int nCount = IntFromBytes4(oDirEntry.nCount, nTiffType);
					int nValueOffset = IntFromBytes4(oDirEntry.nValueOffset, nTiffType);
					// если тип Value меньше 4 байт, тогда в переменной ValueOffset 
					// указан не сдвиг, а самое значение Value (см., например, таг = 297)

					if ( 2 == nType ) // значение типa ASCII
					{
						CString sCur = _T("");
						if (nCount <= 4)
						{
							sCur = TGAASKIIValue(oDirEntry.nValueOffset, nCount);
						}
						else
						{
							for (int nI = 0; nI < nCount; nI++ )
								sCur += (pnFile + nValueOffset)[nI];
						}
						switch ( nTag )
						{
						case 269: m_sDocumentName = sCur; break;
						case 270: m_sDescription = sCur; break;
						case 271: m_sEquipmentType = sCur; break;
						case 272: m_sEquipmentModel = sCur; break;
						case 305: m_sSoftwareID = sCur; break;
						case 306:
							{
								// формат строки "YYYY:MM:DD HH:MM:SS"
								m_oDateTimeCreation.nYear   = _ttoi( TGAASKIIValue(pnFile + nValueOffset, 4) );
								m_oDateTimeCreation.nMonth  = _ttoi( TGAASKIIValue(pnFile + nValueOffset + 5, 2) );
								m_oDateTimeCreation.nDay    = _ttoi( TGAASKIIValue(pnFile + nValueOffset + 8, 2) );
								m_oDateTimeCreation.nHour   = _ttoi( TGAASKIIValue(pnFile + nValueOffset + 11, 2) );
								m_oDateTimeCreation.nMinute = _ttoi( TGAASKIIValue(pnFile + nValueOffset + 14, 2) );
								m_oDateTimeCreation.nSecond = _ttoi( TGAASKIIValue(pnFile + nValueOffset + 17, 2) );
								break;
							}
						case 315: m_sAuthor = sCur; break;
						case 316: m_sHostComputer = sCur; break;
						case 800: m_sTitle = sCur; break;
						}
					}
					else if ( 3 == nType ) // значение типа short
					{
						if ( 0x0102 == nTag )
						{
							nBitPerSample = (short)IntFromBytes2(oDirEntry.nValueOffset, nTiffType);
						}
						else if ( 0x0115 == nTag )
						{
							nSamplePerPixel = (short)IntFromBytes2(oDirEntry.nValueOffset, nTiffType);
						}
						else if ( 297 == nTag )
						{
							m_nPageNumber = (BYTE)oDirEntry.nValueOffset[0];
							m_nPagesCount = (BYTE)oDirEntry.nValueOffset[1];
						}
					}


				}

				if ( m_nBitDepth <= 1 )
					m_nBitDepth = nBitPerSample * nBitPerSample;

				return TRUE;
			}


			BOOL LoadPNGMetaData(BYTE* pnFile, DWORD nFileSize)
			{
				if ( !m_bBitmap )
					return FALSE;
				
				// проверяем сигнатуру PNG файла
				
				if ( !CheckPNGSignature(pnFile) )
					return FALSE;

				BYTE* pCurPoint = pnFile + 8;

				const TChunk& oFirstChunk = *(TChunk*)pCurPoint;
				CString sType = ChunkType(oFirstChunk.nType);

				if ( -1 == sType.Find(_T("IHDR")) )
					return FALSE;

				DWORD nBytesAlreadyRead = 8;
				int nLenght = IntFromBytes4(oFirstChunk.nLenght, 0);
				int nChunkSize = nLenght + 12;

				nBytesAlreadyRead += nChunkSize;

				while ( -1 == sType.Find(_T("IEND")) &&  nBytesAlreadyRead < nFileSize )
				{
					pCurPoint += nChunkSize;


					const TChunk& oCurChunk = *(TChunk*)pCurPoint;
					sType = ChunkType(oCurChunk.nType);
					nLenght = IntFromBytes4(oCurChunk.nLenght, 0);
					nChunkSize = nLenght + 12;

					// чексуммы последних четырех байтов
					crc32 oCRC;
					oCRC.ProcessCRC(pCurPoint + 4, 4 + nLenght);
					BYTE pCRC[4] = {(pCurPoint + 8 + nLenght)[0], (pCurPoint + 8 + nLenght)[1], (pCurPoint + 8 + nLenght)[2], (pCurPoint + 8 + nLenght)[3]};
					DWORD nCRC = IntFromBytes4( pCRC, 0);
					if ( oCRC.m_crc32 !=  nCRC)
						return FALSE;
					
					if ( -1 != sType.Find(_T("zTXt")) )
					{
						CString sKeyword = _T(""), sValue = _T("");
						int nIndex = 0;
						for ( ; nIndex < nLenght && (pCurPoint + 8)[nIndex] != 0; nIndex++ )
							sKeyword += (pCurPoint + 8)[nIndex];

						nIndex++;
						int nCompressionMethod = (pCurPoint + 8)[nIndex];

						nIndex++;
						


						//for ( ; nIndex < nLenght; nIndex++ )
						//	sValue += (pCurPoint + 8)[nIndex];

						//Stream^ ms = gcnew Stream;

						//ms->Position = 0;
						//for (int nI = nIndex; nI < nLenght; nI++ )
						//	ms->WriteByte((pCurPoint + 8)[nI]);
						////ms->EndWrite();
						//ms->Position = 0;
						//DeflateStream ^zipStream = gcnew DeflateStream(ms , CompressionMode::Decompress);
						//for (int nI = nIndex; nI < nLenght; nI++ )
						//	zipStream->WriteByte((pCurPoint + 8)[nI]);
						//
						//for (int i = 0 ; i < ms->Length; i++ )
						//{
						//	BYTE nCurByte;
						//	nCurByte = zipStream->ReadByte();
						//	sValue += nCurByte;
						//}
						////zipStream->Write( pCurPoint + 8 + nIndex, 0, nLength );

						//
						////Stream

						//array<Byte>^decompressedBuffer = gcnew array<Byte>(nLenght + 100);

					}
					if ( -1 != sType.Find(_T("iTXt")) )
					{
						CString sKeyword = _T("");
						int nIndex = 0;
						
						for ( ; nIndex < nLenght && (pCurPoint + 8)[nIndex] != 0; nIndex++ )
							sKeyword += (pCurPoint + 8)[nIndex];
						
						nIndex++;

						int nCompressionFlag   = (pCurPoint + 8)[nIndex];
						int nCompressionMethod = (pCurPoint + 8)[nIndex + 1];

						nIndex += 2;

						CString sLanguageTag = _T("");

						for ( ; nIndex < nLenght && (pCurPoint + 8)[nIndex] != 0; nIndex++ )
							sLanguageTag += (pCurPoint + 8)[nIndex];

						nIndex ++;

						CString sText = _T("");

						for ( ; nIndex < nLenght && (pCurPoint + 8)[nIndex] != 0; nIndex++ )
							sText += (pCurPoint + 8)[nIndex];


					}
					if ( -1 != sType.Find(_T("tEXt")) )
					{
						CString sKeyword = _T(""), sValue = _T("");
						int nIndex = 0;
						for ( ; nIndex < nLenght && (pCurPoint + 8)[nIndex] != 0; nIndex++ )
							sKeyword += (pCurPoint + 8)[nIndex];
						nIndex++;

						for ( ; nIndex < nLenght; nIndex++ )
							sValue += (pCurPoint + 8)[nIndex];

						if ( -1 != sKeyword.Find(_T("Title")) )
							m_sTitle = sValue;
						else if ( -1 != sKeyword.Find(_T("Author")) )
							m_sAuthor = sValue;
						else if ( -1 != sKeyword.Find(_T("Description")) )
							m_sDescription = sValue;
						else if ( -1 != sKeyword.Find(_T("Copyright")) )
							m_sCopyright = sValue;
						else if ( -1 != sKeyword.Find(_T("Software")) )
							m_sSoftwareID = sValue;
						else if ( -1 != sKeyword.Find(_T("Disclaimer")) )
							m_sDisclaimer = sValue;
						else if ( -1 != sKeyword.Find(_T("Warning")) )
							m_sWarning = sValue;
						else if ( -1 != sKeyword.Find(_T("Source")) )
							m_sEquipmentType = sValue;
						else if ( -1 != sKeyword.Find(_T("Comment")) )
							m_sComment = sValue;
						else if ( -1 != sKeyword.Find(_T("Creation Time")) )
							m_oDateTimeCreation.sSummary = sValue;

					}
					if ( -1 != sType.Find(_T("tIME")) )
					{
						const TChunkTIME& oTIMEChunk = *(TChunkTIME*)(pCurPoint + 8);
						int nYear = IntFromBytes2(oTIMEChunk.nYear, 0);

					}
					if ( -1 != sType.Find(_T("acTL")) )
					{
						const TChunkACTL& oACTLChunk = *(TChunkACTL*)(pCurPoint + 8);
						unsigned int unNumFrames = IntFromBytes4( oACTLChunk.unNumFrames, 0 );
						unsigned int unNumPlays  = IntFromBytes4( oACTLChunk.unNumPlays,  0 );
						if ( unNumFrames > 1 )
							m_bMultiPageImage = TRUE;
					}

					nBytesAlreadyRead += nLenght + 12;
				}


				return TRUE;
			}
			BOOL LoadTGAMetaData(BYTE* pnFile, DWORD nFileSize)
			{
				if ( m_bBitmap )
					return FALSE;

				// ищем FileFooter (26 последних байт в файле), если он есть

				BOOL bNewTGA = FALSE;

				if ( nFileSize < 26 )
					return FALSE;

				// Cчитываем Bit Depth (16-ый байт в файле)
				if ( m_nBitDepth <= 1 )
					m_nBitDepth = (short)pnFile[16];

				BYTE* pnFileFooter = pnFile + nFileSize - 26 * sizeof(BYTE);

				const TFileFooter& oFooter = *(TFileFooter*)pnFileFooter;

				BYTE aTemp[19];
				::memcpy( aTemp, oFooter.arrSignature, 18);
				aTemp[18] = '\0';

				CString sSignature(aTemp);

				if ( oFooter.nExtensionAreaOffset > nFileSize )
					return FALSE;

				// Проверяем версию TGA, если в конце есть строка "TRUEVISION-XFILE"
				// значит TGA версии 2.0
				if ( -1 != sSignature.Find(_T("TRUEVISION-XFILE"), 0) && 0 != oFooter.nExtensionAreaOffset )
				{
					BYTE* pnExtArea = pnFile + oFooter.nExtensionAreaOffset * sizeof(BYTE);
					const TExtensionArea& oExtArea = *(TExtensionArea*)pnExtArea;

					// проверяем валидность формата: если значение не 495, то
					// это не TGA 2.0
					if ( 495 == oExtArea.nExtensionSize )
					{
						m_sAuthor = TGAASKIIValue(oExtArea.sAuthorName, 41);
						m_sComment = TGAASKIIValue(oExtArea.sComments, 324);
						m_oDateTimeCreation.nYear   = oExtArea.nDateTime[2];
						m_oDateTimeCreation.nMonth  = oExtArea.nDateTime[0];
						m_oDateTimeCreation.nDay    = oExtArea.nDateTime[1];
						m_oDateTimeCreation.nHour   = oExtArea.nDateTime[3];
						m_oDateTimeCreation.nMinute = oExtArea.nDateTime[4];
						m_oDateTimeCreation.nSecond = oExtArea.nDateTime[5];

						m_sJobName = TGAASKIIValue(oExtArea.sJobName, 41);
						m_oJobTime.nHour   = oExtArea.nJobTime[0];
						m_oJobTime.nMinute = oExtArea.nJobTime[1];
						m_oJobTime.nSecond = oExtArea.nJobTime[2];

						m_sSoftwareID = TGAASKIIValue(oExtArea.sSoftwareID, 41);

						m_nVersionNumber = oExtArea.nSoftwareVersionNumber;
						m_sVersionLetter += oExtArea.nSoftwareVersionLetter;

						return TRUE;

					}
					else
					{
						return FALSE;
					}
				}
				return FALSE;
			}






			BOOL LoadFoveonMetaData(BYTE* pnFile, DWORD nFileSize)
			{
				if ( m_bBitmap )
					return FALSE;

				// это Raw формат, но метаданные в нем имеют
				// струтуру отличную от tiff, поэтому читаются
				// отдельным способом

				if ( !CheckFoveonSignature(pnFile) )
					return FALSE;

				// смещяемся на последние 4 байта, там хранится смещение
				// на Directory Section

				const int nByteOrder = 1;

				BYTE* pCurPoint = pnFile + nFileSize - 4; 

				DWORD nDirectoryOffset = IntFromBytes4(pCurPoint, nByteOrder);
				// считываем заголовок Directory Section (12 байт)

				if ( nDirectoryOffset > nFileSize )
					return FALSE;
				pCurPoint = pnFile + nDirectoryOffset;

				
				const TFoveonDirectorySectionHeader &oDirectoryHeader = *(TFoveonDirectorySectionHeader *) pCurPoint;

				// проверяем сигнатуру Directory Section, должна быть SECd
				if ( !(oDirectoryHeader.pSignature[0] == 'S' && oDirectoryHeader.pSignature[1] == 'E' &&
					   oDirectoryHeader.pSignature[2] == 'C' && oDirectoryHeader.pSignature[3] == 'd') )
					return FALSE;

				DWORD nDirectoryCount = oDirectoryHeader.nDirectoryEntriesCount;
				pCurPoint += 12;

				for ( int nIndex = 0; nIndex < nDirectoryCount; nIndex++, pCurPoint += 12 )
				{
					const TFoveonDirectoryEntries &oEntry = *(TFoveonDirectoryEntries *) pCurPoint;
					
					CString sType = TGAASKIIValue(oEntry.pType, 4);
					DWORD nLenght = oEntry.nLenght;
					DWORD nOffset = oEntry.nOffeset;

					if ( -1 != sType.Find(_T("PROP")) )
					{
						BYTE* pPROP = pnFile + nOffset;
						const TFoveonPROPHeader &oPROPHeader = *(TFoveonPROPHeader *) pPROP;

						CString sPROPSignature = TGAASKIIValue(oPROPHeader.pSignature, 4);
						// проверяем сигнатуру PROP, должна быть SECp
						if ( -1 == sPROPSignature.Find(_T("SECp")) )
							continue;

						DWORD nPROPCount = oPROPHeader.nEntriesCount;
						// 13 - максимальная длина имени типа, а именно IMAGERBOARDID
						DWORD nNameLenght = min(13, oPROPHeader.nTotalLenght);

						BYTE* pPROPEntry = pPROP + 24;

						BYTE* pData = new BYTE[oPROPHeader.nTotalLenght];
						BYTE* pDataStart = pPROP + 24 + 8 * nPROPCount;
						UTF16ToASKII(&pData, pDataStart, oPROPHeader.nTotalLenght);

						for ( int nPROPIndex = 0; nPROPIndex < nPROPCount; nPROPIndex++, pPROPEntry += 8 )
						{
							const TFoveonPROPOffsets oOffsets = *(TFoveonPROPOffsets *) pPROPEntry;
							CString sName = FoveonASKIIValue(pData + oOffsets.nName, oPROPHeader.nTotalLenght);
							CString sValue = FoveonASKIIValue(pData + oOffsets.nValue, oPROPHeader.nTotalLenght);

							if ( -1 != sName.Find(_T("CAMMANUF")) )
							{
								m_sEquipmentType = sValue;
							}

							else if ( -1 != sName.Find(_T("CAMMODEL")) )
							{
								m_sEquipmentModel = sValue;
							}

							else if ( -1 != sName.Find(_T("CAMNAME")) )
							{
								m_sTitle = sValue;
							}
							else if ( -1 != sName.Find(_T("CAMSERIAL")) )
							{
								m_sEquipmentModel += sValue;
							}
							else if ( -1 != sName.Find(_T("FIRMVERS")) )
							{
								m_sComment += _T("Firmware ") + sValue;
							}
							else if ( -1 != sName.Find(_T("TIME")) )
							{
								
								// время в формате UnixTime, т.е. количество 
								// секунд с полуночи 1/1/1970

								_int64 nTime = _ttoi64( sValue.GetBuffer() );
								long nMinutes = long(nTime / 60);
								long nSeconds = long(nTime % 60);
								long nHours   = nMinutes / 60;
								nMinutes = nMinutes % 60;
								long nDays = nHours / 24;
								nHours = nHours % 24;


								int nYear = 1970;

								while ( TRUE )
								{
									int nCurDays = 365;

									if ( 0 == nYear % 4 )
									{
										if ( 0 == nYear % 100 && 0 != nYear % 400)
											nCurDays = 365;
										else
											nCurDays = 366;
									}


									nDays -= nCurDays;
									if ( nDays < 0 )
									{
										nDays += nCurDays;
										break;
									}
									nYear++;
									
								}

								int arMonths[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
								if ( 0 == nYear % 4 )
								{
									if ( !(0 == nYear % 100 && 0 != nYear % 400) )
										arMonths[1]++;
								}

								short nMonth = 1;

								for ( int nMonthIndex = 0; nMonthIndex < 12; nMonthIndex++ )
								{
									if ( nDays < arMonths[nMonthIndex] )
										break;
									nDays -= arMonths[nMonthIndex];
									nMonth++;
								}


								m_oDateTimeCreation.nYear   = short( nYear );
								m_oDateTimeCreation.nMonth  = nMonth;
								m_oDateTimeCreation.nDay    = short( nDays + 1 );
								m_oDateTimeCreation.nHour   = short( nHours );
								m_oDateTimeCreation.nMinute = short( nMinutes );
								m_oDateTimeCreation.nSecond = short( nSeconds );

							}

						}

						delete pData;

					}
				}

				// Bit Depth всегда 24
				m_nBitDepth = 24;

				return TRUE;
			}
			BOOL LoadRawMetaData(CString sFilePath)
			{
			
				ImageRaw::IImageRaw3 *pRawFile = NULL;
				::CoCreateInstance( ImageRaw::CLSID_CImageRaw3, NULL, CLSCTX_INPROC, ImageRaw::IID_IImageRaw3, (void**)&pRawFile );

				if ( NULL == pRawFile )
					return FALSE;

				BSTR bsFilePath = sFilePath.AllocSysString();

				ImageRaw::IImageRaw3Metadata* pRawMetaData = NULL;
				pRawFile->QueryInterface( ImageRaw::IID_IImageRaw3Metadata, (void**)(&pRawMetaData));

				if ( NULL == pRawMetaData )
				{
					RELEASEINTERFACE(pRawFile);
					return FALSE;
				}

				if ( S_OK == pRawMetaData->raw_GetMetaData(bsFilePath) )
				{
					pRawFile->raw_CloseFile();
					::SysFreeString(bsFilePath);

					DWORD nIFDCount = 0;
					pRawMetaData->raw_IFDCount(0, &nIFDCount);

					if ( 0 == nIFDCount )
					{
						RELEASEINTERFACE(pRawMetaData);
						RELEASEINTERFACE(pRawFile);
						return FALSE;
					}
					// открываем файл
					HANDLE hFile = CreateFile(sFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
					if (INVALID_HANDLE_VALUE == hFile)
					{
						RELEASEINTERFACE(pRawMetaData);
						RELEASEINTERFACE(pRawFile);
						return FALSE; // Невозможно открыть файл
					}

					// мапим этот файл в память - так быстрее читаются данные из файла
					DWORD  nFileSize = GetFileSize(hFile, NULL);
					HANDLE hMapFile  = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, nFileSize, NULL);
					if (NULL == hMapFile)
					{
						CloseHandle( hFile );
						RELEASEINTERFACE(pRawMetaData);
						RELEASEINTERFACE(pRawFile);
						return FALSE; // Невозможно создать отображение файла
					}

					// создаем view of file
					DWORD nMaxBytesRead	= nFileSize;
					VOID* pBaseAddress	= MapViewOfFile( hMapFile, FILE_MAP_READ, 0, 0, 0 );
					BYTE* pnFile		= (BYTE*)pBaseAddress;
					DWORD nByteOrder	=0;

					int r = IntFromBytes2(pnFile,0);
					
					if ( 0x4949	== r)	nByteOrder = 1;
					if ( 0x4d4d == r )	nByteOrder = 0;				
					
					for (int nIndex = 0; nIndex < nIFDCount; nIndex++ )
					{
						DWORD nIFDOffset = 0;
						DWORD nBaseIFDOffset = 0;
						unsigned short nIFDTag = 0;
						pRawMetaData->raw_IFDOffset(nIndex, &nIFDOffset);
						pRawMetaData->raw_BaseOffset(nIndex, &nBaseIFDOffset);
						pRawMetaData->raw_IFDTag(nIndex, &nIFDTag);

						r = IntFromBytes2(pnFile+nIFDOffset-min(nIFDOffset,8),0);
						
						if ( 0x4949	== r)	nByteOrder = 1;
						if ( 0x4d4d == r )	nByteOrder = 0;
						
						ReadRawIFD( pnFile, nIFDTag, nIFDOffset,nByteOrder, nIndex, nBaseIFDOffset);

					}

					if ( m_nBitDepth <= 1 )
						m_nBitDepth = 24;

					UnmapViewOfFile(pBaseAddress);
					CloseHandle(hMapFile);
					CloseHandle( hFile );
					RELEASEINTERFACE(pRawFile);
					RELEASEINTERFACE(pRawMetaData);
					return TRUE;
				
				}
				pRawFile->CloseFile();
				::SysFreeString(bsFilePath);
				RELEASEINTERFACE(pRawMetaData);
				RELEASEINTERFACE(pRawFile);
				return FALSE;
			}


			BOOL LoadBitmapMetaData(Bitmap*& bitmap)
			{
				// retrieve info properties
				m_sTitle                     = GetPropertyStr(bitmap, PropertyTagImageTitle); 
				m_sAuthor                    = GetPropertyStr(bitmap, PropertyTagArtist);
				m_sDescription               = GetPropertyStr(bitmap, PropertyTagImageDescription);
				
				CString sDateTime            = GetPropertyStr(bitmap, PropertyTagDateTime);
				
				if ( 19 <= sDateTime.GetLength() )
				{
					TCHAR *pDateTime = sDateTime.GetBuffer();
					m_oDateTimeCreation.nYear   = _ttoi( pDateTime );
					m_oDateTimeCreation.nMonth  = _ttoi( pDateTime + 5 );
					m_oDateTimeCreation.nDay    = _ttoi( pDateTime + 8 );
					m_oDateTimeCreation.nHour   = _ttoi( pDateTime + 11 );
					m_oDateTimeCreation.nMinute = _ttoi( pDateTime + 14 );
					m_oDateTimeCreation.nSecond = _ttoi( pDateTime + 17 );

				}
				else
					m_oDateTimeCreation.sSummary = sDateTime;

				m_sCopyright                 = GetPropertyStr(bitmap, PropertyTagCopyright);
				m_sHostComputer              = GetPropertyStr(bitmap, PropertyTagHostComputer);
				m_sEquipmentType             = GetPropertyStr(bitmap, PropertyTagEquipMake);
				m_sEquipmentModel            = GetPropertyStr(bitmap, PropertyTagEquipModel);
				m_sSoftwareID                = GetPropertyStr(bitmap, PropertyTagSoftwareUsed);
				m_sDocumentName              = GetPropertyStr(bitmap, PropertyTagDocumentName);
				m_sPageName                  = GetPropertyStr(bitmap, PropertyTagPageName);
			
				short nPages = GetPropertyInt(bitmap, PropertyTagPageNumber);

				m_nPageNumber = (BYTE)nPages;  
				m_nPagesCount = (BYTE)nPages>> 8;

				GetPropertyRationalU(bitmap, PropertyTagExifExposureTime, &m_oExposureTime);
				GetPropertyRational (bitmap, PropertyTagExifExposureBias, &m_oExposureCompensation);
				GetPropertyRational (bitmap, PropertyTagExifShutterSpeed, &m_oShutterSpeed);
				GetPropertyRationalU(bitmap, PropertyTagExifAperture,     &m_oLensAperture);
				GetPropertyRationalU(bitmap, PropertyTagExifFocalLength,  &m_oFocalLength);
				GetPropertyRationalU(bitmap, PropertyTagExifFNumber,      &m_oFNumber);
				GetPropertyRational (bitmap, PropertyTagExifBrightness,   &m_oBrightness);
				GetPropertyRationalU (bitmap, PropertyTagExifMaxAperture, &m_oMaxApertureValue);
			
				m_nExposureProgram      = GetPropertyShort(bitmap, PropertyTagExifExposureProg);
				m_nISOSpeed             = GetPropertyShort(bitmap, PropertyTagExifISOSpeed);
				m_nMeteringMode         = GetPropertyShort(bitmap, PropertyTagExifMeteringMode);
				m_nFlashMode            = GetPropertyShort(bitmap, PropertyTagExifFlash);
				m_nColorSpace           = GetPropertyShort(bitmap, PropertyTagExifColorSpace);
				m_nExposureMode			=GetPropertyShort(bitmap,PropertyTagExifExposureMode);
				m_nWhiteBalance			=GetPropertyShort(bitmap,PropertyTagExifWhiteBalance);
				m_nSensingMethod		=GetPropertyShort(bitmap,PropertyTagExifSensingMethod);
				m_nFocalLengthIn35mmFilm=GetPropertyShort(bitmap,PropertyTagExifFocalLengthIn35mmFilm);
				m_nOrientation          = GetPropertyShort(bitmap, PropertyTagOrientation);

				if ( m_nBitDepth <= 1 )
					m_nBitDepth = (short)GetPixelFormatSize( bitmap->GetPixelFormat() );


				// Запрашиваем формат картинки
				GUID guidFormat;
				bitmap->GetRawFormat( &guidFormat );

				// check for format type
				if ( ImageFormatTIFF == guidFormat )
				{
					if( bitmap->GetFrameCount(&FrameDimensionPage) > 1 )
						m_bMultiPageImage = TRUE;
				}
				else if ( ImageFormatGIF == guidFormat )
				{
					if( bitmap->GetFrameCount(&FrameDimensionTime) > 1 )
						m_bMultiPageImage = TRUE;
				}


				// remove property load-save errors here
				if (bitmap->GetLastStatus() != Ok)
					return FALSE;

				return TRUE;
			}
			BOOL LoadPSDMetaData(BYTE* pnFile, DWORD nFileSize)
			{
				if ( m_bBitmap )
					return FALSE;

				DWORD nHeaderSize = sizeof(TPSDFileHeader);

				if( nHeaderSize > nFileSize )
					return FALSE;

				const TPSDFileHeader& oHeader = *(TPSDFileHeader*)pnFile;

				// Проверяем сигнатуру
				if( 0x38425053 /* 8BPS */ != IntFromBytes4( oHeader.nSignature, 0 ) )
					return FALSE;
					
				if( 1 != IntFromBytes2( oHeader.nVersion, 0 ) )
					return FALSE;

				int nWidth  = IntFromBytes4( oHeader.nColumns, 0 );
				int nHeight = IntFromBytes4( oHeader.nRows,    0 );
					
				int nChannels   = IntFromBytes2( oHeader.nChannels, 0 );
				int nPlaneDepth = IntFromBytes2( oHeader.nDepth,    0 );

				m_nBitDepth = nChannels * nPlaneDepth;

				return TRUE;
			}
			BOOL LoadPCXMetaData(BYTE* pnFile, DWORD nFileSize)
			{
				if ( m_bBitmap )
					return FALSE;

				DWORD nHeaderSize = sizeof(TPCXFileHeader);

				if( nHeaderSize > nFileSize )
					return FALSE;

				const TPCXFileHeader& oHeader = *(TPCXFileHeader*)pnFile;

				// Проверяем формат
				if( 10 != oHeader.nManufacturer )
					return FALSE; 

				switch( oHeader.nVersion )
				{
				case 0:	 break;
				case 2:	 break;
				case 3:	 break;
				case 4:  break;
				case 5:	 break;
				default: return FALSE;
				}

				switch( oHeader.bEncoding )
				{
				case 0:	 break;
				case 1:  break;
				default: return FALSE;
				}

				switch( oHeader.nBitsPerPixelPerPlane )
				{
				case 1:  break;
				case 4:  break;
				case 8:  break;
				case 16: break;
				case 24: break;
				case 32: break;
				default: return FALSE;
				}


				int  nWidth     = oHeader.nMaxX - oHeader.nMinX + 1;
				int  nHeight    = oHeader.nMaxY - oHeader.nMinY + 1;
				BOOL bGreyScale = (2 == oHeader.bGrayscalePalette) ? TRUE : FALSE;
				int  nPlanes    = oHeader.nPlanes;
				int  nDepth     = oHeader.nBitsPerPixelPerPlane;
				int  nBytesPerLinePerPlane = oHeader.nBytesPerLinePerPlane;

				m_nBitDepth = nPlanes * nDepth;

				return TRUE;			
			}
			BOOL LoadRasMetaData(BYTE* pnFile, DWORD nFileSize)
			{
				if ( m_bBitmap )
					return FALSE;

				DWORD nHeaderSize = sizeof(TRasFileHeader);

				if( nHeaderSize > nFileSize )
					return FALSE;

				const TRasFileHeader& oHeader = *(TRasFileHeader*)pnFile;

				// Проверяем сигнатуру
				if( 0x59a66a95 != ReverseDword( oHeader.nMagicNumber ) )
					return FALSE;

				int nWidth  = ReverseDword( oHeader.nWidth );
				int nHeight = ReverseDword( oHeader.nHeight );

				if ( 0 == nWidth || 0 == nHeight )
					return FALSE;

				int nDepth = ReverseDword( oHeader.nDepth );
				switch( nDepth )
				{
				case 1:  break;
				case 8:  break;
				case 24: break;
				case 32: break;
				default: return FALSE;
				}


				int nBytesPerLine = ReverseDword(oHeader.nSize) / nHeight;
				if( nBytesPerLine < 2 )
					return FALSE;

				BOOL bPalette;
				switch( ReverseDword(oHeader.nPaletteType) )
				{
				case 0: bPalette = FALSE; break;
				case 1: bPalette = TRUE; break;
				default: return FALSE;
				}

				BOOL bCompressed = FALSE;
				switch( ReverseDword(oHeader.nType) )
				{
				case 0: break; // old format
				case 1: break; // standart format
				case 2: bCompressed = TRUE; break;
				case 3: break; // RGB format
				case 4: break; // convert from TIFF format
				case 5: break; // convert from IFF format
				default: return FALSE;
				}

				m_nBitDepth = nDepth;

				return TRUE;
			}
			BOOL LoadJ2kMetaData(BYTE* pnFile, DWORD nFileSize)
			{
				if ( m_bBitmap )
					return FALSE;

				return ReadJpxHeader( pnFile, nFileSize );
			}
			// Save MetaDatafunctions
			BOOL SaveAsTGA(ATL::CAtlFile& oResultFile)
			{
				//сохранение метадынных (ExtensionArea)

				TExtensionArea oExtArea;
				
				ZeroMemory( &oExtArea, sizeof(TExtensionArea) );

				oExtArea.nExtensionSize = 495;
				BytesFromString(m_sAuthor, oExtArea.sAuthorName, 41);
				BytesFromString(m_sComment, oExtArea.sComments, 324);
				oExtArea.nDateTime[0] = m_oDateTimeCreation.nMonth;
				oExtArea.nDateTime[1] = m_oDateTimeCreation.nDay;
				oExtArea.nDateTime[2] = m_oDateTimeCreation.nYear;
				oExtArea.nDateTime[3] = m_oDateTimeCreation.nHour;
				oExtArea.nDateTime[4] = m_oDateTimeCreation.nMinute;
				oExtArea.nDateTime[5] = m_oDateTimeCreation.nSecond;
				BytesFromString(m_sJobName, oExtArea.sJobName, 41);
				oExtArea.nJobTime[0] = m_oJobTime.nHour;
				oExtArea.nJobTime[1] = m_oJobTime.nMinute;
				oExtArea.nJobTime[2] = m_oJobTime.nSecond;
				BytesFromString(m_sSoftwareID, oExtArea.sSoftwareID, 41);
				BytesFromString(m_sVersionLetter, &oExtArea.nSoftwareVersionLetter, 1);
				oExtArea.nSoftwareVersionNumber = m_nVersionNumber;
				oExtArea.nKeyColor = 0;
				oExtArea.nPixelAspectRatio[0] = 0;
				oExtArea.nPixelAspectRatio[1] = 0;
				oExtArea.nGammaValue[0] = 0;
				oExtArea.nGammaValue[1] = 0;
				oExtArea.nColorCorrectionOffset = 0;
				oExtArea.nPostageStampOffset = 0;
				oExtArea.nScanLineOffset = 0;
				oExtArea.nAttributesType = 0;

				DWORD nBytesWritten = 0;
				if( S_OK != oResultFile.Write( &oExtArea, sizeof(TExtensionArea), &nBytesWritten))
					return FALSE;

				ULONGLONG nLen;
				oResultFile.GetSize((ULONGLONG&)nLen);

				//сохранение FileFooter

				TFileFooter oFileFooter;
				
				ZeroMemory( &oFileFooter, sizeof(TFileFooter) );
				
				//посчитать размер файла и вычесть 495
				oFileFooter.nExtensionAreaOffset = long(nLen) - 495; 
				oFileFooter.nDeveloperAreaOffset = 0;
				BYTE sSignature[18] = {'T', 'R', 'U', 'E', 'V', 'I', 'S', 'I', 'O', 'N', '-', 'X', 'F', 'I', 'L', 'E', '.', 0x00};
				for (int nIndex = 0; nIndex  < 18; nIndex++ )
					oFileFooter.arrSignature[nIndex] = sSignature[nIndex];

				if( S_OK != oResultFile.Write( &oFileFooter, 26, &nBytesWritten))
					return FALSE;

				return TRUE;
			}

			BOOL SaveAsPNG(ATL::CAtlFile& oResultFile)
			{
				if ( 0 < m_sTitle.GetLength() )
					SaveChunk(oResultFile, _T("Title"), m_sTitle);
				if ( 0 < m_sAuthor.GetLength() )
					SaveChunk(oResultFile, _T("Author"), m_sAuthor);
				if ( 0 < m_sDescription.GetLength() )
					SaveChunk(oResultFile, _T("Description"), m_sDescription);
				if ( 0 < m_sCopyright.GetLength() )
					SaveChunk(oResultFile, _T("Copyright"), m_sCopyright);
				if ( 0 < m_oDateTimeCreation.sSummary.GetLength() )
					SaveChunk(oResultFile, _T("Creation Time"), m_oDateTimeCreation.sSummary);
				if ( 0 < m_sSoftwareID.GetLength() )
					SaveChunk(oResultFile, _T("Software"), m_sSoftwareID);
				if ( 0 < m_sDisclaimer.GetLength() )
					SaveChunk(oResultFile, _T("Disclaimer"), m_sDisclaimer);
				if ( 0 < m_sWarning.GetLength() )
					SaveChunk(oResultFile, _T("Warning"), m_sWarning);
				//CString sSource = m_sEquipmentType + " " + m_sEquipmentModel;
				//if ( 1 < sSource.GetLength() )
				//	SaveChunk(oResultFile, "Source", sSource, 0);
				if ( 0 < m_sEquipmentType.GetLength() )
					SaveChunk(oResultFile, _T("Source"), m_sEquipmentType);
				if ( 0 < m_sComment.GetLength() )
					SaveChunk(oResultFile, _T("Comment"), m_sComment);

				TChunk oLastChunk;
				IntToBytes4(0, oLastChunk.nLenght);
				oLastChunk.nType[0] = 'I';
				oLastChunk.nType[1] = 'E';
				oLastChunk.nType[2] = 'N';
				oLastChunk.nType[3] = 'D';

				crc32 oCRC;
				oCRC.ProcessCRC(oLastChunk.nType, 4);



				BYTE CRC[4] = {0, 0, 0, 0};
				IntToBytes4(oCRC.m_crc32, CRC);
				if( S_OK != oResultFile.Write( &oLastChunk, 8))
					return FALSE;
				if( S_OK != oResultFile.Write( CRC, 4))
					return FALSE;

				
				// С записью DATE/TIME что-то не так

				return TRUE;
			}
			
			void WriteNode(XmlUtils::CXmlWriter &oXmlWriter, const CString &m_sName, const CString &m_sValue)
			{
				CString sValue = m_sValue;
				sValue.Replace(_T("&"), _T("&amp;"));
				sValue.Replace(_T("<"), _T("&lt;"));
				sValue.Replace(_T(">"), _T("&gt;"));
				sValue.Replace(_T("'"), _T("&apos;"));
				sValue.Replace(_T("\""), _T("&quot;"));
				oXmlWriter.WriteNode(m_sName, sValue.GetBuffer());
			}

			BOOL GetValueOfAttribute(CString sAttribute, CString *sValue)
			{
				int nPosition = m_sMetaDataXML.Find(sAttribute);

				if ( -1 == nPosition )
					return FALSE;

				nPosition += int(m_sMetaDataXML.GetLength());
				while ( ' ' != m_sMetaDataXML[nPosition] && '\\' != m_sMetaDataXML[nPosition] )
				{
					*sValue += m_sMetaDataXML[nPosition];
					nPosition++;
				}

				return TRUE;
			}

			void ReadNodeText(XmlUtils::CXmlReader &oReader, const CString &sName, CString *sValue)
			{
				oReader.ReadRootNode();
				oReader.ReadNode(sName);
				*sValue = oReader.ReadNodeText();
			}
			void ReadNodeText(XmlUtils::CXmlReader &oReader, const CString &sName, TRational *oValue)
			{
				CString sValue = _T("");
				oReader.ReadRootNode();
				oReader.ReadNode(sName);
				sValue = oReader.ReadNodeText();
				int pos_flash = sValue.Find(_T("/"));
				int pos_point = sValue.Find(_T("."));
				
				if (pos_flash >=0)
				{
					oValue->Fraction = _ttoi(sValue.Left(pos_flash));
					oValue->Denominator = _ttoi(sValue.Mid(pos_flash+1));
				}
				else
				{
					if (pos_point>=0)
					{
						float fValue = _tstof(sValue);
						oValue->Denominator = 100;
						oValue->Fraction = int(fValue*100);
					}
					else
					{
						oValue->Fraction = _ttoi(sValue);
						oValue->Denominator = 0;
					}
				}
			}			
			void ReadNodeText(XmlUtils::CXmlReader &oReader, const CString &sName, TRationalU *oValue)
			{
				CString sValue = _T("");
				oReader.ReadRootNode();
				oReader.ReadNode(sName);
				sValue = oReader.ReadNodeText();
				int pos_flash = sValue.Find(_T("/"));
				int pos_point = sValue.Find(_T("."));
				
				if (pos_flash >=0)
				{
					oValue->Fraction = _ttoi(sValue.Left(pos_flash));
					oValue->Denominator = _ttoi(sValue.Mid(pos_flash+1));
				}
				else
				{
					if (pos_point>=0)
					{
						float fValue = _tstof(sValue);
						oValue->Denominator = 100;
						oValue->Fraction = int(fValue*100);
					}
					else
					{
						oValue->Fraction = _ttoi(sValue);
						oValue->Denominator = 0;
					}
				}
			}
			void ReadNodeText(XmlUtils::CXmlReader &oReader, const CString &sName, short *nValue)
			{
				CString sValue = _T("");
				oReader.ReadRootNode();
				oReader.ReadNode(sName);
				sValue = oReader.ReadNodeText();
				*nValue = _ttoi(sValue);
			}
			void ReadNodeText(XmlUtils::CXmlReader &oReader, const CString &sName, BOOL *nValue)
			{
				CString sValue = _T("");
				oReader.ReadRootNode();
				oReader.ReadNode(sName);
				sValue = oReader.ReadNodeText();
				*nValue = _ttoi(sValue);
			}
			void ReadSubNodeText(XmlUtils::CXmlReader &oReader, const CString &sNodeName, const CString &sSubNodeName, CString *sValue)
			{
				oReader.ReadRootNode();
				oReader.ReadNode(sNodeName);
				oReader.ReadNode(sSubNodeName);
				*sValue = oReader.ReadNodeText();
			}

			void ReadSubNodeText(XmlUtils::CXmlReader &oReader, const CString &sNodeName, const CString &sSubNodeName, short *nValue)
			{
				CString sValue = _T("");
				oReader.ReadRootNode();
				oReader.ReadNode(sNodeName);
				oReader.ReadNode(sSubNodeName);
				sValue = oReader.ReadNodeText();
				*nValue = _ttoi(sValue);
			}

			BOOL MakeXML()
			{
				XmlUtils::CXmlWriter oXmlWriter;

				oXmlWriter.WriteNodeBegin(_T("MetaData"));

				WriteNode(oXmlWriter, _T("Title"), m_sTitle);
				WriteNode(oXmlWriter, _T("Author"), m_sAuthor);
				WriteNode(oXmlWriter, _T("Description"), m_sDescription);

				oXmlWriter.WriteNodeBegin(_T("DateTime"));

				oXmlWriter.WriteNode(_T("Year"), m_oDateTimeCreation.nYear);
				oXmlWriter.WriteNode(_T("Month"), m_oDateTimeCreation.nMonth);
				oXmlWriter.WriteNode(_T("Day"), m_oDateTimeCreation.nDay);
				oXmlWriter.WriteNode(_T("Hour"), m_oDateTimeCreation.nHour);
				oXmlWriter.WriteNode(_T("Minute"), m_oDateTimeCreation.nMinute);
				oXmlWriter.WriteNode(_T("Second"), m_oDateTimeCreation.nSecond);
				WriteNode(oXmlWriter, _T("Summary"), m_oDateTimeCreation.sSummary);
				
				oXmlWriter.WriteNodeEnd(_T("DateTime"));

				WriteNode(oXmlWriter, _T("Copyright"), m_sCopyright);
				WriteNode(oXmlWriter, _T("Disclaimer"), m_sDisclaimer);
				WriteNode(oXmlWriter, _T("Comment"), m_sComment);
				WriteNode(oXmlWriter,_T("EquipmentType"), m_sEquipmentType);
				WriteNode(oXmlWriter, _T("EquipmentModel"), m_sEquipmentModel);

				oXmlWriter.WriteNodeBegin(_T("JobTime"));

				oXmlWriter.WriteNode(_T("Hour"), m_oJobTime.nHour);
				oXmlWriter.WriteNode(_T("Minute"), m_oJobTime.nMinute);
				oXmlWriter.WriteNode(_T("Second"), m_oJobTime.nSecond);

				oXmlWriter.WriteNodeEnd(_T("JobTime"));

				WriteNode(oXmlWriter, _T("JobName"), m_sJobName);
				WriteNode(oXmlWriter, _T("SoftwareID"), m_sSoftwareID);
				oXmlWriter.WriteNode(_T("VersionNumber"), m_nVersionNumber);
				WriteNode(oXmlWriter, _T("VersionLetter"), m_sVersionLetter);
				WriteNode(oXmlWriter, _T("HostComputer"), m_sHostComputer);
				WriteNode(oXmlWriter, _T("Warning"), m_sWarning);
				WriteNode(oXmlWriter, _T("DocumentName"), m_sDocumentName);
				WriteNode(oXmlWriter, _T("PageName"), m_sPageName);
				oXmlWriter.WriteNode(_T("PageNumber"), m_nPageNumber);
				oXmlWriter.WriteNode(_T("PagesCount"), m_nPagesCount);

				if ( 1 == m_oExposureTime.Fraction || ( 0 != m_oExposureTime.Fraction && 0 == fmod( (float)m_oExposureTime.Denominator, (float)m_oExposureTime.Fraction ) ) )
				{
					WriteNode(oXmlWriter, _T("ExposureTime"), RationalUToString(m_oExposureTime) );
				}
				else if ( 0 != m_oExposureTime.Denominator )
				{
					float fExposureTime = (float)m_oExposureTime.Fraction / (float)m_oExposureTime.Denominator;
					CString sExposureTime;
					sExposureTime.Format(_T("%.1f"), fExposureTime);
					WriteNode(oXmlWriter, _T("ExposureTime"), sExposureTime );
				}
				else WriteNode(oXmlWriter, _T("ExposureTime"), _T("0") );
		
				CString sExposureProgram = _T("");
				switch(m_nExposureProgram)
				{
					case 0: sExposureProgram = _T("Not Defined"); break;
					case 1: sExposureProgram = _T("Manual"); break;
					case 2: sExposureProgram = _T("Normal program"); break;
					case 3: sExposureProgram = _T("Aperture priority"); break;
					case 4: sExposureProgram = _T("Shutter priority"); break;
					case 5: sExposureProgram = _T("Creative program (Slow speed)");  break;
					case 6: sExposureProgram = _T("Action program (High speed)"); break;
					case 7: sExposureProgram = _T("Portrait mode"); break;
					case 8: sExposureProgram = _T("Landscape mode"); break;
				}			
				WriteNode(oXmlWriter, _T("ExposureProgram"), sExposureProgram);

				if ( 1 == m_oShutterSpeed.Fraction || ( 0 != m_oShutterSpeed.Fraction && 0 == fmod( (float)m_oShutterSpeed.Denominator, (float)m_oShutterSpeed.Fraction ) ) )
				{
					WriteNode(oXmlWriter, _T("ShutterSpeed"), RationalToString(m_oShutterSpeed) );
				}
				else if ( 0 != m_oShutterSpeed.Denominator )
				{
					float fShutterSpeed = (float)m_oShutterSpeed.Fraction / (float)m_oShutterSpeed.Denominator;
					CString sShutterSpeed;
					sShutterSpeed.Format(_T("%.2f"), fShutterSpeed);
					WriteNode(oXmlWriter, _T("ShutterSpeed"), sShutterSpeed );
				}
				else
				{
					oXmlWriter.WriteNode( _T("ShutterSpeed"), m_oShutterSpeed.Fraction );
				}

				WriteNode(oXmlWriter, _T("LensAperture"), RationalUToString(m_oLensAperture) );
				WriteNode(oXmlWriter, _T("MaxApertureValue"), RationalUToString(m_oMaxApertureValue) );
				WriteNode(oXmlWriter, _T("FocalLength"), RationalUToString(m_oFocalLength) );
					
				if ( 0 != m_oFNumber.Denominator )
				{
					float fFNumber = (float)m_oFNumber.Fraction / (float)m_oFNumber.Denominator;
					CString sFNumber;
					sFNumber.Format(_T("%.2f"), fFNumber);
					WriteNode(oXmlWriter, _T("FNumber"), sFNumber );
				}
				else oXmlWriter.WriteNode( _T("FNumber"), (int)m_oFNumber.Fraction);
			
				if ( 0 != m_oBrightness.Denominator )
				{
					float fFNumber = (float)m_oBrightness.Fraction / (float)m_oBrightness.Denominator;
					CString sFNumber;
					sFNumber.Format(_T("%.2f"), fFNumber);
					WriteNode(oXmlWriter, _T("Brightness"), sFNumber );
				}
				else oXmlWriter.WriteNode( _T("Brightness"), (int)m_oBrightness.Fraction);

				if ( 0 != m_oExposureCompensation.Denominator )
				{
					float fFNumber = (float)m_oExposureCompensation.Fraction / (float)m_oExposureCompensation.Denominator;
					CString sFNumber;
					sFNumber.Format(_T("%.2f"), fFNumber);
					WriteNode(oXmlWriter, _T("ExposureCompensation"), sFNumber );
				}
				else oXmlWriter.WriteNode( _T("ExposureCompensation"), (int)m_oExposureCompensation.Fraction);

				oXmlWriter.WriteNode(_T("ISOSpeed"), m_nISOSpeed);
				oXmlWriter.WriteNode(_T("SensingMethod"), m_nSensingMethod);
				oXmlWriter.WriteNode(_T("WhiteBalance"), m_nWhiteBalance);
				oXmlWriter.WriteNode(_T("ExposureMode"), m_nExposureMode);
				oXmlWriter.WriteNode(_T("FocalLengthIn35mmFilm"), m_nFocalLengthIn35mmFilm);

				CString sMeteringMode = _T("");
				switch(m_nMeteringMode)
				{
					case 0: sMeteringMode = _T("Unknown"); break;
					case 1: sMeteringMode = _T("Average"); break;
					case 2: sMeteringMode = _T("Center-weighted average"); break;
					case 3: sMeteringMode = _T("Spot"); break;
					case 4: sMeteringMode = _T("Multi-spot"); break;
					case 5: sMeteringMode = _T("Pattern");  break;
					case 6: sMeteringMode = _T("Partial"); break;
					case 255: sMeteringMode = _T(""); break;
				}
				WriteNode(oXmlWriter, _T("MeteringMode"), sMeteringMode);

				CString sFlashMode = _T("");
				switch(m_nFlashMode)
				{
					case 0x0: sFlashMode = _T("No Flash"); break;
					case 0x1: sFlashMode = _T("Fired"); break; 
					case 0x5: sFlashMode = _T("Fired, Return not detected"); break; 
					case 0x7: sFlashMode = _T("Fired, Return detected"); break; 
					case 0x8: sFlashMode = _T("On, Did not fire"); break; 
					case 0x9: sFlashMode = _T("On, Fired"); break; 
					case 0xd: sFlashMode = _T("On, Return not detected"); break; 
					case 0xf: sFlashMode = _T("On, Return detected"); break; 
					case 0x10: sFlashMode = _T("Off, Did not fire"); break; 
					case 0x14: sFlashMode = _T("Off, Did not fire, Return not detected"); break; 
					case 0x18: sFlashMode = _T("Auto, Did not fire"); break; 
					case 0x19: sFlashMode = _T("Auto, Fired"); break; 
					case 0x1d: sFlashMode = _T("Auto, Fired, Return not detected"); break; 
					case 0x1f: sFlashMode = _T("Auto, Fired, Return detected"); break; 
					case 0x20: sFlashMode = _T("No flash function"); break; 
					case 0x30: sFlashMode = _T("Off, No flash function"); break; 
					case 0x41: sFlashMode = _T("Fired, Red-eye reduction"); break; 
					case 0x45: sFlashMode = _T("Fired, Red-eye reduction, Return not detected"); break; 
					case 0x47: sFlashMode = _T("Fired, Red-eye reduction, Return detected"); break; 
					case 0x49: sFlashMode = _T("On, Red-eye reduction"); break; 
					case 0x4d: sFlashMode = _T("On, Red-eye reduction, Return not detected"); break; 
					case 0x4f: sFlashMode = _T("On, Red-eye reduction, Return detected"); break; 
					case 0x50: sFlashMode = _T("Off, Red-eye reduction"); break; 
					case 0x58: sFlashMode = _T("Auto, Did not fire, Red-eye reduction"); break; 
					case 0x59: sFlashMode = _T("Auto, Fired, Red-eye reduction"); break; 
					case 0x5d: sFlashMode = _T("Auto, Fired, Red-eye reduction, Return not detected"); break; 
					case 0x5f: sFlashMode = _T("Auto, Fired, Red-eye reduction, Return detected"); break;
				}
				WriteNode(oXmlWriter, _T("FlashMode"), sFlashMode);

				CString sColorSpace = _T("");
				switch(m_nColorSpace)
				{
					case 1: sColorSpace = _T("sRGB"); break;
					case 65535: sColorSpace = _T("Uncalibrated"); break;
				}
				WriteNode(oXmlWriter, _T("ColorSpace"), sColorSpace);


				// Дополнительный параметр
				oXmlWriter.WriteNode(_T("BitDepth"), m_nBitDepth);
				oXmlWriter.WriteNode(_T("MultiPageImage"), (int)m_bMultiPageImage );

				oXmlWriter.WriteNodeEnd(_T("MetaData"));
#ifdef _UNICODE
				oXmlWriter.EncodingUnicodeToUTF8();
#else
				oXmlWriter.EncodingASCIIToUTF8();
#endif
				
				m_sMetaDataXML = oXmlWriter.GetXmlString();

				return TRUE;
			}

			BOOL ParseXML(CString sXML)
			{
				XmlUtils::CXmlReader oReader;
				
				oReader.SetXmlString(sXML);

				ReadNodeText(oReader, _T("Title"),       &m_sTitle);
				ReadNodeText(oReader, _T("Author"),      &m_sAuthor);
				ReadNodeText(oReader, _T("Description"), &m_sDescription);

				ReadSubNodeText(oReader, _T("DateTime"), _T("Year"),    &m_oDateTimeCreation.nYear);
				ReadSubNodeText(oReader, _T("DateTime"), _T("Month"),   &m_oDateTimeCreation.nMonth);
				ReadSubNodeText(oReader, _T("DateTime"), _T("Day"),     &m_oDateTimeCreation.nDay);
				ReadSubNodeText(oReader, _T("DateTime"), _T("Hour"),    &m_oDateTimeCreation.nHour);
				ReadSubNodeText(oReader, _T("DateTime"), _T("Minute"),  &m_oDateTimeCreation.nMinute);
				ReadSubNodeText(oReader, _T("DateTime"), _T("Second"),  &m_oDateTimeCreation.nSecond);
				ReadSubNodeText(oReader, _T("DateTime"), _T("Summary"), &m_oDateTimeCreation.sSummary);

				ReadNodeText(oReader, _T("Copyright"),      &m_sCopyright);
				ReadNodeText(oReader, _T("Disclaimer"),     &m_sDisclaimer);
				ReadNodeText(oReader, _T("Comment"),        &m_sComment);
				ReadNodeText(oReader, _T("EquipmentType"),  &m_sEquipmentType);
				ReadNodeText(oReader, _T("EquipmentModel"), &m_sEquipmentModel);

				ReadSubNodeText(oReader, _T("JobTime"), _T("Hour"),   &m_oJobTime.nHour);
				ReadSubNodeText(oReader, _T("JobTime"), _T("Minute"), &m_oJobTime.nMinute);
				ReadSubNodeText(oReader, _T("JobTime"), _T("Second"), &m_oJobTime.nSecond);

				ReadNodeText(oReader, _T("JobName"),       &m_sJobName);
				ReadNodeText(oReader, _T("SoftwareID"),    &m_sSoftwareID);
				ReadNodeText(oReader, _T("VersionNumber"), &m_nVersionNumber);
				ReadNodeText(oReader, _T("VersionLetter"), &m_sVersionLetter);
				ReadNodeText(oReader, _T("HostComputer"),  &m_sHostComputer);
				ReadNodeText(oReader, _T("Warning"),       &m_sWarning);
				ReadNodeText(oReader, _T("DocumentName"),  &m_sDocumentName);
				ReadNodeText(oReader, _T("PageName"),      &m_sPageName);
				ReadNodeText(oReader, _T("PageNumber"),    &m_nPageNumber);
				ReadNodeText(oReader, _T("PagesCount"),    &m_nPagesCount);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				ReadNodeText(oReader, _T("ExposureTime"),&m_oExposureTime);
				ReadNodeText(oReader, _T("ExposureCompensation"),&m_oExposureCompensation);
				ReadNodeText(oReader, _T("ExposureCompensation"),&m_oBrightness);
				
				CString strExposureProgram;
				ReadNodeText(oReader, _T("ExposureProgram"),&strExposureProgram);
				if(strExposureProgram == _T("Not Defined"))m_nExposureProgram =0;
				if(strExposureProgram == _T("Manual"))m_nExposureProgram =1;
				if(strExposureProgram == _T("Normal program"))m_nExposureProgram =2;
				if(strExposureProgram == _T("Aperture priority"))m_nExposureProgram =3;
				if(strExposureProgram == _T("Shutter priority"))m_nExposureProgram =4;
				if(strExposureProgram == _T("Creative program (Slow speed)"))m_nExposureProgram =5;
				if(strExposureProgram == _T("Action program (High speed)"))m_nExposureProgram =6;
				if(strExposureProgram == _T("Portrait mode"))m_nExposureProgram =7;
				if(strExposureProgram == _T("Landscape mode"))m_nExposureProgram =8;

				ReadNodeText(oReader, _T("LensAperture"),&m_oLensAperture);
				ReadNodeText(oReader, _T("MaxApertureValue"),&m_oMaxApertureValue);
				ReadNodeText(oReader, _T("FocalLength"),&m_oFocalLength);
				ReadNodeText(oReader, _T("FNumber"),&m_oFNumber);

				ReadNodeText(oReader, _T("ISOSpeed"), &m_nISOSpeed);
				ReadNodeText(oReader, _T("SensingMethod"), &m_nSensingMethod);
				ReadNodeText(oReader, _T("WhiteBalance"), &m_nWhiteBalance);
				ReadNodeText(oReader, _T("ExposureMode"), &m_nExposureMode);
				ReadNodeText(oReader, _T("FocalLengthIn35mmFilm"), &m_nFocalLengthIn35mmFilm);
				
				CString strMeteringMode ;
				ReadNodeText(oReader, _T("MeteringMode"), &strMeteringMode);
				if(strMeteringMode == _T("Unknown"))m_nMeteringMode =0;
				if(strMeteringMode == _T("Average"))m_nMeteringMode =1;
				if(strMeteringMode == _T("Center-weighted average"))m_nMeteringMode =2;
				if(strMeteringMode == _T("Spot"))m_nMeteringMode =3;
				if(strMeteringMode == _T("Multi-spot"))m_nMeteringMode =4;
				if(strMeteringMode == _T("Pattern"))m_nMeteringMode =5;
				if(strMeteringMode == _T("Partial"))m_nMeteringMode =6;
				if(strMeteringMode == _T(""))m_nMeteringMode =255;

				CString strFlashMode;
				ReadNodeText(oReader, _T("FlashMode"), &strFlashMode);
				if (strFlashMode == _T("No Flash")) m_nFlashMode =0x0;
				if (strFlashMode == _T("Fired")) m_nFlashMode = 0x1;
				if (strFlashMode == _T("Fired, Return not detected")) m_nFlashMode = 0x5;
				if (strFlashMode == _T("Fired, Return detected")) m_nFlashMode = 0x7;
				if (strFlashMode == _T("On, Did not fire")) m_nFlashMode = 0x8;
				if (strFlashMode == _T("On, Fired")) m_nFlashMode = 0x9;
				if (strFlashMode == _T("On, Return not detected")) m_nFlashMode = 0xd;
				if (strFlashMode == _T("On, Return detected")) m_nFlashMode = 0xf;
				if (strFlashMode == _T("Off, Did not fire")) m_nFlashMode = 0x10;
				if (strFlashMode == _T("Off, Did not fire, Return not detected")) m_nFlashMode = 0x14;
				if (strFlashMode == _T("Auto, Did not fire")) m_nFlashMode = 0x18;
				if (strFlashMode == _T("Auto, Fired")) m_nFlashMode = 0x19;
				if (strFlashMode == _T("Auto, Fired, Return not detected")) m_nFlashMode = 0x1d;
				if (strFlashMode == _T("Auto, Fired, Return detected")) m_nFlashMode = 0x1f;
				if (strFlashMode == _T("No flash function")) m_nFlashMode = 0x20;
				if (strFlashMode == _T("Off, No flash function")) m_nFlashMode = 0x30;
				if (strFlashMode == _T("Fired, Red-eye reduction")) m_nFlashMode = 0x41;
				if (strFlashMode == _T("Fired, Red-eye reduction, Return not detected")) m_nFlashMode = 0x45;
				if (strFlashMode == _T("Fired, Red-eye reduction, Return detected")) m_nFlashMode = 0x47;
				if (strFlashMode == _T("On, Red-eye reduction")) m_nFlashMode = 0x49;
				if (strFlashMode == _T("On, Red-eye reduction, Return not detected")) m_nFlashMode = 0x4d;
				if (strFlashMode == _T("On, Red-eye reduction, Return detected")) m_nFlashMode = 0x4f;
				if (strFlashMode == _T("Off, Red-eye reduction")) m_nFlashMode = 0x50;
				if (strFlashMode == _T("Auto, Did not fire, Red-eye reduction")) m_nFlashMode = 0x58;
				if (strFlashMode == _T("Auto, Fired, Red-eye reduction")) m_nFlashMode = 0x59;
				if (strFlashMode == _T("Auto, Fired, Red-eye reduction, Return not detected")) m_nFlashMode = 0x5d;
				if (strFlashMode == _T("Auto, Fired, Red-eye reduction, Return detected")) m_nFlashMode =0x5f;

				CString strColorSpace;
				ReadNodeText(oReader, _T("ColorSpace"), &strColorSpace);
				if (strColorSpace == _T("sRGB"))m_nColorSpace =1;
				if (strColorSpace == _T("Uncalibrated"))m_nColorSpace =65535;

				ReadNodeText(oReader, _T("BitDepth"),&m_nBitDepth);
				ReadNodeText(oReader, _T("MultiPageImage"), &m_bMultiPageImage );

				return TRUE;
			}
		private:

			struct TDateTime
			{
				short   nYear;
				short   nMonth;
				short   nDay;
				short   nHour;
				short   nMinute;
				short   nSecond;
				CString sSummary;
			};

			CString   m_sAuthor;
			CString   m_sTitle;
			CString   m_sDescription;
			TDateTime m_oDateTimeCreation;
			CString   m_sCopyright;
			CString   m_sDisclaimer;
			CString   m_sComment;
			CString   m_sEquipmentType;
			CString   m_sEquipmentModel;
			TDateTime m_oJobTime;
			CString   m_sJobName;
			CString   m_sSoftwareID;
			short     m_nVersionNumber;
			CString   m_sVersionLetter;
			CString   m_sHostComputer;
			CString   m_sWarning;
			CString   m_sDocumentName;
			CString   m_sPageName;
			short     m_nPageNumber;
			short     m_nPagesCount;

			short      m_nExposureProgram;
			TRationalU m_oExposureTime;
			TRational  m_oExposureCompensation;
			TRational  m_oShutterSpeed;
			TRationalU m_oLensAperture;
			TRationalU m_oFocalLength;
			TRationalU m_oFNumber;
			TRationalU m_oMaxApertureValue;
			TRational  m_oBrightness;

			short     m_nISOSpeed;
			short     m_nMeteringMode; 
			short     m_nFlashMode;
			short     m_nColorSpace;
			
			short	  m_nSensingMethod;
			short	  m_nWhiteBalance;
			short	  m_nExposureMode ;
			short	  m_nFocalLengthIn35mmFilm;

			short     m_nOrientation;

			short     m_nBitDepth;
			BOOL      m_bMultiPageImage;

			CString   m_sMetaDataXML;

			BOOL      m_bBitmap;

		protected:

#pragma pack(push,1)

			// Tiff  структуры
			struct TTiffFileHeader
			{
				BYTE nByteOrder[2];
				BYTE nSignature[2];
				BYTE nIDFoffset[4];
			};

			struct TDirectoryEntry
			{
				BYTE nTag[2];
				BYTE nType[2];
				BYTE nCount[4];
				BYTE nValueOffset[4];
			};



			// PNG структуры 
			struct TChunk
			{
				BYTE nLenght[4];
				BYTE nType[4];
			};
			struct TChunkTIME
			{
				BYTE  nYear[2];
				BYTE  nMonth;
				BYTE  nDay;
				BYTE  nHour;
				BYTE  nMinute;
				BYTE  nSecond;
			};
			struct TChunkACTL
			{
				BYTE unNumFrames[4];
				BYTE unNumPlays[4];
			};


			// TGA структуры
			struct TFileFooter
			{
				DWORD nExtensionAreaOffset;
				DWORD nDeveloperAreaOffset;
				BYTE arrSignature[18];
			};
			struct TExtensionArea
			{
				WORD  nExtensionSize;
				BYTE  sAuthorName[41];
				BYTE  sComments[324];
				short nDateTime[6];
				BYTE  sJobName[41];
				short nJobTime[3];
				BYTE  sSoftwareID[41];
				short nSoftwareVersionNumber;
				BYTE nSoftwareVersionLetter;
				long  nKeyColor;
				short nPixelAspectRatio[2];
				short nGammaValue[2];
				long  nColorCorrectionOffset;
				long  nPostageStampOffset;
				long  nScanLineOffset;
				BYTE  nAttributesType;  
			};


			// Raw струтуры
			// Raw-foveon
			struct TFoveonDirectorySectionHeader
			{
				BYTE  pSignature[4];
				short nVersion[2];
				DWORD nDirectoryEntriesCount;
			};

			struct TFoveonDirectoryEntries
			{
				DWORD nOffeset;
				DWORD nLenght;
				BYTE  pType[4];
			};

			struct TFoveonPROPHeader
			{
				BYTE  pSignature[4];
				DWORD nVersion;
				DWORD nEntriesCount;
				DWORD nFormat;
				BYTE  pRESERVED[4]; // не используется				
				DWORD nTotalLenght; 
				// total, потомучто тут сумма длины имени 
				// свойста и его значения
			};
			struct TFoveonPROPOffsets
			{
				DWORD nName;
				DWORD nValue;
			};

			// PDS структуры
			struct TPSDFileHeader
			{
				BYTE nSignature[4]; // always equal 8BPS, do not read file if not
				BYTE nVersion[2];   // always equal 1, do not read file if not
				BYTE nReserved[6];  // must be zero
				BYTE nChannels[2];  // numer of channels including any alpha channels, supported range 1 to 24
				BYTE nRows[4];      // height in PIXELS, supported range 1 to 30000
				BYTE nColumns[4];   // width in PIXELS, supported range 1 to 30000
				BYTE nDepth[2];     // number of bpp
				BYTE nMode[2];      // colour mode of the EColorMode
			};
			// PCX структуры
			struct TPCXFileHeader
			{
				BYTE nManufacturer;			// код производителя (редактора)
				BYTE nVersion;				// версия файла
				BYTE bEncoding;				// сжатие данных: 0 - нет, 1 - да
				BYTE nBitsPerPixelPerPlane;	// количество бит на пиксел в слое
				WORD nMinX, nMinY;			// координаты изображения 
				WORD nMaxX, nMaxY;			//
				WORD nScreenHorResolution;	// разрешение монитора по горизонтали
				WORD nScreenVerResolution;	// разрешение монитора по вертикали
				BYTE arrColorMap[48];		// палитра RGB на 16 цветов
				BYTE nReserved;				// номер видеорижима (сейчас не используеться)
				BYTE nPlanes;				// количество слоёв
				WORD nBytesPerLinePerPlane; // количество байт на строку в слое
				WORD bGrayscalePalette;		// итерпритация цветов палитры: 0 - оттенки серого, 1 - RGB
				WORD nScannerHorResolution;	// разрешение сканера по горизонтали
				WORD nScannerVerResolution;	// разрешение сканера по вертикали
				BYTE arrExtraData[54];		// дополнительная информация
			};
			// Ras структуры
			struct TRasFileHeader
			{
				DWORD nMagicNumber;     // Magic (identification) number 
				DWORD nWidth;           // Width of image in pixels 
				DWORD nHeight;          // Height of image in pixels 
				DWORD nDepth;           // Number of bits per pixel 
				DWORD nSize;            // Size of image data in bytes
				DWORD nType;            // Type of raster file 
				DWORD nPaletteType;     // Type of color map 
				DWORD nPaletteLength;   // Size of the color map in bytes 
			};


#pragma pack(pop)

			// класс для проверки чексуммы
			// каждого кусочка файлов формата PNG
			class crc32
			{
			
			protected:
	      
				unsigned m_Table[256];

			public:
	      
				unsigned m_crc32;      
				crc32()
				{
					const unsigned CRC_POLY = 0xEDB88320;
					unsigned i, j, r;
					for (i = 0; i < 256; i++)
					{
						for (r = i, j = 8; j; j--)
							r = r & 1? (r >> 1) ^ CRC_POLY: r >> 1;
						m_Table[i] = r;
					}
					m_crc32 = 0;
				}
				void ProcessCRC(void* pData, int nLen)
				{
					const unsigned CRC_MASK = 0xD202EF8D;
					register unsigned char* pdata = reinterpret_cast<unsigned char*>(pData);
					register unsigned crc = m_crc32;
					while (nLen--)
					{
						crc = m_Table[static_cast<unsigned char>(crc) ^ *pdata++] ^ crc >> 8;
						crc ^= CRC_MASK;
					}
					m_crc32 = crc;
				}
			};



		public:


		};




 
