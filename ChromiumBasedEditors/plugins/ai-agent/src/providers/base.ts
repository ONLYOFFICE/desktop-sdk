import type { ThreadMessageLike } from "@assistant-ui/react";

import type { TMCPItem, TProvider } from "@/lib/types";

export interface BaseProvider<TOOL, MESSAGE, CLIENT> {
  client?: CLIENT;
  tools: TOOL[];
  prevMessages: MESSAGE[];

  provider?: TProvider;

  modelKey: string;
  systemPrompt: string;
  apiKey?: string;
  url?: string;

  setProvider(provider: TProvider): void;

  setModelKey(modelKey: string): void;

  setSystemPrompt(systemPrompt: string): void;

  setAPIKey?(apiKey: string): void;

  setURL?(url: string): void;

  setPrevMessages(prevMessages: ThreadMessageLike[]): void;

  setTools(tools: TMCPItem[]): void;

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
