import React from "react";

import { Menu, TextCursorIcon, SaveIcon, TrashIcon } from "lucide-react";

import type { Thread } from "@/lib/types";
import useThreadsStore from "@/store/useThreadsStore";

import { DropdownMenu } from "../../dropdown";

export const useThreads = () => {
  const { threadId, threads, onSwitchToThread, onDeleteThread } =
    useThreadsStore();

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
        onClick: () => onDeleteThread(thread.threadId),
      },
      {
        icon: <SaveIcon className="w-4 h-4" />,
        text: "Save to file",
        onClick: () => onDeleteThread(thread.threadId),
      },
      {
        text: "",
        onClick: () => {},
        isSeparator: true,
      },
      {
        icon: <TrashIcon className="w-4 h-4" />,
        text: "Delete",
        onClick: () => onDeleteThread(thread.threadId),
      },
    ],
    [onDeleteThread]
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
