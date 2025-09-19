import type { ThreadMessageLike } from "@assistant-ui/react";

import type { Model, ProviderType, TMCPItem, TProvider } from "@/lib/types";

import type { TData } from "./settings";

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
  currentProviderInfo?: TProvider;
  currentProviderType?: ProviderType;

  anthropicProvider: AnthropicProvider;
  ollamaProvider: OllamaProvider;

  constructor() {
    this.anthropicProvider = anthropicProvider;
    this.ollamaProvider = ollamaProvider;
  }

  setCurrentProvider = (provider?: TProvider) => {
    if (!provider) {
      this.currentProvider = undefined;
      this.currentProviderInfo = undefined;
      this.currentProviderType = undefined;
      return;
    }

    this.currentProviderInfo = provider;

    switch (provider.type) {
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

    if (this.currentProvider) {
      this.currentProvider.setProvider(provider);
      this.currentProvider.setSystemPrompt(SYSTEM_PROMPT);
    }
  };

  setCurrentProviderModel = (modelKey: string) => {
    if (!this.currentProvider) return;

    this.currentProvider.setModelKey(modelKey);
  };

  setCurrentProviderTools = (tools: TMCPItem[]) => {
    if (!this.currentProvider) return;

    this.currentProvider.setTools(tools);
  };

  setCurrentProviderPrevMessages = (prevMessages: ThreadMessageLike[]) => {
    if (!this.currentProvider) return;

    this.currentProvider.setPrevMessages(prevMessages);
  };

  getCurrentProviderModel = () => {
    if (!this.currentProvider) return;

    return this.currentProvider.modelKey;
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

  getProvidersInfo = () => {
    const anthropic = {
      type: "anthropic" as ProviderType,
      name: this.anthropicProvider.getName(),
      baseUrl: this.anthropicProvider.getBaseUrl(),
    };

    const ollama = {
      type: "ollama" as ProviderType,
      name: this.ollamaProvider.getName(),
      baseUrl: this.ollamaProvider.getBaseUrl(),
    };

    return [anthropic, ollama];
  };

  getProviderInfo = (type: ProviderType) => {
    if (type === "anthropic")
      return {
        type,
        name: this.anthropicProvider.getName(),
        baseUrl: this.anthropicProvider.getBaseUrl(),
      };

    if (type === "ollama")
      return {
        type,
        name: this.ollamaProvider.getName(),
        baseUrl: this.ollamaProvider.getBaseUrl(),
      };

    return {
      name: "",
      baseUrl: "",
    };
  };

  checkNewProvider = (type: ProviderType, data: TData) => {
    if (type === "anthropic") return this.anthropicProvider.checkProvider(data);

    if (type === "ollama") return this.ollamaProvider.checkProvider(data);

    return false;
  };

  getProvidersModels = async (providers: TProvider[]) => {
    const models = new Map<string, Model[]>();

    const actions = providers
      .map((p) => {
        if (p.type === "anthropic")
          return this.anthropicProvider.getProviderModels({
            url: p.baseUrl,
            apiKey: p.key,
          });

        if (p.type === "ollama")
          return this.ollamaProvider.getProviderModels({
            url: p.baseUrl,
            apiKey: p.key,
          });

        return null; // Explicitly return null for unsupported types
      })
      .filter((action): action is Promise<Model[]> => action !== null); // Filter out null values

    const fetchedModels = await Promise.allSettled(actions);

    let actionIndex = 0;
    providers.forEach((provider) => {
      // Only process providers that have supported types
      if (provider.type === "anthropic" || provider.type === "ollama") {
        const model = fetchedModels[actionIndex];
        if (
          model.status === "fulfilled" &&
          model.value &&
          model.value.length > 0
        ) {
          models.set(provider.name, model.value);
        }
        actionIndex++;
      }
    });

    return models;
  };
}

const provider = new Provider();

export { provider };
