class ChatDB {
  private dbName = "ChatHistory";
  private version = 1;
  private db: IDBDatabase | null = null;

  async init(): Promise<void> {
    return new Promise((resolve, reject) => {
      const request = indexedDB.open(this.dbName, this.version);

      request.onerror = () => reject(request.error);
      request.onsuccess = () => {
        this.db = request.result;

        resolve();
      };

      request.onupgradeneeded = (event) => {
        const db = (event.target as IDBOpenDBRequest).result;

        // Create threads store
        if (!db.objectStoreNames.contains("threads")) {
          const threadsStore = db.createObjectStore("threads", {
            keyPath: "threadId",
          });
          threadsStore.createIndex("updatedAt", "updatedAt", {
            unique: false,
          });
        }

        // Create messages store
        if (!db.objectStoreNames.contains("messages")) {
          const messagesStore = db.createObjectStore("messages", {
            keyPath: "id",
          });
          messagesStore.createIndex("threadId", "threadId", { unique: false });
          messagesStore.createIndex("timestamp", "timestamp", {
            unique: false,
          });
        }
      };
    });
  }

  getDB(): IDBDatabase {
    if (!this.db) throw new Error("Database not initialized");
    return this.db;
  }

  async close(): Promise<void> {
    if (this.db) {
      this.db.close();
      this.db = null;
    }
  }
}

// Export singleton instance
export const chatDB = new ChatDB();

// Initialize function
export const initChatDB = () => chatDB.init();
