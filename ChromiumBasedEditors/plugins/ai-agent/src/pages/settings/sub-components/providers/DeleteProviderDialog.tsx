import React from "react";
import { useTranslation } from "react-i18next";

import { Dialog, DialogContent } from "@/components/dialog";
import { Button } from "@/components/button";

import useProviders from "@/store/useProviders";

import type { ProviderType, TProvider } from "@/lib/types";

type DeleteProviderDialogProps = {
  name: string;
  onClose: VoidFunction;
};

const DeleteProviderDialog = ({ name, onClose }: DeleteProviderDialogProps) => {
  const { providers, deleteProvider } = useProviders();

  const [provider, setProvider] = React.useState<TProvider>(() => {
    const provider = providers.find((p) => p.name === name);

    if (!provider) {
      return {
        name: "",
        type: "" as ProviderType,
        icon: "",
        key: "",
        baseUrl: "",
      };
    }

    return provider;
  });

  const { t } = useTranslation();

  React.useEffect(() => {
    const provider = providers.find((p) => p.name === name);

    if (!provider) {
      return;
    }

    setProvider(provider);
  }, [providers, name]);

  const onSubmitAction = async () => {
    await deleteProvider(provider);
    onClose();
  };

  return (
    <Dialog open={true}>
      <DialogContent header={t("Warning")} onClose={onClose} withWarningIcon>
        <div className="flex flex-col justify-between h-full">
          <p className="select-none h-[40px] flex items-center text-[12px] leading-[16px]">
            {t("WantDeleteProvider?")}
          </p>
          <div className="flex flex-row justify-end items-center gap-[8px] h-[48px]">
            <Button size="small" variant="default" onClick={onClose}>
              {t("No")}
            </Button>
            <Button size="small" onClick={onSubmitAction}>
              {t("Yes")}
            </Button>
          </div>
        </div>
      </DialogContent>
    </Dialog>
  );
};

export { DeleteProviderDialog };
