import type { ToolCallMessagePart } from "@assistant-ui/react";
import { START_TOOL_TAG, END_TOOL_TAG } from "./utils";

export const handleToolCall = (content: string) => {
  let isTool = false;
  let isFinish = false;

  let toolCallContent: string = "";

  if (content.includes(START_TOOL_TAG)) {
    isTool = true;
    toolCallContent = content.split(START_TOOL_TAG)[1];
  }

  if (!isTool) return { content };

  if (content.includes(END_TOOL_TAG)) {
    isFinish = true;
    toolCallContent = toolCallContent.split(END_TOOL_TAG)[0];
  }

  const newContent = content.split(START_TOOL_TAG)[0];

  if (isFinish) {
    const toolContent: ToolCallMessagePart = {
      type: "tool-call",
      toolCallId: "",
      toolName: JSON.parse(toolCallContent).name,
      args: JSON.parse(toolCallContent).args,
      argsText: toolCallContent,
    };

    return { content: newContent, toolContent };
  }

  const toolContent: ToolCallMessagePart = {
    type: "tool-call",
    toolCallId: "",
    toolName: "",
    args: {},
    argsText: toolCallContent,
  };

  return { toolContent, content: newContent };
};
