export type TMCPItem = {
  name: string;
  description: string;
  inputSchema: Record<string, unknown>;
  // MCP ToolAnnotations: readOnlyHint, destructiveHint, ...
  annotations?: Record<string, unknown>;
  enabled?: boolean;
};

export type Thread = {
  threadId: string;
  title?: string;
  lastEditDate?: number;
  provider?: TProvider;
  model?: Model;
};

export type ProviderType =
  | "anthropic"
  | "ollama"
  | "openai"
  | "openaicompatible"
  | "together"
  | "openrouter"
  | "genai"
  | "deepseek"
  | "xai"
  | "lm-studio"
  | "mistral";

export type Model = {
  id: string;
  name: string;
  provider: ProviderType;
  reasoning?: boolean;
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
};

export type TAttachmentImage = {
  name: string;
  base64: string;
};

export type TProcess = {
  stdin: (data: string) => void;
  onprocess: (type: number, message: string) => void;
  end: () => void;
  start: () => void;
};
