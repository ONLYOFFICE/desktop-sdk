import React from "react";
import { useTranslation } from "react-i18next";

import { Dialog, DialogContent } from "@/components/dialog";
import { Button } from "@/components/button";

import useThreadsStore from "@/store/useThreadsStore";

type DeleteChatDialogProps = {
  id: string;
  onClose: VoidFunction;
};

const DeleteChatDialog = ({ id, onClose }: DeleteChatDialogProps) => {
  const { onDeleteThread } = useThreadsStore();

  const { t } = useTranslation();

  const onSubmitAction = React.useCallback(async () => {
    await onDeleteThread(id);
    onClose();
  }, [id, onDeleteThread, onClose]);

  // Handle keyboard events
  React.useEffect(() => {
    const handleKeyDown = (event: KeyboardEvent) => {
      if (event.key === "Enter") {
        event.preventDefault();
        onSubmitAction();
      }
    };

    document.addEventListener("keydown", handleKeyDown);

    return () => {
      document.removeEventListener("keydown", handleKeyDown);
    };
  }, [onSubmitAction]);

  return (
    <Dialog open={true}>
      <DialogContent header={t("Warning")} onClose={onClose} withWarningIcon>
        <div className="flex flex-col justify-between h-full">
          <p className="select-none h-[40px] flex items-center text-[12px] leading-[16px]">
            {t("WantDeleteChat?")}
          </p>
          <div className="flex flex-row justify-end items-center gap-[8px] h-[48px]">
            <Button size="small" variant="default" onClick={onClose}>
              {t("No")}
            </Button>
            <Button size="small" onClick={onSubmitAction}>
              {t("Yes")}
            </Button>
          </div>
        </div>
      </DialogContent>
    </Dialog>
  );
};

export { DeleteChatDialog };
