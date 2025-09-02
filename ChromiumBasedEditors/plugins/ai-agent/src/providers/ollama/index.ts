import {
  Ollama,
  type ListResponse,
  type Message,
  type Tool,
} from "ollama/browser";
import type { ThreadMessageLike } from "@assistant-ui/react";
import cloneDeep from "lodash.clonedeep";

import type { Model, TMCPItem } from "@/lib/types";

import type { BaseProvider } from "../base";

import {
  convertMessagesToModelFormat,
  convertToolsToModelFormat,
  convertToolsToString,
} from "./utils";
import { handleToolCall } from "./handlers";

class OllamaProvider implements BaseProvider<Tool, Message, Ollama> {
  modelKey: string = "";
  systemPrompt: string = "";
  apiKey?: string = "";
  url?: string = "http://127.0.0.1:11434";

  prevMessages: Message[] = [];
  tools: Tool[] = [];
  client: Ollama;

  constructor() {
    this.client = new Ollama({ host: this.url });
  }

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

  getModels = async (): Promise<Model[]> => {
    const response: ListResponse = await this.client.list();

    return response.models.map((model) => ({
      id: model.model,
      name: model.name,
      provider: "ollama" as const,
    }));
  };

  async *sendMessage(
    messages: ThreadMessageLike[],
    afterToolCall?: boolean,
    message?: ThreadMessageLike
  ): AsyncGenerator<
    ThreadMessageLike | { isEnd: true; responseMessage: ThreadMessageLike }
  > {
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
            responseMessage.content[responseMessage.content.length - 1].type !==
            "tool-call"
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
            responseMessage.content[responseMessage.content.length - 1].type ===
            "tool-call"
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
    this.client.abort();
  };
}

const ollamaProvider = new OllamaProvider();

export { OllamaProvider, ollamaProvider };
