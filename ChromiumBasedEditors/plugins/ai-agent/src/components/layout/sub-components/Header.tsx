import { useTranslation } from "react-i18next";

import BtnSettingsUrl from "@/assets/btn-settings.svg?url";
import BtnAddUrl from "@/assets/btn-zoomup.svg?url";

import useRouter from "@/store/useRouter";
import useThreadsStore from "@/store/useThreadsStore";

import { IconButton } from "@/components/icon-button";
import { TooltipIconButton } from "@/components/tooltip-icon-button";

const Navigation = () => {
  const { t } = useTranslation();

  const { currentPage, setCurrentPage } = useRouter();
  const { onSwitchToNewThread } = useThreadsStore();

  return (
    <nav className="w-full flex justify-between items-center h-[56px] min-h-[56px] box-border border-b-[1px] border-[var(--header-border-color)] bg-[var(--header-background-color)] px-[32px]">
      <div className="flex items-center gap-[12px]">
        <h3 className="text-[var(--header-color)] font-bold text-[16px] leading-[24px]">
          {t("AIAgent")}
        </h3>
        <TooltipIconButton tooltip={t("NewChat")}>
          <IconButton
            iconName={BtnAddUrl}
            size={24}
            color="var(--header-color)"
            isStroke
            onClick={() => {
              setCurrentPage("chat");
              onSwitchToNewThread();
            }}
          />
        </TooltipIconButton>
      </div>
      <TooltipIconButton tooltip={t("Settings")}>
        <IconButton
          iconName={BtnSettingsUrl}
          size={24}
          color="var(--header-color)"
          isStroke
          isActive={currentPage === "settings"}
          onClick={() =>
            setCurrentPage(currentPage === "settings" ? "chat" : "settings")
          }
        />
      </TooltipIconButton>
    </nav>
  );
};

export { Navigation };
