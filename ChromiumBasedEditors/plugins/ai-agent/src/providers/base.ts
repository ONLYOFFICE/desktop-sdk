import type { ThreadMessageLike } from "@assistant-ui/react";
import type { TMCPItem, Model } from "@/lib/types";

export interface BaseProvider<TOOLS, MESSAGES, CLIENT> {
  modelKey: string;
  apiKey: string;
  url?: string;
  system?: string;
  prevMessages: MESSAGES[];
  tools: TOOLS[];
  client: CLIENT;

  setModelKey(modelKey: string): void;

  setApiKey(apiKey: string): void;

  setUrl(url: string): void;

  setSystem(system: string): void;

  setPrevMessages(prevMessages: MESSAGES[]): void;

  setTools(tools: TOOLS[]): void;

  convertToolsToModelFormat(tools: TMCPItem[]): TOOLS[];

  convertMessagesToModelFormat(messages: ThreadMessageLike[]): MESSAGES[];

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
}
