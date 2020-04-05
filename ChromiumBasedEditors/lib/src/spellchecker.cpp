/*
 * (c) Copyright Ascensio System SIA 2010-2019
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
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
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

#include "../src/applicationmanager_p.h"
#include <map>
#include <list>
#include "../../../../core/DesktopEditor/hunspell-1.3.3/src/hunspell/hunspell.h"
#include "../../../../core/DesktopEditor/graphics/TemporaryCS.h"
#include "../../../../core/DesktopEditor/graphics/Timer.h"
#include "./plugins.h"

class CJsonTextItem
{
protected:
    char*	m_pData;
    size_t	m_lSize;

    char*	m_pDataCur;
    size_t	m_lSizeCur;

public:
    CJsonTextItem()
    {
        m_pData = NULL;
        m_lSize = 0;

        m_pDataCur	= m_pData;
        m_lSizeCur	= m_lSize;
    }
    CJsonTextItem(const CJsonTextItem& oSrc)
    {
        m_pData = NULL;
        *this = oSrc;
    }
    CJsonTextItem& operator=(const CJsonTextItem& oSrc)
    {
        RELEASEMEM(m_pData);

        m_lSize		= oSrc.m_lSize;
        m_lSizeCur	= oSrc.m_lSizeCur;
        m_pData		= (char*)malloc(m_lSize * sizeof(char));

        memcpy(m_pData, oSrc.m_pData, m_lSizeCur * sizeof(char));

        m_pDataCur = m_pData + m_lSizeCur;

        return *this;
    }

    CJsonTextItem(const size_t& nLen)
    {
        m_lSize = nLen;
        m_pData = (char*)malloc(m_lSize * sizeof(char));

        m_lSizeCur = 0;
        m_pDataCur = m_pData;
    }
    virtual ~CJsonTextItem()
    {
        RELEASEMEM(m_pData);
    }

    inline void AddSize(const size_t& nSize)
    {
        if (NULL == m_pData)
        {
            m_lSize = 1000;
            if (nSize > m_lSize)
                m_lSize = nSize;

            m_pData = (char*)malloc(m_lSize * sizeof(char));

            m_lSizeCur = 0;
            m_pDataCur = m_pData;
            return;
        }

        if ((m_lSizeCur + nSize) > m_lSize)
        {
            while ((m_lSizeCur + nSize) > m_lSize)
            {
                m_lSize *= 2;
            }

            char* pRealloc = (char*)realloc(m_pData, m_lSize * sizeof(char));
            if (NULL != pRealloc)
            {
                // реаллок сработал
                m_pData		= pRealloc;
                m_pDataCur	= m_pData + m_lSizeCur;
            }
            else
            {
                char* pMalloc = (char*)malloc(m_lSize * sizeof(char));
                memcpy(pMalloc, m_pData, m_lSizeCur * sizeof(char));

                free(m_pData);
                m_pData		= pMalloc;
                m_pDataCur	= m_pData + m_lSizeCur;
            }
        }
    }

public:

    inline void operator+=(const std::string& oTemp)
    {
        WriteString(oTemp.c_str(), oTemp.length());
    }
    inline char operator[](const size_t& nIndex)
    {
        if (nIndex < m_lSizeCur)
            return m_pData[nIndex];

        return 0;
    }

    inline void AddChar(const char& c)
    {
        AddSize(1);
        *m_pDataCur = c;

        ++m_lSizeCur;
        ++m_pDataCur;
    }

    inline void AddBool(const bool& val)
    {
        if (val)
            WriteString("true", 4);
        else
            WriteString("false", 5);
    }

public:
    inline void WriteString(const char* pString, const size_t& nLen)
    {
        AddSize(nLen);
        memcpy(m_pDataCur, pString, nLen * sizeof(char));
        m_pDataCur += nLen;
        m_lSizeCur += nLen;
    }
    inline size_t GetCurSize()
    {
        return m_lSizeCur;
    }
    inline size_t GetSize()
    {
        return m_lSize;
    }
    inline void Clear()
    {
        RELEASEMEM(m_pData);

        m_pData = NULL;
        m_lSize = 0;

        m_pDataCur	= m_pData;
        m_lSizeCur	= 0;
    }
    inline void ClearNoAttack()
    {
        m_pDataCur	= m_pData;
        m_lSizeCur	= 0;
    }

    inline size_t GetStringLen(wchar_t* pData)
    {
        wchar_t* s = pData;
        for (; *s != 0; ++s);
        return (size_t)(s - pData);
    }

    inline std::string GetCString()
    {
        std::string str(m_pData, (int)m_lSizeCur);
        return str;
    }
    inline char* GetBuffer()
    {
        return m_pData;
    }
};

class CJsonSpellParser
{
public:
    enum SpellTaskType
    {
        sttSpell        = 0,
        sttSuggest      = 1,
        sttAdd          = 2,
        sttClear        = 3
    };

public:
    std::string                 Task;

    SpellTaskType               m_eType;
    std::list<std::string>      m_arWords;
    std::list<int>              m_arLanguages;

public:
    void Parse(const std::string& sTask)
    {
        // убираем }
        Task = sTask.substr(0, sTask.length() - 1);

        m_eType = sttSuggest;
        std::string sTaskType = CPluginsManager::GetStringValue(sTask, "type");
        if (!sTaskType.empty())
        {
            if ("spell" == sTaskType)
                m_eType = sttSpell;
            else if ("suggest" == sTaskType)
                m_eType = sttSuggest;
            else if ("add" == sTaskType)
                m_eType = sttAdd;
            else if ("clear" == sTaskType)
                m_eType = sttClear;
        }

        ParseWords();

        if (sttAdd != m_eType && sttClear != m_eType)
            ParseLanguages();
    }

    int ParseWords()
    {
        std::string sWords = CPluginsManager::GetArrayValue(Task, "usrWords");
        // в этом алгоритме все прокатывает, так как символы -" -[ -] имеют старший байт ноль.
        // т.е. не мешаются в utf8
        const char* pData = sWords.c_str();
        const char* pCur = pData;

        // ищем [
        while (*pCur != 0 && *pCur != '[')
        {
            ++pCur;
        }

        if (*pCur == 0)
            return (int)(pCur - pData);

        while (*pCur != 0 && *pCur != ']')
        {
            // 1) ищем первый "
            while (*pCur != 0 && *pCur != '\"')
                ++pCur;

            if (*pCur == 0)
                return (int)(pCur - pData);

            ++pCur;

            const char* pStart = pCur;

            // 2) ищем второй "
            while (*pCur != 0 && *pCur != '\"')
                ++pCur;

            if (*pCur == 0)
                return (int)(pCur - pData);

            m_arWords.push_back(std::string(pStart, (int)(pCur - pStart)));
            ++pCur;
        }

        return (int)(pCur - pData);
    }

    int ParseLanguages()
    {
        std::string sLangs = CPluginsManager::GetArrayValue(Task, "usrLang");
        // в этом алгоритме все прокатывает, так как символы -" -[ -] имеют старший байт ноль.
        // т.е. не мешаются в utf8
        const char* pData = sLangs.c_str();
        const char* pCur = pData;

        // ищем [
        while (*pCur != 0 && *pCur != '[')
        {
            ++pCur;
        }

        if (*pCur == 0)
            return (int)(pCur - pData);

        while (*pCur != 0)
        {
            // 1) ищем первую цифру
            while (*pCur != 0 && !is_digit(*pCur))
                ++pCur;

            if (*pCur == 0)
                return (int)(pCur - pData);

            const char* pStart = pCur;

            // 2) ищем первую не цифру
            while (*pCur != 0 && is_digit(*pCur))
                ++pCur;

            if (*pCur == 0)
                return (int)(pCur - pData);

            int nValue = 0;
            while (pStart != pCur)
            {
                nValue *= 10;
                nValue += (*pStart - '0');
                ++pStart;
            }

            m_arLanguages.push_back(nValue);
        }

        return (int)(pCur - pData);
    }

    inline bool is_digit(const char& c)
    {
        if (c >= '0' && c <= '9')
            return true;
        return false;
    }
};

#if 0
#define DEBUG_SPELL_CHECKER
#define DEBUG_SPELL_CHECKER_PATH "C:\\ProgramData\\ONLYOFFICE\\spell_checker.log"
#endif

class CAscSpellChecker_Private : public NSTimers::CTimer
{
public:
    class CDictionaryFiles
    {
    public:
        std::string path_aff;
        std::string path_dic;
    };

    typedef struct DictionaryRec_
    {
        const char*		m_name;
        int             m_lang;
    } DictionaryRec;

public:
    CAscApplicationManager*         m_pManager;
    std::map<int, Hunhandle*>       m_map_dictionaries;
    std::map<int, CDictionaryFiles> m_map_dictionaries_files;

    NSCriticalSection::CRITICAL_SECTION m_oCS;

    std::list<std::string>      m_arTask;
    std::list<int>              m_arTaskParent;
    std::list<int_64_type>      m_arTaskParentFrameId;

    std::wstring                m_sUserDictionaries;
    Hunhandle*                  m_pUserAllDict;

    // tmp buffer
    CJsonTextItem               m_oItem;

#ifdef DEBUG_SPELL_CHECKER
    int m_nAddTaskCounter;
    int m_nCheckTastCounter;
#endif

public:
    CAscSpellChecker_Private()
    {
        m_pManager = NULL;
        m_oCS.InitializeCriticalSection();
        m_pUserAllDict = NULL;

#ifdef DEBUG_SPELL_CHECKER
        m_nAddTaskCounter = 0;
        m_nCheckTastCounter = 0;
#endif
    }
    ~CAscSpellChecker_Private()
    {
        Stop();

        for (std::map<int, Hunhandle*>::iterator i = m_map_dictionaries.begin(); i != m_map_dictionaries.end(); i++)
        {
            Hunhandle* pDictionary = i->second;
            Hunspell_destroy(pDictionary);
        }

        if (m_pUserAllDict)
            Hunspell_destroy(m_pUserAllDict);

        m_oCS.DeleteCriticalSection();
    }

    void AddTask(const int& nParentId, const std::string& sTask, int_64_type nId)
    {
        CTemporaryCS oCS(&m_oCS);
        m_arTask.push_back(sTask);
        m_arTaskParent.push_back(nParentId);
        m_arTaskParentFrameId.push_back(nId);

#ifdef DEBUG_SPELL_CHECKER
        FILE* ff = fopen(DEBUG_SPELL_CHECKER_PATH, "a+");
        fprintf(ff, "add_task: %d, task: ", m_nAddTaskCounter++);
        fprintf(ff, sTask.c_str());
        fprintf(ff, ", parent_id: %d, id: %d\n", nParentId, nId);
        fclose(ff);
#endif
    }

    virtual void OnTimer()
    {
        m_oCS.Enter();

        if (0 == m_arTask.size())
        {
            m_oCS.Leave();
            return;
        }

        std::string sTask = *m_arTask.begin();
        m_arTask.pop_front();

        int nParentId = *m_arTaskParent.begin();
        m_arTaskParent.pop_front();

        int_64_type nFrameId = *m_arTaskParentFrameId.begin();
        m_arTaskParentFrameId.pop_front();

        m_oCS.Leave();

#ifdef DEBUG_SPELL_CHECKER
        FILE* ff = fopen(DEBUG_SPELL_CHECKER_PATH, "a+");
        fprintf(ff, "start_check_task: %d, task: ", m_nCheckTastCounter++);
        fprintf(ff, sTask.c_str());
        fprintf(ff, "\n");
        fclose(ff);
#endif

        CJsonSpellParser oParser;
        oParser.Parse(sTask);

        int nCountWords = oParser.m_arWords.size();
        int nCountLangs = oParser.m_arLanguages.size();

        if (nCountWords != nCountLangs && nCountWords > 0)
        {
            if (oParser.m_eType != CJsonSpellParser::sttAdd && oParser.m_eType != CJsonSpellParser::sttClear)
                return;
        }

        m_oItem.ClearNoAttack();

        if (oParser.m_eType == CJsonSpellParser::sttSpell)
        {
            m_oItem += oParser.Task;

            m_oItem += ",\"usrCorrect\":[";

            std::list<std::string>::iterator iWords = oParser.m_arWords.begin();
            std::list<int>::iterator iLangs = oParser.m_arLanguages.begin();
            bool bIsFirst = true;

            Hunhandle* pCurrent = NULL;
            int nCurrentLang = -1;
            for ( ; iWords != oParser.m_arWords.end(); iWords++, iLangs++)
            {
                if (!bIsFirst)
                {
                    m_oItem.AddChar(',');
                }

                bIsFirst = false;

                int nNewLang = *iLangs;
                if (nNewLang != nCurrentLang)
                {
                    pCurrent = SetLanguage(nNewLang);
                    nCurrentLang = nNewLang;
                }
                if (NULL == pCurrent)
                {
                    m_oItem.AddBool(true);
                }
                else
                {
                    int nSpellResult = Hunspell_spell(pCurrent, iWords->c_str());
                    if (0 == nSpellResult)
                    {
                        if (m_pUserAllDict && 0 != Hunspell_spell(m_pUserAllDict, iWords->c_str()))
                            m_oItem.AddBool(true);
                        else
                            m_oItem.AddBool(false);
                    }
                    else
                    {
                        m_oItem.AddBool(true);
                    }
                }
            }

            m_oItem += "],\"usrSuggest\":[]}";
        }
        else if (oParser.m_eType == CJsonSpellParser::sttSuggest)
        {
            m_oItem += oParser.Task;

            m_oItem += ",\"usrCorrect\":[],\"usrSuggest\":[";

            std::list<std::string>::iterator iWords = oParser.m_arWords.begin();
            std::list<int>::iterator iLangs = oParser.m_arLanguages.begin();
            bool bIsFirst = true;

            Hunhandle* pCurrent = NULL;
            int nCurrentLang = -1;
            for ( ; iWords != oParser.m_arWords.end(); iWords++, iLangs++)
            {
                if (!bIsFirst)
                {
                    m_oItem.AddChar(',');
                }

                bIsFirst = false;

                int nNewLang = *iLangs;
                if (nNewLang != nCurrentLang)
                {
                    pCurrent = SetLanguage(nNewLang);
                    nCurrentLang = nNewLang;
                }
                if (NULL == pCurrent)
                {
                    m_oItem.WriteString("[]", 2);
                }
                else
                {
                    char** pSuggest;
                    int nSuggestCount = Hunspell_suggest(pCurrent, &pSuggest, iWords->c_str());

                    m_oItem.AddChar('[');

                    for (int i = 0; i < nSuggestCount; ++i)
                    {
                        if (i != 0)
                            m_oItem.AddChar(',');

                        m_oItem.AddChar('\"');

                        m_oItem.WriteString(pSuggest[i], strlen(pSuggest[i]));

                        m_oItem.AddChar('\"');
                    }

                    if (0 < nSuggestCount)
                        Hunspell_free_list(pCurrent, &pSuggest, nSuggestCount);

                    m_oItem.AddChar(']');
                }
            }

            m_oItem += "]}";
        }
        else if (oParser.m_eType == CJsonSpellParser::sttAdd)
        {
            CreateUserDictionary_All();

            if (m_pUserAllDict)
                AddUserWords_All(oParser.m_arWords);

            m_oItem += "{\"type\":\"add\"}";
        }
        else if (oParser.m_eType == CJsonSpellParser::sttClear)
        {
            CreateUserDictionary_All(true);
            m_oItem += "{\"type\":\"clear\"}";
        }

#ifdef DEBUG_SPELL_CHECKER
        FILE* fff = fopen(DEBUG_SPELL_CHECKER_PATH, "a+");
        fprintf(fff, "end_check_task\n");
        fclose(fff);
#endif

        NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();
        NSEditorApi::CAscSpellCheckType* pData = new NSEditorApi::CAscSpellCheckType();
        pData->put_EditorId(nParentId);
        pData->put_FrameId(nFrameId);
        pData->put_Result(m_oItem.GetCString());
        pEvent->m_pData = pData;
        pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_SPELLCHECK;
        m_pManager->Apply(pEvent);

        // вдруг придет мега запрос
        if (m_oItem.GetSize() > 100000)
            m_oItem.Clear();
    }

    Hunhandle* SetLanguage(const int& nLang)
    {
        std::map<int, Hunhandle*>::iterator i = m_map_dictionaries.find(nLang);
        if (i != m_map_dictionaries.end())
            return i->second;

        std::map<int, CDictionaryFiles>::iterator i2 = m_map_dictionaries_files.find(nLang);
        if (i2 == m_map_dictionaries_files.end())
            return NULL;

        Hunhandle* pDictionary = Hunspell_create(i2->second.path_aff.c_str(), i2->second.path_dic.c_str());
        m_map_dictionaries.insert(std::pair<int, Hunhandle*>(nLang, pDictionary));
        return pDictionary;
    }

    void Init(const std::wstring& sDirectory, const std::wstring& sUserDirectory)
    {        
        #define DictionaryRec_count 45
        static const DictionaryRec Dictionaries[DictionaryRec_count] =
        {
            {"az_Latn_AZ", 1068},
            {"bg_BG", 1026},
            {"ca_ES", 1027},
            {"ca_ES_valencia", 2051},
            {"cs_CZ", 1029},
            {"da_DK", 1030},
            {"de_AT", 3079},
            {"de_CH", 2055},
            {"de_DE", 1031},
            {"el_GR", 1032},
            {"en_AU", 3081},
            {"en_CA", 4105},
            {"en_GB", 2057},
            {"en_US", 1033},
            {"en_ZA", 7177},
            {"es_ES", 3082},
            {"eu_ES", 1069},
            {"fr_FR", 1036},
            {"gl_ES", 1110},
            {"hr_HR", 1050},
            {"hu_HU", 1038},
            {"id_ID", 1057},
            {"it_IT", 1040},
            {"kk_KZ", 1087},
            {"ko_KR", 1042},
            {"lb_LU", 1134},
            {"lt_LT", 1063},
            {"lv_LV", 1062},
            {"mn_MN", 1104},
            {"nb_NO", 1044},
            {"nl_NL", 1043},
            {"nn_NO", 2068},
            {"pl_PL", 1045},
            {"pt_BR", 1046},
            {"pt_PT", 2070},
            {"ro_RO", 1048},
            {"ru_RU", 1049},
            {"sk_SK", 1051},
            {"sl_SI", 1060},
            {"sr_Cyrl_RS", 10266},
            {"sr_Latn_RS", 9242},
            {"sv_SE", 1053},
            {"tr_TR", 1055},
            {"uk_UA", 1058},
            {"vi_VN", 1066}
        };

        for (int i = 0; i < DictionaryRec_count; ++i)
        {
            std::string s = std::string(Dictionaries[i].m_name);

            std::string s1 = "/" + s + "/" + s + ".aff";
            std::string s2 = "/" + s + "/" + s + ".dic";

            std::wstring ws1 = sDirectory + NSFile::CUtf8Converter::GetUnicodeStringFromUTF8((BYTE*)s1.c_str(), (LONG)s1.length());
            std::wstring ws2 = sDirectory + NSFile::CUtf8Converter::GetUnicodeStringFromUTF8((BYTE*)s2.c_str(), (LONG)s2.length());

            NSFile::CFileBinary oFile1;
            bool b1 = oFile1.OpenFile(ws1);
            oFile1.CloseFile();

            NSFile::CFileBinary oFile2;
            oFile2.OpenFile(ws2);
            bool b2 = oFile2.OpenFile(ws2);

            if (b1 && b2)
            {
                CDictionaryFiles files;
                files.path_aff = NSFile::CUtf8Converter::GetUtf8StringFromUnicode2(ws1.c_str(), (LONG)ws1.length());
                files.path_dic = NSFile::CUtf8Converter::GetUtf8StringFromUnicode2(ws2.c_str(), (LONG)ws2.length());

                m_map_dictionaries_files.insert(std::pair<int, CDictionaryFiles>(Dictionaries[i].m_lang, files));
            }
        }

        m_sUserDictionaries = sUserDirectory;
        if (!NSDirectory::Exists(m_sUserDictionaries))
            NSDirectory::CreateDirectory(m_sUserDictionaries);

        CreateUserDictionary_All();
    }

    void CreateUserDictionary_All(bool bIsRecreate = false)
    {
        if (m_pUserAllDict)
        {
            if (bIsRecreate)
            {
                Hunspell_destroy(m_pUserAllDict);
                m_pUserAllDict = NULL;
            }
            return;
        }

        if (!NSDirectory::Exists(m_sUserDictionaries + L"/all"))
            NSDirectory::CreateDirectory(m_sUserDictionaries + L"/all");

        std::wstring sAll_aff = m_sUserDictionaries + L"/all/all.aff";
        std::wstring sAll_dic = m_sUserDictionaries + L"/all/all.dic";

        if (bIsRecreate)
        {
            NSFile::CFileBinary::Remove(sAll_aff);
            NSFile::CFileBinary::Remove(sAll_dic);
        }

        if (!NSFile::CFileBinary::Exists(sAll_aff))
            NSFile::CFileBinary::SaveToFile(sAll_aff, L"SET UTF-8", true);

        if (!NSFile::CFileBinary::Exists(sAll_dic))
            NSFile::CFileBinary::SaveToFile(sAll_dic, L"2\nonlyoffice\nONLYOFFICE", true);

        std::string sAff = U_TO_UTF8(sAll_aff);
        std::string sDic = U_TO_UTF8(sAll_dic);
        m_pUserAllDict = Hunspell_create(sAff.c_str(), sDic.c_str());
    }

    void AddUserWords_All(const std::list<std::string>& words)
    {
        std::wstring sAll_dic = m_sUserDictionaries + L"/all/all.dic";

        BYTE* pFileData = NULL;
        DWORD nFileSize = 0;

        if (!NSFile::CFileBinary::ReadAllBytes(sAll_dic, &pFileData, nFileSize))
            return;

        DWORD nStart = 3;
        DWORD nNewLine = nStart;
        DWORD nCountMax = nNewLine + 100;
        if (nCountMax > nFileSize)
            nCountMax = nFileSize;

        while (nNewLine < nCountMax)
        {
            if ('\n' == pFileData[nNewLine])
                break;
            ++nNewLine;
        }

        if (nNewLine == nCountMax)
            return;

        std::string sCountData((char*)pFileData + nStart, nNewLine - nStart);
        int nCountLine = std::stoi(sCountData);
        nCountLine += (int)words.size();
        sCountData = std::to_string(nCountLine);

        NSFile::CFileBinary::Remove(sAll_dic);

        NSFile::CFileBinary oFile;
        if (!oFile.CreateFileW(sAll_dic))
            return;

        BYTE utf8[3];
        utf8[0] = 0xEF;
        utf8[1] = 0xBB;
        utf8[2] = 0xBF;

        oFile.WriteFile(utf8, 3);
        oFile.WriteFile((BYTE*)sCountData.c_str(), (DWORD)sCountData.length());
        oFile.WriteFile(pFileData + nNewLine, nFileSize - nNewLine);

        utf8[0] = '\n';
        for (std::list<std::string>::const_iterator iter = words.begin(); iter != words.end(); iter++)
        {
            Hunspell_add(m_pUserAllDict, iter->c_str());

            oFile.WriteFile(utf8, 1);
            oFile.WriteFile((BYTE*)iter->c_str(), (DWORD)iter->length());
        }

        oFile.CloseFile();
    }
};

CAscSpellChecker::CAscSpellChecker()
{
    m_pInternal = new CAscSpellChecker_Private();
}

CAscSpellChecker::~CAscSpellChecker()
{
    RELEASEOBJECT(m_pInternal);
}

void CAscSpellChecker::SetApplicationManager(CAscApplicationManager* pManager)
{
    m_pInternal->m_pManager = pManager;
}

void CAscSpellChecker::AddTask(const int& nEditorId, const std::string& sTask, int_64_type nId)
{
    if (m_pInternal->IsRunned())
        m_pInternal->AddTask(nEditorId, sTask, nId);
}

void CAscSpellChecker::Start()
{
    m_pInternal->Start(-2);
}

void CAscSpellChecker::End()
{
    m_pInternal->Stop();
}

void CAscSpellChecker::Init(const std::wstring& sDirectory, const std::wstring& sUserDirectory)
{
    m_pInternal->Init(sDirectory, sUserDirectory);
}
