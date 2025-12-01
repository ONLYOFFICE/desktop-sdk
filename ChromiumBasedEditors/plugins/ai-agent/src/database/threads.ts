import type { Thread } from "@/lib/types";

import { chatDB } from "./index";
import { deleteMessagesInThread } from "./messages";

// Create thread
export const createThread = async (
  threadId: string,
  title: string
): Promise<void> => {
  const db = chatDB.getDB();
  const threadData: Thread = {
    threadId,
    title,
    lastEditDate: Date.now(),
  };

  return new Promise((resolve, reject) => {
    const transaction = db.transaction(["threads"], "readwrite");
    const store = transaction.objectStore("threads");
    const request = store.put(threadData);

    request.onerror = () => reject(request.error);
    request.onsuccess = () => resolve();
  });
};

// Read single thread
export const readThread = async (threadId: string): Promise<Thread | null> => {
  const db = chatDB.getDB();

  return new Promise((resolve, reject) => {
    const transaction = db.transaction(["threads"], "readonly");
    const store = transaction.objectStore("threads");
    const request = store.get(threadId);

    request.onerror = () => reject(request.error);
    request.onsuccess = () => resolve(request.result || null);
  });
};

// Read all threads sorted by lastEditDate (latest at end)
export const readAllThreads = async (): Promise<Thread[]> => {
  const db = chatDB.getDB();

  return new Promise((resolve, reject) => {
    const transaction = db.transaction(["threads"], "readonly");
    const store = transaction.objectStore("threads");
    const request = store.getAll();

    request.onerror = () => reject(request.error);
    request.onsuccess = () => {
      const threads = request.result.sort(
        (a, b) => b.lastEditDate - a.lastEditDate
      );
      resolve(threads);
    };
  });
};

// Update thread
export const updateThread = async (
  threadId: string,
  title?: string
): Promise<void> => {
  const db = chatDB.getDB();

  return new Promise((resolve, reject) => {
    const transaction = db.transaction(["threads"], "readwrite");
    const store = transaction.objectStore("threads");

    // First get the existing thread
    const getRequest = store.get(threadId);

    getRequest.onerror = () => reject(getRequest.error);
    getRequest.onsuccess = () => {
      const existingThread = getRequest.result;
      if (!existingThread) {
        reject(new Error("Thread not found"));
        return;
      }

      // Update the thread
      const updatedThread: Thread = {
        ...existingThread,
        ...(title && { title }),
        lastEditDate: Date.now(),
      };

      const putRequest = store.put(updatedThread);
      putRequest.onerror = () => reject(putRequest.error);
      putRequest.onsuccess = () => resolve();
    };
  });
};

// Update thread's lastEditDate (for when messages are added)
export const touchThread = async (threadId: string): Promise<void> => {
  const db = chatDB.getDB();

  return new Promise((resolve, reject) => {
    const transaction = db.transaction(["threads"], "readwrite");
    const store = transaction.objectStore("threads");

    const getRequest = store.get(threadId);

    getRequest.onerror = () => reject(getRequest.error);
    getRequest.onsuccess = () => {
      const existingThread = getRequest.result;
      if (!existingThread) {
        reject(new Error("Thread not found"));
        return;
      }

      const updatedThread: Thread = {
        ...existingThread,
        lastEditDate: Date.now(),
      };

      const putRequest = store.put(updatedThread);
      putRequest.onerror = () => reject(putRequest.error);
      putRequest.onsuccess = () => resolve();
    };
  });
};

// Delete thread
export const deleteThread = async (threadId: string): Promise<void> => {
  // First delete all messages associated with this thread
  await deleteMessagesInThread(threadId);

  // Then delete the thread itself
  const db = chatDB.getDB();

  return new Promise((resolve, reject) => {
    const transaction = db.transaction(["threads"], "readwrite");
    const store = transaction.objectStore("threads");
    const request = store.delete(threadId);

    request.onerror = () => reject(request.error);
    request.onsuccess = () => resolve();
  });
};
