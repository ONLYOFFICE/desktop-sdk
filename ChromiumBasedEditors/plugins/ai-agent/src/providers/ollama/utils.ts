import type { Message, Tool } from "ollama/browser";
import type { ThreadMessageLike } from "@assistant-ui/react";

import type { TMCPItem } from "@/lib/types";

export const START_TOOL_TAG = "<TOOL_CALL>";
export const END_TOOL_TAG = "</TOOL_CALL>";

export const convertToolsToModelFormat = (tools: TMCPItem[]): Tool[] => {
  return tools.map((tool) => {
    const newTool: Tool = {
      type: "string",
      function: {
        name: tool.name,
        description: tool.description,
        parameters: tool.inputSchema,
      },
    };

    return newTool;
  });
};

export const convertToolsToString = (tools: Tool[]): string => {
  let str = "\n\nAvailable tools:\n\n";

  const toolsStr = JSON.stringify(tools);

  str += toolsStr;

  const exampleObj = {
    name: "toolName",
    args: {
      arg1: "arg1",
      arg2: "arg2",
    },
  };

  const exampleStr = JSON.stringify(exampleObj);

  str +=
    "\n\n When you call a tool, output must look EXACTLY like this: \n" +
    `${START_TOOL_TAG}\n${exampleStr}\n${END_TOOL_TAG}` +
    "\n\n Do not add explanations, markdown, or extra text outside the tags.";

  return str;
};

export const convertMessagesToModelFormat = (
  messages: ThreadMessageLike[]
): Message[] => {
  const convertedMessages: Message[] = [];

  messages.forEach((message) => {
    if (message.role === "user") {
      const content: Message["content"] =
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
    } else if (message.role === "system") {
      const content: Message["content"] =
        typeof message.content === "string"
          ? message.content
          : message.content.find((part) => part.type === "text")?.text ?? "";

      convertedMessages.push({
        role: "user",
        content,
      });
    } else {
      let resultContent: string = "";

      const strContent: string = Array.isArray(message.content)
        ? message.content
            .map((part) => {
              if (part.type === "text") {
                return part.text;
              }

              if (part.type === "tool-call") {
                const result = {
                  name: part.toolName,
                  result: part.result,
                };

                resultContent = `${JSON.stringify(result)}\n`;

                return `${START_TOOL_TAG}${part.argsText}${END_TOOL_TAG}`;
              }
            })
            .join("")
        : "";

      const content: Message["content"] =
        typeof message.content === "string" ? message.content : strContent;

      convertedMessages.push({
        role: "assistant",
        content,
      });

      if (resultContent) {
        convertedMessages.push({
          role: "user",
          content: resultContent,
        });
      }
    }
  });

  return convertedMessages;
};
