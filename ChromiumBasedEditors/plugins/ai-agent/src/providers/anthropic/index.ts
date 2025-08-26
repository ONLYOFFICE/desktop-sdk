import type { ThreadMessageLike } from "@assistant-ui/react";
import Anthropic from "@anthropic-ai/sdk";
import type {
  MessageParam,
  ToolResultBlockParam,
  ToolUnion,
} from "@anthropic-ai/sdk/resources/messages";
import cloneDeep from "lodash.clonedeep";

import type { TMCPItem } from "@/lib/types";

import type { BaseProvider } from "../base";

import { convertImageAttachmentsToContent } from "./utils";
import {
  handleContentBlockDelta,
  handleContentBlockStart,
  handleMessageStart,
} from "./handlers";

class AnthropicProvider
  implements BaseProvider<ToolUnion, MessageParam, Anthropic>
{
  modelKey: string = "claude-3-7-sonnet-latest";
  apiKey: string = "";
  url?: string = "";
  system?: string = `You are an AI assistant with access to a set of MCP tools.
Your default behavior is to act as a helpful, conversational LLM assistant: answer questions, explain concepts, brainstorm, or chat naturally.
Only call a tool when the user’s request clearly indicates the need for it (e.g., "search", "open URL", "analyze file", "plot data", etc.).
If the user’s intent is ambiguous, respond conversationally instead of calling a tool.
Do not push the user toward tools unnecessarily.
When you do use a tool, explain in natural language what you’re doing if appropriate.
Otherwise, behave as if you are a standalone LLM chat without tools.
`;
  prevMessages: MessageParam[] = [];
  tools: ToolUnion[] = [];
  client: Anthropic;

  constructor() {
    this.client = new Anthropic({
      apiKey: this.apiKey,
      baseURL: this.url,
      dangerouslyAllowBrowser: true,
    });
  }

  setModelKey = (modelKey: string) => {
    this.modelKey = modelKey;
  };

  setApiKey = (apiKey: string) => {
    this.apiKey = apiKey;

    this.client = new Anthropic({
      apiKey: this.apiKey,
      baseURL: this.url,
      dangerouslyAllowBrowser: true,
    });
  };

  setUrl = (url: string) => {
    this.url = url;

    this.client = new Anthropic({
      apiKey: this.apiKey,
      baseURL: this.url,
      dangerouslyAllowBrowser: true,
    });
  };

  setSystem = (system: string) => {
    this.system = system;
  };

  setPrevMessages = (prevMessages: MessageParam[]) => {
    this.prevMessages = prevMessages;
  };

  setTools = (tools: ToolUnion[]) => {
    this.tools = tools;
  };

  convertToolsToModelFormat = (tools: TMCPItem[]): ToolUnion[] => {
    return tools.map((tool) => ({
      name: tool.name,
      description: tool.description,
      input_schema: {
        type: "object",
        ...tool.inputSchema,
      },
    }));
  };

  convertMessagesToModelFormat = (
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
      } else {
        const content: MessageParam["content"] =
          typeof message.content === "string" ? message.content : [];

        const toolsResults: ToolResultBlockParam[] = [];

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
              return;
            }
          });
        }

        convertedMessages.push({
          role: "assistant",
          content,
        });

        if (toolsResults.length) {
          convertedMessages.push({ role: "user", content: toolsResults });
        }
      }
    });

    return convertedMessages;
  };

  getModels = async () => {
    const modelsRes = await this.client.models.list();

    const body = modelsRes.data;

    return body.map((model) => ({
      id: model.id,
      name: model.display_name,
      provider: "anthropic" as const,
    }));
  };

  async *sendMessage(
    messages: ThreadMessageLike[],
    afterToolCall?: boolean,
    message?: ThreadMessageLike
  ): AsyncGenerator<
    ThreadMessageLike | { isEnd: true; responseMessage: ThreadMessageLike }
  > {
    const isSystem = messages[0].role === "system";

    const convertedMessage = this.convertMessagesToModelFormat(messages);

    const stream = await this.client.messages.create({
      messages: [...this.prevMessages, ...convertedMessage],
      model: this.modelKey,
      system: this.system,
      tools: this.tools,
      stream: true,
      max_tokens: 2048,
    });

    if (!isSystem) {
      this.prevMessages.push(...convertedMessage);
    }

    let responseMessage: ThreadMessageLike =
      afterToolCall && message
        ? cloneDeep(message)
        : {
            role: "assistant",
            content: [],
          };

    for await (const messageStreamEvent of stream) {
      const { type } = messageStreamEvent;

      if (type === "message_start") {
        if (afterToolCall && message) {
          yield message;

          continue;
        }

        responseMessage = handleMessageStart(messageStreamEvent);
      }

      if (type === "content_block_start") {
        responseMessage = handleContentBlockStart(
          messageStreamEvent,
          responseMessage
        );
      }

      if (type === "content_block_delta") {
        responseMessage = handleContentBlockDelta(
          messageStreamEvent,
          responseMessage
        );
      }

      if (type === "message_stop") {
        if (afterToolCall && message) {
          const newContent = responseMessage.content.slice(
            message.content.length
          );

          const newMsg = {
            ...responseMessage,
            content: newContent,
          };

          const providerMsg = this.convertMessagesToModelFormat([newMsg]);

          this.prevMessages.push(...providerMsg);

          yield { isEnd: true, responseMessage };
          continue;
        }
        const providerMsg = this.convertMessagesToModelFormat([
          responseMessage,
        ]);

        this.prevMessages.push(...providerMsg);

        yield { isEnd: true, responseMessage };
        continue;
      }

      yield responseMessage;
    }
  }

  async *sendMessageAfterToolCall(
    message: ThreadMessageLike
  ): AsyncGenerator<
    ThreadMessageLike | { isEnd: true; responseMessage: ThreadMessageLike }
  > {
    if (typeof message.content === "string") return message;

    const result = message.content.find((c) => c.type === "tool-call");

    if (!result) return message;

    const toolResult: ToolResultBlockParam = {
      type: "tool_result",
      content: result.result,
      tool_use_id: result.toolCallId!,
    };

    this.prevMessages.push({
      role: "user",
      content: [toolResult],
    });

    yield* this.sendMessage(
      [
        {
          role: "system",
          content: [
            {
              type: "text",
              text: "What should I do next?",
            },
          ],
        },
      ],
      true,
      message
    );

    return message;
  }
}

const anthropicProvider = new AnthropicProvider();

export { AnthropicProvider, anthropicProvider };
