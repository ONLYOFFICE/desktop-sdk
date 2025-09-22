import OpenAI from "openai";
import type {
  ChatCompletionMessageParam,
  ChatCompletionTool,
} from "openai/resources/chat/completions";

import type { Model, TProvider } from "@/lib/types";

import type { BaseProvider } from "../base";
import type { SettingsProvider, TData } from "../settings";

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

  constructor() {
    const client = new OpenAI({});

    client.chat.completions.create();
  }

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
    const newClient = new OpenAI({
      baseURL: data.url,
      apiKey: data.apiKey,
    });

    const response: ListResponse = await newClient.models.list();

    return response.models.map((model) => ({
      id: model.model,
      name: model.name,
      provider: "ollama" as const,
    }));
  };
}

const openaiProvider = new OpenAIProvider();

export { OpenAIProvider, openaiProvider };
