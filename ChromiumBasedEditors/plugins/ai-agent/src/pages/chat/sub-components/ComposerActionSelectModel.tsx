import React from "react";
import { useTranslation } from "react-i18next";

import useModelsStore from "@/store/useModelsStore";
import useProviders from "@/store/useProviders";
import useMessageStore from "@/store/useMessageStore";
import useServersStore from "@/store/useServersStore";

import type { TProvider } from "@/lib/types";

import { provider } from "@/providers";

import { ComboBox } from "@/components/combo-box";

const SelectModel = () => {
  const { currentModel, selectModel } = useModelsStore();
  const { providers, providersModels, currentProvider, setCurrentProvider } =
    useProviders();
  const { tools } = useServersStore();
  const { messages } = useMessageStore();

  const { t } = useTranslation();

  const onSelectModel = React.useCallback(
    (providerInfo: TProvider, modelId: string) => {
      if (
        currentModel?.id === modelId &&
        provider.currentProviderInfo?.name === providerInfo.name
      )
        return;

      const model = providersModels
        .get(providerInfo.name)
        ?.find((model) => model.id === modelId);

      if (!model) return;

      if (provider.currentProviderInfo?.name !== providerInfo.name) {
        setCurrentProvider(providerInfo);

        provider.setCurrentProviderModel(modelId);
        provider.setCurrentProviderTools(tools);
        provider.setCurrentProviderPrevMessages(messages);
      }

      selectModel(model);
    },
    [
      providersModels,
      messages,
      tools,
      currentModel,
      selectModel,
      setCurrentProvider,
    ]
  );

  const items = providers
    .map((p) => ({
      text: p.name,
      id: p.name,
      onClick: () => {},
      subMenu:
        providersModels.get(p.name)?.map((model) => ({
          text: model.name,
          id: model.id,
          onClick: () => onSelectModel(p, model.id),
          isActive: false,
          checked:
            model.id === currentModel?.id &&
            p.name === provider.currentProviderInfo?.name,
        })) || [],
    }))
    .filter((item) => item.subMenu.length > 0);

  const currentProviderExists = providers.some(
    (p) => p.name === currentProvider?.name
  );

  return (
    <ComboBox
      placeholder={t("SelectModel")}
      value={currentProviderExists ? currentModel?.name || "" : ""}
      items={items}
      withoutBg
    />
  );
};

export { SelectModel };
