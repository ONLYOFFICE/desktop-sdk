import type { ThreadMessageLike } from "@assistant-ui/react";
import Anthropic from "@anthropic-ai/sdk";
import type {
  MessageParam,
  ToolResultBlockParam,
  ToolUnion,
} from "@anthropic-ai/sdk/resources/messages";
import cloneDeep from "lodash.clonedeep";

import type { Model, TMCPItem, TProvider } from "@/lib/types";

import type { BaseProvider } from "../base";
import type { SettingsProvider, TData, TErrorData } from "../settings";

import { CREATE_TITLE_SYSTEM_PROMPT } from "../Providers.utils";

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
  implements BaseProvider<ToolUnion, MessageParam, Anthropic>, SettingsProvider
{
  modelKey: string = "claude-3-7-sonnet-latest";
  systemPrompt: string = "";

  apiKey?: string;
  url?: string;
  provider?: TProvider;

  messageStopped: boolean = false;

  prevMessages: MessageParam[] = [];
  tools: ToolUnion[] = [];
  client?: Anthropic;

  constructor() {}

  setProvider = (provider: TProvider) => {
    this.provider = provider;

    this.client = new Anthropic({
      apiKey: provider.key,
      baseURL: provider.baseUrl,
      dangerouslyAllowBrowser: true,
    });

    if (provider.key) this.setApiKey(provider.key);
    if (provider.baseUrl) this.setUrl(provider.baseUrl);
  };

  setModelKey = (modelKey: string) => {
    this.modelKey = modelKey;
  };

  setSystemPrompt = (systemPrompt: string) => {
    this.systemPrompt = systemPrompt;
  };

  setApiKey = (apiKey: string) => {
    this.apiKey = apiKey;
    if (this.client) this.client.apiKey = apiKey;
  };

  setUrl = (url: string) => {
    this.url = url;
    if (this.client) this.client.baseURL = url;
  };

  setPrevMessages = (prevMessages: ThreadMessageLike[]) => {
    this.prevMessages = convertMessagesToModelFormat(prevMessages);
  };

  setTools = (tools: TMCPItem[]) => {
    this.tools = convertToolsToModelFormat(tools);
  };

  async createChatName(message: string) {
    try {
      if (!this.client) return message.substring(0, 25);

      const response = await this.client.messages.create({
        messages: [{ role: "user", content: message }],
        model: this.modelKey,
        system: CREATE_TITLE_SYSTEM_PROMPT,
        max_tokens: 2048,
        stream: false,
      });

      const title = response.content.find((c) => c.type === "text")?.text;

      return title ?? message.substring(0, 25);
    } catch {
      return message.substring(0, 25);
    }
  }

  async *sendMessage(
    messages: ThreadMessageLike[],
    afterToolCall?: boolean,
    message?: ThreadMessageLike
  ): AsyncGenerator<
    ThreadMessageLike | { isEnd: true; responseMessage: ThreadMessageLike }
  > {
    try {
      if (!this.client) return;

      const convertedMessage = convertMessagesToModelFormat(messages);

      const stream = await this.client.messages.create({
        messages: [...this.prevMessages, ...convertedMessage],
        model: this.modelKey,
        system: this.systemPrompt,
        tools: this.tools,
        stream: true,
        max_tokens: 2048,
        tool_choice: {
          disable_parallel_tool_use: true,
          type: "auto",
        },
      });

      this.prevMessages.push(...convertedMessage);

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
    } catch (e) {
      yield {
        isEnd: true,
        responseMessage: {
          role: "assistant",
          content: "",
          status: {
            type: "incomplete",
            reason: "error",
            error: e,
          },
        } as ThreadMessageLike,
      };
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
      tool_use_id: result.toolCallId ?? "",
    };

    this.prevMessages.push({
      role: "user",
      content: [toolResult],
    });

    yield* this.sendMessage([], true, message);

    return message;
  }

  stopMessage = () => {
    this.messageStopped = true;
  };

  getBaseUrl = (): string => {
    return "https://api.anthropic.com";
  };

  getName = (): string => {
    return "Anthropic";
  };

  checkProvider = async (data: TData): Promise<boolean | TErrorData> => {
    const checkClient = new Anthropic({
      apiKey: data.apiKey,
      baseURL: data.url,
      dangerouslyAllowBrowser: true,
    });

    try {
      await checkClient.models.list();

      return true;
    } catch (error) {
      if (typeof error === "object" && error) {
        if ("status" in error && error.status === 401) {
          return {
            field: "key",
            message: "Invalid API key",
          };
        }

        if ("status" in error && error.status === 404) {
          return {
            field: "url",
            message: "Invalid URL",
          };
        }

        if (data.apiKey) {
          return {
            field: "url",
            message: "Invalid URL",
          };
        }
      }
      return {
        field: "key",
        message: "Empty key",
      };
    }
  };

  getProviderModels = async (data: TData): Promise<Model[]> => {
    const checkClient = new Anthropic({
      apiKey: data.apiKey,
      baseURL: data.url,
      dangerouslyAllowBrowser: true,
    });

    try {
      const modelsRes = await checkClient.models.list();

      const body = modelsRes.data;

      return body.map((model) => ({
        id: model.id,
        name: model.display_name,
        provider: "anthropic" as const,
      }));
    } catch (error) {
      console.log(error);
      return [];
    }
  };
}

const anthropicProvider = new AnthropicProvider();

export { AnthropicProvider, anthropicProvider };
