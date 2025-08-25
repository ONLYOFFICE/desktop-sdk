import type { ThreadMessageLike } from "@assistant-ui/react";

import type { Model, TMCPItem } from "@/lib/types";

import { anthropicProvider, AnthropicProvider } from "./anthropic";

type ProviderType = AnthropicProvider;

export type SendMessageReturnType = ReturnType<ProviderType["sendMessage"]>;

class Provider {
  currentProvider: AnthropicProvider;

  system: string;
  tools: TMCPItem[];

  currentModel: Model;
  models: Model[];

  modelsLoading: boolean = true;

  constructor() {
    this.currentProvider = anthropicProvider;
    this.system = "";
    this.tools = [];

    this.currentModel = {
      id: "claude-3-7-sonnet-20250219",
      name: "Claude Sonnet 3.7",
      provider: "anthropic",
    };
    this.models = [];

    this.getModels();
  }

  setSystem = (system: string) => {
    this.system = system;
  };

  setTools = (tools: TMCPItem[]) => {
    this.tools = tools;

    this.updateProvider();
  };

  updateProvider = () => {
    this.currentProvider.setSystem(this.system);

    const tools = this.currentProvider.convertToolsToModelFormat(this.tools);

    this.currentProvider.setTools(tools);

    this.currentProvider.setModelKey(this.currentModel.id);
  };

  selectModel = (modelId: string) => {
    this.currentModel = this.models.find((model) => model.id === modelId) || {
      id: "claude-3-7-sonnet-20250219",
      name: "Claude Sonnet 3.7",
      provider: "anthropic",
    };

    console.log(this.currentModel.id);

    this.updateProvider();
  };

  getModels = async () => {
    this.modelsLoading = true;

    const models = await this.currentProvider.getModels();

    this.models = models;

    this.modelsLoading = false;

    return models;
  };

  setAnthropicProvider = () => {
    this.currentProvider = anthropicProvider;
  };

  sendMessage = (messages: ThreadMessageLike[]) => {
    return this.currentProvider.sendMessage(messages);
  };

  sendMessageAfterToolCall = (message: ThreadMessageLike) => {
    return this.currentProvider.sendMessageAfterToolCall(message);
  };
}

const provider = new Provider();

export { provider };
