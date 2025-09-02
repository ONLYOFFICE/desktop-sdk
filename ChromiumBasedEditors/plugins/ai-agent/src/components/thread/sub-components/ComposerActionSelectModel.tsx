import React from "react";

import ArrowDownIconUrl from "@/assets/arrow.bottom.svg?url";

import useModelsStore from "@/store/useModelsStore";

import { DropdownMenu } from "@/components/dropdown";
import { IconButton } from "@/components/icon-button";

const SelectModel = () => {
  const { models, currentModel, selectModel } = useModelsStore();

  const [isOpen, setIsOpen] = React.useState(false);

  const onOpenChange = React.useCallback((open: boolean) => {
    setIsOpen(open);
  }, []);

  const onSelectModel = React.useCallback(
    (modelId: string) => {
      const model = models.find((model) => model.id === modelId);

      if (!model) return;

      selectModel(model);
    },
    [models, selectModel]
  );

  const trigger = (
    <div
      className="flex flex-row items-center cursor-pointer select-none h-[24px] box-border rounded-[4px] bg-[var(--select-models-trigger-background-color)] hover:bg-[var(--select-models-trigger-background-hover-color)]"
      style={{
        backgroundColor: isOpen
          ? "var(--select-models-trigger-background-active-color)"
          : "",
      }}
    >
      <p className="px-[4px] text-[var(--select-models-trigger-text-color)] text-[12px] leading-[22px]">
        {currentModel ? currentModel.name : "Select model"}
      </p>
      <IconButton
        iconName={ArrowDownIconUrl}
        size={16}
        color="var(--select-models-trigger-icon-color)"
      />
    </div>
  );

  return (
    <DropdownMenu
      trigger={trigger}
      align="start"
      side="bottom"
      maxWidth="300px"
      items={models.map((model) => ({
        text: model.name,
        id: model.id,
        onClick: () => onSelectModel(model.id),
      }))}
      onOpenChange={onOpenChange}
    />
  );
};

export { SelectModel };
