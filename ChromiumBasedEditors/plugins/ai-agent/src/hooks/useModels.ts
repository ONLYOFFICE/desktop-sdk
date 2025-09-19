import { useEffect } from "react";

import { provider } from "@/providers";
import useModelsStore from "@/store/useModelsStore";
import useProviders from "@/store/useProviders";

const useModels = () => {
  const { initCurrentModel, currentModel, deleteSelectedModel } =
    useModelsStore();
  const { currentProvider } = useProviders();

  useEffect(() => {
    if (!currentModel) return;

    provider.setCurrentProviderModel(currentModel.id);
  }, [currentModel]);

  useEffect(() => {
    if (!currentProvider) {
      deleteSelectedModel();
      return;
    }
  }, [currentProvider, deleteSelectedModel]);

  useEffect(() => {
    initCurrentModel();
  }, [initCurrentModel]);

  return {};
};

export default useModels;
