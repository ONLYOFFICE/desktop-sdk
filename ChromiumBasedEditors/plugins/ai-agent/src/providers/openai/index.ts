import OpenAI from "openai";
import type {
  ChatCompletionChunk,
  ChatCompletionMessageParam,
  ChatCompletionSystemMessageParam,
  ChatCompletionTool,
  ChatCompletionToolMessageParam,
} from "openai/resources/chat/completions";
import cloneDeep from "lodash.clonedeep";
import type { Model as OpenAIModel } from "openai/resources/models";
import type { ThreadMessageLike } from "@assistant-ui/react";

import type { Model, TMCPItem, TProvider } from "@/lib/types";

import type { BaseProvider } from "../base";
import type { SettingsProvider, TData, TErrorData } from "../settings";

import {
  convertToolsToModelFormat,
  convertMessagesToModelFormat,
} from "./utils";
import { handleTextMessage, handleToolCall } from "./handlers";
import { CREATE_TITLE_SYSTEM_PROMPT } from "../Providers.utils";

class OpenAIProvider
  implements
    BaseProvider<ChatCompletionTool, ChatCompletionMessageParam, OpenAI>,
    SettingsProvider
{
  modelKey: string = "";
  systemPrompt: string = "";

  apiKey?: string;
  url?: string;
  provider?: TProvider;

  prevMessages: ChatCompletionMessageParam[] = [];
  tools: ChatCompletionTool[] = [];
  client?: OpenAI;

  stopStream = false;

  constructor() {}

  setProvider = (provider: TProvider) => {
    this.provider = provider;

    this.client = new OpenAI({
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
      if (!this.client) return "";

      const systemMessage: ChatCompletionSystemMessageParam = {
        role: "system",
        content: CREATE_TITLE_SYSTEM_PROMPT,
      };

      const response = await this.client.chat.completions.create({
        messages: [systemMessage, { role: "user", content: message }],
        model: this.modelKey,
        stream: false,
      });

      const title = response.choices[0].message.content;

      return title ?? message.substring(0, 25);
    } catch {
      return "";
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

      const systemMessage: ChatCompletionSystemMessageParam = {
        role: "system",
        content: this.systemPrompt,
      };

      const stream = await this.client.chat.completions.create({
        messages: [systemMessage, ...this.prevMessages, ...convertedMessage],
        model: this.modelKey,
        tools: this.tools,
        stream: true,
      });

      this.prevMessages.push(...convertedMessage);

      let responseMessage: ThreadMessageLike =
        afterToolCall && message
          ? cloneDeep(message)
          : {
              role: "assistant",
              content: [],
            };

      let stop = false;

      for await (const messageStreamEvent of stream) {
        const chunks: ChatCompletionChunk["choices"] =
          messageStreamEvent.choices;

        chunks.forEach((chunk) => {
          if (stop) return;

          if (chunk.finish_reason) {
            stop = true;

            const curMsg = afterToolCall
              ? {
                  ...responseMessage,
                  content:
                    typeof responseMessage.content === "string"
                      ? responseMessage.content
                      : responseMessage.content.filter((part, index) => {
                          // Keep tool-call parts and new text parts added after tool execution
                          if (part.type === "tool-call") return true;
                          // Only keep text parts that were added after the original message
                          const originalLength = message?.content.length ?? 0;
                          return index >= originalLength;
                        }),
                }
              : responseMessage;

            const providerMsg = convertMessagesToModelFormat([curMsg]);

            this.prevMessages.push(...providerMsg);

            return;
          }

          if (chunk.delta.content) {
            responseMessage = handleTextMessage(
              responseMessage,
              chunk,
              afterToolCall
            );
          }

          if (
            chunk.delta.tool_calls &&
            typeof responseMessage.content !== "string"
          ) {
            responseMessage = handleToolCall(responseMessage, chunk);
          }
        });

        if (this.stopStream) {
          const providerMsg = convertMessagesToModelFormat([responseMessage]);

          this.prevMessages.push(...providerMsg);

          stream.controller.abort();

          this.stopStream = false;

          yield {
            isEnd: true,
            responseMessage,
          };

          continue;
        }

        if (stop) {
          yield {
            isEnd: true,
            responseMessage,
          };
          continue;
        } else {
          yield responseMessage;
        }
      }
    } catch (e) {
      console.log(e);
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

    const toolResult: ChatCompletionToolMessageParam = {
      role: "tool",
      content: result.result,
      tool_call_id: result.toolCallId!,
    };

    this.prevMessages.push(toolResult);

    yield* this.sendMessage([], true, message);

    return message;
  }

  stopMessage = () => {
    this.stopStream = true;
  };

  getName = () => {
    return "OpenAI";
  };

  getBaseUrl = () => {
    return "https://api.openai.com/v1";
  };

  checkProvider = async (data: TData): Promise<boolean | TErrorData> => {
    const checkClient = new OpenAI({
      baseURL: data.url,
      apiKey: data.apiKey,
      dangerouslyAllowBrowser: true,
    });

    try {
      await checkClient.models.list();

      return true;
    } catch (error) {
      console.log(JSON.stringify(error));
      const errorObj = error as { code: string };

      if (errorObj.code === "invalid_api_key") {
        return {
          field: "key",
          message: "Invalid API Key",
        };
      }
    }

    if (data.apiKey) {
      return {
        field: "url",
        message: "Invalid URL",
      };
    }

    return {
      field: "key",
      message: "Empty key",
    };
  };

  getProviderModels = async (data: TData): Promise<Model[]> => {
    const newClient = new OpenAI({
      baseURL: data.url,
      apiKey: data.apiKey,
      dangerouslyAllowBrowser: true,
    });

    const response: OpenAIModel[] = (await newClient.models.list()).data;

    return response
      .filter(
        (model) =>
          model.id === "gpt-4.1" ||
          model.id === "gpt-5" ||
          model.id === "gpt-5.1-2025-11-13"
      )
      .map((model) => ({
        id: model.id,
        name:
          model.id === "gpt-5.1-2025-11-13"
            ? "GPT-5.1"
            : model.id.toUpperCase(),
        provider: "openai" as const,
      }))
      .reverse();
  };
}

const openaiProvider = new OpenAIProvider();

export { OpenAIProvider, openaiProvider };
