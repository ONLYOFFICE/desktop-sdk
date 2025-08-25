export type TMCPItem = {
  name: string;
  description: string;
  inputSchema: Record<string, unknown>;
};

export type Thread = {
  threadId: string;
  title?: string;
  lastEditDate?: number;
};

export type Model = {
  id: string;
  name: string;
  provider: "anthropic";
};
