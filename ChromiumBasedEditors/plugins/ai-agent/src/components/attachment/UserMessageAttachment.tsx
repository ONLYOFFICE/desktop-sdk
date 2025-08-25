import { MessagePrimitive } from "@assistant-ui/react";

import AttachmentUI from "./sub-components/UI";

export const UserMessageAttachments = () => {
  return (
    <div className="flex w-full flex-row gap-3 col-span-full col-start-1 row-start-1 justify-end">
      <MessagePrimitive.Attachments components={{ Attachment: AttachmentUI }} />
    </div>
  );
};
