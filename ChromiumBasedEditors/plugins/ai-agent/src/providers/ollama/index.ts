import {
  Ollama,
  type ListResponse,
  type Message,
  type Tool,
} from "ollama/browser";
import type { ThreadMessageLike } from "@assistant-ui/react";
import cloneDeep from "lodash.clonedeep";

import type { Model, TMCPItem, TProvider } from "@/lib/types";

import type { BaseProvider } from "../base";

import {
  convertMessagesToModelFormat,
  convertToolsToModelFormat,
  convertToolsToString,
} from "./utils";
import { handleToolCall } from "./handlers";
import type { SettingsProvider, TData, TErrorData } from "../settings";

class OllamaProvider
  implements BaseProvider<Tool, Message, Ollama>, SettingsProvider
{
  modelKey: string = "";
  systemPrompt: string = "";

  apiKey?: string;
  url?: string;
  provider?: TProvider;

  prevMessages: Message[] = [];
  tools: Tool[] = [];
  client?: Ollama;

  constructor() {}

  setProvider = (provider: TProvider) => {
    this.provider = provider;

    this.client = new Ollama({
      host: provider.baseUrl,
    });

    if (provider.key) this.setAPIKey(provider.key);
    if (provider.baseUrl) this.setURL(provider.baseUrl);
  };

  setModelKey = (modelKey: string) => {
    this.modelKey = modelKey;
  };

  setSystemPrompt = (systemPrompt: string) => {
    this.systemPrompt = systemPrompt;
  };

  setAPIKey = (apiKey: string) => {
    this.apiKey = apiKey;
  };

  setURL = (url: string) => {
    this.url = url;
  };

  setPrevMessages = (prevMessages: ThreadMessageLike[]) => {
    this.prevMessages = convertMessagesToModelFormat(prevMessages);
  };

  setTools = (tools: TMCPItem[]) => {
    this.tools = convertToolsToModelFormat(tools);
  };

  async *sendMessage(
    messages: ThreadMessageLike[],
    afterToolCall?: boolean,
    message?: ThreadMessageLike
  ): AsyncGenerator<
    ThreadMessageLike | { isEnd: true; responseMessage: ThreadMessageLike }
  > {
    try {
      if (!this.client) return;

      const convertedMessages = convertMessagesToModelFormat(messages);

      const toolsString = convertToolsToString(this.tools);

      const systemMsg = {
        role: "system",
        content: this.systemPrompt + toolsString,
      };

      const response = await this.client.chat({
        model: this.modelKey,
        messages: [systemMsg, ...this.prevMessages, ...convertedMessages],
        stream: true,
      });

      if (!afterToolCall) {
        this.prevMessages.push(...convertedMessages);
      }

      const responseMessage: ThreadMessageLike =
        afterToolCall && message
          ? cloneDeep(message)
          : {
              role: "assistant",
              content: [],
            };

      let msg = "";

      for await (const part of response) {
        msg += part.message.content;

        const { toolContent, content } = handleToolCall(msg);

        if (Array.isArray(responseMessage.content)) {
          if (responseMessage.content.length === 0) {
            responseMessage.content.push({ type: "text", text: content });
          } else if (toolContent) {
            if (
              responseMessage.content[responseMessage.content.length - 1]
                .type !== "tool-call"
            ) {
              responseMessage.content[responseMessage.content.length - 1] = {
                type: "text",
                text: content,
              };

              responseMessage.content.push(toolContent);
            } else {
              responseMessage.content[responseMessage.content.length - 1] =
                toolContent;
            }
          } else {
            if (
              responseMessage.content[responseMessage.content.length - 1]
                .type === "tool-call"
            ) {
              responseMessage.content.push({
                type: "text",
                text: content,
              });
            } else {
              responseMessage.content[responseMessage.content.length - 1] = {
                type: "text",
                text: content,
              };
            }
          }
        }

        if (part.done) {
          this.prevMessages.push({
            role: "assistant",
            content: msg,
          });

          yield {
            isEnd: true,
            responseMessage,
          };
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

    const toolResultStr: string = JSON.stringify({
      name: result.toolName,
      result: result.result,
    });

    this.prevMessages.push({
      role: "user",
      content: toolResultStr,
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
    if (!this.client) return;

    this.client.abort();
  };

  getName = () => {
    return "Ollama";
  };

  getBaseUrl = () => {
    return "http://localhost:11434";
  };

  checkProvider = async (data: TData): Promise<boolean | TErrorData> => {
    const checkClient = new Ollama({
      host: data.url,
    });

    try {
      await checkClient.list();

      return true;
    } catch (error) {
      console.log(error);

      return {
        field: "url",
        message: "Invalid URL",
      };
    }
  };

  getProviderModels = async (data: TData): Promise<Model[]> => {
    const newClient = new Ollama({
      host: data.url,
    });

    const response: ListResponse = await newClient.list();

    return response.models.map((model) => ({
      id: model.model,
      name: model.name,
      provider: "ollama" as const,
    }));
  };
}

const ollamaProvider = new OllamaProvider();

export { OllamaProvider, ollamaProvider };
