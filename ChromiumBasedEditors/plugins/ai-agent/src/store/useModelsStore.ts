import { create } from "zustand";

import type { Model } from "@/lib/types";
import { provider } from "@/providers";

const CURRENT_MODEL_KEY = "current-model";

type UseModelsStoreProps = {
  models: Model[];
  currentModel: Model | null;
  isInitLoading: boolean;

  initModels: () => Promise<void>;
  initCurrentModel: () => void;
  selectModel: (model: Model | null) => void;
};

const useModelsStore = create<UseModelsStoreProps>((set) => ({
  models: [],
  currentModel: null,
  isInitLoading: false,

  initModels: async () => {
    try {
      set({ isInitLoading: true });

      const models = await provider.getModels();

      set({ models, isInitLoading: false });
    } catch (error) {
      console.error("Failed to initialize models:", error);
    } finally {
      set({ isInitLoading: false });
    }
  },
  initCurrentModel: () => {
    try {
      const currentModel = localStorage.getItem(CURRENT_MODEL_KEY);

      if (!currentModel) return;

      const model = JSON.parse(currentModel);

      set({ currentModel: model });
    } catch (error) {
      console.error("Failed to initialize current model:", error);
    }
  },
  selectModel: (model) => {
    set({ currentModel: model });

    localStorage.setItem(CURRENT_MODEL_KEY, JSON.stringify(model));
  },
}));

export default useModelsStore;
