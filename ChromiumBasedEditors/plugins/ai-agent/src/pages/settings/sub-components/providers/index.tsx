import React from "react";

import { useTranslation } from "react-i18next";

import useProviders from "@/store/useProviders";

import { cn } from "@/lib/utils";

import { Button } from "@/components/button";

import { ProviderItem } from "./ProviderItem";
import { AddProviderDialog } from "./AddProviderDialog";

type ProvidersProps = {
  isActive: boolean;
};

const Providers = ({ isActive }: ProvidersProps) => {
  const [addProvderVisible, setAddProvderVisible] = React.useState(false);

  const { providers } = useProviders();

  const { t } = useTranslation();

  return (
    <>
      <Button
        className="max-w-[fit-content]"
        onClick={() => setAddProvderVisible(true)}
        disabled={!isActive}
      >
        {t("AddProvider")}
      </Button>

      <div
        className={cn(
          "flex flex-wrap gap-[16px]",
          isActive ? "" : "opacity-70 pointer-events-none"
        )}
      >
        {providers.map((provider) => (
          <ProviderItem key={provider.name} provider={provider} />
        ))}
      </div>
      {addProvderVisible ? (
        <AddProviderDialog onClose={() => setAddProvderVisible(false)} />
      ) : null}
    </>
  );
};

export { Providers };
