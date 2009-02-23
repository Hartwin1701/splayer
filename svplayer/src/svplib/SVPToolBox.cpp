#include "SVPToolBox.h"
#include "..\..\include\unrar\unrar.h"
#include "..\zlib\zlib.h"
#include <atlpath.h>
#include <io.h> 
#include <wchar.h> 

BOOL CALLBACK EnumFamCallBack(LPLOGFONT lplf, LPNEWTEXTMETRIC lpntm, DWORD FontType, LPVOID aFontCount) 
{ 
	int * aiFontCount = (int *)aFontCount;
	*aiFontCount = 1;
	
	return TRUE;  
} 

CSVPToolBox::CSVPToolBox(void)
{
}

CSVPToolBox::~CSVPToolBox(void)
{
}
void CSVPToolBox::MergeAltList( CAtlList<CString>& szaRet,  CAtlList<CString>& szaIn  ){
	POSITION pos = szaIn.GetHeadPosition();
	while(pos){
		CString szBuf = szaIn.GetNext(pos);
		if ( szaRet.Find( szBuf) == NULL){
			szaRet.AddTail(szBuf);
		}
	}
}

BOOL  CSVPToolBox::isAlaphbet(WCHAR wchr){
	CString szAlaphbet = _T("1234567890-=qwertyuiop[]asdfghjkl;'zxcvbnm,./`~!@#$%^&*()_+QWERTYUIOP{}ASDFGHJKL:\"\\ZXCVBNM<>?	 ∫♩♫♬€¶♯$¥∮“”‘’；：，。、《》？！·—");
	return !!( szAlaphbet.Find(wchr) >= 0);
}
void CSVPToolBox::findMoreFileByFile( CString szFile, CAtlList<CString>& szaRet,  CAtlArray<CString>& szaExt  ){
	findMoreFileByDir( this->GetDirFromPath(szFile), szaRet, szaExt);
}
void CSVPToolBox::findMoreFileByDir(  CString szDir, CAtlList<CString>& szaRet,  CAtlArray<CString>& szaExt ){
	
	for(UINT i = 0; i < szaExt.GetCount(); i++){
		CFileFind finder;
		CString szFindPattern = szDir +  szaExt.GetAt(i).TrimRight(_T(";"));

		BOOL bWorking = finder.FindFile(szFindPattern);
		while (bWorking)
		{
			bWorking = finder.FindNextFile();

			if (finder.IsDots())
				continue;

			if (finder.IsDirectory())
				continue;
			CString szTmp = finder.GetFilePath();
			szTmp.MakeLower();
			if( szTmp.Find(_T("sample")) >= 0){
				continue;
			}
			szaRet.AddTail(finder.GetFilePath());
		}

		finder.Close();
	}
}
BOOL CSVPToolBox::bFontExist(CString szFontName){
	int aFontCount = 0;
	BOOL ret;
	HDC hdc = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);

	ret =  EnumFontFamilies(hdc,szFontName, (FONTENUMPROC) EnumFamCallBack, (LPARAM) &aFontCount) ;

	CancelDC(hdc);
	DeleteDC(hdc);
	return !!aFontCount;
}
CString CSVPToolBox::fileGetContent(CString szFilePath){
	CStdioFile f;

	CString szBuf;
	if(f.Open(szFilePath, CFile::modeRead | CFile::typeText))
	{
		f.ReadString(szBuf);
		f.Close();
	}
	return szBuf;
}
void CSVPToolBox::filePutContent(CString szFilePath, CString szData, BOOL bAppend){
	CStdioFile f;
	
	if(f.Open(szFilePath, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
	{
		f.WriteString(szData);

		
		f.Close();
	}
}
CString CSVPToolBox::getFileVersionHash(CString szPath){
	DWORD             dwHandle;
	UINT              dwLen;
	UINT              uLen;
	LPVOID            lpBuffer;
	VS_FIXEDFILEINFO  *lpBuffer2;
	
	dwBuild = 0;

	dwLen  = GetFileVersionInfoSize(szPath, &dwHandle);

	TCHAR * lpData = (TCHAR*) malloc(dwLen);
	if(!lpData)
		return _T("");
	memset((char*)lpData, 0 , dwLen);

	/* GetFileVersionInfo() requires a char *, but the api doesn't
	* indicate that it will modify it */
	if(GetFileVersionInfo(szPath, dwHandle, dwLen, lpData) != 0)
	{
		if(VerQueryValue(lpData, _T("\\"), &lpBuffer, &uLen) != 0)
		{
			lpBuffer2 = (VS_FIXEDFILEINFO *)lpBuffer;
			dwMajor   = HIWORD(lpBuffer2->dwFileVersionMS);
			dwMinor   = LOWORD(lpBuffer2->dwFileVersionMS);
			dwRelease = HIWORD(lpBuffer2->dwFileVersionLS);
			dwBuild   = LOWORD(lpBuffer2->dwFileVersionLS);
		}
	}
	long iFileLen;
	int fp;
	if( _wsopen_s ( &fp, szPath, _O_RDONLY, _SH_DENYNO,
		_S_IREAD ) == 0 )
	{

		iFileLen = filelength(fp);
		_close( fp);

	}
	
	CString szRet;
	szRet.Format(_T("%d.%d.%d.%d.%d"), dwMajor, dwMinor, dwRelease, dwBuild,iFileLen);
	return szRet;
}
BOOL CSVPToolBox::isWriteAble(CString szPath){
	FILE* fp;
	if ( _wfopen_s( &fp, szPath, _T("ab") ) != 0){
		return 0; //input file open error
	}else{
		fclose(fp);
	}
	return 1;
}
CString CSVPToolBox::getSameTmpName(CString fnin )
{
	CStringArray szaPathinfo;
	this->getVideoFileBasename(fnin, &szaPathinfo);
	CString fntdir = this->GetTempDir();
	CString fnout = fntdir + szaPathinfo.GetAt(3) + szaPathinfo.GetAt(1) ;
	int i = 0;
	while(this->ifFileExist(fnout)){
		i++;
		CString szBuf;
		szBuf.Format(_T(".svr%d"), i);
		fnout = fntdir + szaPathinfo.GetAt(3) + szBuf + szaPathinfo.GetAt(1) ;
	}
	return fnout;
}
CString CSVPToolBox::getSameTmpExt(CString fnin )
{
	CStringArray szaPathinfo;
	this->getVideoFileBasename(fnin, &szaPathinfo);
	CString fnout = this->getTmpFileName();
	fnout += szaPathinfo.GetAt(1) ;
	return fnout;
}
int CSVPToolBox::packGZfile(CString fnin , CString fnout)
{
	
	FILE* fp;
	int ret = 0;
	if ( _wfopen_s( &fp, fnin, _T("rb") ) != 0){
		return -1; //input file open error
	}
	int iDescLen;
	char * szFnout = this->CStringToUTF8(fnout, &iDescLen, CP_ACP);

	gzFile gzfOut = gzopen( szFnout , "wb9");	
	if (gzfOut){

		char buff[4096];
		int iBuffReadLen ;
		do{
			iBuffReadLen = fread( buff, sizeof( char), 4096 ,fp);
			if(iBuffReadLen > 0 ){
				if( gzwrite( gzfOut , buff,  iBuffReadLen ) <= 0 ){
					//gz file compress write error
					ret = 1;
				}
			}else if(iBuffReadLen < 0){
				ret = -3; //file read error
				break;
			}else{
				break;
			}
		}while(1);

		fclose(fp);
		gzclose(gzfOut);
	}else{
		ret = -2; //gz file open error
	}
	free(szFnout);

	if (ret != 0 ){
		CString szLog ; 
		szLog.Format(_T("Gz pack file fail: %s to %s ret %d"), fnin , fnout, ret);
		SVP_LogMsg(szLog);
	}
	return ret;
}
int CSVPToolBox::unpackGZfile(CString fnin , CString fnout)
{
	
	FILE* fout;
	int ret = 0;
	if ( _wfopen_s( &fout, fnout, _T("wb") ) != 0){
		return -1; //output file open error
	}
	int iDescLen;
	char * szFnin = this->CStringToUTF8(fnin, &iDescLen, CP_ACP);
	
	gzFile gzfIn = gzopen( szFnin , "rb");	
	if (gzfIn){
	
		char buff[4096];
		int iBuffReadLen ;
		do{
			iBuffReadLen = gzread(gzfIn, buff, 4096 );
			if(iBuffReadLen > 0 ){
				if( fwrite(buff, sizeof( char ),  iBuffReadLen, fout) <= 0 ){
				    //file write error
					ret = 1;
				}
			}else if(iBuffReadLen < 0){
				ret = -3; //decompress error
				break;
			}else{
				break;
			}
		}while(1);

		fclose(fout);
		gzclose(gzfIn);
	}else{
		ret = -2; //gz file open error
	}
	free(szFnin);
	if (ret != 0 ){
		CString szLog ; 
		szLog.Format(_T("Gz unpack file fail: %s to %s ret %d"), fnin , fnout, ret);
		SVP_LogMsg(szLog);
	}
	return ret;
}
static unsigned char* RARbuff = NULL;
static unsigned int RARpos = 0;

static int PASCAL MyProcessDataProc(unsigned char* Addr, int Size)
{
	ASSERT(RARbuff);

	memcpy(&RARbuff[RARpos], Addr, Size);
	RARpos += Size;

	return(1);
}
CString CSVPToolBox::extractRarFile(CString rarfn){
	CString szRet = _T("");
	HMODULE h = LoadLibrary(_T("unrar.dll"));
	if(!h) return szRet;

	RAROpenArchiveEx OpenArchiveEx = (RAROpenArchiveEx)GetProcAddress(h, "RAROpenArchiveEx");
	RARCloseArchive CloseArchive = (RARCloseArchive)GetProcAddress(h, "RARCloseArchive");
	RARReadHeaderEx ReadHeaderEx = (RARReadHeaderEx)GetProcAddress(h, "RARReadHeaderEx");
	RARProcessFile ProcessFile = (RARProcessFile)GetProcAddress(h, "RARProcessFile");
	RARSetChangeVolProc SetChangeVolProc = (RARSetChangeVolProc)GetProcAddress(h, "RARSetChangeVolProc");
	RARSetProcessDataProc SetProcessDataProc = (RARSetProcessDataProc)GetProcAddress(h, "RARSetProcessDataProc");
	RARSetPassword SetPassword = (RARSetPassword)GetProcAddress(h, "RARSetPassword");

	if(!(OpenArchiveEx && CloseArchive && ReadHeaderEx && ProcessFile 
		&& SetChangeVolProc && SetProcessDataProc && SetPassword))
	{
		FreeLibrary(h);
		return szRet;
	}

	struct RAROpenArchiveDataEx ArchiveDataEx;
	memset(&ArchiveDataEx, 0, sizeof(ArchiveDataEx));

	ArchiveDataEx.ArcNameW = (LPTSTR)(LPCTSTR)rarfn;
	char fnA[MAX_PATH];
	if(wcstombs(fnA, rarfn, rarfn.GetLength()+1) == -1) fnA[0] = 0;
	ArchiveDataEx.ArcName = fnA;

	ArchiveDataEx.OpenMode = RAR_OM_EXTRACT;
	ArchiveDataEx.CmtBuf = 0;
	HANDLE hrar = OpenArchiveEx(&ArchiveDataEx);
	if(!hrar) 
	{
		FreeLibrary(h);
		return szRet;
	}

	SetProcessDataProc(hrar, MyProcessDataProc);

	struct RARHeaderDataEx HeaderDataEx;
	HeaderDataEx.CmtBuf = NULL;
	szRet = this->getTmpFileName() ;
	szRet += _T(".sub");
	CFile m_sub ( szRet, CFile::modeCreate|CFile::modeReadWrite|CFile::typeBinary );


	while(ReadHeaderEx(hrar, &HeaderDataEx) == 0)
	{

		CString subfn(HeaderDataEx.FileNameW);


		if(!subfn.Right(4).CompareNoCase(_T(".sub")))
		{
			CAutoVectorPtr<BYTE> buff;
			if(!buff.Allocate(HeaderDataEx.UnpSize))
			{
				CloseArchive(hrar);
				FreeLibrary(h);
				return szRet;
			}

			RARbuff = buff;
			RARpos = 0;

			if(ProcessFile(hrar, RAR_TEST, NULL, NULL))
			{
				CloseArchive(hrar);
				FreeLibrary(h);

				return szRet;
			}

			m_sub.SetLength(HeaderDataEx.UnpSize);
			m_sub.SeekToBegin();
			m_sub.Write(buff, HeaderDataEx.UnpSize);
			m_sub.Flush();
			m_sub.SeekToBegin();
			m_sub.Close();

			//free(buff);
			RARbuff = NULL;
			RARpos = 0;

			break;
		}

		ProcessFile(hrar, RAR_SKIP, NULL, NULL);
	}

	CloseArchive(hrar);
	FreeLibrary(h);
	return szRet;
}
int CSVPToolBox::DetectFileCharset(CString fn){
// 	;
// 	;
	//
	FILE *stream ;
	if ( _wfopen_s( &stream, fn, _T("rb") ) == 0 ){
		//detect bom?

		int totalWideChar = 0;
		int totalGBKChar = 0;
		int totalBig5Char = 0;
		
		int ch, ch2;
		int xch;

		for( int i=0; (i < 4096 ) && ( feof( stream ) == 0 ); i++ )
		{
			ch = 0xff & fgetc( stream );
			if (ch >= 0x80 ){
				totalWideChar++;
				ch2 = 0xff & fgetc( stream );

				if ( ch >= 0xA1 && ch < 0xA9 && ch2 >= 0xA1 && ch2 <= 0xFE ){
					totalGBKChar++;
				}else if ( ch >= 0xB0 && ch < 0xF7 && ch2 >= 0xA1 && ch2 <= 0xFE){
					totalGBKChar++;
				}
				
				xch = ch << 8 | ch2;
				if ( xch >= 0xa440 && xch <= 0xc67e){
					totalBig5Char++;
				}

			}
			
		}

		fclose( stream );
		if ( totalGBKChar > totalBig5Char && totalGBKChar > totalWideChar/2 && totalWideChar > 500){
			return GB2312_CHARSET;
		}else if ( totalGBKChar < totalBig5Char && totalBig5Char > totalWideChar/2 && totalWideChar > 500 ){
			return CHINESEBIG5_CHARSET;
		}

	}
	
	


	return DEFAULT_CHARSET;
}
int CSVPToolBox::ClearTmpFiles(){
	int i;
	for( i = 0; i < this->szaTmpFileNames.GetCount(); i++){
		_wremove(szaTmpFileNames.GetAt(i));
	}
	return i;
}
WCHAR* CSVPToolBox::getTmpFileName(){
	WCHAR* tmpnamex ;
	int i;

	for (i = 0; i < 5; i++) //try 5 times for tmpfile creation
	{
		tmpnamex = _wtempnam( this->GetTempDir(), _T("svp") );
		if (tmpnamex)
			break;

	}
	if (!tmpnamex){
		SVP_LogMsg(_T("TMP FILE name genarater error")); 
		return 0;
	}else{
		//SVP_LogMsg(tmpnamex);
		return tmpnamex;
	}
}
FILE* CSVPToolBox::getTmpFileSteam(){
	FILE *stream;
	WCHAR* tmpnamex = this->getTmpFileName();
	errno_t err;

	if (!tmpnamex){
		SVP_LogMsg(_T("TMP FILE stream name genarater error")); 
		return 0;
	}else{
		
		err = _wfopen_s(&stream, tmpnamex, _T("wb+"));
		if(err){
			SVP_LogMsg(_T("TMP FILE stream Open for Write error")); 
			stream = 0;
		}else{
			this->szaTmpFileNames.Add(tmpnamex);
			free(tmpnamex);
		}
	}
	return stream;
}
CString CSVPToolBox::GetPlayerPath(CString progName){
	CString path;
	GetModuleFileName(AfxGetInstanceHandle(), path.GetBuffer(MAX_PATH), MAX_PATH);
	path.ReleaseBuffer();
	if (progName.IsEmpty()){
		return path;
	}else{
		CPath cpath(path);
		cpath.RemoveFileSpec();
		cpath.AddBackslash();
		cpath.Append(progName);
		return cpath;
	}
}
CString CSVPToolBox::GetDirFromPath(CString path){
	CPath cpath(path);
	cpath.RemoveFileSpec();
	cpath.AddBackslash();
	return cpath;
}

int CSVPToolBox::HandleSubPackage(FILE* fp){
	//Extract Package
	SVP_LogMsg( _T("Extracting Package") );


	char szSBuff[8];
	int err;
	if ( fread(szSBuff , sizeof(char), 4, fp) < 4){
		
		SVP_LogMsg( _T("Fail to retrive Package Data Length ") );
		return -1;
	}
	int iPackageLength = this->Char4ToInt(szSBuff);
	
	if ( fread(szSBuff , sizeof(char), 4, fp) < 4){
		SVP_LogMsg(_T("Fail to retrive Desc Data Length"));
		return -2;
	}

	
	size_t iDescLength = this->Char4ToInt(szSBuff);

	if(iDescLength > 0){
		SVP_LogMsg(_T("retriving Desc Data"));
		char * szDescData = this->ReadToPTCharByLength(fp, iDescLength);
		if(!szDescData){
			SVP_LogMsg(_T("Fail to retrive Desc Data"));
			return -4;
		}
		// convert szDescData to Unicode and save to CString
		szaSubDescs.Add(this->UTF8ToCString( szDescData , iDescLength));
		free(szDescData);
	}else{
		szaSubDescs.Add(_T(""));
	}
	err = this->ExtractSubFiles(fp);
	
// 	fpos_t pos;
// 	fgetpos( fp, &pos ) ;
// 	CString szErr;
// 	szErr.Format(_T("Reading At %d got next %ld") , pos,iDescLength);
// 	SVP_LogMsg(szErr);

	

	return 0;

}
int CSVPToolBox::ExtractSubFiles(FILE* fp){
	char szSBuff[8];
	if ( fread(szSBuff , sizeof(char), 4, fp) < 4){
		SVP_LogMsg(_T("Fail to retrive File Data Length"));
		return -1;
	}

	size_t iFileDataLength = this->Char4ToInt(szSBuff); //确认是否确实有文件下载
	if(!iFileDataLength){
		SVP_LogMsg(_T("No real file released"));
		return 0;
	}

	if ( fread(szSBuff , sizeof(char), 1, fp) < 1){
		SVP_LogMsg(_T("Fail to retrive how many Files are there"));
		return -2;
	}
	int iFileCount = szSBuff[0];
	if( iFileCount <= 0 ){
		SVP_LogMsg(_T("No file in File Data"));
		return -3;
	}

	int iCurrentId = this->szaSubTmpFileList.GetCount();
	this->szaSubTmpFileList.Add(_T(""));
	for(int k = 0; k < iFileCount; k++){
		if(this->ExtractEachSubFile(fp, iCurrentId) ){
			SVP_LogMsg(_T("File Extract Error ") );
			return -4;
		}
	}

	return 0;
}
char* CSVPToolBox::ReadToPTCharByLength(FILE* fp, size_t length){
	char * szBuffData =(char *)calloc( length + 1, sizeof(char));

	if(!szBuffData){
		SVP_LogMsg(_T("Fail to calloc Data "));
		return 0;
	}

	if ( fread(szBuffData , sizeof(char), length, fp) < length){
		SVP_LogMsg(_T("Fail to retrive  Data "));
		return 0;
	}

	return szBuffData;
}
int CSVPToolBox::ExtractEachSubFile(FILE* fp, int iSubPosId){
	// get file ext name
	char szSBuff[4096];
	if ( fread(szSBuff , sizeof(char), 4, fp) < 4){
		SVP_LogMsg(_T("Fail to retrive Single File Pack Length"));
		return -1;
	}

	if ( fread(szSBuff , sizeof(char), 4, fp) < 4){
		SVP_LogMsg(_T("Fail to retrive File Ext Name Length"));
		return -1;
	}

	size_t iExtLength = this->Char4ToInt(szSBuff);
	char* szExtName = this->ReadToPTCharByLength(fp, iExtLength);
	if(!szExtName){
		SVP_LogMsg(_T("Fail to retrive File Ext Name"));
		return -2;
	}
	
	
	//get filedata length
	if ( fread(szSBuff , sizeof(char), 4, fp) < 4){
		SVP_LogMsg(_T("Fail to retrive File Data Length"));
		return -1;
	}

	size_t iFileLength = this->Char4ToInt(szSBuff);
	
	// gen tmp name and tmp file point
	WCHAR* otmpfilename = this->getTmpFileName();
	if(!otmpfilename){
		SVP_LogMsg(_T("TMP FILE name for sub file error")); 
		return -5;
	}
	FILE* fpt;
	errno_t err = _wfopen_s(&fpt, otmpfilename, _T("wb"));
	if(err){
		SVP_LogMsg(_T("TMP FILE stream for sub file  Write error")); 
		free(otmpfilename);
		return -4;
	}

	// copy date to tmp file
	size_t leftoread = iFileLength;
	do{
		size_t needtoread = SVP_MIN( 4096, leftoread );
		size_t accturead = fread(szSBuff , sizeof(char), needtoread, fp);
		if(accturead == 0){
			//wtf
			break;
		}
		leftoread -= accturead;
		err = fwrite(szSBuff,  sizeof(char), accturead , fpt);

		
	}while(leftoread > 0);
	fclose( fpt );

	WCHAR* otmpfilenameraw = this->getTmpFileName();
	CString szLogmsg;
	int gzret = this->unpackGZfile( otmpfilename , otmpfilenameraw );
	szLogmsg.Format(_T(" Gzip decompress %s -> %s : %d "),  otmpfilename , otmpfilenameraw , gzret );

	SVP_LogMsg(szLogmsg);
	// add filename and tmp name to szaTmpFileNames
	this->szaSubTmpFileList[iSubPosId].Append( this->UTF8ToCString(szExtName, iExtLength)); //why cant use + ???
	this->szaSubTmpFileList[iSubPosId].Append(_T("|") );
	this->szaSubTmpFileList[iSubPosId].Append(otmpfilenameraw);
	this->szaSubTmpFileList[iSubPosId].Append( _T(";"));
	//SVP_LogMsg(this->szaSubTmpFileList[iSubPosId] + _T(" is the szaTmpFileName"));
	//this->szaSubTmpFileList[iSubPosId].SetString( otmpfilename);
	
	free(szExtName);
	free(otmpfilename);

	return 0;
}
#define SVPATH_BASENAME 0  //Without Dot
#define SVPATH_EXTNAME 1  //With Dot
#define SVPATH_DIRNAME 2 //With Slash
#define SVPATH_FILENAME 3  //Without Dot
CString CSVPToolBox::getVideoFileBasename(CString szVidPath, CStringArray* szaPathInfo = NULL){

	int posDot = szVidPath.ReverseFind(_T('.'));
	
	int posSlash = szVidPath.ReverseFind(_T('\\'));
	int posSlash2 = szVidPath.ReverseFind(_T('/'));
	if(posSlash2 > posSlash){posSlash = posSlash2;}

	if(posDot > posSlash ){
		if (szaPathInfo != NULL){
			CString szBaseName = szVidPath.Left(posDot);
			CString szExtName = szVidPath.Right(szVidPath.GetLength() - posDot).MakeLower();
			CString szFileName = szVidPath.Mid(posSlash+1, (posDot - posSlash - 1));
			CString szDirName = szVidPath.Left(posSlash + 1) ;
			szaPathInfo->RemoveAll();
			szaPathInfo->Add(szBaseName); // Base Name
			szaPathInfo->Add(szExtName ); //ExtName
			szaPathInfo->Add(szDirName); //Dir Name ()
			szaPathInfo->Add(szFileName); // file name only
			SVP_LogMsg(szBaseName);
			SVP_LogMsg(szaPathInfo->GetAt(0) + _T(" | ") + szaPathInfo->GetAt(1) + _T(" | ") + szaPathInfo->GetAt(2) + _T(" | ") + szaPathInfo->GetAt(3) );
		}
		return szVidPath.Left(posDot);
	}

	return szVidPath;
}
CString CSVPToolBox::Implode(CString szTok, CStringArray* szaOut){
	CString szRet;
	for(int i = 0; i < szaOut->GetCount(); i++){
		if(i > 0) { szRet.Append(szTok); }
		szRet.Append(szaOut->GetAt(i));
	}
	return szRet;
}
int CSVPToolBox::Explode(CString szIn, CString szTok, CStringArray* szaOut){
	szaOut->RemoveAll();

	CString resToken;
	int curPos= 0;

	resToken= szIn.Tokenize(szTok, curPos);
	while (resToken != _T(""))
	{
		szaOut->Add(resToken);
		resToken= szIn.Tokenize(szTok,curPos);
	};

	return 0;
}
BOOL CSVPToolBox::ifFileExist(CString szPathname){
	FILE* fp = NULL;

	
	fp = _wfopen( szPathname, _T("rb") );
	if( fp != NULL )
	{
		fclose( fp );
		return true;
	}
	return false;
}
BOOL CSVPToolBox::ifDirWritable(CString szDir){
	FILE* fp = NULL;
	fp = _wfopen( szDir + _T("svpwrtst"), _T("wb") );
	if( fp != NULL )
	{
		fclose( fp );
		_wremove(szDir + _T("svpwrtst"));
		return true;
	}
	return false;
}
CString CSVPToolBox::getSubFileByTempid(int iTmpID, CString szVidPath){
	//get base path name
	CStringArray szVidPathInfo ;
	CString szTargetBaseName = _T("");
	CString szDefaultSubPath = _T("");

	CString szBasename = this->getVideoFileBasename(szVidPath, &szVidPathInfo);
	if(!this->ifDirWritable(szVidPathInfo.GetAt(SVPATH_DIRNAME)) ){
		szBasename = this->GetTempDir() + szVidPathInfo.GetAt(SVPATH_FILENAME);
	}
	//set new file name
	CStringArray szSubfiles;
	CString szXTmpdata = this->szaSubTmpFileList.GetAt(iTmpID);
	this->Explode(szXTmpdata, _T(";"), &szSubfiles);
	bool bIsIdxSub = FALSE;
	if ( szXTmpdata.Find(_T("idx|")) >= 0 && szXTmpdata.Find(_T("sub|")) >= 0){
		if ( !this->ifFileExist( szBasename + _T(".idx") ) && !this->ifFileExist( szBasename + _T(".sub") ) ){
			bIsIdxSub = TRUE;
		}
	}

	if( szSubfiles.GetCount() < 1){
		SVP_LogMsg( _T("Not enough files in tmp array"));
		
	}
	for(int i = 0; i < szSubfiles.GetCount(); i++){
		CStringArray szSubTmpDetail;
		this->Explode(szSubfiles[i], _T("|"), &szSubTmpDetail);
		if (szSubTmpDetail.GetCount() < 2){
			SVP_LogMsg( _T("Not enough detail in sub tmp string") + szSubfiles[i]);
			continue;
		}
		CString szSource = szSubTmpDetail[1];
		CString szLangExt  = _T(".chn"); //TODO: use correct language perm 
		if(bIsIdxSub){
			szLangExt  = _T("");
		}
		if (szSubTmpDetail[0].GetAt(0) != _T('.')){
			szSubTmpDetail[0] = CString(_T(".")) + szSubTmpDetail[0];
		}
		CString szTarget = szBasename + szLangExt + szSubTmpDetail[0];
			szTargetBaseName = szBasename + szLangExt ;
			//check if target exist
			CString szTmp = _T("") ;
			int ilan = 1;
			while( this->ifFileExist(szTarget) ){ 
				//TODO: compare if its the same file
				
				szTmp.Format(_T("%d"), ilan);
				szTarget = szBasename + szLangExt + szTmp + szSubTmpDetail[0]; 
				szTargetBaseName = szBasename + szLangExt + szTmp;
				ilan++;
			}
			
		
		if( !CopyFile( szSource, szTarget, true) ){
			SVP_LogMsg( szSource + _T(" to ") + szTarget + _T(" Fail to copy subtitle file to position"));
		}else if ( ( (bIsIdxSub && szSubTmpDetail[0].CompareNoCase(_T(".idx")) == 0) || !bIsIdxSub )
			&& szDefaultSubPath.IsEmpty()){
				szDefaultSubPath = szTarget;
		}
		CStringArray szaDesclines;
		if(szaSubDescs.GetCount() > iTmpID){
			this->Explode( szaSubDescs.GetAt(iTmpID) , _T("\x0b\x0b") , &szaDesclines);
			if(szaDesclines.GetCount() > 0){
				int iDelay = 0;
				 swscanf_s(szaDesclines.GetAt(0), _T("delay=%d"), &iDelay);
				 if(iDelay){
					 CString szBuf;
					 szBuf.Format(_T("%d"), iDelay);
					 filePutContent( szTarget + _T(".delay"), szBuf );
					 SVP_LogMsg(szTarget + _T(" has delay ") + szBuf);
				 }else{
					 _wremove(szTarget + _T(".delay"));
				 }
				
			}
		}else{
			SVP_LogMsg(_T("Count of szaSubDescs not match with count of subs "));
		}
		
		_wremove(szSource);
	}
	
	return szDefaultSubPath;
	
}
CString CSVPToolBox::PackageSubFiles(CStringArray* szaSubFiles){
	return _T("");
}
CString CSVPToolBox::GetTempDir(){
	TCHAR lpPathBuffer[MAX_PATH];
	GetTempPath(MAX_PATH,  lpPathBuffer); 
	CString szTmpPath (lpPathBuffer);
	if (szTmpPath.Right(1) != _T("\\") && szTmpPath.Right(1) != _T("/")){
		szTmpPath.Append(_T("\\"));
	}
		
	return szTmpPath;
}
int CSVPToolBox::FindAllSubfile(CString szSubPath , CStringArray* szaSubFiles){
	szaSubFiles->RemoveAll();
	szaSubFiles->Add(szSubPath);
	CStringArray szaPathInfo ;
	CString szBaseName = this->getVideoFileBasename( szSubPath, &szaPathInfo );
	CString szExt = szaPathInfo.GetAt(1);
	
	if(szExt == _T(".idx")){
		szSubPath = szBaseName + _T(".sub");
		if(this->ifFileExist(szSubPath)){
			szaSubFiles->Add(szSubPath);
		}else{
			szSubPath = szBaseName + _T(".rar");
			if(this->ifFileExist(szSubPath)){
				CString szSubFilepath = this->extractRarFile(szSubPath);
				if(!szSubFilepath.IsEmpty()){
					SVP_LogMsg(szSubFilepath + _T(" unrar ed"));
					szaSubFiles->Add(szSubFilepath);
				}else{
					SVP_LogMsg(_T("Unrar error"));
				}
			}
		}
	}
	
	//TODO: finding other subfile
	return 0;
}
int CSVPToolBox::Char4ToInt(char* szBuf){

	int iData =   ( ((int)szBuf[0] & 0xff) << 24) |  ( ((int)szBuf[1] & 0xff) << 16) | ( ((int)szBuf[2] & 0xff) << 8) |  szBuf[3] & 0xff;;
	
	return iData;
}
char* CSVPToolBox::CStringToUTF8(CString szIn, int* iDescLen, UINT codePage)
{
	char* szOut = 0;
	int   targetLen = ::WideCharToMultiByte(codePage,0,szIn,-1,szOut,0,NULL,NULL);
	szOut   =   new   char[targetLen+1];          
	memset(szOut,0,targetLen+1);                
	targetLen = ::WideCharToMultiByte(codePage,0,szIn,-1,szOut,targetLen,NULL,NULL);                
	*iDescLen = targetLen;
	return szOut;
}

CString CSVPToolBox::UTF8ToCString(char* szIn, int iLength)
{
	int   targetLen = ::MultiByteToWideChar(CP_UTF8,0,szIn,iLength+1,0,0);
	WCHAR* szOut   =  (WCHAR *)calloc( targetLen + 1, sizeof(WCHAR)); 

	::MultiByteToWideChar(CP_UTF8,0,szIn,iLength+1,szOut,targetLen);
	CString szBuf(szOut,targetLen);
	free(szOut);
	return szBuf;
}
