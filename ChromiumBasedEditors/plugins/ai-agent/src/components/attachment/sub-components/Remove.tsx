import { AttachmentPrimitive } from "@assistant-ui/react";
import { CircleXIcon } from "lucide-react";

import { TooltipIconButton } from "../../tooltip-icon-button";

const AttachmentRemove = () => {
  return (
    <AttachmentPrimitive.Remove asChild>
      <TooltipIconButton
        tooltip="Remove file"
        className="text-muted-foreground [&>svg]:bg-background absolute -right-3 -top-3 size-6 [&>svg]:size-4 [&>svg]:rounded-full"
        side="top"
      >
        <CircleXIcon />
      </TooltipIconButton>
    </AttachmentPrimitive.Remove>
  );
};

export default AttachmentRemove;
