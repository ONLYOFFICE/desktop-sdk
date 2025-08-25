import { useEffect, useState } from "react";

import type { Thread } from "@/lib/types";
import {
  createThread,
  deleteThread,
  readAllThreads,
  touchThread,
} from "@/database/threads";

type UseThreadProps = {
  isReady: boolean;
};

const useThread = ({ isReady }: UseThreadProps) => {
  const [threadId, setThreadId] = useState<string>(crypto.randomUUID());
  const [threads, setThreads] = useState<Thread[]>([]);

  useEffect(() => {
    const getAllThreads = () => {
      readAllThreads().then((threads) => {
        setThreads(threads);
      });
    };

    if (isReady) getAllThreads();
  }, [isReady]);

  const insertThread = (title: string) => {
    setThreads((currentThreads) => [{ threadId, title }, ...currentThreads]);

    createThread(threadId, title);
  };

  const insertNewMessageToThread = () => {
    touchThread(threadId);

    setThreads((prevThreads) => {
      const currentThread = prevThreads.find(
        (thread) => thread.threadId === threadId
      );
      if (!currentThread) return prevThreads;

      currentThread.lastEditDate = Date.now();
      return [
        currentThread,
        ...prevThreads.filter((t) => t.threadId !== threadId),
      ];
    });
  };

  const onSwitchToNewThread = () => {
    const uuid = crypto.randomUUID();

    setThreadId(uuid);
  };

  const onSwitchToThread = (id: string) => {
    if (id === threadId) return;

    setThreadId(id);
  };

  const onDeleteThread = (id: string) => {
    if (threadId === id) {
      onSwitchToNewThread();
    }
    setThreads((currentThreads) =>
      currentThreads.filter((t) => t.threadId !== id)
    );
    deleteThread(id);
  };

  return {
    threadId,
    threads,
    setThreadId,
    insertThread,
    insertNewMessageToThread,
    onSwitchToNewThread,
    onSwitchToThread,
    onDeleteThread,
  };
};

export default useThread;
