import type { CompletionCreateParams } from "together-ai/resources/chat/completions";
import type { Tools } from "together-ai/resources";
import type { ThreadMessageLike } from "@assistant-ui/react";

import type { TMCPItem } from "@/lib/types";

export type TogetherMessageParam = CompletionCreateParams["messages"][number];

export const convertToolsToModelFormat = (tools: TMCPItem[]): Tools[] => {
  return tools.map((tool) => ({
    type: "function",
    function: {
      name: tool.name,
      description: tool.description,
      parameters: {
        type: "object",
        ...tool.inputSchema,
      },
    },
  }));
};

export const convertMessagesToModelFormat = (
  messages: ThreadMessageLike[]
): TogetherMessageParam[] => {
  const convertedMessages: TogetherMessageParam[] = [];

  messages.forEach((message) => {
    if (message.role === "user" || message.role === "system") {
      const content: string =
        typeof message.content === "string"
          ? message.content
          : message.content
              .map((part) => {
                if (part.type === "text") {
                  return part.text;
                }

                if (part.type === "file") {
                  const path = JSON.parse(part.mimeType).path;

                  const fileContent = `File: ${path
                    .split("/")
                    .pop()}\nFile content:\n${part.data}`;
                  return fileContent;
                }

                return "";
              })
              .filter((part) => part !== "")
              .join("\n\n");

      convertedMessages.push({
        role: "user",
        content,
      });
    } else {
      let content: string =
        typeof message.content === "string" ? message.content : "";

      const toolsResults: CompletionCreateParams.ChatCompletionToolMessageParam[] =
        [];
      const toolsCalls: CompletionCreateParams.ChatCompletionAssistantMessageParam["tool_calls"] =
        [];

      let idx = 0;

      const msgContent = message.content;
      if (typeof msgContent !== "string" && typeof content !== "string") {
        msgContent.forEach((part) => {
          if (part.type === "text") {
            content += part.text;
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
              index: idx++,
              type: "function",
              function: { arguments: part.argsText ?? "", name: part.toolName },
            });
            return;
          }
        });
      }

      const msg: CompletionCreateParams.ChatCompletionAssistantMessageParam = {
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
