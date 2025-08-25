import { useAttachment, AttachmentPrimitive } from "@assistant-ui/react";
import {
  Tooltip,
  TooltipTrigger,
  TooltipContent,
  TooltipProvider,
} from "@radix-ui/react-tooltip";

import AttachmentRemove from "./Remove";
import AttachmentThumb from "./Thumb";
import AttachmentPreviewDialog from "./PreviewDialog";

const AttachmentUI = () => {
  const canRemove = useAttachment((a) => a.source !== "message");
  const typeLabel = useAttachment((a) => {
    const type = a.type;
    switch (type) {
      case "image":
        return "Image";
      case "document":
        return "Document";
      case "file":
        return "File";
      default: {
        const _exhaustiveCheck: never = type;
        throw new Error(`Unknown attachment type: ${_exhaustiveCheck}`);
      }
    }
  });
  return (
    <TooltipProvider>
      <Tooltip>
        <AttachmentPrimitive.Root className="relative mt-3">
          <AttachmentPreviewDialog>
            <TooltipTrigger asChild>
              <div className="flex h-12 w-40 items-center justify-center gap-2 rounded-lg border p-1">
                <AttachmentThumb />
                <div className="flex-grow basis-0">
                  <p className="text-muted-foreground line-clamp-1 text-ellipsis break-all text-xs font-bold">
                    <AttachmentPrimitive.Name />
                  </p>
                  <p className="text-muted-foreground text-xs">{typeLabel}</p>
                </div>
              </div>
            </TooltipTrigger>
          </AttachmentPreviewDialog>
          {canRemove && <AttachmentRemove />}
        </AttachmentPrimitive.Root>
        <TooltipContent side="top">
          <AttachmentPrimitive.Name />
        </TooltipContent>
      </Tooltip>
    </TooltipProvider>
  );
};

export default AttachmentUI;
