import { useTranslation } from "react-i18next";

import { Dialog, DialogContent } from "@/components/dialog";
import { Button } from "@/components/button";

import useServersStore from "@/store/useServersStore";

type DeleteServerDialogProps = {
  name: string;
  onClose: VoidFunction;
};

const DeleteServerDialog = ({ name, onClose }: DeleteServerDialogProps) => {
  const { t } = useTranslation();

  const { deleteCustomServer } = useServersStore();

  const onSubmitAction = () => {
    deleteCustomServer(name);
    onClose();
  };

  return (
    <Dialog open={true}>
      <DialogContent header={t("Warning")} onClose={onClose} withWarningIcon>
        <div className="flex flex-col justify-between h-full">
          <p className="select-none h-[40px] flex items-center text-[12px] leading-[16px] text-[var(--text-normal)]">
            {t("WantDeleteServer")}
          </p>
          <div className="flex flex-row justify-end items-center gap-[8px] h-[48px]">
            <Button variant="default" onClick={onClose}>
              {t("No")}
            </Button>
            <Button onClick={onSubmitAction}>{t("Yes")}</Button>
          </div>
        </div>
      </DialogContent>
    </Dialog>
  );
};

export default DeleteServerDialog;
