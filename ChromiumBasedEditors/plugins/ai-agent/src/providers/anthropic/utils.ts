import type {
  ContentBlockParam,
  MessageParam,
  ToolResultBlockParam,
  ToolUnion,
} from "@anthropic-ai/sdk/resources/index.mjs";
import type {
  CompleteAttachment,
  ThreadMessageLike,
} from "@assistant-ui/react";

import type { TMCPItem } from "@/lib/types";

export const convertImageAttachmentsToContent = (
  attachments: readonly CompleteAttachment[]
) => {
  const contentBlock: ContentBlockParam[] = [];

  attachments.forEach(({ content }) => {
    content.forEach((part) => {
      if (part.type === "image") {
        // Extract base64 data and mime type from data URL
        const imageData = part.image;
        const base64Data = imageData.split(",")[1];
        const mimeType = imageData.split(";")[0].split(":")[1];

        // Validate and map MIME type to Anthropic's expected types
        let validMimeType:
          | "image/jpeg"
          | "image/png"
          | "image/gif"
          | "image/webp";
        if (mimeType === "image/jpeg" || mimeType === "image/jpg") {
          validMimeType = "image/jpeg";
        } else if (mimeType === "image/png") {
          validMimeType = "image/png";
        } else if (mimeType === "image/gif") {
          validMimeType = "image/gif";
        } else if (mimeType === "image/webp") {
          validMimeType = "image/webp";
        } else {
          // Default to JPEG if unknown type
          validMimeType = "image/jpeg";
        }

        contentBlock.push({
          type: "image",
          source: {
            type: "base64",
            media_type: validMimeType,
            data: base64Data,
          },
        });
      }
    });
  });

  return contentBlock;
};

export const convertToolsToModelFormat = (tools: TMCPItem[]): ToolUnion[] => {
  return tools.map((tool) => ({
    name: tool.name,
    description: tool.description,
    input_schema: {
      type: "object",
      ...tool.inputSchema,
    },
  }));
};

export const convertMessagesToModelFormat = (
  messages: ThreadMessageLike[]
): MessageParam[] => {
  const convertedMessages: MessageParam[] = [];

  messages.forEach((message) => {
    if (message.role === "user") {
      const content: MessageParam["content"] =
        typeof message.content === "string"
          ? message.content
          : message.content.map((part) => {
              if (part.type === "text") {
                return { type: "text", text: part.text };
              }

              if (part.type === "file") {
                return {
                  type: "document",
                  source: {
                    type: "text",
                    media_type: "text/plain",
                    data: part.data,
                  },
                  context: JSON.parse(part.mimeType).path,
                };
              }

              return { type: "text", text: "" };
            });

      if (message.attachments?.length) {
        const imageContent = convertImageAttachmentsToContent(
          message.attachments
        );

        if (Array.isArray(content)) {
          content.push(...imageContent);
        }
      }

      convertedMessages.push({
        role: "user",
        content,
      });
    } else if (message.role === "system") {
      const content: MessageParam["content"] =
        typeof message.content === "string"
          ? message.content
          : message.content.map((part) => {
              if (part.type === "text") {
                return { type: "text", text: part.text };
              }

              return { type: "text", text: "" };
            });

      convertedMessages.push({
        role: "user",
        content,
      });
    } else {
      let content: MessageParam["content"] =
        typeof message.content === "string" ? message.content : [];

      let toolsResults: ToolResultBlockParam[] = [];

      if (Array.isArray(message.content)) {
        message.content.forEach((part) => {
          if (!Array.isArray(content)) return;

          if (part.type === "text") {
            content.push({ type: "text", text: part.text });
          }

          if (part.type === "tool-call") {
            if (part.result) {
              toolsResults.push({
                type: "tool_result",
                content: part.result,
                tool_use_id: part.toolCallId!,
              });
            }

            content.push({
              type: "tool_use",
              id: part.toolCallId!,
              name: part.toolName,
              input: part.args || {},
            });

            convertedMessages.push({
              role: "assistant",
              content,
            });

            if (toolsResults.length) {
              convertedMessages.push({ role: "user", content: toolsResults });

              toolsResults = [];
            }

            content = [];
            return;
          }
        });
      }

      if (content.length) {
        convertedMessages.push({
          role: "assistant",
          content,
        });
      }
    }
  });

  return convertedMessages;
};
