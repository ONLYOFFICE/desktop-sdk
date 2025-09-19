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
    const result = await addProvider({
      type: selectedProviderInfo.type,
      name: value.name,
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
  };

  React.useEffect(() => {
    setValue((prevValue) => ({
      ...prevValue,
      url: selectedProviderInfo.baseUrl,
    }));
  }, [selectedProviderInfo]);

  const isDisabled =
    !value.name || !value.url || !!error.key || !!error.url || !!error.name;

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
              {t("AddProvider")}
            </Button>
          </div>
        </div>
      </DialogContent>
    </Dialog>
  );
};

export { AddProviderDialog };
