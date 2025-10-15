import React from "react";

import useServersStore from "@/store/useServersStore";

import client from "@/servers";

import AvailableToolsItem from "./AvailableToolsItem";

const AvailableTools = () => {
  const [customServers, setCustomServers] = React.useState({});

  const { servers, tools } = useServersStore();

  const [allToolsCount, setAllToolsCount] = React.useState<number>(
    tools.length
  );

  React.useEffect(() => {
    setAllToolsCount(tools.length);
  }, [tools.length]);

  React.useEffect(() => {
    setCustomServers(client.getCustomServers());

    const interval = setInterval(() => {
      setCustomServers(client.getCustomServers());
    }, 1000);

    return () => clearInterval(interval);
  }, []);

  return (
    <div className="w-[640px] h-[560px] border border-[var(--servers-available-tools-border-color)] rounded-[8px] flex flex-col">
      <div className="min-h-[44px] h-[44px] flex items-center justify-between px-[16px] border-b border-[var(--servers-available-tools-border-color)]">
        <p className="font-bold text-[14px] leading-[20px] text-[var(--servers-available-tools-header-color)]">
          Available tools
        </p>
        <p className="font-normal text-[14px] leading-[20px] text-[var(--servers-available-tools-sub-header-color)]">
          <span className="text-[var(--servers-available-tools-current-tool-color)]">
            {allToolsCount}
          </span>
          /100 Tools
        </p>
      </div>
      <div className="flex flex-col gap-[8px] pt-[12px] pb-[16px] px-[8px] overflow-y-auto">
        <AvailableToolsItem
          name="desktop-editor"
          mcpItems={servers["desktop-editor"] ?? []}
          isLoading={false}
          isSystem
        />
        {Object.keys(customServers).map((type) => (
          <AvailableToolsItem
            key={type}
            name={type}
            mcpItems={servers[type] ?? []}
            isLoading={!servers[type]?.length}
            isSystem={false}
          />
        ))}
      </div>
    </div>
  );
};

export default AvailableTools;
