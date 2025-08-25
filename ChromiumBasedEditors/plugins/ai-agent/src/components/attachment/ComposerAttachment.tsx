import { ComposerPrimitive } from "@assistant-ui/react";

import AttachmentUI from "./sub-components/UI";

export const ComposerAttachments = () => {
  return (
    <div className="flex w-full flex-row gap-3 overflow-x-auto">
      <ComposerPrimitive.Attachments
        components={{ Attachment: AttachmentUI }}
      />
    </div>
  );
};
