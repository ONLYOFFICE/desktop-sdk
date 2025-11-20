import React from "react";
import { useTranslation } from "react-i18next";

import { Dialog, DialogContent } from "@/components/dialog";
import { ComboBox } from "@/components/combo-box";
import { Button } from "@/components/button";
import { FieldContainer } from "@/components/field-container";

import { provider } from "@/providers";

import useProviders from "@/store/useProviders";

import {
  dialogMainContainerStyles,
  dialogContentContainerStyles,
  dialogButtonContainerStyles,
} from "./Providers.styles";
import { Input } from "@/components/input";
import { Loader } from "@/components/loader";

type AddProviderDialogProps = {
  onClose: VoidFunction;
};

const providersInfo = provider.getProvidersInfo();

const AddProviderDialog = ({ onClose }: AddProviderDialogProps) => {
  const { addProvider } = useProviders();

  const { t } = useTranslation();

  const [selectedProviderInfo, setSelectedProvderInfo] = React.useState(
    providersInfo[0]
  );
  const [value, setValue] = React.useState({
    name: "",
    url: selectedProviderInfo.baseUrl,
    key: "",
  });
  const [error, setError] = React.useState({
    key: "",
    url: "",
    name: "",
  });
  const [isRequestRunning, setIsRequestRunning] = React.useState(false);
  const isRequestRunningRef = React.useRef(isRequestRunning);

  const dialogRef = React.useRef<HTMLDivElement>(null);
  const buttonRef = React.useRef<HTMLButtonElement>(null);
  const [buttonWidth, setButtonWidth] = React.useState<number | undefined>(
    undefined
  );

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

  const isDisabled =
    !value.name || !value.url || !!error.key || !!error.url || !!error.name;

  const onSubmitAction = React.useCallback(async () => {
    if (isRequestRunningRef.current || isDisabled) return;
    isRequestRunningRef.current = true;
    setIsRequestRunning(true);

    const result = await addProvider({
      type: selectedProviderInfo.type,
      name: value.name.trim(),
      key: value.key,
      baseUrl: value.url,
    });

    if (typeof result === "boolean" && result) {
      onClose();
    } else if (result) {
      setError((prev) => ({
        ...prev,
        [result.field]: result.message,
      }));
    }
    isRequestRunningRef.current = false;
    setIsRequestRunning(false);
  }, [addProvider, selectedProviderInfo, value, onClose, isDisabled]);

  React.useEffect(() => {
    setValue((prevValue) => ({
      ...prevValue,
      url: selectedProviderInfo.baseUrl,
    }));
  }, [selectedProviderInfo]);

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
      <DialogContent header={t("AIProvider")} onClose={onClose} ref={dialogRef}>
        <div className={dialogMainContainerStyles}>
          <div className={dialogContentContainerStyles}>
            <FieldContainer header={t("Provider")}>
              <ComboBox
                value={selectedProviderInfo.name}
                items={providersInfo.map((p) => ({
                  text: p.name,
                  id: p.name,
                  onClick: () => setSelectedProvderInfo(p),
                }))}
              />
            </FieldContainer>
            <FieldContainer header={t("Name")} error={error.name}>
              <Input
                name="name"
                onChange={onChange}
                value={value.name}
                isError={!!error.name}
                placeholder={t("EnterName")}
                className="w-full"
                maxLength={128}
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
                t("AddProvider")
              )}
            </Button>
          </div>
        </div>
      </DialogContent>
    </Dialog>
  );
};

export { AddProviderDialog };
