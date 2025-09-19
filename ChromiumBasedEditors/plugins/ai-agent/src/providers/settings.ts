import type { Model } from "@/lib/types";

export type TData = {
  url: string;
  apiKey?: string;
};

export type TErrorData = {
  field: "key" | "url" | "name";
  message: string;
};

export interface SettingsProvider {
  getName(): string;
  getBaseUrl(): string;
  checkProvider(data: TData): Promise<boolean | TErrorData>;
  getProviderModels(data: TData): Promise<Model[]>;
}
