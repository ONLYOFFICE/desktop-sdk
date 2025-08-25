import { PaperclipIcon } from "lucide-react";
import { ComposerPrimitive } from "@assistant-ui/react";

import { TooltipIconButton } from "../tooltip-icon-button";

export const ComposerAddAttachment = () => {
  return (
    <ComposerPrimitive.AddAttachment asChild>
      <TooltipIconButton
        className="my-2.5 size-8 p-2 transition-opacity ease-in"
        tooltip="Add Attachment"
        variant="ghost"
      >
        <PaperclipIcon />
      </TooltipIconButton>
    </ComposerPrimitive.AddAttachment>
  );
};
