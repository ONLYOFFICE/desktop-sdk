import type { ThreadMessageLike } from "@assistant-ui/react";

import type { TMCPItem, Model } from "@/lib/types";

export interface BaseProvider<TOOL, MESSAGE, CLIENT> {
  client: CLIENT;

  tools: TOOL[];

  prevMessages: MESSAGE[];

  modelKey: string;
  systemPrompt: string;
  apiKey?: string;
  url?: string;

  setModelKey(modelKey: string): void;

  setSystemPrompt(systemPrompt: string): void;

  setAPIKey?(apiKey: string): void;

  setURL?(url: string): void;

  setPrevMessages(prevMessages: ThreadMessageLike[]): void;

  setTools(tools: TMCPItem[]): void;

  getModels(): Promise<Model[]>;

  sendMessage(
    messages: ThreadMessageLike[],
    afterToolCall?: boolean
  ): AsyncGenerator<
    ThreadMessageLike | { isEnd: true; responseMessage: ThreadMessageLike }
  >;

  sendMessageAfterToolCall(
    message: ThreadMessageLike
  ):
    | AsyncGenerator<
        ThreadMessageLike | { isEnd: true; responseMessage: ThreadMessageLike }
      >
    | ThreadMessageLike;

  stopMessage(): void;
}
