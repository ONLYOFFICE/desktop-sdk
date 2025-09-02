import { create } from "zustand";

import {
  readAllThreads,
  createThread,
  touchThread,
  deleteThread,
} from "@/database/threads";
import type { Thread } from "@/lib/types";

type UseThreadsStoreProps = {
  threadId: string;
  threads: Thread[];

  initThreads: () => Promise<void>;
  insertThread: (title: string) => void;
  insertNewMessageToThread: () => void;
  onSwitchToNewThread: () => void;
  onSwitchToThread: (id: string) => void;
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
