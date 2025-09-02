import { create } from "zustand";
import type { ThreadMessageLike } from "@assistant-ui/react";

import type { ProviderType, TMCPItem } from "@/lib/types";
import { provider } from "@/providers";
import { readMessages } from "@/database/messages";

type UseMessageStoreProps = {
  messages: ThreadMessageLike[];
  isStreamRunning: boolean;
  setIsStreamRunning: (value: boolean) => void;
  addMessage: (message: ThreadMessageLike) => void;
  updateLastMessage: (message: ThreadMessageLike) => void;
  fetchPrevMessages: (threadId: string) => Promise<void>;
  getCurrentProviderModel: () => string | undefined;
  setCurrentProvider: (provider: ProviderType) => void;
  updateCurrentProvider: (
    modelKey?: string,
    tools?: TMCPItem[],
    messages?: ThreadMessageLike[]
  ) => void;
  stopMessage: () => void;
};

const useMessageStore = create<UseMessageStoreProps>((set, get) => ({
  messages: [],
  isStreamRunning: false,
  setIsStreamRunning: (value) => {
    set({ isStreamRunning: value });
  },
  addMessage: (message) => {
    set({ messages: [...get().messages, message] });
  },
  updateLastMessage: (message) => {
    set({ messages: [...get().messages.slice(0, -1), message] });
  },
  fetchPrevMessages: async (threadId: string) => {
    const messages = await readMessages(threadId);

    set({ messages });

    get().updateCurrentProvider(undefined, undefined, messages);
  },
  getCurrentProviderModel: () => provider.getCurrentProviderModel(),
  setCurrentProvider: (value) => {
    provider.setCurrentProvider(value);
  },
  updateCurrentProvider: (modelKey, tools, messages) => {
    provider.updateCurrentProvider(modelKey, tools, messages);
  },
  stopMessage: () => {
    const thisStore = get();

    thisStore.setIsStreamRunning(false);
    provider.stopMessage();
  },
}));

export default useMessageStore;
