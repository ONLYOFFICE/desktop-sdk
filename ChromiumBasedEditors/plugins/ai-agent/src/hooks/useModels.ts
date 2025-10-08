import { useEffect } from "react";

import { provider } from "@/providers";
import useModelsStore from "@/store/useModelsStore";

const useModels = () => {
  const { initCurrentModel, currentModel } = useModelsStore();
  // const { currentProvider } = useProviders();

  useEffect(() => {
    if (!currentModel) return;

    provider.setCurrentProviderModel(currentModel.id);
  }, [currentModel]);

  // useEffect(() => {
  //   if (!currentProvider) {
  //     deleteSelectedModel();
  //     return;
  //   }
  // }, [currentProvider, deleteSelectedModel]);

  useEffect(() => {
    initCurrentModel();
  }, [initCurrentModel]);

  return {};
};

export default useModels;
