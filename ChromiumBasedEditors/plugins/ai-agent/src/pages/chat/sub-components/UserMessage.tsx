import { MessagePrimitive, useMessage } from "@assistant-ui/react";

import { motion } from "framer-motion";

import type { TAttachmentFile } from "@/lib/types";
import { FileItem } from "@/components/file-item";

import { MarkdownText } from "@/components/markdown";

export const UserMessage = () => {
  const message = useMessage();

  const files: TAttachmentFile[] = message.content
    .filter((item) => item.type === "file")
    .map((item) => {
      return {
        type: JSON.parse(item.mimeType).type,
        content: item.data,
        path: JSON.parse(item.mimeType).path,
      };
    });
  return (
    <MessagePrimitive.Root asChild>
      <motion.div
        className="mx-auto grid w-full max-w-[var(--thread-max-width)] auto-rows-auto grid-cols-[minmax(72px,1fr)_auto] gap-y-1 px-[var(--thread-padding-x)] py-4 [&:where(>*)]:col-start-2"
        initial={{ y: 5, opacity: 0 }}
        animate={{ y: 0, opacity: 1 }}
        data-role="user"
      >
        {files.length > 0 ? (
          <div className="flex w-full flex-row gap-[8px] col-span-full col-start-1 row-start-1 justify-end mb-[8px]">
            {files.map((file) => (
              <FileItem key={file.path} file={file} withoutClose />
            ))}
          </div>
        ) : null}

        <div className="bg-[var(--chat-user-message-background)] text-[var(--chat-user-message-color)] col-start-2 break-words rounded-[16px] rounded-br-[0px] px-[12px] py-[8px]">
          <MessagePrimitive.Content components={{ Text: MarkdownText }} />
        </div>
      </motion.div>
    </MessagePrimitive.Root>
  );
};
