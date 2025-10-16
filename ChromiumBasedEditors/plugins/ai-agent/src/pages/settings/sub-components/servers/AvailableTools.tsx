import React from "react";
import { useTranslation } from "react-i18next";

import useServersStore from "@/store/useServersStore";

import client from "@/servers";

import {
  MAX_TOOL_COUNT,
  MAX_TOOL_COUNT_WITH_WEB_SEARCH,
} from "@/lib/constants";

import AvailableToolsItem from "./AvailableToolsItem";

const AvailableTools = () => {
  const { t } = useTranslation();

  const [customServers, setCustomServers] = React.useState({});

  const { servers, tools, webSearchEnabled } = useServersStore();

  const [allToolsCount, setAllToolsCount] = React.useState<number>(
    webSearchEnabled ? tools.length - 2 : tools.length
  );

  React.useEffect(() => {
    setAllToolsCount(webSearchEnabled ? tools.length - 2 : tools.length);
  }, [tools.length, webSearchEnabled]);

  React.useEffect(() => {
    setCustomServers(client.getCustomServers());

    const interval = setInterval(() => {
      setCustomServers(client.getCustomServers());
    }, 1000);

    return () => clearInterval(interval);
  }, []);

  const disableEnable = webSearchEnabled
    ? tools.length >= MAX_TOOL_COUNT_WITH_WEB_SEARCH
    : tools.length >= MAX_TOOL_COUNT;

  return (
    <div className="w-[640px] h-[560px] border border-[var(--servers-available-tools-border-color)] rounded-[8px] flex flex-col">
      <div className="min-h-[44px] h-[44px] flex items-center justify-between px-[16px] border-b border-[var(--servers-available-tools-border-color)]">
        <p className="font-bold text-[14px] leading-[20px] text-[var(--servers-available-tools-header-color)]">
          {t("AvailableTools")}
        </p>
        <p className="font-normal text-[14px] leading-[20px] text-[var(--servers-available-tools-sub-header-color)]">
          <span className="text-[var(--servers-available-tools-current-tool-color)]">
            {allToolsCount}
          </span>
          /{MAX_TOOL_COUNT} {t("Tools")}
        </p>
      </div>
      <div className="flex flex-col gap-[8px] pt-[12px] pb-[16px] px-[8px] overflow-y-auto">
        <AvailableToolsItem
          name="desktop-editor"
          mcpItems={servers["desktop-editor"] ?? []}
          isLoading={false}
          isSystem
          disableEnable={disableEnable}
        />
        {Object.keys(customServers).map((type) => (
          <AvailableToolsItem
            key={type}
            name={type}
            mcpItems={servers[type] ?? []}
            isLoading={!servers[type]?.length}
            isSystem={false}
            disableEnable={disableEnable}
          />
        ))}
      </div>
    </div>
  );
};

export default AvailableTools;
