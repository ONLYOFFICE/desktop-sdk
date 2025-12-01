import React from "react";
import { useTranslation } from "react-i18next";

import { Button } from "@/components/button";

import ConfigDialog from "./ConfigDialog";
import AvailableTools from "./AvailableTools";

const Servers = () => {
  const { t } = useTranslation();
  const [isOpen, setIsOpen] = React.useState(false);

  return (
    <>
      <div className="flex flex-col gap-[16px] mt-[16px] pb-[32px]">
        <p className="font-normal text-[14px] leading-[20px] text-[var(--servers-description-color)]">
          {t("CustomServersDescription")}
        </p>
        <Button className="w-fit" onClick={() => setIsOpen(true)}>
          {t("EditConfiguration")}
        </Button>
        <AvailableTools />
      </div>
      {<ConfigDialog open={isOpen} onClose={() => setIsOpen(false)} />}
    </>
  );
};

export { Servers };
