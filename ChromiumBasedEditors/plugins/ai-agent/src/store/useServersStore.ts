import { create } from "zustand";
import type { ThreadMessageLike } from "@assistant-ui/react";

import type { TMCPItem } from "@/lib/types";
import client from "@/servers";

const DISABLED_TOOLS_NAME = "disabledTools";
const MCP_SERVERS_NAME = "mcpServers";

type UseServersStoreProps = {
  servers: Record<string, TMCPItem[]>;
  tools: TMCPItem[];
  disabledTools: Record<string, string[]>;
  manageToolData?: {
    message: ThreadMessageLike;
    idx: number;
    messageUID: string;
  };

  initServers: () => void;
  getTools: () => Promise<void>;
  changeToolStatus: (type: string, name: string, enabled: boolean) => void;
  callTools: (name: string, args: Record<string, unknown>) => unknown;
  checkAllowAlways: (type: string) => boolean;
  setAllowAlways: (value: boolean, type: string) => void;
  setManageToolData: (data: UseServersStoreProps["manageToolData"]) => void;
  saveConfig: (config: {
    mcpServers: Record<string, Record<string, unknown>>;
  }) => void;
  getConfig: () => Record<string, Record<string, unknown>>;
  deleteCustomServer: (name: string) => void;
  getCustomServersLogs: () => Record<string, string[]>;
};

const useServersStore = create<UseServersStoreProps>((set, get) => ({
  servers: {},
  tools: [],
  disabledTools: {},
  manageToolData: undefined,

  initServers: () => {
    const customServers = localStorage.getItem(MCP_SERVERS_NAME);

    if (customServers) {
      const parsedCustomServers = JSON.parse(customServers);
      client.setCustomServers(parsedCustomServers);
      client.startCustomServers();
    }
  },

  getTools: async () => {
    const allTools = await client.getTools();
    const disabledToolsNamesStr = localStorage.getItem(DISABLED_TOOLS_NAME);

    const tools: TMCPItem[] = [];
    const servers: Record<string, TMCPItem[]> = {};

    if (disabledToolsNamesStr) {
      const disabledTools = JSON.parse(disabledToolsNamesStr);

      Object.entries(allTools).map(([type, serverTools]) => {
        servers[type] = serverTools.map((tool) => {
          if (!disabledTools[type]) {
            disabledTools[type] = [];
          }
          const enabled = !disabledTools[type].includes(tool.name);

          if (enabled) tools.push({ ...tool, name: `${type}_${tool.name}` });

          return {
            ...tool,
            enabled,
          };
        });
      });

      set({ disabledTools });
    } else {
      const disabledTools: Record<string, string[]> = {};
      Object.entries(allTools).map(([type, serverTools]) => {
        const enabledTools = serverTools.map((t) => ({ ...t, enabled: true }));
        disabledTools[type] = [];
        servers[type] = enabledTools;
        tools.push(
          ...enabledTools.map((tool) => ({
            ...tool,
            name: `${type}_${tool.name}`,
          }))
        );
      });

      set({ disabledTools });
    }

    set({ tools, servers });
  },

  changeToolStatus: (type: string, name: string, enabled: boolean) => {
    const thisStore = get();

    const servers = thisStore.servers;

    const tool = servers[type].find((tool) => tool.name === name);

    if (!tool) return;

    if (enabled) {
      const newDisabledTools = {
        ...thisStore.disabledTools,
        [type]: thisStore.disabledTools[type].filter((tool) => tool !== name),
      };
      set({
        tools: [...thisStore.tools, { ...tool, name: `${type}_${tool.name}` }],
        disabledTools: newDisabledTools,
      });
      localStorage.setItem(
        DISABLED_TOOLS_NAME,
        JSON.stringify(newDisabledTools)
      );
    } else {
      const disabled = [...thisStore.disabledTools[type], name];
      const allDisabledTools = {
        ...thisStore.disabledTools,
        [type]: disabled,
      };
      const tools = thisStore.tools.filter(
        (tool) => tool.name !== `${type}_${name}`
      );
      set({
        tools,
        disabledTools: allDisabledTools,
      });

      localStorage.setItem(
        DISABLED_TOOLS_NAME,
        JSON.stringify(allDisabledTools)
      );
    }

    set({
      servers: {
        ...servers,
        [type]: servers[type].map((tool) => {
          if (tool.name === name) {
            return {
              ...tool,
              enabled,
            };
          }

          return tool;
        }),
      },
    });
  },

  checkAllowAlways: (type: string) => {
    return client.checkAllowAlways(type);
  },

  setAllowAlways: (value: boolean, type: string) => {
    client.setAllowAlways(value, type);
  },

  callTools: (name: string, args: Record<string, unknown>) => {
    const thisStore = get();

    const type = name.split("_")[0];
    const toolName = name.replace(type + "_", "");

    const tool = thisStore.disabledTools[type].find(
      (tool) => tool === toolName
    );

    if (tool) return;

    return client.callTools(type, toolName, args);
  },

  setManageToolData: (data: UseServersStoreProps["manageToolData"]) => {
    set({ manageToolData: data });
  },

  getConfig: () => {
    return JSON.parse(localStorage.getItem(MCP_SERVERS_NAME) || "{}");
  },

  saveConfig: (config: {
    mcpServers: Record<string, Record<string, unknown>>;
  }) => {
    localStorage.setItem(MCP_SERVERS_NAME, JSON.stringify(config));
    client.setCustomServers(config);
    client.startCustomServers();
  },

  deleteCustomServer: (name: string) => {
    client.deleteCustomServer(name);
    const config = get().getConfig();
    delete config.mcpServers[name];
    localStorage.setItem(MCP_SERVERS_NAME, JSON.stringify(config));
  },

  getCustomServersLogs: () => {
    return client.getCustomServersLogs();
  },
}));

export default useServersStore;
