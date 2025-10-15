import { useEffect, useState } from "react";
import type { ToolCallMessagePartComponent } from "@assistant-ui/react";
import { ReactSVG } from "react-svg";
import { useTranslation } from "react-i18next";

import ToolCalledIconUrl from "@/assets/tool.called.svg?url";
import CodeIconUrl from "@/assets/code.svg?url";
import ArrowBottomIconUrl from "@/assets/arrow.bottom.svg?url";
import ArrowRightIconUrl from "@/assets/arrow.right.svg?url";
import CopyIconUrl from "@/assets/btn-copy.svg?url";
import CheckedIconUrl from "@/assets/checked.svg?url";

import { Loader } from "../loader";

export const ToolFallback: ToolCallMessagePartComponent = ({
  toolName,
  argsText,
  result,
}) => {
  const { t } = useTranslation();
  const [isCollapsed, setIsCollapsed] = useState(true);
  const [isArgsCopied, setIsArgsCopied] = useState(false);
  const [isResultCopied, setIsResultCopied] = useState(false);

  useEffect(() => {
    if (isArgsCopied) {
      setTimeout(() => {
        setIsArgsCopied(false);
      }, 2000);
    }
  }, [isArgsCopied]);

  useEffect(() => {
    if (isResultCopied) {
      setTimeout(() => {
        setIsResultCopied(false);
      }, 2000);
    }
  }, [isResultCopied]);

  const type = toolName.split("_")[0];
  const name = toolName.replace(type + "_", "");

  const isLoading = result === undefined;

  const isWebSearch = name === "web_search";
  const isWebCrawling = name === "web_crawling";

  const webSearchName = isWebSearch
    ? JSON.parse(argsText).query
    : isWebCrawling
    ? JSON.parse(argsText).urls[0]
    : "";

  return (
    <div className="my-[16px] flex w-full flex-col gap-3">
      <div
        className="flex items-center gap-[10px] cursor-pointer"
        onClick={() => {
          if (isWebCrawling) {
            return;
          }

          if (isWebSearch && result === undefined) {
            return;
          }

          setIsCollapsed(!isCollapsed);
        }}
      >
        {!isLoading ? (
          <ReactSVG src={ToolCalledIconUrl} />
        ) : (
          <Loader size={16} />
        )}
        {isLoading ? (
          <p className="text-[var(--chat-message-tool-call-header-color)] text-[14px] font-normal leading-[16px]">
            {t("ToolExecuted")}
          </p>
        ) : null}
        <span className="flex items-center gap-[8px] rounded-[4px] ps-[4px] pe-[8px] text-[14px] leading-[20px] font-normal text-[var(--chat-message-tool-call-name-color)] bg-[var(--chat-message-tool-call-name-background-color)] min-w-0 flex-grow">
          {!isWebSearch && !isWebCrawling ? (
            <ReactSVG src={CodeIconUrl} />
          ) : null}
          <span className="truncate">
            {name}
            {isWebSearch || isWebCrawling ? ` | ${webSearchName}` : null}
          </span>
        </span>
        {isWebCrawling || (isWebSearch && result === undefined) ? null : (
          <ReactSVG
            src={!isCollapsed ? ArrowBottomIconUrl : ArrowRightIconUrl}
          />
        )}
      </div>
      {!isCollapsed ? (
        <div className="flex flex-col gap-[24px] mt-[8px] p-[12px] bg-[var(--chat-message-tool-call-body-background-color)] rounded-[12px]">
          {isWebSearch ? null : (
            <div className="">
              <p className="flex flex-row items-center justify-between text-[var(--chat-message-tool-call-header-color)] text-[14px] font-bold leading-[20px]">
                {t("ToolCallArguments")}
                <ReactSVG
                  src={isArgsCopied ? CheckedIconUrl : CopyIconUrl}
                  onClick={() => setIsArgsCopied(true)}
                />
              </p>
              <pre className="max-h-[200px] overflow-y-auto whitespace-pre-wrap text-[var(--chat-message-tool-call-pre-color)] border border-[var(--chat-message-tool-call-pre-border-color)] bg-[var(--chat-message-tool-call-pre-background-color)] px-[8px] py-[2px] rounded-[4px]">
                {argsText ? argsText : "{}"}
              </pre>
            </div>
          )}
          {result !== undefined && (
            <div className="">
              {isWebSearch ? (
                <div className="max-h-[400px] overflow-y-auto">
                  {(() => {
                    try {
                      const parsedResult =
                        typeof result === "string"
                          ? JSON.parse(result)
                          : result;
                      const searchResults = parsedResult?.data || [];

                      return searchResults.length > 0 ? (
                        <div className="space-y-2">
                          {searchResults.map(
                            (
                              item: {
                                id: string;
                                title: string;
                                url: string;
                                publishedDate?: string;
                                author?: string;
                              },
                              index: number
                            ) => (
                              <div
                                key={index}
                                className="p-3 border border-[var(--chat-message-tool-call-pre-border-color)] bg-[var(--chat-message-tool-call-pre-background-color)] rounded-[4px] cursor-pointer hover:bg-[var(--drop-down-menu-item-hover-color)] transition-colors"
                                onClick={() => window.open(item.url, "_blank")}
                              >
                                <h4 className="text-[14px] font-medium text-[var(--chat-message-tool-call-pre-color)] mb-1 line-clamp-2">
                                  {item.title}
                                </h4>
                                <p className="text-[12px] text-[var(--chat-message-tool-call-header-color)] opacity-70 truncate">
                                  {item.url}
                                </p>
                                {item.publishedDate && (
                                  <p className="text-[11px] text-[var(--chat-message-tool-call-header-color)] opacity-50 mt-1">
                                    {new Date(
                                      item.publishedDate
                                    ).toLocaleDateString()}
                                  </p>
                                )}
                              </div>
                            )
                          )}
                        </div>
                      ) : (
                        <p className="text-[var(--chat-message-tool-call-pre-color)] text-[14px] p-3">
                          No search results found
                        </p>
                      );
                    } catch {
                      return (
                        <pre className="max-h-[200px] overflow-y-auto whitespace-pre-wrap text-[var(--chat-message-tool-call-pre-color)] border border-[var(--chat-message-tool-call-pre-border-color)] bg-[var(--chat-message-tool-call-pre-background-color)] px-[8px] py-[2px] rounded-[4px]">
                          {typeof result === "string"
                            ? result
                            : JSON.stringify(result, null, 2)}
                        </pre>
                      );
                    }
                  })()}
                </div>
              ) : (
                <>
                  <p className="flex flex-row items-center justify-between text-[var(--chat-message-tool-call-header-color)] text-[14px] font-bold leading-[20px]">
                    {t("ToolCallResult")}
                    <ReactSVG
                      src={isResultCopied ? CheckedIconUrl : CopyIconUrl}
                      onClick={() => setIsResultCopied(true)}
                    />
                  </p>
                  <pre className="max-h-[200px] overflow-y-auto whitespace-pre-wrap text-[var(--chat-message-tool-call-pre-color)] border border-[var(--chat-message-tool-call-pre-border-color)] bg-[var(--chat-message-tool-call-pre-background-color)] px-[8px] py-[2px] rounded-[4px]">
                    {typeof result === "string"
                      ? result
                      : JSON.stringify(result, null, 2)}
                  </pre>
                </>
              )}
            </div>
          )}
        </div>
      ) : null}
    </div>
  );
};
