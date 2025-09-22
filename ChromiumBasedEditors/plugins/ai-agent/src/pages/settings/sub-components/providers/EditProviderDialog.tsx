import React from "react";
import { useTranslation } from "react-i18next";

import type { ProviderType, TProvider } from "@/lib/types";

import { Dialog, DialogContent } from "@/components/dialog";
import { Button } from "@/components/button";
import { FieldContainer } from "@/components/field-container";
import { ComboBox } from "@/components/combo-box";
import { Input } from "@/components/input";

import useProviders from "@/store/useProviders";

import {
  dialogMainContainerStyles,
  dialogContentContainerStyles,
  dialogButtonContainerStyles,
} from "./Providers.styles";

type EditProviderDialogProps = {
  name: string;
  onClose: VoidFunction;
};

const EditProviderDialog = ({ name, onClose }: EditProviderDialogProps) => {
  const { t } = useTranslation();

  const { providers, editProvider } = useProviders();

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

  const [value, setValue] = React.useState({
    name: provider!.name,
    url: provider!.baseUrl,
    key: provider!.key,
  });
  const [error, setError] = React.useState({
    key: "",
    url: "",
    name: "",
  });

  React.useEffect(() => {
    const provider = providers.find((p) => p.name === name);

    if (!provider) {
      return;
    }

    setProvider(provider);

    setValue({
      name: provider.name,
      url: provider.baseUrl,
      key: provider.key,
    });
  }, [providers, name]);

  React.useEffect(() => {
    setValue({
      name: provider.name,
      url: provider.baseUrl,
      key: provider.key,
    });
  }, [provider]);

  const dialogRef = React.useRef<HTMLDivElement>(null);

  const onChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    setValue((prevValue) => ({
      ...prevValue,
      [e.target.name]: e.target.value,
    }));
    setError((prev) => ({
      ...prev,
      [e.target.name]: "",
    }));
  };

  const onSubmitAction = async () => {
    const result = await editProvider(
      {
        type: provider!.type,
        name: value.name,
        key: value.key,
        baseUrl: value.url,
      },
      provider.name
    );

    if (typeof result === "boolean" && result) {
      onClose();
    } else if (result) {
      setError((prev) => ({
        ...prev,
        [result.field]: result.message,
      }));
    }
  };

  const isSameUrlAndName =
    value.name === provider.name && value.url === provider.baseUrl;

  const isSameKey = value.key === provider.key;

  const isDisabled =
    (isSameKey && isSameUrlAndName) ||
    !!error.key ||
    !!error.url ||
    !!error.name;

  return (
    <Dialog open={true}>
      <DialogContent
        header={t("EditAIProvider")}
        onClose={onClose}
        ref={dialogRef}
      >
        <div className={dialogMainContainerStyles}>
          <div className={dialogContentContainerStyles}>
            <FieldContainer header={t("Provider")}>
              <ComboBox value={provider.type} items={[]} />
            </FieldContainer>
            <FieldContainer header={t("Name")} error={error.name}>
              <Input
                name="name"
                onChange={onChange}
                value={value.name}
                isError={!!error.name}
                placeholder={t("EnterName")}
                className="w-full"
              />
            </FieldContainer>
            <FieldContainer header={t("URL")} error={error.url}>
              <Input
                name="url"
                onChange={onChange}
                value={value.url}
                isError={!!error.url}
                placeholder={t("EnterURL")}
                className="w-full"
              />
            </FieldContainer>
            <FieldContainer header={t("APIKey")} error={error.key}>
              <Input
                name="key"
                onChange={onChange}
                value={value.key}
                isError={!!error.key}
                placeholder={t("EnterKey")}
                className="w-full"
              />
            </FieldContainer>
          </div>

          <div className={dialogButtonContainerStyles}>
            <Button variant="default" onClick={onClose}>
              {t("Cancel")}
            </Button>
            <Button onClick={onSubmitAction} disabled={isDisabled}>
              {t("Save")}
            </Button>
          </div>
        </div>
      </DialogContent>
    </Dialog>
  );
};

export { EditProviderDialog };
