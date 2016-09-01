#include "./filedownloader.h"
#include <Cocoa/Cocoa.h>

static NSString* StringWToNSString ( const std::wstring& Str )
{
    NSString* pString = [ [ NSString alloc ]
                         initWithBytes : (char*)Str.data()
                         length : Str.size() * sizeof(wchar_t)
                         encoding : CFStringConvertEncodingToNSStringEncoding ( kCFStringEncodingUTF32LE ) ];
    return pString;
}

int CFileDownloader::DownloadFileAll(std::wstring sFileURL)
{
    @try
    {
    NSString* stringURL = StringWToNSString(sFileURL);
    NSURL  *url = [NSURL URLWithString:stringURL];
    NSData *urlData = [NSData dataWithContentsOfURL:url];
    if ( urlData )
    {
        NSString  *filePath = StringWToNSString ( m_sFilePath );
        [urlData writeToFile:filePath atomically:YES];

#ifndef _ASC_USE_ARC_
        //[urlData release]; // autoreleased object!
        //[url release];
        //[stringURL release];
#endif
        return 1;
    }
#ifndef _ASC_USE_ARC_
    //[stringURL release];
    //[url release];
#endif
    return 0;
    }
    @catch(NSException * e)
    {
        return 0;
    }
}
