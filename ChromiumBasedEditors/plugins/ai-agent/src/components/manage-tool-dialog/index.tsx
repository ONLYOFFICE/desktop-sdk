import { useState } from "react";
import { useTranslation } from "react-i18next";

import useServersStore from "@/store/useServersStore";

import { Button } from "../button";
import { Dialog, DialogContent } from "../dialog";
import { ToolFallback } from "../tool-fallback";
import { Checkbox } from "../checkbox";

type ManageToolDialogProps = {
  onAllow: (allowAlways: boolean) => void;
  onDeny: () => void;
  onClose: () => void;
};

const ManageToolDialog = ({
  onAllow,
  onDeny,
  onClose,
}: ManageToolDialogProps) => {
  const { t } = useTranslation();
  const { manageToolData } = useServersStore();

  const [isAllowAlways, setIsAllowAlways] = useState(false);

  const onAllowAction = () => {
    onAllow(isAllowAlways);
    onClose();
  };

  const onDenyAction = () => {
    onDeny();
    onClose();
  };

  const toolCall = manageToolData?.message?.content[manageToolData.idx];

  if (
    !toolCall ||
    typeof toolCall !== "object" ||
    !("type" in toolCall) ||
    toolCall.type !== "tool-call"
  )
    return;

  return (
    <Dialog open={true}>
      <DialogContent header={t("Confirmation")} onClose={onDeny} isHuge>
        <div className="flex flex-col justify-between h-full mb-[16px] text-[var(--tool-fallback-color)]">
          <p className="mt-[8px] font-[14px] leading-[20px] font-normal text-[var(--tool-fallback-color)]">
            {t("AIWouldLikeUseThisTool")}
          </p>
          <ToolFallback
            type="tool-call"
            toolCallId={toolCall.toolCallId!}
            args={toolCall.args ?? {}}
            toolName={toolCall.toolName}
            argsText={toolCall.argsText ?? "{}"}
            result={toolCall.result}
            status={{ type: "running" }}
            addResult={() => {}}
          />
          <p className="font-[14px] leading-[20px] font-normal text-[var(--tool-fallback-color)]">
            {t("ReviewEachAction")}
          </p>
          <p className="flex items-center gap-[8px] mt-[24px] font-[14px] leading-[20px] font-normal text-[var(--tool-fallback-color)]">
            <Checkbox
              checked={isAllowAlways}
              onChange={setIsAllowAlways}
              id="allow-always-checkbox"
            />
            {t("AlwaysAllow")}
          </p>
        </div>
        <div className="flex flex-row justify-end items-center gap-[8px] h-[64px] border-t-[1px] border-[var(--border-divider)] mx-[-32px] px-[32px]">
          <Button variant="default" onClick={onDenyAction}>
            {t("Deny")}
          </Button>
          <Button onClick={onAllowAction}>{t("Allow")}</Button>
        </div>
      </DialogContent>
    </Dialog>
  );
};

export { ManageToolDialog };
