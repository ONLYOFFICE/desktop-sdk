import React from "react";

import useServersStore from "@/store/useServersStore";

import { Dialog, DialogContent } from "@/components/dialog";
import { Button } from "@/components/button";

type LogsDialogProps = {
  type: string;
  open: boolean;
  onClose: () => void;
};

const LogsDialog = ({ type, open, onClose }: LogsDialogProps) => {
  const [logs, setLogs] = React.useState<string[]>([]);

  const { getCustomServersLogs } = useServersStore();

  React.useEffect(() => {
    const fetchLogs = async () => {
      const allLogs = getCustomServersLogs();
      setLogs(allLogs[type]);
    };

    fetchLogs();

    const interval = setInterval(() => {
      fetchLogs();
    }, 500);

    return () => clearInterval(interval);
  }, [getCustomServersLogs, type]);

  const copyToClipboard = async () => {
    await navigator.clipboard.writeText(logs.join("\n"));
    onClose();
  };

  return (
    <Dialog open={open} onOpenChange={() => {}}>
      <DialogContent
        header={`${type} logs`}
        onClose={onClose}
        className="w-[720px] h-[520px]"
      >
        <div className="flex flex-col gap-[8px] h-[400px] py-[8px]">
          <div className="flex flex-col border h-[384px] border-[var(--servers-logs-dialog-border-color)] overflow-y-auto rounded-[8px]">
            {logs.map((log) => (
              <p
                className="text-[var(--servers-logs-dialog-log-color)] font-normal text-[14px] leading-[20px]"
                key={log}
              >
                {log}
              </p>
            ))}
          </div>
        </div>
        <div className="flex flex-row items-center justify-end gap-[16px] h-[64px] border-t border-[var(--servers-edit-config-buttons-border-color)] mx-[-32px] px-[32px]">
          <Button onClick={copyToClipboard} variant="default">
            Copy to clipboard
          </Button>
          <Button onClick={onClose}>Close</Button>
        </div>
      </DialogContent>
    </Dialog>
  );
};

export default LogsDialog;
