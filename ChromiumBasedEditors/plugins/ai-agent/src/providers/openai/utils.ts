import type {
  ChatCompletionMessageParam,
  ChatCompletionTool,
  ChatCompletionToolMessageParam,
  ChatCompletionMessageFunctionToolCall,
  ChatCompletionAssistantMessageParam,
  ChatCompletionContentPart,
} from "openai/resources/chat/completions";
import type { ThreadMessageLike } from "@assistant-ui/react";

import type { TMCPItem } from "@/lib/types";

export const convertToolsToModelFormat = (
  tools: TMCPItem[]
): ChatCompletionTool[] => {
  return tools.map((tool) => ({
    type: "function",
    function: {
      name: tool.name,
      description: tool.description,
      input_schema: {
        type: "object",
        ...tool.inputSchema,
      },
    },
  }));
};

export const convertMessagesToModelFormat = (
  messages: ThreadMessageLike[]
): ChatCompletionMessageParam[] => {
  const convertedMessages: ChatCompletionMessageParam[] = [];

  messages.forEach((message) => {
    if (message.role === "user" || message.role === "system") {
      const content: string | ChatCompletionContentPart[] =
        typeof message.content === "string"
          ? message.content
          : message.content.map((part): ChatCompletionContentPart => {
              if (part.type === "text") {
                return { type: "text", text: part.text };
              }

              if (part.type === "file") {
                return {
                  type: "text",
                  text: JSON.stringify({
                    file_data: part.data,
                    filename: JSON.parse(part.mimeType).path,
                    file_id: JSON.parse(part.mimeType).path,
                  }),
                };
              }

              return { type: "text", text: "" };
            });

      //   if (message.attachments?.length) {
      //     const imageContent = convertImageAttachmentsToContent(
      //       message.attachments
      //     );

      //     if (Array.isArray(content)) {
      //       content.push(...imageContent);
      //     }
      //   }

      convertedMessages.push({
        role: "user",
        content,
      } as ChatCompletionMessageParam);
    } else {
      const content: ChatCompletionAssistantMessageParam["content"] =
        typeof message.content === "string" ? message.content : [];

      const toolsResults: ChatCompletionToolMessageParam[] = [];
      const toolsCalls: ChatCompletionMessageFunctionToolCall[] = [];

      const msgContent = message.content;
      if (typeof msgContent !== "string" && typeof content !== "string") {
        msgContent.forEach((part) => {
          if (!Array.isArray(content)) return;

          if (part.type === "text") {
            content.push({ type: "text", text: part.text });
          }

          if (part.type === "tool-call") {
            if (part.result) {
              toolsResults.push({
                role: "tool",
                content: part.result,
                tool_call_id: part.toolCallId!,
              });
            }

            toolsCalls.push({
              id: part.toolCallId!,
              type: "function",
              function: { arguments: part.argsText ?? "", name: part.toolName },
            });
            return;
          }
        });
      }

      const msg: ChatCompletionAssistantMessageParam = {
        role: "assistant",
        content,
      };

      if (toolsCalls.length) msg.tool_calls = toolsCalls;

      convertedMessages.push(msg);

      if (toolsResults.length) {
        convertedMessages.push(...toolsResults);
      }
    }
  });

  return convertedMessages;
};
