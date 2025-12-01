import type { ThreadMessageLike } from "@assistant-ui/react";

import { chatDB } from "./index";

interface Message {
  id: string;
  threadId: string;
  message: ThreadMessageLike;
  timestamp: number;
}

// Create message
export const createMessage = async (
  threadId: string,
  id: string,
  message: ThreadMessageLike
): Promise<void> => {
  const db = chatDB.getDB();
  const messageData: Message = {
    id,
    threadId,
    message,
    timestamp: Date.now(),
  };

  return new Promise((resolve, reject) => {
    const transaction = db.transaction(["messages"], "readwrite");
    const store = transaction.objectStore("messages");
    const request = store.put(messageData);

    request.onerror = () => reject(request.error);
    request.onsuccess = () => resolve();
  });
};

// Read messages
export const readMessages = async (
  threadId: string,
  limit?: number
): Promise<ThreadMessageLike[]> => {
  const db = chatDB.getDB();

  return new Promise((resolve, reject) => {
    const transaction = db.transaction(["messages"], "readonly");
    const store = transaction.objectStore("messages");
    const index = store.index("threadId");
    const request = index.getAll(IDBKeyRange.only(threadId));

    request.onerror = () => reject(request.error);
    request.onsuccess = () => {
      let messages = request.result
        .sort((a, b) => a.timestamp - b.timestamp)
        .map((item) => item.message);

      if (limit) {
        messages = messages.slice(-limit);
      }

      resolve(messages);
    };
  });
};

// Read single message by thread ID and message ID
export const readMessageById = async (
  threadId: string,
  messageId: string
): Promise<ThreadMessageLike | null> => {
  const db = chatDB.getDB();

  return new Promise((resolve, reject) => {
    const transaction = db.transaction(["messages"], "readonly");
    const store = transaction.objectStore("messages");
    const request = store.get(messageId);

    request.onerror = () => reject(request.error);
    request.onsuccess = () => {
      const result = request.result;
      
      // Verify the message belongs to the specified thread
      if (result && result.threadId === threadId) {
        resolve(result.message);
      } else {
        resolve(null);
      }
    };
  });
};

// Update message
export const updateMessage = async (
  messageId: string,
  updatedMessage: ThreadMessageLike
): Promise<void> => {
  const db = chatDB.getDB();

  return new Promise((resolve, reject) => {
    const transaction = db.transaction(["messages"], "readwrite");
    const store = transaction.objectStore("messages");

    // First get the existing message
    const getRequest = store.get(messageId);

    getRequest.onerror = () => reject(getRequest.error);
    getRequest.onsuccess = () => {
      const existingMessage = getRequest.result;
      if (!existingMessage) {
        reject(new Error("Message not found"));
        return;
      }

      // Update the message
      const updatedData: Message = {
        ...existingMessage,
        message: updatedMessage,
        timestamp: Date.now(),
      };

      const putRequest = store.put(updatedData);
      putRequest.onerror = () => reject(putRequest.error);
      putRequest.onsuccess = () => resolve();
    };
  });
};

// Delete message
export const deleteMessage = async (messageId: string): Promise<void> => {
  const db = chatDB.getDB();

  return new Promise((resolve, reject) => {
    const transaction = db.transaction(["messages"], "readwrite");
    const store = transaction.objectStore("messages");
    const request = store.delete(messageId);

    request.onerror = () => reject(request.error);
    request.onsuccess = () => resolve();
  });
};

// Delete all messages in a thread
export const deleteMessagesInThread = async (
  threadId: string
): Promise<void> => {
  const db = chatDB.getDB();

  return new Promise((resolve, reject) => {
    const transaction = db.transaction(["messages"], "readwrite");
    const store = transaction.objectStore("messages");
    const index = store.index("threadId");
    const request = index.openCursor(IDBKeyRange.only(threadId));

    request.onsuccess = (event) => {
      const cursor = (event.target as IDBRequest).result;
      if (cursor) {
        cursor.delete();
        cursor.continue();
      } else {
        resolve();
      }
    };

    request.onerror = () => reject(request.error);
  });
};

// Replace all messages in a thread
export const replaceThreadMessages = async (
  threadId: string,
  messages: ThreadMessageLike[]
): Promise<void> => {
  const db = chatDB.getDB();

  return new Promise((resolve, reject) => {
    const transaction = db.transaction(["messages"], "readwrite");
    const store = transaction.objectStore("messages");
    const index = store.index("threadId");

    // Clear existing messages for this thread
    const deleteRequest = index.openCursor(IDBKeyRange.only(threadId));
    deleteRequest.onsuccess = (event) => {
      const cursor = (event.target as IDBRequest).result;
      if (cursor) {
        cursor.delete();
        cursor.continue();
      } else {
        // Add new messages
        messages.forEach((message, index) => {
          const messageData: Message = {
            id: `${threadId}-${Date.now()}-${index}`,
            threadId,
            message,
            timestamp: Date.now() + index,
          };
          store.put(messageData);
        });
      }
    };

    transaction.onerror = () => reject(transaction.error);
    transaction.oncomplete = () => resolve();
  });
};

// Search messages
export const searchMessages = async (
  query: string
): Promise<{ threadId: string; message: ThreadMessageLike }[]> => {
  const db = chatDB.getDB();

  return new Promise((resolve, reject) => {
    const transaction = db.transaction(["messages"], "readonly");
    const store = transaction.objectStore("messages");
    const request = store.getAll();

    request.onerror = () => reject(request.error);
    request.onsuccess = () => {
      const results = request.result.filter((item) => {
        const content = Array.isArray(item.message.content)
          ? item.message.content
          : [{ type: "text", text: item.message.content }];

        return content.some(
          (part: { type: string; text: string }) =>
            part.type === "text" &&
            part.text.toLowerCase().includes(query.toLowerCase())
        );
      });

      resolve(
        results.map((item) => ({
          threadId: item.threadId,
          message: item.message,
        }))
      );
    };
  });
};
