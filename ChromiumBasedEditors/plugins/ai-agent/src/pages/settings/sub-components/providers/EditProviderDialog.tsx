import React from "react";
import { useTranslation } from "react-i18next";

import type { ProviderType, TProvider } from "@/lib/types";

import { Dialog, DialogContent } from "@/components/dialog";
import { Button } from "@/components/button";
import { FieldContainer } from "@/components/field-container";
import { ComboBox } from "@/components/combo-box";
import { Input } from "@/components/input";
import { Loader } from "@/components/loader";

import useProviders from "@/store/useProviders";

import { provider as providerInstance } from "@/providers";

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

  const { providers, editProvider, currentProvider, setCurrentProvider } =
    useProviders();

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
  const [isRequestRunning, setIsRequestRunning] = React.useState(false);
  const isRequestRunningRef = React.useRef(isRequestRunning);

  const buttonRef = React.useRef<HTMLButtonElement>(null);
  const [buttonWidth, setButtonWidth] = React.useState<number | undefined>(
    undefined
  );

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

  const isSameUrlAndName =
    value.name === provider.name && value.url === provider.baseUrl;

  const isSameKey = value.key === provider.key;

  const isDisabled =
    (isSameKey && isSameUrlAndName) ||
    !!error.key ||
    !!error.url ||
    !!error.name;

  const onSubmitAction = React.useCallback(async () => {
    if (isRequestRunningRef.current || isDisabled) return;
    isRequestRunningRef.current = true;
    setIsRequestRunning(true);

    const updatedProviderInfo = {
      type: provider!.type,
      name: value.name,
      key: value.key,
      baseUrl: value.url,
    };

    const result = await editProvider(updatedProviderInfo, provider.name);

    if (typeof result === "boolean" && result) {
      // Check if the edited provider is the current provider
      if (currentProvider?.name === provider.name) {
        // Update current provider with new info
        const updatedProvider = {
          ...provider,
          ...updatedProviderInfo,
        };
        setCurrentProvider(updatedProvider);
        providerInstance.setCurrentProvider(updatedProvider);
      }
      onClose();
    } else if (result) {
      setError((prev) => ({
        ...prev,
        [result.field]: result.message,
      }));
    }
    isRequestRunningRef.current = false;
    setIsRequestRunning(false);
  }, [
    isDisabled,
    editProvider,
    provider,
    value,
    onClose,
    currentProvider,
    setCurrentProvider,
  ]);

  React.useEffect(() => {
    if (buttonRef.current && buttonWidth === undefined) {
      const width = buttonRef.current.offsetWidth + 1;
      setButtonWidth(width);
    }
  }, [buttonWidth]);

  React.useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      if (e.key === "Enter") {
        e.preventDefault();
        onSubmitAction();
      }
    };

    window.addEventListener("keydown", handleKeyDown);

    return () => {
      window.removeEventListener("keydown", handleKeyDown);
    };
  }, [onSubmitAction]);

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
                type="password"
              />
            </FieldContainer>
          </div>

          <div className={dialogButtonContainerStyles}>
            <Button variant="default" onClick={onClose}>
              {t("Cancel")}
            </Button>
            <Button
              ref={buttonRef}
              onClick={onSubmitAction}
              disabled={isDisabled || isRequestRunning}
              style={buttonWidth ? { width: `${buttonWidth}px` } : undefined}
            >
              {isRequestRunning ? (
                <Loader className="border-[var(--text-contrast-background)] border-r-transparent" />
              ) : (
                t("Save")
              )}
            </Button>
          </div>
        </div>
      </DialogContent>
    </Dialog>
  );
};

export { EditProviderDialog };
