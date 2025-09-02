import type { ThreadMessageLike } from "@assistant-ui/react";
import Anthropic from "@anthropic-ai/sdk";
import type {
  MessageParam,
  ToolResultBlockParam,
  ToolUnion,
} from "@anthropic-ai/sdk/resources/messages";
import cloneDeep from "lodash.clonedeep";

import type { Model, TMCPItem } from "@/lib/types";
import { CLAUDE_KEY } from "@/config";

import type { BaseProvider } from "../base";

import {
  convertMessagesToModelFormat,
  convertToolsToModelFormat,
} from "./utils";
import {
  handleContentBlockDelta,
  handleContentBlockStart,
  handleMessageStart,
} from "./handlers";

class AnthropicProvider
  implements BaseProvider<ToolUnion, MessageParam, Anthropic>
{
  modelKey: string = "claude-3-7-sonnet-latest";
  systemPrompt: string = "";
  apiKey?: string = CLAUDE_KEY;
  url?: string = "";

  messageStopped: boolean = false;

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
    this.client.apiKey = apiKey;
  };

  setUrl = (url: string) => {
    this.url = url;
    this.client.baseURL = url;
  };

  setSystemPrompt = (systemPrompt: string) => {
    this.systemPrompt = systemPrompt;
  };

  setPrevMessages = (prevMessages: ThreadMessageLike[]) => {
    this.prevMessages = convertMessagesToModelFormat(prevMessages);
  };

  setTools = (tools: TMCPItem[]) => {
    this.tools = convertToolsToModelFormat(tools);
  };

  getModels = async (): Promise<Model[]> => {
    if (!this.client.apiKey) return [];

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
    if (!this.client.apiKey) return;

    const isSystem = messages[0].role === "system";

    const convertedMessage = convertMessagesToModelFormat(messages);

    const stream = await this.client.messages.create({
      messages: [...this.prevMessages, ...convertedMessage],
      model: this.modelKey,
      system: this.systemPrompt,
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

      if (this.messageStopped) {
        this.messageStopped = false;

        stream.controller.abort();
      }

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

          const providerMsg = convertMessagesToModelFormat([newMsg]);

          this.prevMessages.push(...providerMsg);

          yield { isEnd: true, responseMessage };
          continue;
        }
        const providerMsg = convertMessagesToModelFormat([responseMessage]);

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

    const result = message.content
      .filter((c) => c.type === "tool-call")
      .reverse()[0];

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

  stopMessage = () => {
    this.messageStopped = true;
  };
}

const anthropicProvider = new AnthropicProvider();

export { AnthropicProvider, anthropicProvider };
