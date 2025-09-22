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
          content += `*Tool Call: ${part.toolName}*\n`;

          if ("args" in part && part.args) {
            content += `\`\`\`json\n${JSON.stringify(
              part.args,
              null,
              2
            )}\n\`\`\`\n\n`;
          }

          if ("result" in part && part.result !== undefined) {
            content += `*Tool Result:*\n`;
            content += `\`\`\`json\n${part.result}\n\`\`\`\n\n`;
          }
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
