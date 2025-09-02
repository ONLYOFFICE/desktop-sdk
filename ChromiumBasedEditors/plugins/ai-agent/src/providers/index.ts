import type { ThreadMessageLike } from "@assistant-ui/react";

import type { Model, ProviderType, TMCPItem } from "@/lib/types";

import { anthropicProvider, AnthropicProvider } from "./anthropic";
import { ollamaProvider, OllamaProvider } from "./ollama";

import { SYSTEM_PROMPT } from "./Providers.utils";

export type SendMessageReturnType = AsyncGenerator<
  | ThreadMessageLike
  | {
      isEnd: true;
      responseMessage: ThreadMessageLike;
    }
>;

class Provider {
  currentProvider?: AnthropicProvider | OllamaProvider;
  currentProviderType?: ProviderType;

  anthropicProvider: AnthropicProvider;
  ollamaProvider: OllamaProvider;

  constructor() {
    this.anthropicProvider = anthropicProvider;
    this.ollamaProvider = ollamaProvider;
  }

  setCurrentProvider = (providerType: ProviderType) => {
    if (!providerType) return;

    switch (providerType) {
      case "anthropic":
        this.currentProvider = anthropicProvider;
        this.currentProviderType = "anthropic";
        break;

      case "ollama":
        this.currentProvider = ollamaProvider;
        this.currentProviderType = "ollama";
        break;

      default:
        this.currentProvider = undefined;
        this.currentProviderType = undefined;
    }
  };

  getCurrentProviderModel = () => {
    if (!this.currentProvider) return;

    return this.currentProvider.modelKey;
  };

  getModels = async () => {
    const promiseModels = await Promise.allSettled([
      this.anthropicProvider.getModels(),
      this.ollamaProvider.getModels(),
    ]);

    const fulfilledModels = promiseModels.filter(
      (model) => model.status === "fulfilled"
    );

    const models: Model[] = [];

    fulfilledModels.forEach((model) => {
      models.push(...model.value);
    });

    return models;
  };

  updateCurrentProvider = (
    modelKey?: string,
    tools?: TMCPItem[],
    prevMessages?: ThreadMessageLike[]
  ) => {
    if (!this.currentProvider) return;

    this.currentProvider.setSystemPrompt(SYSTEM_PROMPT);
    if (modelKey) this.currentProvider.setModelKey(modelKey);
    if (tools) this.currentProvider.setTools(tools);
    if (prevMessages) this.currentProvider.setPrevMessages(prevMessages);
  };

  sendMessage = (
    messages: ThreadMessageLike[]
  ): SendMessageReturnType | undefined => {
    if (!this.currentProvider) return;

    return this.currentProvider.sendMessage(messages);
  };

  sendMessageAfterToolCall = (
    message: ThreadMessageLike
  ): SendMessageReturnType | undefined => {
    if (!this.currentProvider) return;

    return this.currentProvider.sendMessageAfterToolCall(message);
  };

  stopMessage = () => {
    if (!this.currentProvider) return;

    this.currentProvider.stopMessage();
  };
}

const provider = new Provider();

export { provider };
