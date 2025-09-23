export type TMCPItem = {
  name: string;
  description: string;
  inputSchema: Record<string, unknown>;
  enabled?: boolean;
};

export type Thread = {
  threadId: string;
  title?: string;
  lastEditDate?: number;
};

export type ProviderType = "anthropic" | "ollama" | "openai" | "together";

export type Model = {
  id: string;
  name: string;
  provider: ProviderType;
};

export type TProvider = {
  type: ProviderType;
  name: string;
  key?: string;
  baseUrl: string;
};

export type TAttachmentFile = {
  path: string;
  content: string;
  type: number;
  isImage?: boolean;
};

export type TProcess = {
  onprocess: (type: number, message: string) => void;
  end: () => void;
  start: () => void;
};
