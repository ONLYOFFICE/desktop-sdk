import React from "react";

import useModelsStore from "@/store/useModelsStore";

import { DropdownMenu } from "../../dropdown";

export const useSelectModel = () => {
  const { models, currentModel, selectModel } = useModelsStore();

  const onSelectModel = React.useCallback(
    (modelId: string) => {
      const model = models.find((model) => model.id === modelId);

      if (!model) return;

      selectModel(model);
    },
    [models, selectModel]
  );

  const useSelectModelComponent = (
    <DropdownMenu
      trigger={<p>{currentModel ? currentModel.name : "Select model"}</p>}
      align="start"
      side="bottom"
      maxWidth="300px"
      items={models.map((model) => ({
        text: model.name,
        id: model.id,
        onClick: () => onSelectModel(model.id),
      }))}
    />
  );

  return {
    useSelectModelComponent,
  };
};
