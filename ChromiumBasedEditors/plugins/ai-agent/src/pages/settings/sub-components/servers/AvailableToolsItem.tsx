import React from "react";
import { useTranslation } from "react-i18next";

import useServersStore from "@/store/useServersStore";

import client from "@/servers";

import { IconButton } from "@/components/icon-button";
import { DropdownMenu } from "@/components/dropdown";
import { Loader } from "@/components/loader";
import { ToggleButton } from "@/components/toggle-button";
import { Tooltip, TooltipTrigger, TooltipContent } from "@/components/tooltip";

import ArrowBottomIconSvgUrl from "@/assets/arrow.right.svg?url";
import MoreIconSvgUrl from "@/assets/more.svg?url";
import RemoveIconSvgUrl from "@/assets/btn-remove.svg?url";
import ResetIconSvgUrl from "@/assets/btn-reset.svg?url";
import NavigationIconSvgUrl from "@/assets/btn-menu-navigation.svg?url";
import StatusErrorIconUrl from "@/assets/status.error.svg?url";

import type { TMCPItem } from "@/lib/types";

import LogsDialog from "./LogsDialog";
import DeleteServerDialog from "./DeleteServerDialog";

type AvailableToolsItemProps = {
  name: string;
  mcpItems: TMCPItem[];
  isLoading: boolean;
  isSystem: boolean;
  disableEnable: boolean;
};

const AvailableToolsItem = ({
  name,
  mcpItems,
  isLoading,
  isSystem,
  disableEnable,
}: AvailableToolsItemProps) => {
  const { t } = useTranslation();

  const [opened, setOpened] = React.useState(false);
  const [dropdownOpen, setDropdownOpen] = React.useState(false);
  const [openLogsDialog, setOpenLogsDialog] = React.useState(false);
  const [isStoped, setIsStoped] = React.useState(false);
  const [deleteDialogOpen, setDeleteDialogOpen] = React.useState(false);

  const containerRef = React.useRef<HTMLDivElement>(null);

  const { changeToolStatus } = useServersStore();

  const onEnableAllTools = () => {
    mcpItems
      .filter((tool) => !tool.enabled)
      .forEach((tool) => {
        changeToolStatus(name, tool.name, true);
      });
  };

  const onDisableAllTools = () => {
    mcpItems
      .filter((tool) => tool.enabled)
      .forEach((tool) => {
        changeToolStatus(name, tool.name, false);
      });
  };

  const openLogs = () => setOpenLogsDialog(true);

  React.useEffect(() => {
    if (isLoading) setOpened(false);
  }, [isLoading]);

  React.useEffect(() => {
    const interval = setInterval(() => {
      setIsStoped(client.getCustomServersStoped().includes(name));
    }, 1000);

    return () => clearInterval(interval);
  }, [name]);

  const isLoadingAction = isStoped ? false : isLoading;

  return (
    <div className="flex flex-col">
      <div
        className={`h-[36px] px-[8px] rounded-[4px] flex items-center justify-between ${
          isLoadingAction ? "" : "cursor-pointer"
        } ${
          opened
            ? "bg-[var(--servers-available-tools-item-active-background-color)]"
            : "bg-[var(--servers-available-tools-item-background-color)]"
        } ${
          !isLoading && !opened
            ? "hover:bg-[var(--servers-available-tools-item-hover-background-color)]"
            : ""
        }`}
        onClick={() => {
          if (isLoading || dropdownOpen || mcpItems.length === 0) return;
          setOpened((val) => !val);
        }}
      >
        <div className="flex items-center gap-[8px]">
          <IconButton
            iconName={ArrowBottomIconSvgUrl}
            size={24}
            disableHover
            isStroke
            isTransform={opened}
          />
          <p className="text-[var(--servers-available-tools-item-name-color)]">
            {name}
          </p>
          {!isLoadingAction && isStoped ? (
            <IconButton
              iconName={StatusErrorIconUrl}
              size={16}
              disableHover
              noColor
            />
          ) : null}
          {isLoading ? null : (
            <p className="font-normal text-[14px] text-[var(--servers-available-tools-sub-header-color)]">
              <span className="text-[var(--servers-available-tools-current-tool-color)]">
                {mcpItems.filter((tool) => tool.enabled).length}
              </span>
              /{mcpItems.length} {t("ToolsEnabled")}
            </p>
          )}
        </div>
        <div ref={containerRef}>
          {isLoadingAction ? (
            <Loader />
          ) : (
            <DropdownMenu
              onOpenChange={setDropdownOpen}
              trigger={
                <IconButton
                  iconName={MoreIconSvgUrl}
                  size={20}
                  isActive={dropdownOpen}
                  insideElement
                />
              }
              items={[
                ...(mcpItems.length === 0
                  ? []
                  : [
                      {
                        text: t("EnableAllTools"),
                        onClick: onEnableAllTools,
                        withSpace: !isSystem,
                      },
                      {
                        text: t("DisableAllTools"),
                        onClick: onDisableAllTools,
                        withSpace: !isSystem,
                      },
                      ...(!isSystem
                        ? [{ text: "", onClick: () => {}, isSeparator: true }]
                        : []),
                    ]),
                ...(isSystem
                  ? []
                  : [
                      {
                        icon: (
                          <IconButton
                            iconName={ResetIconSvgUrl}
                            size={20}
                            disableHover
                            isStroke
                          />
                        ),
                        text: t("Restart"),
                        onClick: () => {
                          client.restartCustomServer(name);
                        },
                      },
                      {
                        icon: (
                          <IconButton
                            iconName={NavigationIconSvgUrl}
                            size={20}
                            disableHover
                            isStroke
                          />
                        ),
                        text: t("Logs"),
                        onClick: openLogs,
                      },
                      { text: "", onClick: () => {}, isSeparator: true },
                      {
                        icon: (
                          <IconButton
                            iconName={RemoveIconSvgUrl}
                            size={20}
                            disableHover
                          />
                        ),
                        text: t("Delete"),
                        onClick: () => {
                          setDeleteDialogOpen(true);
                        },
                      },
                    ]),
              ]}
              side="right"
              align="start"
              sideOffset={0}
              containerRef={containerRef.current}
            />
          )}
        </div>
      </div>
      {opened ? (
        <div className="flex flex-col gap-[12px] mt-[4px]">
          {mcpItems.map((tool) => {
            const description = isSystem
              ? tool.description?.split(". ")[0] + "."
              : tool.description;
            return (
              <div
                key={tool.name}
                className="rounded-[4px] cursor-pointer flex flex-col ps-[40px] pe-[8px] hover:bg-[var(--servers-available-tools-item-hover-background-color)]"
                onClick={() => {
                  changeToolStatus(name, tool.name, !tool.enabled);
                }}
              >
                <div className="flex items-center justify-between w-full">
                  <p className="text-[var(--servers-available-tools-item-name-color)]">
                    {tool.name}
                  </p>
                  <ToggleButton
                    checked={tool.enabled ?? false}
                    disabled={disableEnable && !tool.enabled}
                    onCheckedChange={() => {
                      // empty change because change will be applied in onClick at div element
                    }}
                  />
                </div>
                {description && (
                  <Tooltip>
                    <TooltipTrigger asChild>
                      <p
                        className="text-[13px] leading-[18px] line-clamp-2 text-[var(--servers-available-tools-sub-header-color)]"
                        style={{
                          display: "-webkit-box",
                          WebkitLineClamp: 2,
                          WebkitBoxOrient: "vertical",
                          overflow: "hidden",
                        }}
                      >
                        {description}
                      </p>
                    </TooltipTrigger>
                    <TooltipContent>
                      <div className="max-w-[300px]">{description}</div>
                    </TooltipContent>
                  </Tooltip>
                )}
              </div>
            );
          })}
        </div>
      ) : null}
      {openLogsDialog ? (
        <LogsDialog
          type={name}
          open={openLogsDialog}
          onClose={() => setOpenLogsDialog(false)}
        />
      ) : null}
      {deleteDialogOpen ? (
        <DeleteServerDialog
          name={name}
          onClose={() => setDeleteDialogOpen(false)}
        />
      ) : null}
    </div>
  );
};

export default AvailableToolsItem;
