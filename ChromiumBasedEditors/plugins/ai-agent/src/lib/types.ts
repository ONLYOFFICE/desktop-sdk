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

export type ProviderType = "anthropic" | "ollama";

export type Model = {
  id: string;
  name: string;
  provider: ProviderType;
};
