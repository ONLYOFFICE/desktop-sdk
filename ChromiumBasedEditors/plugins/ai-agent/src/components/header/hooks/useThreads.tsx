import { Menu, TextCursorIcon, SaveIcon, TrashIcon } from "lucide-react";

import type { Thread } from "@/lib/types";

import { DropdownMenu } from "../../dropdown";
import React from "react";

export type UseThreadsProps = {
  threadId: string;
  threads: Thread[];
  onSwitchToThread: (id: string) => void;
  onDelete: (id: string) => void;
};

export const useThreads = ({
  threadId,
  threads,
  onSwitchToThread,
  onDelete,
}: UseThreadsProps) => {
  const onClickThread = React.useCallback(
    (thread: Thread) => {
      onSwitchToThread(thread.threadId);
    },
    [onSwitchToThread]
  );

  const getActionMenu = React.useCallback(
    (thread: Thread) => [
      {
        icon: <TextCursorIcon className="w-4 h-4" />,
        text: "Rename",
        onClick: () => onDelete(thread.threadId),
      },
      {
        icon: <SaveIcon className="w-4 h-4" />,
        text: "Save to file",
        onClick: () => onDelete(thread.threadId),
      },
      {
        text: "",
        onClick: () => {},
        isSeparator: true,
      },
      {
        icon: <TrashIcon className="w-4 h-4" />,
        text: "Delete",
        onClick: () => onDelete(thread.threadId),
      },
    ],
    [onDelete]
  );

  const items = React.useMemo(() => {
    return threads.map((thread) => {
      return {
        text: thread.title ?? "",
        isActive: thread.threadId === threadId,
        onClick: () => onClickThread(thread),
        actionsMenu: getActionMenu(thread),
      };
    });
  }, [threads, threadId, onClickThread, getActionMenu]);

  const threadsComponent = (
    <DropdownMenu
      trigger={<Menu />}
      align="start"
      side="bottom"
      maxWidth="300px"
      items={items}
    />
  );
  return {
    threadsComponent,
  };
};
