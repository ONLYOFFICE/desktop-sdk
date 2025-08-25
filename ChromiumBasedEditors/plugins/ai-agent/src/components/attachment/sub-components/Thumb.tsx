import { useAttachment } from "@assistant-ui/react";
import { FileIcon } from "lucide-react";

import { Avatar, AvatarFallback, AvatarImage } from "../..//avatar";
import useAttachmentSrc from "../hooks/useAttachmentSrc";

const AttachmentThumb = () => {
  const isImage = useAttachment((a) => a.type === "image");
  const src = useAttachmentSrc();
  return (
    <Avatar className="bg-muted flex size-10 items-center justify-center rounded border text-sm">
      <AvatarFallback delayMs={isImage ? 200 : 0}>
        <FileIcon />
      </AvatarFallback>
      <AvatarImage src={src} />
    </Avatar>
  );
};

export default AttachmentThumb;
