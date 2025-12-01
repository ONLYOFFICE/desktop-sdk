import React from "react";
import { useTranslation } from "react-i18next";

import OpenChatListUrl from "@/assets/btn-list-search.svg?url";
import ArrowLeftUrl from "@/assets/btn-previtem.svg?url";
import SearchUrl from "@/assets/search.svg?url";

import useThreadsStore from "@/store/useThreadsStore";
import useRouter from "@/store/useRouter";

import { IconButton } from "@/components/icon-button";
import { Input } from "@/components/input";
import { TooltipIconButton } from "@/components/tooltip-icon-button";

import { ChatListItem } from "./ChatListItem";

const ChatList = () => {
  const {
    threads,
    threadId,
    onSwitchToThread,
    onRenameThread,
    onDownloadThread,
  } = useThreadsStore();
  const { setCurrentPage } = useRouter();

  const [isOpen, setIsOpen] = React.useState(false);
  const [searchValue, setSearchValue] = React.useState("");
  const [showingThreads, setShowingThreads] = React.useState(threads);

  const { t } = useTranslation();

  const onChangeSearchValue = (e: React.ChangeEvent<HTMLInputElement>) => {
    const value = e.target.value;
    setSearchValue(value);
  };

  React.useEffect(() => {
    const filteredThreads = threads.filter((thread) => {
      return thread.title?.toLowerCase().includes(searchValue.toLowerCase());
    });
    setShowingThreads(filteredThreads);
  }, [threads, searchValue]);

  const isEmptyList = threads.length === 0;

  return isOpen ? (
    <div
      className="w-[264px] min-w-[264px] h-full max-h-full border-r-[1px] border-[var(--chat-list-border-right)] flex flex-col overflow-y-auto"
      style={{ maxHeight: "100%" }}
    >
      <div className="sticky top-0 bg-[var(--layout-background-color)] z-10 pt-[24px] px-[24px]">
        <div className="flex items-center justify-between px-[8px]">
          <h4 className="text-[14px] leading-[20px] font-bold text-[var(--chat-list-color)]">
            {t("ChatHistory")}
          </h4>
          <IconButton
            iconName={ArrowLeftUrl}
            size={24}
            isStroke
            onClick={() => setIsOpen(false)}
          />
        </div>
        {!isEmptyList && (
          <div className="mt-[32px]">
            <Input
              className="w-full"
              type="search"
              placeholder={t("Search")}
              icon={SearchUrl}
              value={searchValue}
              onChange={onChangeSearchValue}
              onClear={() => setSearchValue("")}
            />
          </div>
        )}
      </div>
      <div className="flex flex-col px-[24px] pb-[24px]">
        {isEmptyList ? (
          <p className="text-[var(--chat-list-empty-color)] font-normal text-[14px] leading-[20px] w-full text-center mt-[24px]">
            {t("NoChatYet")}
          </p>
        ) : (
          <div className="flex flex-col mt-[16px]">
            {showingThreads.length ? (
              showingThreads.map((thread) => {
                const isActive = thread.threadId === threadId;
                return (
                  <ChatListItem
                    key={thread.threadId}
                    thread={thread}
                    isActive={isActive}
                    onSwitchToThread={onSwitchToThread}
                    setCurrentPage={setCurrentPage}
                    onRenameThread={onRenameThread}
                    onDownloadThread={onDownloadThread}
                  />
                );
              })
            ) : (
              <p className="text-[var(--chat-list-empty-color)] font-normal text-[14px] leading-[20px] w-full">
                {t("NoChatYet")}
              </p>
            )}
          </div>
        )}
      </div>
    </div>
  ) : (
    <div className="ml-[32px] mt-[24px]">
      <TooltipIconButton tooltip={t("ChatHistory")}>
        <IconButton
          iconName={OpenChatListUrl}
          size={24}
          // isStroke
          onClick={() => {
            setCurrentPage("chat");
            setIsOpen(true);
          }}
        />
      </TooltipIconButton>
    </div>
  );
};

export { ChatList };
