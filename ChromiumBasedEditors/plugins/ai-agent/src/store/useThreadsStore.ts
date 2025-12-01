import { create } from "zustand";

import {
  readAllThreads,
  createThread,
  touchThread,
  updateThread,
  deleteThread,
} from "@/database/threads";
import { readMessages } from "@/database/messages";
import type { Thread } from "@/lib/types";
import { convertMessagesToMd, removeSpecialCharacter } from "@/lib/utils";

type UseThreadsStoreProps = {
  threadId: string;
  threads: Thread[];

  initThreads: () => Promise<void>;
  insertThread: (title: string) => void;
  insertNewMessageToThread: () => void;
  onSwitchToNewThread: () => void;
  onSwitchToThread: (id: string) => void;
  onDownloadThread: (id: string) => void;
  onRenameThread: (id: string, title: string) => void;
  onDeleteThread: (id: string) => void;
};

const useThreadsStore = create<UseThreadsStoreProps>((set, get) => ({
  threadId: crypto.randomUUID(),
  threads: [],

  initThreads: async () => {
    const threads = await readAllThreads();

    set({ threads });
  },
  insertThread: (title: string) => {
    const thisStore = get();

    set({
      threads: [{ threadId: thisStore.threadId, title }, ...thisStore.threads],
    });

    createThread(thisStore.threadId, title);
  },
  insertNewMessageToThread: () => {
    const thisStore = get();

    touchThread(thisStore.threadId);

    set({
      threads: thisStore.threads.map((thread) => {
        if (thread.threadId === thisStore.threadId) {
          return {
            ...thread,
            lastEditDate: Date.now(),
          };
        }
        return thread;
      }),
    });
  },
  onSwitchToNewThread: () => {
    set({ threadId: crypto.randomUUID() });
  },
  onSwitchToThread: (id: string) => {
    set({ threadId: id });
  },
  onDownloadThread: async (id: string) => {
    const thisStore = get();
    const thread = thisStore.threads.find((t) => t.threadId === id);
    const messages = await readMessages(id);

    const title = removeSpecialCharacter(thread?.title || "Chat Export");

    const content = convertMessagesToMd(messages);

    window.AscDesktopEditor.SaveFilenameDialog(`${title}.docx`, (path) => {
      if (!path) return;

      window.AscDesktopEditor.saveAndOpen(content, 0x5c, path, 0x41, (code) => {
        if (!code) console.log("Conversion error");
      });
    });
  },
  onRenameThread: (id: string, title: string) => {
    const thisStore = get();

    set({
      threads: thisStore.threads.map((thread) => {
        if (thread.threadId === id) {
          return {
            ...thread,
            title,
          };
        }
        return thread;
      }),
    });

    updateThread(id, title);
  },
  onDeleteThread: (id: string) => {
    const thisStore = get();

    if (thisStore.threadId === id) {
      thisStore.onSwitchToNewThread();
    }
    set({ threads: thisStore.threads.filter((t) => t.threadId !== id) });
    deleteThread(id);
  },
}));

export default useThreadsStore;
