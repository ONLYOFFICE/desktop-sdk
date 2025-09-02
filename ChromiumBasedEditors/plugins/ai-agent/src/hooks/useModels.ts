import { useEffect } from "react";

import useModelsStore from "@/store/useModelsStore";

const useModels = () => {
  const { initModels, initCurrentModel } = useModelsStore();

  useEffect(() => {
    initModels();
    initCurrentModel();
  }, [initModels, initCurrentModel]);

  return {};
};

export default useModels;
