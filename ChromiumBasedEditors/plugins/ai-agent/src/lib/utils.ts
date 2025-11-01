import type { ThreadMessageLike } from "@assistant-ui/react";
import { clsx, type ClassValue } from "clsx";
import { twMerge } from "tailwind-merge";

export const cn = (...inputs: ClassValue[]) => {
  return twMerge(clsx(inputs));
};

export const convertMessagesToMd = (messages: ThreadMessageLike[]) => {
  let content = "";

  messages.forEach((message, index) => {
    const role = message.role === "user" ? "**User**" : "**Assistant**";
    content += `## ${role}\n\n`;

    if (Array.isArray(message.content)) {
      message.content.forEach((part: ThreadMessageLike["content"]) => {
        if (typeof part === "string") {
          content += `${part}\n\n`;
          return;
        }

        if (!part || typeof part !== "object" || !("type" in part)) return;

        if (part.type === "text" && "text" in part) {
          content += `${part.text}\n\n`;
        } else if (part.type === "tool-call" && "toolName" in part) {
          return;
        }
      });
    } else if (typeof message.content === "string") {
      content += `${message.content}\n\n`;
    }

    if (index < messages.length - 1) {
      content += `---\n\n`;
    }
  });

  return content;
};

export const isDocument = (type: number) => {
  return !!(type & 0x40);
};

export const isPresentation = (type: number) => {
  return !!(type & 0x80);
};

export const isSpreadsheet = (type: number) => {
  return !!(type & 0x0100);
};

export const isPdf = (type: number) => {
  return type === 0x0201 || type === 0x0209;
};
