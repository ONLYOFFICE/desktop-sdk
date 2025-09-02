import { ComposerPrimitive } from "@assistant-ui/react";

import AttachmentIconUrl from "@/assets/attachment.svg?url";

import { IconButton } from "../icon-button";

export const ComposerAddAttachment = () => {
  return (
    <ComposerPrimitive.AddAttachment asChild>
      <IconButton
        iconName={AttachmentIconUrl}
        size={24}
        className="cursor-pointer rounded-[4px] bg-[var(--attachment-trigger-background-color)] hover:bg-[var(--attachment-trigger-background-hover-color)] active:bg-[var(--attachment-trigger-background-active-color)]"
      />
    </ComposerPrimitive.AddAttachment>
  );
};
