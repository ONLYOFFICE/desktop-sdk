import { useTranslation } from "react-i18next";

import useRouter from "@/store/useRouter";

import { Button } from "@/components/button";

const EmptyScreen = () => {
  const { t } = useTranslation();

  const { setCurrentPage } = useRouter();

  return (
    <div className="flex items-center justify-center h-full">
      <div className="max-w-[573px] text-center flex flex-col items-center gap-[40px]">
        <div className="flex flex-col gap-[16px]">
          <h1 className="select-none text-center text-[32px] font-bold leading-[36px] text-[var(--empty-screen-color)]">
            {t("ConnectAIModel")}
          </h1>
          <p className="select-none text-center text-[16px] font-normal leading-[24px] text-[var(--empty-screen-description-color)]">
            {t("ConnectAIModelDescription")}
          </p>
        </div>
        <Button onClick={() => setCurrentPage("settings")}>
          {t("ConnectAIModelBtn")}
        </Button>
      </div>
    </div>
  );
};

export default EmptyScreen;
