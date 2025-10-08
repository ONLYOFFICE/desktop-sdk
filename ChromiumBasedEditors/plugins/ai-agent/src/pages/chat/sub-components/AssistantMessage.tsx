import {
  MessagePrimitive,
  ActionBarPrimitive,
  ErrorPrimitive,
  useMessage,
  type ThreadMessageLike,
} from "@assistant-ui/react";
import { motion } from "framer-motion";

import DownloadIconUrl from "@/assets/btn-download.svg?url";
import BtnCopyIconUrl from "@/assets/btn-copy.svg?url";
import BtnCheckIconUrl from "@/assets/checked.svg?url";

import useMessageStore from "@/store/useMessageStore";

import { convertMessagesToMd } from "@/lib/utils";

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
  const { isStreamRunning } = useMessageStore();

  const message = useMessage();

  const msg: ThreadMessageLike = message;

  const mdValue = convertMessagesToMd([msg]);

  const onDownload = () => {
    window.AscDesktopEditor.SaveFilenameDialog(
      "Message.md",
      (path) => {
        if (path) {
          const name = path.split("/").pop() ?? "";
          window.AscDesktopEditor.openTemplate(path, name);
        }
      },
      mdValue
    );
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
        <TooltipIconButton tooltip="Download">
          <IconButton
            iconName={DownloadIconUrl}
            size={20}
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
        <div className="leading-[20px] text-[14px] col-span-2 col-start-2 row-start-1 ml-4 break-words leading-7">
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
