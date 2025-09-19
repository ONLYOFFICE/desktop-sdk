import type Anthropic from "@anthropic-ai/sdk";
import type {
  ThreadMessageLike,
  TextMessagePart,
  ToolCallMessagePart,
} from "@assistant-ui/react";

export const handleMessageStart = (
  messageStreamEvent: Anthropic.Messages.RawMessageStartEvent
) => {
  const { message } = messageStreamEvent;

  return {
    role: message.role,
    content: [],
  };
};

export const handleContentBlockStart = (
  messageStreamEvent: Anthropic.Messages.ContentBlockStartEvent,
  prevResponseMessage: ThreadMessageLike
) => {
  const responseMessage = { ...prevResponseMessage };
  const { content_block } = messageStreamEvent;

  if (content_block.type === "text") {
    if (Array.isArray(responseMessage.content)) {
      const item: TextMessagePart = {
        type: "text",
        text: content_block.text,
      };

      responseMessage.content.push(item);
    }
  }

  if (content_block.type === "tool_use") {
    if (Array.isArray(responseMessage.content)) {
      const item: ToolCallMessagePart = {
        type: "tool-call",
        toolCallId: content_block.id,
        toolName: content_block.name,
        args: {},
        argsText: "",
      };

      responseMessage.content.push(item);
    }
  }

  return responseMessage;
};

export const handleContentBlockDelta = (
  messageStreamEvent: Anthropic.Messages.RawContentBlockDeltaEvent,
  prevResponseMessage: ThreadMessageLike
) => {
  const responseMessage = { ...prevResponseMessage };
  const { delta } = messageStreamEvent;

  if (delta.type === "text_delta") {
    if (Array.isArray(responseMessage.content)) {
      const lastContent =
        responseMessage.content[responseMessage.content.length - 1];

      if (lastContent.type !== "text") {
        return responseMessage;
      }

      const item: TextMessagePart = {
        type: "text",
        text: lastContent.text + delta.text,
      };

      responseMessage.content[responseMessage.content.length - 1] = item;
    }
  }

  if (delta.type === "input_json_delta") {
    if (Array.isArray(responseMessage.content)) {
      const lastContent =
        responseMessage.content[responseMessage.content.length - 1];

      if (lastContent.type !== "tool-call") {
        return responseMessage;
      }

      const argsText = lastContent.argsText + delta.partial_json;

      let args = {};

      if (argsText.trim().endsWith("}")) {
        try {
          args = JSON.parse(argsText);
        } catch (e) {
          console.log(e);
        }
      }

      const item: ToolCallMessagePart = {
        ...lastContent,

        args,
        argsText,
      };

      responseMessage.content[responseMessage.content.length - 1] = item;
    }
  }

  return responseMessage;
};
