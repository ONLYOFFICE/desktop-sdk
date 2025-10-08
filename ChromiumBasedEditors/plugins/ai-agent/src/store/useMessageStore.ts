import { create } from "zustand";
import type { ThreadMessageLike } from "@assistant-ui/react";

import { provider } from "@/providers";
import { readMessages } from "@/database/messages";

type UseMessageStoreProps = {
  messages: ThreadMessageLike[];
  isStreamRunning: boolean;
  isRequestRunning: boolean;
  attachmentFiles: { path: string; content: string }[];

  setIsStreamRunning: (value: boolean) => void;
  setIsRequestRunning: (value: boolean) => void;
  addMessage: (message: ThreadMessageLike) => void;
  updateLastMessage: (message: ThreadMessageLike) => void;
  fetchPrevMessages: (threadId: string) => Promise<void>;
  stopMessage: () => void;
};

const useMessageStore = create<UseMessageStoreProps>((set, get) => ({
  messages: [],
  isStreamRunning: false,
  isRequestRunning: false,
  attachmentFiles: [],
  fetchPrevMessages: async (threadId: string) => {
    const messages = await readMessages(threadId);

    set({ messages });

    provider.setCurrentProviderPrevMessages(messages);
  },
  setIsStreamRunning: (value) => {
    set({ isStreamRunning: value });
  },
  setIsRequestRunning: (value) => {
    set({ isRequestRunning: value });
  },
  addMessage: (message) => {
    const thisStore = get();

    if (
      thisStore.messages.length &&
      thisStore.messages[thisStore.messages.length - 1].status?.type ===
        "incomplete"
    ) {
      set({
        messages: [...thisStore.messages.slice(0, -1), { ...message }],
      });

      return;
    }

    set({ messages: [...thisStore.messages, message] });
  },
  updateLastMessage: (message) => {
    const thisStore = get();
    set({ messages: [...thisStore.messages.slice(0, -1), message] });
  },
  addAttachmentFile: (file: { path: string; content: string }) => {
    set({ attachmentFiles: [...get().attachmentFiles, file] });
  },
  clearAttachmentFiles: () => {
    set({ attachmentFiles: [] });
  },
  stopMessage: () => {
    const thisStore = get();

    thisStore.setIsStreamRunning(false);
    provider.stopMessage();
  },
}));

export default useMessageStore;
