#ifndef XLSX_SERIALIZER
#define XLSX_SERIALIZER

#if defined(_WIN32) || defined (_WIN64)
	#include <atlbase.h>
	#include <atlstr.h>
#else
	#include "../../Common/DocxFormat/Source/Base/ASCString.h"
#endif

#include <string>

namespace OOX
{
namespace Spreadsheet
{
class CChartSpace;
}
}
namespace NSBinPptxRW{
	class CDrawingConverter;
}
namespace NSBinPptxRW{
	class CBinaryFileReader;
	class CBinaryFileWriter;
}
namespace BinXlsxRW {

	class CXlsxSerializer{
	private:
		CString m_sFontDir;
		CString m_sEmbeddedFontsDir;
		NSBinPptxRW::CDrawingConverter* m_pExternalDrawingConverter;
	public:
		CXlsxSerializer();
		~CXlsxSerializer();
		
		void CreateXlsxFolders(CString& sXmlOptions, CString sDstPath,  CString& sMediaPath, CString& sEmbedPath);

        bool loadFromFile(const CString& sSrcFileName, const CString& sDstPath, const CString& sXMLOptions, const CString& sMediaDir, const CString& sEmbedPath);
        bool saveToFile(const CString& sSrcFileName, const CString& sDstPath, const CString& sXMLOptions);
		
		bool loadChart(CString& sChartPath, NSBinPptxRW::CBinaryFileWriter& oBufferedStream, long& lDataSize);
        bool saveChart(NSBinPptxRW::CBinaryFileReader& oBufferedStream, long lLength, CString& sFilename, CString& sContentTypePath, CString** sContentTypeElement, const long& lChartNumber);
		
		void setFontDir(CString& sFontDir);
		void setEmbeddedFontsDir(CString& sEmbeddedFontsDir);
		void setDrawingConverter(NSBinPptxRW::CDrawingConverter* pDrawingConverter);

		void writeChartXlsx(const std::wstring& sDstFile ,const OOX::Spreadsheet::CChartSpace& oChart);
	};
}
#endif	// #ifndef XLSX_SERIALIZER
