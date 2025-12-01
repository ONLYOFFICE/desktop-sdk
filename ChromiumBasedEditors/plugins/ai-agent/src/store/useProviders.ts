import { create } from "zustand";

import type { TProvider, Model } from "@/lib/types";
import {
  PROVIDERS_LOCAL_STORAGE_KEY,
  CURRENT_PROVIDER_KEY,
} from "@/lib/constants";
import { provider } from "@/providers";
import type { TErrorData } from "@/providers/settings";

interface ProvidersState {
  providers: TProvider[];
  currentProvider: TProvider | null;
  providersModels: Map<string, Model[]>;
  fetchProvidersModels: () => Promise<void>;
  setCurrentProvider: (providerInfo: TProvider) => void;
  addProvider: (
    providerInfo: TProvider
  ) => Promise<boolean | TErrorData | undefined>;
  editProvider: (
    providerInfo: TProvider,
    prevName: string
  ) => Promise<boolean | TErrorData | undefined>;
  deleteProvider: (providerInfo: TProvider) => Promise<void>;
}

const useProviders = create<ProvidersState>()((set, get) => ({
  providers: (() => {
    const saved = localStorage.getItem(PROVIDERS_LOCAL_STORAGE_KEY);
    return saved ? JSON.parse(saved) : [];
  })(),
  currentProvider: (() => {
    const saved = localStorage.getItem(CURRENT_PROVIDER_KEY);
    if (!saved) return null;
    const parsed: TProvider = JSON.parse(saved);

    // Since checkNewProvider is async, we need to handle this differently
    // For now, just set the provider and validate it asynchronously
    provider.setCurrentProvider(parsed);

    // Validate the provider asynchronously
    const validationResult = provider.checkNewProvider(parsed.type, {
      url: parsed.baseUrl,
      apiKey: parsed.key,
    });

    // Handle both sync (false) and async (Promise) results
    if (validationResult instanceof Promise) {
      validationResult
        .then((result: boolean | TErrorData) => {
          if (typeof result !== "boolean" || !result) {
            localStorage.removeItem(CURRENT_PROVIDER_KEY);
          }
        })
        .catch((error: unknown) => {
          console.error("Provider validation error:", error);
          localStorage.removeItem(CURRENT_PROVIDER_KEY);
        });
    } else if (!validationResult) {
      localStorage.removeItem(CURRENT_PROVIDER_KEY);
      return null;
    }

    return parsed;
  })(),
  providersModels: new Map<string, Model[]>(),

  fetchProvidersModels: async () => {
    const providers = get().providers;
    const models = await provider.getProvidersModels(providers);

    set({ providersModels: models });
  },
  setCurrentProvider: (providerInfo: TProvider) => {
    set({ currentProvider: providerInfo });
    provider.setCurrentProvider(providerInfo);
    localStorage.setItem(CURRENT_PROVIDER_KEY, JSON.stringify(providerInfo));
  },

  addProvider: async (providerInfo: TProvider) => {
    // Check for duplicate name
    const currentProviders = get().providers;
    const nameExists = currentProviders.some(
      (p) => p.name.toLowerCase() === providerInfo.name.toLowerCase()
    );

    if (nameExists) {
      return {
        field: "name" as const,
        message: "Duplicate name",
      };
    }

    const checkResult = await provider.checkNewProvider(providerInfo.type, {
      url: providerInfo.baseUrl,
      apiKey: providerInfo.key,
    });

    if (typeof checkResult === "boolean" && checkResult) {
      set((state) => {
        const newProviders = [...state.providers, providerInfo];
        localStorage.setItem(
          PROVIDERS_LOCAL_STORAGE_KEY,
          JSON.stringify(newProviders)
        );
        return { providers: newProviders };
      });
      return true;
    } else {
      return checkResult;
    }
  },

  editProvider: async (providerInfo: TProvider, prevName: string) => {
    // Check for duplicate name (excluding the current provider being edited)
    const currentProviders = get().providers;
    const nameExists = currentProviders.some(
      (p) =>
        p.name.toLowerCase() === providerInfo.name.toLowerCase() &&
        p.baseUrl !== providerInfo.baseUrl &&
        p.key !== providerInfo.key &&
        p.type !== providerInfo.type
    );

    if (nameExists) {
      return {
        field: "name" as const,
        message: "Duplicate name",
      };
    }

    const checkResult = await provider.checkNewProvider(providerInfo.type, {
      url: providerInfo.baseUrl,
      apiKey: providerInfo.key,
    });

    if (typeof checkResult === "boolean" && checkResult) {
      set((state) => {
        const newProviders = state.providers.map((p) =>
          p.name === prevName ? providerInfo : p
        );
        localStorage.setItem(
          PROVIDERS_LOCAL_STORAGE_KEY,
          JSON.stringify(newProviders)
        );
        return { providers: newProviders };
      });
      return true;
    } else {
      return checkResult;
    }
  },
  deleteProvider: async (providerInfo: TProvider) => {
    set((state) => {
      const newProviders = state.providers.filter(
        (p) => p.name !== providerInfo.name
      );

      if (state.currentProvider?.name === providerInfo.name) {
        state.currentProvider = null;
        localStorage.removeItem(CURRENT_PROVIDER_KEY);
        provider.setCurrentProvider();
      }

      localStorage.setItem(
        PROVIDERS_LOCAL_STORAGE_KEY,
        JSON.stringify(newProviders)
      );
      return { providers: newProviders };
    });
  },
}));

export default useProviders;
