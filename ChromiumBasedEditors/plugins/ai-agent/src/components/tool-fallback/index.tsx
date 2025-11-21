import { useEffect, useState, useCallback } from "react";
import type { ToolCallMessagePartComponent } from "@assistant-ui/react";
import { ReactSVG } from "react-svg";
import { useTranslation } from "react-i18next";

import ToolCalledIconUrl from "@/assets/tool.called.svg?url";
import CodeIconUrl from "@/assets/code.svg?url";
import ArrowBottomIconUrl from "@/assets/arrow.bottom.svg?url";
import ArrowRightIconUrl from "@/assets/arrow.right.svg?url";
import CopyIconUrl from "@/assets/btn-copy.svg?url";
import CheckedIconUrl from "@/assets/checked.svg?url";
import SearchIconUrl from "@/assets/btn-web-search.svg?url";
import ExternalIconUrl from "@/assets/btn-external.svg?url";

import server from "@/servers";

import { Loader } from "../loader";
import { IconButton } from "../icon-button";

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

  const type = server.getServerType(toolName);
  const name = toolName.replace(type + "_", "");

  const isLoading = result === undefined;

  console.log(result);

  const isWebSearch = name === "web_search";
  const isWebCrawling = name === "web_crawling";

  let webSearchName = "";

  let argsTextFinal = argsText;

  try {
    const parsedArgs = JSON.parse(argsTextFinal);

    if (parsedArgs.args) {
      argsTextFinal = JSON.stringify(parsedArgs.args);
    }

    webSearchName = argsTextFinal
      ? isWebSearch
        ? JSON.parse(argsTextFinal).query
        : isWebCrawling
        ? JSON.parse(argsTextFinal).urls[0]
        : ""
      : "";
  } catch {
    //ignore
  }

  const handleBeforeInjectionFill = useCallback((svg: SVGSVGElement) => {
    const paths = svg.querySelectorAll("path");
    paths.forEach((path) => {
      path.setAttribute("fill", "var(--chat-message-tool-call-name-color)");
    });
    const circles = svg.querySelectorAll("circle");
    circles.forEach((circle) => {
      circle.setAttribute("fill", "var(--chat-message-tool-call-name-color)");
    });
  }, []);

  const handleBeforeInjection = useCallback((svg: SVGSVGElement) => {
    const paths = svg.querySelectorAll("path");
    paths.forEach((path) => {
      path.setAttribute("stroke", "var(--chat-message-tool-call-name-color)");
    });
    const circles = svg.querySelectorAll("circle");
    circles.forEach((circle) => {
      circle.setAttribute("stroke", "var(--chat-message-tool-call-name-color)");
    });
  }, []);

  return (
    <div className="my-[16px] flex w-full flex-col gap-3">
      <div
        className="flex items-center gap-[10px] cursor-pointer"
        onClick={() => {
          if (isWebCrawling) {
            window.open(webSearchName, "_blank");
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
        {isLoading && !isWebSearch && !isWebCrawling ? (
          <p className="text-[var(--chat-message-tool-call-header-color)] text-[14px] font-normal leading-[16px]">
            {t("ToolExecuted")}
          </p>
        ) : null}
        <span className="flex items-center gap-[8px] rounded-[4px] ps-[4px] pe-[8px] text-[14px] leading-[20px] font-normal text-[var(--chat-message-tool-call-name-color)] bg-[var(--chat-message-tool-call-name-background-color)] min-w-0 w-fit">
          {isWebSearch ? (
            <ReactSVG
              src={SearchIconUrl}
              beforeInjection={handleBeforeInjectionFill}
            />
          ) : !isWebCrawling ? (
            <ReactSVG
              src={CodeIconUrl}
              beforeInjection={handleBeforeInjection}
            />
          ) : null}
          <span className="truncate">
            {isWebSearch
              ? webSearchName
              : isWebCrawling
              ? `${name} | ${webSearchName}`
              : name}
          </span>
        </span>
        {isWebCrawling ? (
          <ReactSVG
            src={ExternalIconUrl}
            beforeInjection={handleBeforeInjection}
          />
        ) : isWebSearch && result === undefined ? null : (
          <ReactSVG
            src={!isCollapsed ? ArrowBottomIconUrl : ArrowRightIconUrl}
            beforeInjection={handleBeforeInjection}
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
                  beforeInjection={
                    !isArgsCopied
                      ? handleBeforeInjectionFill
                      : handleBeforeInjection
                  }
                />
              </p>
              <pre className="max-h-[200px] overflow-y-auto whitespace-pre-wrap text-[var(--chat-message-tool-call-pre-color)] border border-[var(--chat-message-tool-call-pre-border-color)] bg-[var(--chat-message-tool-call-pre-background-color)] px-[8px] py-[2px] rounded-[4px]">
                {argsTextFinal ? argsTextFinal : "{}"}
              </pre>
            </div>
          )}
          {result !== undefined && (
            <div className="">
              {isWebSearch ? (
                <div>
                  {(() => {
                    try {
                      const parsedResult =
                        typeof result === "string"
                          ? JSON.parse(result)
                          : result;

                      // Check if there's an error in the result
                      if (parsedResult?.error) {
                        return (
                          <pre className="max-h-[200px] overflow-y-auto whitespace-pre-wrap text-[var(--chat-message-tool-call-pre-color)] border border-[var(--chat-message-tool-call-pre-border-color)] bg-[var(--chat-message-tool-call-pre-background-color)] px-[8px] py-[2px] rounded-[4px]">
                            {typeof result === "string"
                              ? result
                              : JSON.stringify(result, null, 2)}
                          </pre>
                        );
                      }

                      const searchResults = parsedResult?.data || [];

                      return searchResults.length > 0 ? (
                        <div className="flex flex-col gap-[10px]">
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
                                className="group h-[36px] px-[8px] rounded-[4px] flex flex-row items-center justify-between cursor-pointer hover:bg-[var(--drop-down-menu-item-hover-color)] transition-colors"
                                onClick={() => window.open(item.url, "_blank")}
                              >
                                <div className="flex flex-row items-center gap-[8px] min-w-0 flex-1">
                                  <IconButton
                                    iconName={SearchIconUrl}
                                    size={24}
                                    disableHover
                                  />
                                  <h4 className="text-[14px] font-normal text-[var(--chat-message-tool-call-pre-color)] truncate">
                                    {item.title}
                                  </h4>
                                </div>
                                <div className="opacity-0 group-hover:opacity-100 transition-opacity">
                                  <IconButton
                                    iconName={ExternalIconUrl}
                                    size={24}
                                    disableHover
                                  />
                                </div>
                              </div>
                            )
                          )}
                        </div>
                      ) : (
                        <pre className="max-h-[200px] overflow-y-auto whitespace-pre-wrap text-[var(--chat-message-tool-call-pre-color)] border border-[var(--chat-message-tool-call-pre-border-color)] bg-[var(--chat-message-tool-call-pre-background-color)] px-[8px] py-[2px] rounded-[4px]">
                          {typeof result === "string"
                            ? result
                            : JSON.stringify(result, null, 2)}
                        </pre>
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
                      beforeInjection={
                        !isResultCopied
                          ? handleBeforeInjectionFill
                          : handleBeforeInjection
                      }
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
