import type { ThreadMessageLike } from "@assistant-ui/react";
import { chatDB } from "./index";

interface Thread {
  threadId: string;
  title?: string;
  messages: ThreadMessageLike[];
  createdAt: number;
  updatedAt: number;
}

// Create/Save thread
export const saveThread = async (
  thread: Omit<Thread, "createdAt" | "updatedAt">
): Promise<void> => {
  const db = chatDB.getDB();
  
  return new Promise((resolve, reject) => {
    const transaction = db.transaction(["threads"], "readwrite");
    const store = transaction.objectStore("threads");

    const request = store.put({
      ...thread,
      createdAt: Date.now(),
      updatedAt: Date.now(),
    });

    request.onerror = () => reject(request.error);
    request.onsuccess = () => resolve();
  });
};

// Read thread
export const getThread = async (threadId: string): Promise<Thread | null> => {
  const db = chatDB.getDB();

  return new Promise((resolve, reject) => {
    const transaction = db.transaction(["threads"], "readonly");
    const store = transaction.objectStore("threads");
    const request = store.get(threadId);

    request.onerror = () => reject(request.error);
    request.onsuccess = () => resolve(request.result || null);
  });
};

// Read all threads
export const getAllThreads = async (): Promise<Thread[]> => {
  const db = chatDB.getDB();

  return new Promise((resolve, reject) => {
    const transaction = db.transaction(["threads"], "readonly");
    const store = transaction.objectStore("threads");
    const index = store.index("updatedAt");
    const request = index.getAll();

    request.onerror = () => reject(request.error);
    request.onsuccess = () => {
      const threads = request.result.sort(
        (a, b) => b.updatedAt - a.updatedAt
      );
      resolve(threads);
    };
  });
};

// Update thread
export const updateThread = async (
  threadId: string,
  updates: Partial<Omit<Thread, "threadId" | "createdAt">>
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
        ...updates,
        updatedAt: Date.now(),
      };

      const putRequest = store.put(updatedThread);
      putRequest.onerror = () => reject(putRequest.error);
      putRequest.onsuccess = () => resolve();
    };
  });
};

// Delete thread
export const deleteThread = async (threadId: string): Promise<void> => {
  const db = chatDB.getDB();

  return new Promise((resolve, reject) => {
    const transaction = db.transaction(["threads"], "readwrite");
    const store = transaction.objectStore("threads");
    const request = store.delete(threadId);

    request.onerror = () => reject(request.error);
    request.onsuccess = () => resolve();
  });
};