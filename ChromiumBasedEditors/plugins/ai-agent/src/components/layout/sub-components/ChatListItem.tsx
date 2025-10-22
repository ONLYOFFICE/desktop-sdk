import React from "react";
import { useTranslation } from "react-i18next";
import { ReactSVG } from "react-svg";

import MoreIconSvgUrl from "@/assets/more.svg?url";
import RemoveIconSvgUrl from "@/assets/btn-remove.svg?url";
import RenameIconSvgUrl from "@/assets/btn-rename.svg?url";
import DownloadIconSvgUrl from "@/assets/btn-download.svg?url";

import type { Page } from "@/store/useRouter";

import type { Thread } from "@/lib/types";

import { DropdownMenu } from "@/components/dropdown";
import { IconButton } from "@/components/icon-button";
import { Input } from "@/components/input";

import { DeleteChatDialog } from "./DeleteChatDialog";

type ChatListItemProps = {
  thread: Thread;
  isActive: boolean;
  onSwitchToThread: (threadId: string) => void;
  onRenameThread: (threadId: string, newTitle: string) => void;
  onDownloadThread: (threadId: string) => void;
  setCurrentPage: (page: Page) => void;
};

const handleDownloadIconInjection = (svg: SVGSVGElement) => {
  const path = svg.querySelector("path");
  if (path) {
    path.setAttribute("stroke", "var(--icon-button-color)");
  }
};

const handleRenameIconInjection = (svg: SVGSVGElement) => {
  const path = svg.querySelector("path");
  if (path) {
    path.setAttribute("fill", "var(--icon-button-color)");
  }
};

const handleRemoveIconInjection = (svg: SVGSVGElement) => {
  const path = svg.querySelector("path");
  if (path) {
    path.setAttribute("fill", "var(--icon-button-color)");
  }
};

const DownloadIcon = () => (
  <ReactSVG
    src={DownloadIconSvgUrl}
    beforeInjection={handleDownloadIconInjection}
  />
);

const RenameIcon = () => (
  <ReactSVG
    src={RenameIconSvgUrl}
    beforeInjection={handleRenameIconInjection}
  />
);

const RemoveIcon = () => (
  <ReactSVG
    src={RemoveIconSvgUrl}
    beforeInjection={handleRemoveIconInjection}
  />
);

const ChatListItem = ({
  thread,
  isActive,
  onSwitchToThread,
  onRenameThread,
  onDownloadThread,
  setCurrentPage,
}: ChatListItemProps) => {
  const [isOpen, setIsOpen] = React.useState(false);
  const [isHovered, setIsHovered] = React.useState(false);
  const [isRenameInputVisible, setIsRenameInputVisible] = React.useState(false);
  const [isDeleteDialogVisible, setIsDeleteDialogVisible] =
    React.useState(false);

  const [newTitle, setNewTitle] = React.useState(thread.title);

  const containerRef = React.useRef<HTMLDivElement>(null);
  const inputRef = React.useRef<HTMLInputElement>(null);

  const { t } = useTranslation();

  // Handle keyboard events for rename input
  React.useEffect(() => {
    const handleKeyDown = (event: KeyboardEvent) => {
      if (!isRenameInputVisible) return;

      if (event.key === "Escape") {
        event.preventDefault();
        setIsRenameInputVisible(false);
        setNewTitle(thread.title); // Reset to original title
      } else if (event.key === "Enter") {
        event.preventDefault();
        inputRef.current?.blur(); // This will trigger the onBlur event
      }
    };

    if (isRenameInputVisible) {
      document.addEventListener("keydown", handleKeyDown);
    }

    return () => {
      document.removeEventListener("keydown", handleKeyDown);
    };
  }, [isRenameInputVisible, thread.title]);

  const onSwitchToThreadAction = () => {
    if (isOpen || isRenameInputVisible || isDeleteDialogVisible) return;

    onSwitchToThread(thread.threadId);
    setCurrentPage("chat");
  };

  const onDeleteClick = () => {
    setIsDeleteDialogVisible(true);
    setIsOpen(false);
  };

  const onRenameClick = () => {
    setIsRenameInputVisible(true);
    setIsOpen(false);
  };

  const onDownloadClick = () => {
    onDownloadThread(thread.threadId);
    setIsOpen(false);
  };

  return (
    <div
      ref={containerRef}
      key={thread.threadId}
      className={`cursor-pointer rounded-[4px] h-[36px] min-h-[36px] flex items-center justify-between ${
        isRenameInputVisible
          ? ""
          : `px-[12px] ${
              isActive || isOpen
                ? "bg-[var(--chat-list-item-active-background-color)]"
                : "hover:bg-[var(--chat-list-item-hover-background-color)] active:bg-[var(--chat-list-item-active-background-color)]"
            }`
      }`}
      onMouseEnter={() => setIsHovered(true)}
      onMouseLeave={() => {
        if (!isOpen) {
          setIsHovered(false);
        }
      }}
      onClick={onSwitchToThreadAction}
    >
      {isRenameInputVisible ? (
        <Input
          ref={inputRef}
          value={newTitle}
          onChange={(e) => setNewTitle(e.target.value)}
          onBlur={() => {
            setIsRenameInputVisible(false);

            if (newTitle) {
              onRenameThread(thread.threadId, newTitle);
            }
          }}
          autoFocus
          className="w-full"
        />
      ) : (
        <>
          <p className="text-[var(--chat-list-item-color)] font-normal text-[14px] leading-[20px] truncate">
            {thread.title}
          </p>
          {isHovered && !isRenameInputVisible ? (
            <DropdownMenu
              open={isOpen}
              onOpenChange={setIsOpen}
              trigger={
                <IconButton
                  iconName={MoreIconSvgUrl}
                  size={20}
                  isActive={isOpen}
                  insideElement
                />
              }
              items={[
                {
                  icon: <DownloadIcon />,
                  text: t("Download"),
                  onClick: onDownloadClick,
                },
                {
                  icon: <RenameIcon />,
                  text: t("Rename"),
                  onClick: onRenameClick,
                },
                {
                  icon: <RemoveIcon />,
                  text: t("Delete"),
                  onClick: onDeleteClick,
                },
              ]}
              side="right"
              align="start"
              sideOffset={0}
              containerRef={containerRef.current}
            />
          ) : null}
        </>
      )}

      {isDeleteDialogVisible ? (
        <DeleteChatDialog
          id={thread.threadId}
          onClose={() => setIsDeleteDialogVisible(false)}
        />
      ) : null}
    </div>
  );
};

export { ChatListItem };
