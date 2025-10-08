import { ThreadPrimitive } from "@assistant-ui/react";

import { useTranslation } from "react-i18next";

const ThreadWelcome = () => {
  const { t } = useTranslation();

  return (
    <ThreadPrimitive.Empty>
      <div className="mx-auto flex w-full max-w-[var(--thread-max-width)] flex-col px-[var(--thread-padding-x)] justify-center items-center mb-[16px]">
        <div className="max-w-[490px] flex flex-col items-center gap-[16px] text-center">
          <p
            className="font-[700] text-[32px] leading-[24px]"
            style={{ color: "var(--chat-welcome-color)" }}
          >
            {t("WelcomeTitle")}
          </p>
          <p
            className="text-[20px] leading-[20px]"
            style={{ color: "var(--chat-welcome-description-color)" }}
          >
            {t("WelcomeDescription")}
          </p>
        </div>
      </div>
    </ThreadPrimitive.Empty>
  );
};

export { ThreadWelcome };
