import { useMemo, useState } from "react";
import { useTranslation } from "react-i18next";

import ToolsIconUrl from "@/assets/tools.svg?url";

import useServersStore from "@/store/useServersStore";

import { IconButton } from "@/components/icon-button";
import { DropdownMenu } from "@/components/dropdown";
import { TooltipIconButton } from "@/components/tooltip-icon-button";

const ServersSettings = () => {
  const { servers, changeToolStatus } = useServersStore();

  const [isOpen, setIsOpen] = useState(false);

  const { t } = useTranslation();

  const trigger = useMemo(
    () => (
      <TooltipIconButton visible={!isOpen} tooltip={t("MCPServers")}>
        <IconButton
          iconName={ToolsIconUrl}
          size={24}
          color="var(--chat-composer-action-servers-color)"
          isActive={isOpen}
        />
      </TooltipIconButton>
    ),
    [isOpen, t]
  );

  const toolsActions = useMemo(
    () =>
      Object.entries(servers)
        .map(([type, tools]) => {
          return {
            text: type,
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
        })
        .filter((item) => item.subMenu.length > 0),
    [servers, changeToolStatus]
  );

  const actions = useMemo(
    () => [
      ...toolsActions,

      // {
      //   text: "",
      //   onClick: () => {},
      //   isSeparator: true,
      // },
      // {
      //   text: "Manage connection",
      //   icon: ToolsIconUrl,
      //   iconSize: 16,
      //   iconColor: "var(--tools-settings-trigger-icon-color)",
      //   onClick: () => {},
      // },
    ],
    [toolsActions]
  );

  if (!servers) return null;

  if (!toolsActions.length) return null;

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
