import type { CodeHeaderProps } from "@assistant-ui/react-markdown";
import { useTranslation } from "react-i18next";

import BtnCopyIconUrl from "@/assets/btn-copy.svg?url";
import BtnCheckIconUrl from "@/assets/checked.svg?url";

import { TooltipIconButton } from "../../tooltip-icon-button";
import { IconButton } from "../../icon-button";

import useCopyToClipboard from "../hooks/useCopyToClipboard";

const CodeHeader = ({ language, code }: CodeHeaderProps) => {
  const { isCopied, copyToClipboard } = useCopyToClipboard();

  const { t } = useTranslation();

  const onCopy = () => {
    if (!code || isCopied) return;
    copyToClipboard(code);
  };

  return (
    <div className="mt-4 flex items-center justify-between gap-4 rounded-t-lg bg-[var(--chat-message-code-block-background-color)] text-[var(--chat-message-code-block-header-color)] text-[14px] leading-[20px] px-[12px] pt-[12px] pb-[8px]">
      <span className="lowercase">{language}</span>
      <TooltipIconButton
        tooltip={t("Copy")}
        onClick={onCopy}
        visible={!isCopied}
      >
        {!isCopied && <IconButton iconName={BtnCopyIconUrl} size={16} />}
        {isCopied && (
          <IconButton
            iconName={BtnCheckIconUrl}
            size={16}
            isStroke
            disableHover
          />
        )}
      </TooltipIconButton>
    </div>
  );
};

export default CodeHeader;
