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

class OpenRouterProvider
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
        temperature: 0,
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
    return "OpenRouter";
  };

  getBaseUrl = () => {
    return "https://openrouter.ai/api/v1";
  };

  checkProvider = async (data: TData): Promise<boolean | TErrorData> => {
    try {
      const response = await fetch(`${data.url}/models/user`, {
        headers: {
          Authorization: `Bearer ${data.apiKey}`,
        },
      });

      if (!response.ok) {
        if (response.status === 401 || !data.apiKey) {
          return {
            field: "key",
            message: "Invalid API Key",
          };
        }
        return {
          field: "url",
          message: "Invalid URL",
        };
      }

      return true;
    } catch (error) {
      console.log(error);
      return {
        field: "url",
        message: "Failed to connect",
      };
    }
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
          model.id === "openai/gpt-5.1" ||
          model.id === "anthropic/claude-haiku-4.5" ||
          model.id === "anthropic/claude-sonnet-4.5" ||
          model.id === "anthropic/claude-opus-4.1" ||
          model.id === "x-ai/grok-4" ||
          // model.id === "google/gemini-2.5-flash" ||
          // model.id === "google/gemini-2.5-pro" ||
          // model.id === "deepseek/deepseek-chat-v3.1" ||
          model.id === "qwen/qwen3-235b-a22b-2507" ||
          model.id === "deepseek/deepseek-v3.1-terminus" ||
          model.id === "qwen/qwen3-max"
      )
      .map((model) => ({
        id: model.id,
        name:
          model.id === "openai/gpt-5.1"
            ? "GPT-5.1"
            : model.id === "anthropic/claude-haiku-4.5"
            ? "Claude Haiku 4.5"
            : model.id === "anthropic/claude-sonnet-4.5"
            ? "Claude Sonnet 4.5"
            : model.id === "anthropic/claude-opus-4.1"
            ? "Claude Opus 4.1"
            : model.id === "x-ai/grok-4"
            ? "Grok 4"
            : model.id === "google/gemini-2.5-flash"
            ? "Gemini 2.5 Flash"
            : model.id === "google/gemini-2.5-pro"
            ? "Gemini 2.5 Pro"
            : model.id === "qwen/qwen3-235b-a22b-2507"
            ? "Qwen 3"
            : model.id === "deepseek/deepseek-v3.1-terminus"
            ? "DeepSeek V3.1 Terminus"
            : model.id === "qwen/qwen3-max"
            ? "Qwen 3 Max"
            : model.id.toUpperCase(),
        provider: "openrouter" as const,
      }));
  };
}

const openrouterProvider = new OpenRouterProvider();

export { OpenRouterProvider, openrouterProvider };
