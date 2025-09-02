import { ThreadPrimitive, ComposerPrimitive } from "@assistant-ui/react";
import { ArrowDownIcon } from "lucide-react";

import { TooltipIconButton } from "../../tooltip-icon-button";

import { ComposerAction } from "./ComposerAction";

const ThreadScrollToBottom = () => {
  return (
    <ThreadPrimitive.ScrollToBottom asChild>
      <TooltipIconButton
        tooltip="Scroll to bottom"
        variant="outline"
        className="dark:bg-background dark:hover:bg-accent absolute -top-12 z-10 self-center rounded-full p-4 disabled:invisible"
      >
        <ArrowDownIcon />
      </TooltipIconButton>
    </ThreadPrimitive.ScrollToBottom>
  );
};

const Composer = () => {
  return (
    <div className="relative mx-auto flex w-full max-w-[var(--thread-max-width)] flex-col gap-4 px-[var(--thread-padding-x)] pb-4 md:pb-6">
      <ThreadScrollToBottom />

      <ComposerPrimitive.Root className="composer-root relative flex w-full flex-col gap-[16px] rounded-[16px] border px-[24px] py-[16px] box-border">
        <ComposerPrimitive.Input
          placeholder="Ask AI"
          className="composer-input max-h-[calc(50dvh)] min-h-[16px] w-full resize-none outline-none"
          rows={1}
          autoFocus
          aria-label="Message input"
        />
        <ComposerAction />
      </ComposerPrimitive.Root>
    </div>
  );
};

export { Composer };
