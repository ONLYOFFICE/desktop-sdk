import type { CodeHeaderProps } from "@assistant-ui/react-markdown";
import { CopyIcon, CheckIcon } from "lucide-react";

import { TooltipIconButton } from "../../tooltip-icon-button";

import useCopyToClipboard from "../hooks/useCopyToClipboard";

const CodeHeader = ({ language, code }: CodeHeaderProps) => {
  const { isCopied, copyToClipboard } = useCopyToClipboard();
  const onCopy = () => {
    if (!code || isCopied) return;
    copyToClipboard(code);
  };

  return (
    <div className="mt-4 flex items-center justify-between gap-4 rounded-t-lg bg-zinc-900 px-4 py-2 text-sm font-semibold text-white">
      <span className="lowercase [&>span]:text-xs">{language}</span>
      <TooltipIconButton tooltip="Copy" onClick={onCopy}>
        {!isCopied && <CopyIcon />}
        {isCopied && <CheckIcon />}
      </TooltipIconButton>
    </div>
  );
};

export default CodeHeader;
