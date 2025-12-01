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

export type ProviderType =
  | "anthropic"
  | "ollama"
  | "openai"
  | "together"
  | "openrouter";

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
  stdin: (data: string) => void;
  onprocess: (type: number, message: string) => void;
  end: () => void;
  start: () => void;
};
