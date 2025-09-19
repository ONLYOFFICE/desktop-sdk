import { create } from "zustand";

import type { Model } from "@/lib/types";
import { CURRENT_MODEL_KEY } from "@/lib/constants";
import { provider } from "@/providers";

type UseModelsStoreProps = {
  currentModel: Model | null;
  isInitLoading: boolean;

  initCurrentModel: () => void;
  selectModel: (model: Model) => void;
  deleteSelectedModel: () => void;
};

const useModelsStore = create<UseModelsStoreProps>((set) => ({
  currentModel: null,
  isInitLoading: false,

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
    provider.setCurrentProviderModel(model.id);
    localStorage.setItem(CURRENT_MODEL_KEY, JSON.stringify(model));
  },

  deleteSelectedModel: () => {
    set({ currentModel: null });
    localStorage.removeItem(CURRENT_MODEL_KEY);
    provider.setCurrentProviderModel("");
  },
}));

export default useModelsStore;
