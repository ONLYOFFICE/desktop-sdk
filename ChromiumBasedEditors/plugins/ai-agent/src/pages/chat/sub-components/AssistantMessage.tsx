import {
  ActionBarPrimitive,
  ErrorPrimitive,
  useMessage,
  MessagePrimitive,
} from "@assistant-ui/react";
import { motion } from "framer-motion";
import { useTranslation } from "react-i18next";

import DownloadIconUrl from "@/assets/btn-save.svg?url";
import BtnCopyIconUrl from "@/assets/btn-copy.svg?url";
import BtnCheckIconUrl from "@/assets/checked.svg?url";

import useMessageStore from "@/store/useMessageStore";

import { convertMessagesToMd, getMessageTitleFromMd } from "@/lib/utils";

import { MarkdownText } from "@/components/markdown";
import { TooltipIconButton } from "@/components/tooltip-icon-button";
import { ToolFallback } from "@/components/tool-fallback";
import { IconButton } from "@/components/icon-button";

const MessageError = () => {
  return (
    <MessagePrimitive.Error>
      <ErrorPrimitive.Root className="border border-[var(--chat-message-error-border-color)] rounded-[4px] p-[6px]">
        <ErrorPrimitive.Message className="text-[var(--chat-message-error-color)] text-[14px] leading-[20px] font-normal" />
      </ErrorPrimitive.Root>
    </MessagePrimitive.Error>
  );
};

const AssistantActionBar = () => {
  const { t } = useTranslation();

  const { isStreamRunning, messages } = useMessageStore();

  const message = useMessage();

  if (message.status?.type === "incomplete" && message.status?.error) return;

  const onDownload = () => {
    const parentMessage = messages[Number(message.parentId)];

    const mdValue = convertMessagesToMd([parentMessage, message]);

    const title = getMessageTitleFromMd(mdValue);

    window.AscDesktopEditor.SaveFilenameDialog(`${title}.docx`, (path) => {
      if (!path) return;

      window.AscDesktopEditor.saveAndOpen(mdValue, 0x5c, path, 0x41, (code) => {
        if (!code) console.log("Conversion error");
      });
    });
  };

  return (
    <ActionBarPrimitive.Root
      hidden={isStreamRunning}
      // autohide="not-last"
      // autohideFloat="single-branch"
      className="col-start-3 row-start-2 ml-3 mt-3 flex gap-[8px]"
    >
      <ActionBarPrimitive.Copy asChild>
        <TooltipIconButton tooltip="Copy">
          <MessagePrimitive.If copied>
            <IconButton
              iconName={BtnCheckIconUrl}
              size={24}
              isStroke
              disabled
            />
          </MessagePrimitive.If>
          <MessagePrimitive.If copied={false}>
            <IconButton iconName={BtnCopyIconUrl} size={24} />
          </MessagePrimitive.If>
        </TooltipIconButton>
      </ActionBarPrimitive.Copy>
      <div>
        <TooltipIconButton tooltip={t("Save")}>
          <IconButton
            iconName={DownloadIconUrl}
            size={24}
            onClick={onDownload}
            isStroke
          />
        </TooltipIconButton>
      </div>
      {/* <ActionBarPrimitive.Reload asChild>
        <TooltipIconButton tooltip="Refresh">
          <RefreshCwIcon />
        </TooltipIconButton>
      </ActionBarPrimitive.Reload> */}
    </ActionBarPrimitive.Root>
  );
};

export const AssistantMessage = () => {
  return (
    <MessagePrimitive.Root asChild>
      <motion.div
        className="relative mx-auto grid w-full max-w-[var(--thread-max-width)] grid-cols-[auto_auto_1fr] grid-rows-[auto_1fr] px-[var(--thread-padding-x)] py-4"
        initial={{ y: 5, opacity: 0 }}
        animate={{ y: 0, opacity: 1 }}
        data-role="assistant"
      >
        <div className="leading-[20px] text-[14px] col-span-2 col-start-2 row-start-1 ml-4 break-words leading-7 text-[var(--chat-message-color)]">
          <MessagePrimitive.Content
            components={{
              Text: MarkdownText,
              tools: { Fallback: ToolFallback },
            }}
          />
          <MessageError />
        </div>

        <AssistantActionBar />
      </motion.div>
    </MessagePrimitive.Root>
  );
};
