import {
  MessagePrimitive,
  ActionBarPrimitive,
  ErrorPrimitive,
} from "@assistant-ui/react";
import { CopyIcon, CheckIcon } from "lucide-react";
import { motion } from "framer-motion";

import { MarkdownText } from "../../markdown";
import { TooltipIconButton } from "../../tooltip-icon-button";
import { ToolFallback } from "../../tool-fallback";

import { BranchPicker } from "./BranchPicker";

const MessageError = () => {
  return (
    <MessagePrimitive.Error>
      <ErrorPrimitive.Root className="border-destructive bg-destructive/10 dark:bg-destructive/5 text-destructive mt-2 rounded-md border p-3 text-sm dark:text-red-200">
        <ErrorPrimitive.Message className="line-clamp-2" />
      </ErrorPrimitive.Root>
    </MessagePrimitive.Error>
  );
};

const AssistantActionBar = () => {
  return (
    <ActionBarPrimitive.Root
      hideWhenRunning
      autohide="not-last"
      autohideFloat="single-branch"
      className="text-muted-foreground data-floating:bg-background data-floating:absolute data-floating:mt-2 data-floating:rounded-md data-floating:border data-floating:p-1 data-floating:shadow-sm col-start-3 row-start-2 ml-3 mt-3 flex gap-1"
    >
      <ActionBarPrimitive.Copy asChild>
        <TooltipIconButton tooltip="Copy">
          <MessagePrimitive.If copied>
            <CheckIcon />
          </MessagePrimitive.If>
          <MessagePrimitive.If copied={false}>
            <CopyIcon />
          </MessagePrimitive.If>
        </TooltipIconButton>
      </ActionBarPrimitive.Copy>
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
        <div className="text-foreground col-span-2 col-start-2 row-start-1 ml-4 break-words leading-7">
          <MessagePrimitive.Content
            components={{
              Text: MarkdownText,
              tools: { Fallback: ToolFallback },
            }}
          />
          <MessageError />
        </div>

        <AssistantActionBar />

        <BranchPicker className="col-start-2 row-start-2 -ml-2 mr-2" />
      </motion.div>
    </MessagePrimitive.Root>
  );
};
