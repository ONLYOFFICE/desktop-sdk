import { ThreadPrimitive, ComposerPrimitive } from "@assistant-ui/react";
import { ArrowDownIcon, ArrowUpIcon, Square } from "lucide-react";
import { motion } from "framer-motion";

import { ComposerAttachments, ComposerAddAttachment } from "../../attachment";
import { Button } from "../../button";
import { TooltipIconButton } from "../../tooltip-icon-button";

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

const ThreadWelcomeSuggestions = () => {
  return (
    <div className="grid w-full gap-2 sm:grid-cols-2">
      {[
        {
          title: "What is ONLYOFFICE",
          label: "and its key features?",
          action: "What is ONLYOFFICE and what are its key features?",
        },
        {
          title: "Compare ONLYOFFICE Desktop",
          label: "vs cloud solutions",
          action:
            "Compare ONLYOFFICE Desktop with cloud-based office solutions",
        },
        {
          title: "How does ONLYOFFICE DocSpace",
          label: "improve collaboration?",
          action:
            "How does ONLYOFFICE DocSpace improve team collaboration and document management?",
        },
        {
          title: "Setup guide for",
          label: "ONLYOFFICE integration",
          action:
            "Provide a setup guide for integrating ONLYOFFICE with existing workflows",
        },
      ].map((suggestedAction, index) => (
        <motion.div
          initial={{ opacity: 0, y: 20 }}
          animate={{ opacity: 1, y: 0 }}
          exit={{ opacity: 0, y: 20 }}
          transition={{ delay: 0.05 * index }}
          key={`suggested-action-${suggestedAction.title}-${index}`}
          className="[&:nth-child(n+3)]:hidden sm:[&:nth-child(n+3)]:block"
        >
          <ThreadPrimitive.Suggestion
            prompt={suggestedAction.action}
            method="replace"
            autoSend
            asChild
          >
            <Button
              variant="ghost"
              className="dark:hover:bg-accent/60 h-auto w-full flex-1 flex-wrap items-start justify-start gap-1 rounded-xl border px-4 py-3.5 text-left text-sm sm:flex-col"
              aria-label={suggestedAction.action}
            >
              <span className="font-medium">{suggestedAction.title}</span>
              <p className="text-muted-foreground">{suggestedAction.label}</p>
            </Button>
          </ThreadPrimitive.Suggestion>
        </motion.div>
      ))}
    </div>
  );
};

const ComposerAction = () => {
  return (
    <div className="bg-muted border-border dark:border-muted-foreground/15 relative flex flex-col rounded-b-2xl border-x border-b p-2">
      <ComposerAttachments />
      <div className="bg-muted border-border dark:border-muted-foreground/15 relative flex items-center justify-between rounded-b-2xl">
        <ComposerAddAttachment />
        <ThreadPrimitive.If running={false}>
          <ComposerPrimitive.Send asChild>
            <Button
              type="submit"
              variant="default"
              className="dark:border-muted-foreground/90 border-muted-foreground/60 hover:bg-primary/75 size-8 rounded-full border"
              aria-label="Send message"
            >
              <ArrowUpIcon className="size-5" />
            </Button>
          </ComposerPrimitive.Send>
        </ThreadPrimitive.If>

        <ThreadPrimitive.If running>
          <ComposerPrimitive.Cancel asChild>
            <Button
              type="button"
              variant="default"
              className="dark:border-muted-foreground/90 border-muted-foreground/60 hover:bg-primary/75 size-8 rounded-full border"
              aria-label="Stop generating"
            >
              <Square className="size-3.5 fill-white dark:size-4 dark:fill-black" />
            </Button>
          </ComposerPrimitive.Cancel>
        </ThreadPrimitive.If>
      </div>
    </div>
  );
};

const Composer = () => {
  return (
    <div className="bg-background relative mx-auto flex w-full max-w-[var(--thread-max-width)] flex-col gap-4 px-[var(--thread-padding-x)] pb-4 md:pb-6">
      <ThreadScrollToBottom />
      <ThreadPrimitive.Empty>
        <ThreadWelcomeSuggestions />
      </ThreadPrimitive.Empty>
      <ComposerPrimitive.Root className="relative flex w-full flex-col rounded-2xl focus-within:ring-2 focus-within:ring-black focus-within:ring-offset-2 dark:focus-within:ring-white">
        <ComposerPrimitive.Input
          placeholder="Send a message..."
          className="bg-muted border-border dark:border-muted-foreground/15 focus:outline-primary placeholder:text-muted-foreground max-h-[calc(50dvh)] min-h-16 w-full resize-none rounded-t-2xl border-x border-t px-4 pb-3 pt-2 text-base outline-none"
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
