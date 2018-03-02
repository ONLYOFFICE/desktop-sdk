/*
 * (c) Copyright Ascensio System SIA 2010-2017
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at Lubanas st. 125a-25, Riga, Latvia,
 * EU, LV-1021.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 */

#ifndef APPLICATION_MANAGER_EVENTS_H
#define APPLICATION_MANAGER_EVENTS_H

#include "../../../../core/DesktopEditor/Word_Api/Editor_Api.h"

#define ASC_MENU_EVENT_TYPE_CEF_CREATETAB       1001
#define ASC_MENU_EVENT_TYPE_CEF_TABEDITORTYPE   1002
#define ASC_MENU_EVENT_TYPE_CEF_SPELLCHECK      1003
#define ASC_MENU_EVENT_TYPE_CEF_KEYBOARDLAYOUT  1004
#define ASC_MENU_EVENT_TYPE_CEF_CONTROL_ID      1005
#define ASC_MENU_EVENT_TYPE_CEF_SYNC_COMMAND    1006
#define ASC_MENU_EVENT_TYPE_CEF_CLOSE           1007
#define ASC_MENU_EVENT_TYPE_CEF_DESTROY         1008
#define ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD        1009
#define ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD_START  1010
#define ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD_END    1011
#define ASC_MENU_EVENT_TYPE_CEF_MODIFY_CHANGED  1012

#define ASC_MENU_EVENT_TYPE_CEF_DOCUMENT_NAME   1013
#define ASC_MENU_EVENT_TYPE_CEF_ONLOGOUT        1014

#define ASC_MENU_EVENT_TYPE_CEF_ONOPENLINK      1015
#define ASC_MENU_EVENT_TYPE_CEF_DESTROY_SAFE    1016

#define ASC_MENU_EVENT_TYPE_CEF_ONCLOSE         2001
#define ASC_MENU_EVENT_TYPE_CEF_ONBEFORECLOSE   2002

#define ASC_MENU_EVENT_TYPE_CEF_SAVE            3001
#define ASC_MENU_EVENT_TYPE_CEF_ONSAVE          3002

#define ASC_MENU_EVENT_TYPE_CEF_ONKEYBOARDDOWN      3100
#define ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENENTER   3101
#define ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENLEAVE   3102

#define ASC_MENU_EVENT_TYPE_CEF_JS_MESSAGE      4001
#define ASC_MENU_EVENT_TYPE_CEF_PORTAL_OPEN     4002

#define ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_START        5001
#define ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_PROGRESS     5002
#define ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_END          5003

#define ASC_MENU_EVENT_TYPE_CEF_PRINT_START                 5011
#define ASC_MENU_EVENT_TYPE_CEF_PRINT_PAGE                  5012
#define ASC_MENU_EVENT_TYPE_CEF_PRINT_END                   5013
#define ASC_MENU_EVENT_TYPE_CEF_PRINT_PAGE_CHECK            5014

#define ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND             5015
#define ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS          5016
#define ASC_MENU_EVENT_TYPE_CEF_EDITOR_EXECUTE_COMMAND      5017

#define ASC_MENU_EVENT_TYPE_CEF_SCRIPT_EDITOR_VERSION       6001

#define ASC_MENU_EVENT_TYPE_CEF_SAVEFILEDIALOG              7001
#define ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_OPEN              7002
#define ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECENTS           7003
#define ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECENTOPEN        7004
#define ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECOVERS          7005
#define ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECOVEROPEN       7006
#define ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_CREATE            7007
#define ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_SAVE              7008
#define ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_SAVE_PATH         7009
#define ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_ADDIMAGE          7010
#define ASC_MENU_EVENT_TYPE_CEF_LOCALFILES_OPEN             7011

#define ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_ADD_PLUGIN      7013
#define ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_VIEW_DELAY      7014

#define ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_OPENFILENAME_DIALOG 7015

#define ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_SAVE_YES_NO     7016

#define ASC_MENU_EVENT_TYPE_REPORTER_MESSAGE_TO             7017
#define ASC_MENU_EVENT_TYPE_REPORTER_MESSAGE_FROM           7018
#define ASC_MENU_EVENT_TYPE_REPORTER_CREATE                 7019
#define ASC_MENU_EVENT_TYPE_REPORTER_END                    7020

#define ASC_MENU_EVENT_TYPE_SSO_TOKEN                       7030

#define ASC_MENU_EVENT_TYPE_UI_THREAD_MESSAGE               7031
#define ASC_MENU_EVENT_TYPE_PAGE_LOAD_START                 7032
#define ASC_MENU_EVENT_TYPE_PAGE_LOAD_END                   7033

#define ASC_MENU_EVENT_TYPE_CEF_DESTROYWINDOW               8000


#define ASC_MENU_EVENT_TYPE_WINDOWS_MESSAGE_USER_COUNT      10

namespace NSEditorApi
{
    class CAscCreateTab : public IMenuEventDataBase
    {
    private:
        std::wstring m_sUrl;
        bool m_bActive;
        int m_nIdEqual;

    public:
        CAscCreateTab()
        {
            m_bActive = true;
            m_nIdEqual = -1;
        }
        virtual ~CAscCreateTab()
        {
        }

        LINK_PROPERTY_STRING(Url)
        LINK_PROPERTY_BOOL(Active)
        LINK_PROPERTY_INT(IdEqual)
    };

    class CAscDownloadFileInfo : public IMenuEventDataBase
    {
    private:
        std::wstring    m_sUrl;
        std::wstring    m_sFilePath;

        int             m_nId;

        double          m_dSpeed;       // Kb/sec
        int             m_nPercent;
        bool            m_bIsComplete;
        bool            m_bIsCanceled;
        
        unsigned int    m_nIdDownload;

    public:
        CAscDownloadFileInfo()
        {
            m_nId           = -1;

            m_dSpeed        = 0;
            m_nPercent      = 0;
            m_bIsComplete   = false;
            m_bIsCanceled     = false;
        }
        virtual ~CAscDownloadFileInfo()
        {
        }

        LINK_PROPERTY_STRING(Url)
        LINK_PROPERTY_STRING(FilePath)
        LINK_PROPERTY_INT(Percent)
        LINK_PROPERTY_BOOL(IsComplete)
        LINK_PROPERTY_BOOL(IsCanceled)
        LINK_PROPERTY_INT(Id)
        LINK_PROPERTY_DOUBLE(Speed)
        LINK_PROPERTY_UINT(IdDownload)
    };

    class CAscOnOpenExternalLink : public IMenuEventDataBase
    {
    private:
        std::wstring    m_sUrl;
        int             m_nId;

    public:
        CAscOnOpenExternalLink()
        {
            m_nId           = -1;
        }
        virtual ~CAscOnOpenExternalLink()
        {
        }

        LINK_PROPERTY_STRING(Url)
        LINK_PROPERTY_INT(Id)
    };

    class CAscTabEditorType : public IMenuEventDataBase
    {
    private:
        int m_nId;
        int m_nType;

    public:
        CAscTabEditorType()
        {
        }
        virtual ~CAscTabEditorType()
        {
        }

        LINK_PROPERTY_INT(Id)
        LINK_PROPERTY_INT(Type)
    };

    class CAscSpellCheckType : public IMenuEventDataBase
    {
    private:
        std::string m_sResult;
        int         m_nEditorId;
        int         m_nFrameId;

    public:
        CAscSpellCheckType()
        {
            m_nEditorId = 0;
            m_nFrameId = 0;
        }
        virtual ~CAscSpellCheckType()
        {
        }

        LINK_PROPERTY_STRINGA(Result)
        LINK_PROPERTY_INT(EditorId)
        LINK_PROPERTY_INT(FrameId)
    };

    class CAscKeyboardLayout : public IMenuEventDataBase
    {
    private:
        int         m_nLanguage;

    public:
        CAscKeyboardLayout()
        {
        }
        virtual ~CAscKeyboardLayout()
        {
        }

        LINK_PROPERTY_INT(Language)
    };

    class CAscTypeId : public IMenuEventDataBase
    {
    private:
        int m_nId;

    public:
        CAscTypeId()
        {
        }
        virtual ~CAscTypeId()
        {
        }

        LINK_PROPERTY_INT(Id)
    };

    class CAscDocumentOnSaveData : public IMenuEventDataBase
    {
    private:
        int m_nId;
        bool m_bIsCancel;

    public:
        CAscDocumentOnSaveData()
        {
            m_bIsCancel = false;
        }
        virtual ~CAscDocumentOnSaveData()
        {
        }

        LINK_PROPERTY_INT(Id)
        LINK_PROPERTY_BOOL(IsCancel)
    };

    class CAscDocumentModifyChanged : public IMenuEventDataBase
    {
    private:
        int m_nId;
        bool m_bChanged;

    public:
        CAscDocumentModifyChanged()
        {
        }
        virtual ~CAscDocumentModifyChanged()
        {
        }

        LINK_PROPERTY_INT(Id)
        LINK_PROPERTY_BOOL(Changed)
    };

    class CAscDocumentName : public IMenuEventDataBase
    {
    private:
        std::wstring m_sName;
        std::wstring m_sPath;
        std::wstring m_sUrl;
        int m_nId;

    public:
        CAscDocumentName()
        {
        }
        virtual ~CAscDocumentName()
        {
        }

        LINK_PROPERTY_STRING(Name)
        LINK_PROPERTY_STRING(Path)
        LINK_PROPERTY_STRING(Url)
        LINK_PROPERTY_INT(Id)
    };

    class CAscJSMessage : public IMenuEventDataBase
    {
    private:
        std::wstring m_sName;
        std::wstring m_sValue;
        int m_nId;

    public:
        CAscJSMessage()
        {
        }
        virtual ~CAscJSMessage()
        {
        }

        LINK_PROPERTY_STRING(Name)
        LINK_PROPERTY_STRING(Value)
        LINK_PROPERTY_INT(Id)
    };

    class CAscPrintProgress : public IMenuEventDataBase
    {
    private:
        int m_nId;
        int m_nProgress;

    public:
        CAscPrintProgress()
        {
        }
        virtual ~CAscPrintProgress()
        {
        }

        LINK_PROPERTY_INT(Id)
        LINK_PROPERTY_INT(Progress)
    };

    class CAscPrintEnd : public IMenuEventDataBase
    {
    private:
        int m_nId;
        int m_nPagesCount;
        int m_nCurrentPage;

    public:
        CAscPrintEnd()
        {
        }
        virtual ~CAscPrintEnd()
        {
        }

        LINK_PROPERTY_INT(Id)
        LINK_PROPERTY_INT(PagesCount)
        LINK_PROPERTY_INT(CurrentPage)
    };

    class CAscPrinterContextBase : public IMenuEventDataBase
    {
    public:
        CAscPrinterContextBase() : IMenuEventDataBase() {}
        virtual ~CAscPrinterContextBase() {}

        virtual void GetLogicalDPI(int& nDpiX, int& nDpiY) = 0;
        virtual void GetPhysicalRect(int& nX, int& nY, int& nW, int& nH) = 0;
        virtual void GetPrintAreaSize(int& nW, int& nH) = 0;
        virtual void BitBlt(unsigned char* pBGRA, const int& nRasterX, const int& nRasterY, const int& nRasterW, const int& nRasterH,
                            const double& x, const double& y, const double& w, const double& h, const double& dAngle) = 0;
    };

    class CAscPrintPage : public IMenuEventDataBase
    {
    private:
        CAscPrinterContextBase* m_pContext;
        int                     m_nPage;
        bool                    m_bIsRotate;

    public:
        CAscPrintPage()
        {
            m_pContext = NULL;
            m_nPage = 0;
        }
        virtual ~CAscPrintPage()
        {
            if (NULL != m_pContext)
                m_pContext->Release();
        }

        LINK_PROPERTY_INT(Page)
        LINK_PROPERTY_BOOL(IsRotate);

        inline CAscPrinterContextBase* get_Context()
        {
            return m_pContext;
        }
        inline void put_Context(CAscPrinterContextBase* pContext)
        {
            m_pContext = pContext;
        }
    };

    class CAscEditorScript : public IMenuEventDataBase
    {
    private:
        std::wstring m_sUrl;
        std::wstring m_sDestination;
        int m_nId;
        int m_nFrameId;

    public:
        CAscEditorScript()
        {
            m_nId = -1;
            m_nFrameId = -1;
        }
        virtual ~CAscEditorScript()
        {
        }

        LINK_PROPERTY_INT(Id)
        LINK_PROPERTY_STRING(Url)
        LINK_PROPERTY_STRING(Destination)
        LINK_PROPERTY_INT(FrameId)
    };

    class CAscEditorSaveQuestion : public IMenuEventDataBase
    {
    private:
        int m_nId;
        bool m_bValue;

    public:
        CAscEditorSaveQuestion()
        {
            m_nId = -1;
            m_bValue = false;
        }
        virtual ~CAscEditorSaveQuestion()
        {
        }

        LINK_PROPERTY_BOOL(Value)
        LINK_PROPERTY_INT(Id)
    };
}

namespace NSEditorApi
{
    class CAscSaveDialog : public IMenuEventDataBase
    {
    private:
        std::wstring    m_sFilePath;
        int             m_nId;
        unsigned int    m_nIdDownload;

    public:
        CAscSaveDialog()
        {
            m_nId           = -1;
        }
        virtual ~CAscSaveDialog()
        {
        }

        LINK_PROPERTY_STRING(FilePath)
        LINK_PROPERTY_INT(Id)
        LINK_PROPERTY_UINT(IdDownload)
    };
}

namespace NSEditorApi
{
    class CAscKeyboardDown : public IMenuEventDataBase
    {
    private:
        int m_nKeyCode;
        bool m_bIsCtrl;
        bool m_bIsShift;
        bool m_bIsAlt;
        bool m_bIsCommandMac;

    public:
        CAscKeyboardDown()
        {
            m_nKeyCode  = -1;
            m_bIsCtrl   = false;
            m_bIsShift  = false;
            m_bIsAlt    = false;
            m_bIsCommandMac = false;
        }
        virtual ~CAscKeyboardDown()
        {
        }

        LINK_PROPERTY_INT(KeyCode)
        LINK_PROPERTY_BOOL(IsCtrl)
        LINK_PROPERTY_BOOL(IsShift)
        LINK_PROPERTY_BOOL(IsAlt)
        LINK_PROPERTY_BOOL(IsCommandMac)
    };
}

namespace NSEditorApi
{
    class CAscLocalSaveFileDialog : public IMenuEventDataBase
    {
    private:
        int m_nId;
        int m_nFileType;
        std::wstring m_sPath;
        std::vector<int> m_arSupportFormats;

    public:
        CAscLocalSaveFileDialog()
        {
            m_nId  = -1;
            m_nFileType = 0;
        }
        virtual ~CAscLocalSaveFileDialog()
        {
        }

        LINK_PROPERTY_INT(Id)
        LINK_PROPERTY_INT(FileType)
        LINK_PROPERTY_STRING(Path)

        std::vector<int>& get_SupportFormats()
        {
            return m_arSupportFormats;
        }
    };

    class CAscLocalOpenFileDialog : public IMenuEventDataBase
    {
    private:
        int m_nId;
        std::wstring m_sPath;
        std::wstring m_sFilter;

    public:
        CAscLocalOpenFileDialog()
        {
            m_nId  = -1;
        }
        virtual ~CAscLocalOpenFileDialog()
        {
        }

        LINK_PROPERTY_INT(Id)
        LINK_PROPERTY_STRING(Path)
        LINK_PROPERTY_STRING(Filter)
    };

    class CAscLocalOpenFiles : public IMenuEventDataBase
    {
    private:
        std::vector<std::wstring> m_arFiles;

    public:
        CAscLocalOpenFiles()
        {
        }
        virtual ~CAscLocalOpenFiles()
        {
        }

        std::vector<std::wstring>& get_Files()
        {
            return m_arFiles;
        }
    };

    class CAscEditorExecuteCommand : public IMenuEventDataBase
    {
    private:
        std::wstring m_sCommand;
        std::wstring m_sParams;

    public:
        CAscEditorExecuteCommand()
        {
        }
        virtual ~CAscEditorExecuteCommand()
        {
        }

        LINK_PROPERTY_STRING(Command)
        LINK_PROPERTY_STRING(Params)
    };
}

namespace NSEditorApi
{
    class CAscAddPlugin : public IMenuEventDataBase
    {
    private:
        std::wstring m_sPath;

    public:
        CAscAddPlugin()
        {
        }
        virtual ~CAscAddPlugin()
        {
        }

        LINK_PROPERTY_STRING(Path)
    };
}

namespace NSEditorApi
{
    class CAscReporterMessage : public IMenuEventDataBase
    {
    private:
        std::wstring m_sMessage;
        int m_nReceiverId;

    public:
        CAscReporterMessage()
        {
            m_nReceiverId = -1;
        }
        virtual ~CAscReporterMessage()
        {
        }

        LINK_PROPERTY_STRING(Message)
        LINK_PROPERTY_INT(ReceiverId)
    };

    class CAscReporterCreate : public IMenuEventDataBase
    {
    private:
        void* m_pData;

    public:
        CAscReporterCreate()
        {
            m_pData = NULL;
        }
        virtual ~CAscReporterCreate()
        {
        }

        void* get_Data() { return m_pData; }
        void put_Data(void* data) { m_pData = data; }
    };
}

namespace NSEditorApi
{
    class CAscSSOToken : public IMenuEventDataBase
    {
    private:
        std::wstring m_sUrl;
        std::wstring m_sToken;

    public:
        CAscSSOToken()
        {
        }
        virtual ~CAscSSOToken()
        {
        }

        LINK_PROPERTY_STRING(Url)
        LINK_PROPERTY_STRING(Token)
    };
}

namespace NSEditorApi
{
    class CAscUIThreadMessage : public IMenuEventDataBase
    {
    private:
        int m_nType;
        std::vector<std::wstring> m_arArgs;

    public:
        CAscUIThreadMessage()
        {
        }
        virtual ~CAscUIThreadMessage()
        {
        }

        LINK_PROPERTY_INT(Type)
        std::vector<std::wstring>& GetArgs() { return m_arArgs; }
    };
}

#endif // APPLICATION_MANAGER_EVENTS_H
