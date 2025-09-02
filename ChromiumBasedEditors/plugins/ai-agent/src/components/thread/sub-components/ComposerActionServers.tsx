import { useMemo, useState } from "react";

import ToolsIconUrl from "@/assets/tools.svg?url";
import OnlyOfficeIconUrl from "@/assets/onlyoffice.svg?url";

import useServersStore from "@/store/useServersStore";

import { IconButton } from "@/components/icon-button";
import { DropdownMenu } from "@/components/dropdown";

const ServersSettings = () => {
  const { servers, changeToolStatus } = useServersStore();

  const [isOpen, setIsOpen] = useState(false);

  const trigger = useMemo(
    () => (
      <div
        className="flex items-center gap-[4px] cursor-pointer select-none h-[24px] pe-[4px] box-border rounded-[4px] bg-[var(--tools-settings-trigger-background-color)] hover:bg-[var(--tools-settings-trigger-background-hover-color)]"
        style={{
          backgroundColor: isOpen
            ? "var(--tools-settings-trigger-background-active-color)"
            : "",
        }}
      >
        <IconButton
          iconName={ToolsIconUrl}
          size={24}
          color="var(--tools-settings-trigger-icon-color)"
        />
        <span className="font-normal text-[12px] leading-[22px] text-[var(--tools-settings-trigger-label-color)] select-none">
          Tools
        </span>
      </div>
    ),
    [isOpen]
  );

  const toolsActions = useMemo(
    () =>
      Object.entries(servers).map(([type, tools]) => {
        return {
          text: type,
          icon: OnlyOfficeIconUrl,
          iconSize: 16,
          onClick: () => {},
          subMenu: tools.map((tool) => {
            return {
              text: tool.name,
              onClick: () => {},
              withToggle: true,
              toggleChecked: tool.enabled,
              onToggleChange: (checked: boolean) => {
                changeToolStatus(type, tool.name, checked);
              },
            };
          }),
        };
      }),
    [servers, changeToolStatus]
  );

  const actions = useMemo(
    () => [
      ...toolsActions,
      {
        text: "",
        onClick: () => {},
        isSeparator: true,
      },
      {
        text: "Manage connection",
        icon: ToolsIconUrl,
        iconSize: 16,
        iconColor: "var(--tools-settings-trigger-icon-color)",
        onClick: () => {},
      },
    ],
    [toolsActions]
  );

  if (!servers) return null;

  const onOpenChange = (open: boolean) => {
    setIsOpen(open);
  };

  return (
    <DropdownMenu
      trigger={trigger}
      items={actions}
      onOpenChange={onOpenChange}
    />
  );
};

export { ServersSettings };
