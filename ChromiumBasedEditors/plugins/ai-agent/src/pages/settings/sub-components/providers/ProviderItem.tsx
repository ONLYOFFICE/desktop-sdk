import React from "react";
import { useTranslation } from "react-i18next";

import MoreIconSvgUrl from "@/assets/more.svg?url";
import RemoveIconSvgUrl from "@/assets/btn-remove.svg?url";

import type { TProvider } from "@/lib/types";

import { IconButton } from "@/components/icon-button";
import { DropdownMenu } from "@/components/dropdown";

import { EditProviderDialog } from "./EditProviderDialog";
import { DeleteProviderDialog } from "./DeleteProviderDialog";

type ProviderItemProps = {
  provider: TProvider;
};

const ProviderItem = ({ provider }: ProviderItemProps) => {
  const [editProviderVisible, setEditProviderVisible] = React.useState(false);
  const [deleteProviderVisible, setDeleteProviderVisible] =
    React.useState(false);

  const [containerElement, setContainerElement] =
    React.useState<HTMLDivElement | null>(null);
  const [isOpen, setIsOpen] = React.useState(false);

  const containerRef = React.useCallback((node: HTMLDivElement | null) => {
    setContainerElement(node);
  }, []);

  const { t } = useTranslation();

  return (
    <>
      <div className="flex flex-row justify-between gap-[12px] px-[16px] py-[12px] w-[274px] rounded-[8px] bg-[var(--ai-provider-item-background-color)] shadow-[var(--ai-provider-item-shadow)]">
        <div className="flex flex-col">
          <p className="font-normal text-[14px] leading-[20px] text-[var(--ai-provider-item-color)]">
            {provider.name}
          </p>
          <p className="text-[12px] leading-[14px] text-[var(--ai-provider-item-description-color)]">
            {provider.type}
            <br />
            {provider.baseUrl}
          </p>
        </div>
        <div
          className="flex items-center justify-end flex-1"
          ref={containerRef}
        >
          <DropdownMenu
            onOpenChange={setIsOpen}
            trigger={
              <IconButton
                iconName={MoreIconSvgUrl}
                color="var(--ai-provider-item-icon-color)"
                size={20}
                isActive={isOpen}
              />
            }
            items={[
              { text: t("Edit"), onClick: () => setEditProviderVisible(true) },
              { text: "", onClick: () => {}, isSeparator: true },
              {
                icon: <IconButton iconName={RemoveIconSvgUrl} size={20} />,
                text: t("Delete"),
                onClick: () => setDeleteProviderVisible(true),
              },
            ]}
            side="right"
            align="start"
            sideOffset={0}
            containerRef={containerElement}
          />
        </div>
      </div>
      {editProviderVisible ? (
        <EditProviderDialog
          name={provider.name}
          onClose={() => setEditProviderVisible(false)}
        />
      ) : null}
      {deleteProviderVisible ? (
        <DeleteProviderDialog
          name={provider.name}
          onClose={() => setDeleteProviderVisible(false)}
        />
      ) : null}
    </>
  );
};

export { ProviderItem };
