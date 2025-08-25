import { MessagePrimitive, ActionBarPrimitive } from "@assistant-ui/react";
import { PencilIcon } from "lucide-react";

import { motion } from "framer-motion";

import { MarkdownText } from "../../markdown";
import { UserMessageAttachments } from "../../attachment";
import { TooltipIconButton } from "../../tooltip-icon-button";

import { BranchPicker } from "./BranchPicker";

const UserActionBar = () => {
  return (
    <ActionBarPrimitive.Root
      hideWhenRunning
      autohide="not-last"
      className="col-start-1 mr-3 mt-2.5 flex flex-col items-end"
    >
      <ActionBarPrimitive.Edit asChild>
        <TooltipIconButton tooltip="Edit">
          <PencilIcon />
        </TooltipIconButton>
      </ActionBarPrimitive.Edit>
    </ActionBarPrimitive.Root>
  );
};

export const UserMessage = () => {
  return (
    <MessagePrimitive.Root asChild>
      <motion.div
        className="mx-auto grid w-full max-w-[var(--thread-max-width)] auto-rows-auto grid-cols-[minmax(72px,1fr)_auto] gap-y-1 px-[var(--thread-padding-x)] py-4 [&:where(>*)]:col-start-2"
        initial={{ y: 5, opacity: 0 }}
        animate={{ y: 0, opacity: 1 }}
        data-role="user"
      >
        <UserMessageAttachments />

        <UserActionBar />

        <div className="bg-muted text-foreground col-start-2 break-words rounded-3xl px-5 py-2.5">
          <MessagePrimitive.Content components={{ Text: MarkdownText }} />
        </div>

        <BranchPicker className="col-span-full col-start-1 row-start-3 -mr-1 justify-end" />
      </motion.div>
    </MessagePrimitive.Root>
  );
};
