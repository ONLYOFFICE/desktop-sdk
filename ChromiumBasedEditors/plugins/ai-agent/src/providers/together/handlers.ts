import type {
  ThreadMessageLike,
  ToolCallMessagePart,
} from "@assistant-ui/react";
import cloneDeep from "lodash.clonedeep";
import type { ChatCompletionChunk } from "together-ai/resources/chat/completions";

export const handleTextMessage = (
  responseMessage: ThreadMessageLike,
  chunk: ChatCompletionChunk.Choice,
  afterToolCall?: boolean
) => {
  // Return early if no content in delta
  if (!chunk.delta.content) return responseMessage;

  let newResponseMessage = cloneDeep(responseMessage);

  const content = newResponseMessage.content;

  if (typeof content === "string") return newResponseMessage;

  const lstContent = content[content.length - 1];

  if (
    (!lstContent || afterToolCall) &&
    typeof responseMessage.content !== "string"
  ) {
    newResponseMessage = {
      ...newResponseMessage,
      content: [
        ...content,
        {
          type: "text",
          text: chunk.delta.content,
        },
      ],
    };
  }

  if (
    lstContent &&
    typeof lstContent !== "string" &&
    typeof content !== "string"
  ) {
    if (lstContent.type === "text") {
      const text = lstContent.text + chunk.delta.content;
      const newLstContent = cloneDeep({ ...lstContent, text });
      newResponseMessage = {
        ...newResponseMessage,
        content: [...content.slice(0, -1), newLstContent],
      };
    }
  }

  return newResponseMessage;
};

export const handleToolCall = (
  responseMessage: ThreadMessageLike,
  chunk: ChatCompletionChunk.Choice
) => {
  let newResponseMessage = cloneDeep(responseMessage);

  if (!chunk.delta.tool_calls || typeof newResponseMessage.content === "string")
    return newResponseMessage;

  const toolCallDelta = chunk.delta.tool_calls[0];

  const lastContent =
    newResponseMessage.content[newResponseMessage.content.length - 1];

  if (!lastContent || lastContent?.type !== "tool-call") {
    const toolCall: ToolCallMessagePart = {
      type: "tool-call",
      args: {},
      argsText: toolCallDelta?.function?.arguments ?? "",
      toolName: toolCallDelta?.function?.name ?? "",
      toolCallId: toolCallDelta?.id ?? "",
    };

    newResponseMessage = {
      ...newResponseMessage,
      content: [...newResponseMessage.content, toolCall],
    };
  } else {
    const toolCall = lastContent;

    const argsText =
      toolCall.argsText + (toolCallDelta?.function?.arguments ?? "");
    const name = toolCall.toolName || toolCallDelta?.function?.name || "";
    const toolCallId = toolCall.toolCallId || toolCallDelta?.id || "";

    const newToolCall: ToolCallMessagePart = {
      ...toolCall,
      args: {},
      argsText,
      toolName: name,
      toolCallId,
    };

    newResponseMessage = {
      ...newResponseMessage,
      content: [...newResponseMessage.content.slice(0, -1), newToolCall],
    };
  }

  return newResponseMessage;
};
