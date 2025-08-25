import React from "react";

import { provider } from "@/providers";

import { DropdownMenu } from "../../dropdown";

export const useSelectModel = () => {
  const { models, modelsLoading, currentModel } = provider;

  const onSelectModel = React.useCallback((modelId: string) => {
    provider.selectModel(modelId);
  }, []);

  const useSelectModelComponent = modelsLoading ? null : (
    <DropdownMenu
      trigger={<p>{currentModel.name}</p>}
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
