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

#include "./client_scheme.h"
#include "./client_app.h"

#include <algorithm>
#include <string>

#include "include/cef_scheme.h"
#include "include/wrapper/cef_helpers.h"
#include "../../../../../core/DesktopEditor/common/File.h"
#include "include/cef_parser.h"

namespace asc_scheme
{

std::string GetMimeTypeFromExt(const std::wstring& sFile)
{
    std::wstring sExt = NSFile::GetFileExtention(sFile);

    if (sExt == L"html")
        return "text/html";
    if (sExt == L"js")
        return "text/javascript";
    if (sExt == L"css")
        return "text/css";
    if (sExt == L"json")
        return "application/json";
    if (sExt == L"png")
        return "image/png";
    if (sExt == L"jpg" || sExt == L"jpeg")
        return "image/jpeg";

    return "*/*";
}
std::vector<std::wstring> get_url_params(std::wstring& sUrl)
{
    std::vector<std::wstring> ret;
    std::wstring::size_type pos = sUrl.find(L"__ascdesktopeditor__param__");
    std::wstring::size_type posF = pos;
    while (pos != std::wstring::npos)
    {
        std::wstring::size_type posParam = pos + 27; // len(__ascdesktopeditor__param__) = 27
        pos = sUrl.find(L"__ascdesktopeditor__param__", posParam);

        std::wstring::size_type posEnd = (pos == std::wstring::npos) ? sUrl.length() : pos;
        if (posEnd > posParam)
            ret.push_back(sUrl.substr(posParam, posEnd - posParam));
    }
    if (posF != std::wstring::npos)
        sUrl = sUrl.substr(0, posF);
    return ret;
}
unsigned long read_file_with_urls(std::wstring& sUrl, unsigned char*& data)
{
    data = NULL;

    std::vector<std::wstring> arParams = get_url_params(sUrl);

    unsigned long start = 0;
    unsigned long count = 0;
    if (arParams.size() > 0)
        start = std::stoul(arParams[0]);
    if (arParams.size() > 1)
        count = std::stoul(arParams[1]);

    DWORD dwSize = 0;
    NSFile::CFileBinary oFile;
    if (oFile.OpenFile(sUrl))
    {
        unsigned long fileSize = (unsigned long)oFile.GetFileSize();
        if (fileSize)
        {
            if (start > fileSize)
                start = fileSize - 1;
            if (count == 0)
                count = fileSize;
            if ((start + count) > fileSize)
                count = fileSize - start;

            if (start > 0)
                oFile.SeekFile((int)start);

            data = new unsigned char[count];
            oFile.ReadFile(data, (DWORD)count, dwSize);
        }
    }
    return (unsigned long)dwSize;
}

// Implementation of the schema handler for client:// requests.
class ClientSchemeHandler : public CefResourceHandler
{
public:
    ClientSchemeHandler(CAscApplicationManager* pManager = NULL) : offset_(0)
    {
        data_binary_ = NULL;
        data_binary_len_ = 0;

        m_pManager = pManager;
    }
    virtual ~ClientSchemeHandler()
    {
        if (NULL != data_binary_)
            delete [] data_binary_;
    }

    virtual bool ProcessRequest(CefRefPtr<CefRequest> request,
                              CefRefPtr<CefCallback> callback)
                              OVERRIDE
    {
        CEF_REQUIRE_IO_THREAD();

        std::string url = request->GetURL().ToString();

        // страница, которая показывается при падении процесса рендерера
        std::string::size_type posFind = url.find("ascdesktop://crash.html");
        if (posFind != std::string::npos)
        {
            std::string sPageCrash1 = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>crash</title></head><style>.image_crash { background-image: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAKoAAACqCAMAAAAKqCSwAAAABGdBTUEAALGPC/xhBQAAAMBQTFRF4uLivr6+lpaWvr6+qKiotLS0u7u7m8fsoqKira2tvLy8m8fsmMXrl8TrlMProsXincjt////////iLzohLrn////////////////////erTl////////oMrtlsTr////////fbbmerTl////////h7jiy+L1fLXmerTlerTle7XmerTlgLjns9Hq/f39o8bjocfofbXkAAAAqqqq5+fn3NzcjY2N8/Pzz8/Pubm5mJiYwsLCvb29u7u7erTl////4T6idQAAADN0Uk5Tfezfg6T3tbV1StaCyZhyLeSOu/tKVkjJrZt1OGf+/doorMR761kb1+6cZoc+BfsVCyoAqmsEbAAABpJJREFUeNrt3Gl3ojAUBuCZ6Ux3q9alLsWdVZFltC5A+P//agDRsgVBg0nOkI9z5ujT+ObeJNL+0KkZP0pqSS2pZFNH9FDfOyNaqB17WVfooNZt2559yjRQp7Y7hm0KqJ/2YbyPiKe2faq97HKEU/v2aaCPLFoqZwcG6siipcp2aKCNLOLGOvOjWkRkEVM/fOPqFNmpTCj13Rdu1t+RbZFJ7R6Bq20gsn0SqfWTbxuw2kgii5g6PemWxnb5bV1OVdKorW/dxjKWwSrbIozaDuDWlrEJVtmPPlHUUdC2jVivjCxiaqizboyo9arIoj4GBuNpryzLDFuviSxq6iwEWzvWVdh6eWRRUz/CLsPaxax2hyOC+h5WbUzLsmLWyyKLmtqNqFbWzrLWUauz8cZPrUdR22TrBZFFTf2MmQzLwSZY7bzXG6iprZjIi6u1TbDmvN5ATW3HRV5cre0yAZsrsqipowSQG1cz2ZonsqipStIHbbgRMJOt2SOL/H41ybPxImBGNgR5I4ucOkzSrF2rCbNmvN5ATv1IxGy9CECtma43kFM7iRSnYrlLC27NcL2BnNpNlqwO02oaK5j17PUGcuoUIvHjmmI9d72BnPoJgxj+vJpwa/rGGzm1BWO4DdZ0l5a5hlvTIouc2ocqVn4ETCPNCo8scuoIrtgeI5BqhUYWOVVNQRh+FTCNbZoVEln0X1wu4QRvP+hNq7FdpmKTbuTQU4cpgpWV1ZpwvYGe+pEGcPaDhypw3ho7K6KndlLf3zjMqkM1tpuU/zf76NRbRVO7qXO1OVWsJOts+N6dfrb7nHyTZTVN/1zXpypwsi6HzhROW/2Rets9ALyzBqqrP62G4UzhKOtdC3pq6ww1GNd7nCfWtM56iuspAk94qaNz1GNcTeOnjJeqnp3Vv8dpvdPxUiNXrPFl9fV1LFhV3NRh+pR+HaiO9VnHTU3rrOu9I/3a7bxpvcNOhXfW5bML/dr/sbx5VbBT69Bt6EjxpPu/3rTe69ipsM5ad/rmoyt1qc60svipyZ115t2fqL/2e39Wf6r4qe3E+z4/mI9H6u63jp/aT73y/XWkPhFA5eL3vYHD/ZM7rY70WSeAKsdOHqFW//tAvSOBGumsw8jhUztQq0RQQ521G1vozrT+ueDzL4T6Hi1R4aHs98+7RzKonTNf/D7u73cKGdRjZ11CvpWSf/2+18mgTuMlKjyeHllCqJ8JJSpiVQmhthNKVCQCOiHUflKJun4UQeVmrQJetRCqzOm0UAsaJfX8xYbG0UJdTERaqBoQaKHKAlBoWVYTsKCFKgKJFuoczGmhSkCkharNm2W3Kql0UKWeTAs1d7vCR+UBRw9VK6kltaQi3lqNqSlWZWP9v6kqi/dspYmZ318DPFaqBMAkYwlqgjEmqjZRvFOIADKeRLGdrWTeP4CqIsh2FMV2YpUAr36HIMu8jgGm5wH4wBuzIEvL5DHdroQvy3pZ5ksAeB5deMu9RlRxgqcFjBusXvhAQ31tRIKnLkilvozDwVMEQSWUGhuTcwWrtyCFKoFJjpKBlaqA9Fok5d4BIKNqSiwBzfRe1cNErQze8rV4Ie8ZEBmVHbxG/yX1E17k3qwio6qDQeRhJE1Io4q5t1XosloZVPL8ZJd8/siWVWxaU3e3kqhjo+oV5pWWE6tSY95oOVw3H8KlFUB3AbKCmRpvV9DNbUMigCpnaPJao7HAT2Vr1e8iD9mvqHxDJCAAD0yteeaYL48bvEoAVa4wzIvs96PkQFYaDU0nYlm9Mcxr9XB21iDSi48yqCvAosa4BVaBrKq3y6UF/CLzC+NqFMh29YoH24p42N6bvujsyezDlX+HsaAWoA2YWoU9TaHSrNQGgzcSqRr34mSWYWqv7kw+OMzB4FUicVY5916w+vJQYwbehYbjfKle/aqFUMXjaUXxgFUOyV8LLYKqAaAV8LJFUCdA1OmgskBQ6KByApB0Oqi9C+54cAVAUqih3vgYuBDH4iV7oIV8a+ocuGN+weIfy7elsuAw2Nwr6pK7qKuoE5+a7/sa9zvL3q2zCo4jVzvlgcDefFkJvjTPfX0TAF67fQUQfWqeVq7yc1W/PVU7TKuQ8SCk9FxkwVBYsdLchRV4XkKaQwumwo5BQU0/Y7fSFsHg8a58LmlR72LuFYvxAic1SuK9SLgnZkmcz+ei6PHcVjGRFF0nh+pGsdkbe0v8WHR73o/QVPVbjfzblabUcwer6Tce5dNrJbWk/s/Uf7OkVQroCk8dAAAAAElFTkSuQmCC\");  background-size: 170px 170px;  width: 170px;  height: 170px;} @media only screen and (-Webkit-min-device-pixel-ratio: 1.5),only screen and (-moz-min-device-pixel-ratio: 1.5),only screen and (-o-min-device-pixel-ratio: 3/2),only screen and (min-device-pixel-ratio: 1.5) {  .image_crash {    background-image: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAVQAAAFUCAMAAABMTDSHAAAABGdBTUEAALGPC/xhBQAAAMBQTFRF39/f////lZWVlpaWt8PNwMDA////vr6+srKyo6Oj////////////nMfsmMXrmsbs////ncjs////////////////fbbm/////////////////f39////e7Xm////fLXmerTlf7fmfrfme7Xmgrbhe7XmerTle7XmgrfjstLt/Pz8o8npncXnn8ntAAAArq6upaWlib3o3Nzc7u7uk8LqhYWFj4+PzMzMmJiYx8fHwsLCurq6vb29uLi4erTl////rmfTXAAAAC90Uk5TwaCy32GmpdGDVJyUq9puqHzGs9BMiphvwtzqJ12nOmDeiUq7Ju/MeDca+xAH/gCnWeErAAAOwUlEQVR42u3d63aiSBAA4JndzCSZJOZuLsagCAKC7IxmYlTU93+r5aICKtLdNG21VP3cc3Z35jvV1dU3/OZicI9vSICoiIqoGIiKqIiKgaiIiqgYiIqoiHqYuGwgKvd46T3aiMo5nheL1ycHUfkO/4Uf9w+IyjMuFmHcNRCVX9xEqAv5Sitg1LfFKmQrrYBRHxZx3L8jKpd4XyRDpq4VMKqVQpWptAJGdRYb8fqGqIXjdVNVltIKGfV+sR1SlFbIqHc7UKUorZBRvy92BvzSChn1cZER0EsrZNSnRWb8bCBq0XWqZKUVMur7Yl8ALq2QURuL/fH8jqjUYS/yAmhpBX2a2tswHG6X1hsHUeniZdNwsJ2sL2+IShXPm4LDkRSlFTTq5RbgYLSztFqIShwX236jcW9X1wqrtIJGvdnB5012qcIqraBRdy2phrPJEHrXChr1YRfeYObtVl38shCVdZ06zlSFUlpBo1q76aazaYbq4uUBUXPC2S03nM1mQ8gbArAv/b7uhhv5qoPMDYHDl1bYqPcZbpO9qr1D3xKCjXqXwTac7lU9dGmFjfo9S23go85GUPdaYaNmHv2FZXWv6uLCQtSdkX301/NyVQ93ARM26p6jv6Cvms12bq8cvLTCRt139BeW1dlkr+qB7rbDRt179DeOVId7VXuHKK2wUfce/UVlNXMj4IClFfgzyr2DOyqruarinw0BR33ZqzWKVKd5qqJLK3DU5/1Yk0h1lqvau7ARdRWX+63C5eps/5L1AKUVOOpFDtVgRqwqsLQCR73JkxpRqAorrcBR33KLpbdSHRGoCrqACRz1IddpOKNRFVNagaO+5zMN6FRFlFbgqA0CpfFadf/2irhnQ8BRHZJCuS6redsrokor9K/9vBIYxWU1b3slUVqrjHpPQhSX1fyNABHPhqCjPhMN5wmDaomlFTrqTyKf9XKVaHul9NIKHfWRzCdRAAi2V8p+NgQd9YmQZ5RAnQ6IVcsprdBR30iH8iSZqxSqZZRW6KjvpDbDGasq/9IKHbVBbJMqq4RL1pJKK3RUi5wmWVZnUypVzqUV/PdTe+TD2GPPVb4vMsGjvpC7pMsq6fZKCaUVPOo9hUu6rJJur/AvreBR72hYJhuqQzpVXhcwwaP+okFJLleD2YpalU9pBY96Q2WyUQCmHrUqj2dD4FGf6EzSfRWLKocXmeBRHyhJUmV1OmVRLVxawaO+U4Ikyuo0QJ16A3rVgs+GwKM2aD0Gs7Sqx6RaqLSCR3WoPUYbmcqmWqS0wv+Zj1fqJPNSicqsyl5a4aO+UGOsy+p0mamsqqylFT7qM73FYLalOmJTZSut8FF/MliM4/Efkk68yajHxsrybAg+6iNLgnnpTPVRmVUZSit81CcWiGFq9vdVJwVUqUsrfNQ3JodRavafhKyjIasq5QVM+";
            std::string sPageCrash2 = "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\");  }} </style> <body style=\"background-color:#E8E8E8;-webkit-user-select:none\"> <div class=\"image_crash\" style=\"position: fixed;top: 50%;left: 50%; margin-top: -200px; margin-left: -85px;\" width=\"170\" height=\"170\" /> <div style=\"position:fixed; top:50%; left: 0; width:100%;text-align: center;font-family: Open Sans Semibold,sans-serif,Arial;font-size:18px;color:#313437;\">Something has gone wrong... <div style=\"position:fixed; top:50%; margin-top:46px; margin-left: -330px; left: 50%; width:660px;text-align: center;font-family: Open Sans,sans-serif,Arial;font-size:12px;color:#666666;\">Work with the file has been canceled for some reason. This could be caused by lack of memory or some other reason. Please reopen the file or close this tab to continue.</div> </div> </body></html>";

            std::string sPageCrash = sPageCrash1 + sPageCrash2;

            mime_type_ = "text/html";
            data_ = sPageCrash;

            callback->Continue();
            return true;
        }

        // страница, которая показывается при ошибке загрузки
        posFind = url.find("ascdesktop://loaderror.html");
        if (posFind != std::string::npos)
        {
            std::string sPageCrash1 = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>crash</title></head><style>.image_crash { background-image: url(\"data:image/png;base64,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\");  background-size: 170px 170px;  width: 170px;  height: 170px;} @media only screen and (-Webkit-min-device-pixel-ratio: 1.5),only screen and (-moz-min-device-pixel-ratio: 1.5),only screen and (-o-min-device-pixel-ratio: 3/2),only screen and (min-device-pixel-ratio: 1.5) {  .image_crash {    background-image: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAVQAAAFUCAMAAABMTDSHAAAABGdBTUEAALGPC/xhBQAAAGBQTFRFMzMz/1JS/1JS/1JS/1JSMzMz/1JS/1JS/1JS/1JS/1JSMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzVDg4MzMzMzMzMzMzMzMzAAAA/29v/+Dg/5GR/7Gx//X1/////1JSjj5UDwAAABl0Uk5T4RHE8Sn5hqRn3kaDlF5ySzamtgfDESLMAMWS87gAAA44SURBVHja7Z3ZgqMqEIbJnggIbuicnum8/1uepOOWxERAKLFT/8Vc9XTjZ20gUKRCORdBBAgVoSJUFEJFqAgVhVARKkJFIVSEilBRCBWhIlQUQkWoCBWFUBEqQkUhVISKUFEIFaEiVBRCRagIFaGiECpCRagohIpQESoKoSJUhIpCqAgVoaIQKkJFqCiEilARKgqhIlSEikKoCBWhohAqQkWoCBWFUBEqQkUhVISKUFEIFaEiVBRCRagIFYVQESpCRSFUhIpQUQgVoSJUFEJFqAgVhVARKkJFqCiEilARKgqhIlSEikKoCBWhohAqQkWoKISKUBEqCqEiVISKQqgIFaGiECpCRagohIpQESpCRSFUhOpXSvKYsiRN87LM8zxNGI25VAjVlqeIkwvKQeUJ5SpDqEYqBEvLMeWMK4Sqa6JxUuoqjSVCHbdRrk+04aoQ6jtJWtqIiQyhvpBIhjJSzIVURVFVWVEoJQWn7Dl/pTxDqENIH1NTEotXjl1I/kg2h8VKFmillwQ0ykg+hF9Qa10AVMXsUs9DWkslQm2UxX03pmZkVNwPG0wh1Jsbp30XLszfST905ByhXpD0qqhE2L6XXvhgxcdDVZ2ZpmKKuXfWmosPhypyZyVRrySLsw+G2stQtHDw29o3lBQfC7Vgjmsh1caAVH0o1CJxaKa1sfI2msiPhNqmKKeJpa3PPFP1DXW1jY77w037/THaroyYJspPTPFbBHiEmkX7w+n8pNNhH2WaTJnzTN1mP7FAqNvjenN+qc36uNVgGvuo0wAigBeoq+P6PKr18UUkKBqmfiaVMvdO1QPU6HDW1CEaihqJZwdtqarFQI3WZwOtn7Ey70GvoZoWy4BqhnQIKwdIJA1VtgSoW2OkP1i3A1nK7yKd9Bq1XULN9mdL7bPHuVRc+VXjDzJwqNHpbK1TdE+VeV9Jin2GVTK7mT4bawGwlJzV+ZAGDHW1Pk/UelWBqqmGRbBQo815sjYRLNU6WaVZoFCPZxfaHGGpxt7mwiQYphfBUs3qAKBChLo/O9N+jgBAA4TqkCk0VeqpWCVBMQWmWuR+ZqsklHg6S1yN/UTViVCjs3NF8KZKg4K63biHutkCUq3XAIqAoGbrswetM3BTjQOCejh70QE8qqbhQI3OngQYVpWPqmoC1JV2QP36+6Mv7bAKuLjCPKQqAuD8/33X+i/AAHD7ZJ1nYUDVdv6/363+hhcAsty9/9tDPRkb6vf3P+1PAdBzVRoEVP2p1L8O6neAEyvhPv/bQs1OfqGewIrVwv1UlXg3VDuogKaaOP+sQrwbqiVUOFONnQdV4t1QLaHCmeptrTqZH+raP9Q1bFB1WanaQd2e/UM9g61Wpa4zlR3UAwRUsGkVc13+W0HNNhBQN1CpirrerGYF1Wx5yhYq2FyVu15TJd693x4qlP8L1zWVFdQTDFSoBQDp+puqDVSj3D8BKlT+V64LVRuoRyioR0io6bxQD1BQgYJqEQLUExTU0+dAXZ2hoJ5XHwM1goMafQzUIxxUmEwVQvbfw0HdfwzUAxxUmPQfQvG/hoMKs6YawjT1BAcVpqaKA1hQ2cBB3QAu/QmE6lJJAIvUZzioZxCoeQCfU34b1FtFlVefAhXE/YXzIyoYU+s8FSNUl0qd76XEOlW5P6ACMKOy2Z8KOKPiznf9QMz9vyx2UgPO/Zn7K2oAVqm6/emmTCFWqYoyiO3p5sdR//wcT/n6E+";
            std::string sPageCrash2 = "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\");  }} </style> <body style=\"background-color:#E8E8E8;-webkit-user-select:none\"> <div class=\"image_crash\" style=\"position: fixed;top: 50%;left: 50%; margin-top: -200px; margin-left: -85px;\" width=\"170\" height=\"170\" /> <div style=\"position:fixed; top:50%; left: 0; width:100%;text-align: center;font-family: Open Sans Semibold,sans-serif,Arial;font-size:18px;color:#313437;\">Internet connection failed...<div style=\"position:fixed; top:50%; margin-top:46px; margin-left: -330px; left: 50%; width:660px;text-align: center;font-family: Open Sans,sans-serif,Arial;font-size:12px;color:#666666;\">You are unable to edit the document because the Internet connection is lost or restricted. Please check your connection and re-open the document to continue.</div> </div> </body></html>";

            std::string sPageCrash = sPageCrash1 + sPageCrash2;

            mime_type_ = "text/html";
            data_ = sPageCrash;

            callback->Continue();
            return true;
        }

        posFind = url.find("ascdesktop://fonts/");
        if (posFind != std::string::npos)
        {
            std::wstring sFile = read_file_path(request).substr(19);
#ifndef WIN32
            if (!sFile.empty())
            {
                if ('/' != sFile.c_str()[0])
                    sFile = L"/" + sFile;
            }
#endif
            read_binary_file(sFile, false);

            callback->Continue();
            return true;
        }

        posFind = url.find("ascdesktop://plugin_content/");
        if (posFind != std::string::npos)
        {
            std::wstring sFile = read_file_path(request).substr(28);

            std::wstring::size_type posSearch = sFile.find(L"?lang=");
            if (std::wstring::npos != posSearch)
                sFile = sFile.substr(0, posSearch);

            if (0 == sFile.find(L"file:///"))
            {
                sFile = sFile.substr(7);
                if (!NSFile::CFileBinary::Exists(sFile))
                    sFile = sFile.substr(1);
            }                        

            read_binary_file(sFile, true);

            callback->Continue();
            return true;
        }

        return false;
    }

    virtual void GetResponseHeaders(CefRefPtr<CefResponse> response,
                                  int64& response_length,
                                  CefString& redirectUrl) OVERRIDE
    {
        CEF_REQUIRE_IO_THREAD();

        DCHECK(!data_.empty() || !data_binary_);

        response->SetMimeType(mime_type_);
        response->SetStatus(200);

        // Set the resulting response length
        response_length = (NULL == data_binary_) ? data_.length() : data_binary_len_;
    }

    virtual void Cancel() OVERRIDE
    {
        CEF_REQUIRE_IO_THREAD();
    }

    virtual bool ReadResponse(void* data_out,
                            int bytes_to_read,
                            int& bytes_read,
                            CefRefPtr<CefCallback> callback)
                            OVERRIDE
    {
        CEF_REQUIRE_IO_THREAD();

        bool has_data = false;
        bytes_read = 0;

        BYTE* read_data = (BYTE*)data_binary_;
        size_t read_data_len = data_binary_len_;

        if (!read_data)
        {
            read_data = (BYTE*)data_.c_str();
            read_data_len = data_.length();
        }

        if (offset_ < read_data_len)
        {
            // Copy the next block of data into the buffer.
            int transfer_size = std::min(bytes_to_read, static_cast<int>(read_data_len - offset_));
            memcpy(data_out, read_data + offset_, transfer_size);
            offset_ += transfer_size;

            bytes_read = transfer_size;
            return true;
        }

        return false;
    }

private:

    std::wstring read_file_path(CefRefPtr<CefRequest>& request)
    {
        int nFlag = UU_SPACES | UU_REPLACE_PLUS_WITH_SPACE;
#if defined (_LINUX) && !defined(_MAC)
        nFlag |= UU_URL_SPECIAL_CHARS_EXCEPT_PATH_SEPARATORS;
#else
#ifndef CEF_2623
        nFlag |= UU_URL_SPECIAL_CHARS_EXCEPT_PATH_SEPARATORS;
#endif
#endif

        CefString cefUrl = request->GetURL();
        CefString cefFile = CefURIDecode(cefUrl, false, static_cast<cef_uri_unescape_rule_t>(nFlag));

        return cefFile.ToWString();
    }

    void read_binary_file(std::wstring& sFile, const bool& isCheckExt = false)
    {
        data_binary_len_ = (size_t)read_file_with_urls(sFile, data_binary_);

        if (isCheckExt)
            mime_type_ = GetMimeTypeFromExt(sFile);

        if (mime_type_.empty())
            mime_type_ = "*/*";
    }    

private:
    std::string data_;
    std::string mime_type_;
    size_t offset_;

    BYTE* data_binary_;
    size_t data_binary_len_;

    CAscApplicationManager* m_pManager;

    IMPLEMENT_REFCOUNTING(ClientSchemeHandler)
};

// Implementation of the factory for for creating schema handlers.
class ClientSchemeHandlerFactory : public CefSchemeHandlerFactory
{
private:
    CAscApplicationManager* m_pManager;

public:
    ClientSchemeHandlerFactory(CAscApplicationManager* pManager = NULL)
    {
        m_pManager = pManager;
    }

public:
    // Return a new scheme handler instance to handle the request.
    virtual CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser,
                                               CefRefPtr<CefFrame> frame,
                                               const CefString& scheme_name,
                                               CefRefPtr<CefRequest> request)
                                               OVERRIDE
    {
        CEF_REQUIRE_IO_THREAD();
        return new ClientSchemeHandler(m_pManager);
    }

    IMPLEMENT_REFCOUNTING(ClientSchemeHandlerFactory)
};

#ifdef CEF_2623
void RegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar,
                           std::vector<CefString>& cookiable_schemes)
{
    registrar->AddCustomScheme("ascdesktop", true, false, false);
}
#else
void RegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar,
                           std::vector<CefString>& cookiable_schemes)
{
    registrar->AddCustomScheme("ascdesktop", true, false, false, true, true, false);
}
#endif

bool InitScheme(CAscApplicationManager* pManager)
{    
    return CefRegisterSchemeHandlerFactory("ascdesktop", "", new ClientSchemeHandlerFactory(pManager));
}

}  // namespace asc_scheme

void client::ClientApp::RegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar,
                                              std::vector<CefString>& cookiable_schemes)
{
    return asc_scheme::RegisterCustomSchemes(registrar, cookiable_schemes);
}
