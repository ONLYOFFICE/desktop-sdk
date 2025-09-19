import { useTranslation } from "react-i18next";

import { Button } from "@/components/button";

type EmptyScreenProps = {
  onConnect: () => void;
};

const EmptyScreen = ({ onConnect }: EmptyScreenProps) => {
  const { t } = useTranslation();

  return (
    <div className="flex items-center justify-center h-full">
      <div className="max-w-[537px] text-center flex flex-col items-center gap-[40px]">
        <div className="flex flex-col gap-[16px]">
          <h1 className="select-none text-center text-[32px] font-bold leading-[24px] text-[var(--empty-screen-color)]">
            {t("ConnectAIModel")}
          </h1>
          <p className="select-none text-center text-[16px] font-normal leading-[24px] text-[var(--empty-screen-description-color)]">
            {t("ConnectAIModelDescription")}
          </p>
        </div>
        <Button onClick={onConnect}>{t("ConnectAIModelBtn")}</Button>
      </div>
    </div>
  );
};

export default EmptyScreen;
