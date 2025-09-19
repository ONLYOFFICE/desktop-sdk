import React from "react";

import useServersStore from "@/store/useServersStore";

import { ToggleButton } from "@/components/toggle-button";
import { Button } from "@/components/button";
import ConfigDialog from "./ConfigDialog";
import { Tabs } from "@/components/tabs";

const Servers = () => {
  const { servers, tools, disabledTools, getCustomServersLogs } =
    useServersStore();

  const [allToolsCount, setAllToolsCount] = React.useState<number>(
    Object.values(servers).flat().length
  );
  const [selectedServer, setSelectedServer] = React.useState<string>(
    Object.keys(servers)[0]
  );

  const [logs, setLogs] = React.useState<Record<string, string[]>>({});

  const timeout = React.useRef<NodeJS.Timeout>(null);

  const [isOpen, setIsOpen] = React.useState(false);

  React.useEffect(() => {
    const allToolsCount = Object.values(servers).flat().length;

    setAllToolsCount(allToolsCount);
  }, [servers]);

  React.useEffect(() => {
    const l = getCustomServersLogs();

    setLogs(Object.assign({}, l));

    timeout.current = setInterval(() => {
      const newLogs = getCustomServersLogs();

      setLogs(Object.assign({}, newLogs));
    }, 500);

    return () => {
      if (timeout.current) {
        clearInterval(timeout.current);
      }
    };
  }, [getCustomServersLogs]);

  return (
    <>
      <div className="flex flex-col gap-[16px] mt-[16px]">
        <p>
          Available tools: {tools.length}/{allToolsCount}
        </p>
        <Button className="w-fit" onClick={() => setIsOpen(true)}>
          Add server
        </Button>
        <div className="flex flex-row gap-[16px]">
          <div className="flex flex-col gap-[8px] min-w-[300px]">
            {Object.entries(servers).map(([type, tools]) => (
              <div
                key={type}
                onClick={() => setSelectedServer(type)}
                className="p-[8px] cursor-pointer"
                style={{
                  backgroundColor:
                    selectedServer === type ? "gray" : "transparent",
                }}
              >
                <p>
                  {type}
                  <span className="mx-[8px]">
                    {tools.length - disabledTools[type]?.length}/{tools.length}
                  </span>
                  <span
                    style={{ backgroundColor: "green" }}
                    className="w-[8px] h-[8px] rounded-full inline-block"
                  ></span>
                </p>
              </div>
            ))}
            {Object.keys(logs).map((key) => (
              <div
                className="p-[8px] cursor-pointer"
                onClick={() => setSelectedServer(key)}
                key={key}
                style={{
                  backgroundColor:
                    selectedServer === key ? "gray" : "transparent",
                }}
              >
                <p>{key}</p>
              </div>
            ))}
          </div>

          <Tabs
            items={[
              {
                value: "tools",
                label: "Tools",
                content: (
                  <div className="p-[16px] flex flex-col gap-[8px]">
                    {servers[selectedServer]?.map((tool) => (
                      <div key={tool.name}>
                        <p className="flex flex-row justify-between w-[250px]">
                          <span>{tool.name}</span>
                          <ToggleButton
                            checked={tool.enabled ?? false}
                            onCheckedChange={() => {}}
                          />
                        </p>
                      </div>
                    ))}
                  </div>
                ),
              },
              {
                value: "logs",
                label: "Logs",
                content: (
                  <div className="p-[16px] flex flex-col gap-[8px]">
                    {logs[selectedServer]?.map((log) => (
                      <div key={log}>{log}</div>
                    ))}
                  </div>
                ),
              },
            ]}
          />
        </div>
      </div>
      {<ConfigDialog open={isOpen} onClose={() => setIsOpen(false)} />}
    </>
  );
};

export { Servers };
