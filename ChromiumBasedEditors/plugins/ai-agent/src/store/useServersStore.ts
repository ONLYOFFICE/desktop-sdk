import { create } from "zustand";
import type { ThreadMessageLike } from "@assistant-ui/react";

import type { TMCPItem } from "@/lib/types";
import {
  MAX_TOOL_COUNT,
  MAX_TOOL_COUNT_WITH_WEB_SEARCH,
} from "@/lib/constants";
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
  webSearchEnabled: boolean;

  initServers: () => void;
  getTools: () => Promise<void>;
  changeToolStatus: (type: string, name: string, enabled: boolean) => void;
  callTools: (name: string, args: Record<string, unknown>) => Promise<unknown>;
  checkAllowAlways: (type: string) => boolean;
  setAllowAlways: (value: boolean, type: string) => void;
  setManageToolData: (data: UseServersStoreProps["manageToolData"]) => void;
  saveConfig: (config: {
    mcpServers: Record<string, Record<string, unknown>>;
  }) => void;
  getConfig: () => Record<string, Record<string, unknown>>;
  deleteCustomServer: (name: string) => void;
  getCustomServersLogs: () => Record<string, string[]>;
  getWebSearchEnabled: () => boolean;
};

const useServersStore = create<UseServersStoreProps>((set, get) => ({
  servers: {},
  tools: [],
  disabledTools: {},
  manageToolData: undefined,
  webSearchEnabled: false,

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
    let webSearchEnabled = false;

    if (disabledToolsNamesStr) {
      const disabledTools = JSON.parse(disabledToolsNamesStr);

      Object.entries(allTools).map(([type, serverTools]) => {
        if (type === "web-search") {
          servers[type] = [...serverTools];

          if (disabledTools["web-search"].length) {
            set({ webSearchEnabled: false });
            return;
          }

          const items = serverTools.map((tool) => {
            return {
              ...tool,
              name: `${type}_${tool.name}`,
            };
          });

          disabledTools[type] = [];

          tools.push(...items);
          set({ webSearchEnabled: serverTools.length > 0 });
          webSearchEnabled = serverTools.length > 0;

          return;
        }

        servers[type] = serverTools.map((tool) => {
          if (!disabledTools[type]) {
            disabledTools[type] = [];
          }
          const enabled = !disabledTools[type].includes(tool.name);

          if (
            enabled && webSearchEnabled
              ? tools.length === MAX_TOOL_COUNT_WITH_WEB_SEARCH
              : tools.length === MAX_TOOL_COUNT
          ) {
            disabledTools[type].push(tool.name);

            return {
              ...tool,
              enabled: false,
            };
          }

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

      let webSearchEnabled = false;

      Object.entries(allTools).map(([type, serverTools]) => {
        disabledTools[type] = [];

        if (type === "web-search") {
          servers[type] = [...serverTools];

          const items = serverTools.map((tool) => {
            return {
              ...tool,
              name: `${type}_${tool.name}`,
              enabled: true,
            };
          });

          tools.push(...items);
          set({ webSearchEnabled: serverTools.length > 0 });
          webSearchEnabled = serverTools.length > 0;

          return;
        }

        const serverToolsWithStatus = serverTools.map((t, index) => {
          if (
            tools.length + index >=
            (webSearchEnabled ? MAX_TOOL_COUNT_WITH_WEB_SEARCH : MAX_TOOL_COUNT)
          ) {
            disabledTools[type].push(t.name);

            return { ...t, enabled: false };
          }
          return { ...t, enabled: true };
        });

        servers[type] = serverToolsWithStatus;
        tools.push(
          ...serverToolsWithStatus.map((tool) => ({
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
      if (type === "web-search") {
        const newDisabledTools = {
          ...thisStore.disabledTools,
          [type]: [],
        };
        set({ disabledTools: newDisabledTools });
        set({ webSearchEnabled: true });
        set({
          servers: {
            ...servers,
            [type]: servers[type].map((tool) => {
              return {
                ...tool,
                enabled: true,
              };
            }),
          },
        });
        localStorage.setItem(
          DISABLED_TOOLS_NAME,
          JSON.stringify(newDisabledTools)
        );
        return;
      }

      if (
        thisStore.tools.length >=
        (thisStore.webSearchEnabled
          ? MAX_TOOL_COUNT_WITH_WEB_SEARCH
          : MAX_TOOL_COUNT)
      ) {
        return;
      }

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

      let tools: TMCPItem[] = [];

      if (type === "web-search") {
        allDisabledTools[type] = [
          ...thisStore.servers[type].map((tool) => tool.name),
        ];
        set({ webSearchEnabled: false });
        tools = thisStore.tools.filter((tool) => !tool.name.includes(type));
      } else {
        tools = thisStore.tools.filter(
          (tool) => tool.name !== `${type}_${name}`
        );
      }

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

  callTools: async (name: string, args: Record<string, unknown>) => {
    const thisStore = get();

    const type = name.split("_")[0];
    const toolName = name.replace(type + "_", "");

    const tool = thisStore.disabledTools[type].find(
      (tool) => tool === toolName
    );

    if (tool) return;

    return await client.callTools(type, toolName, args);
  },

  setManageToolData: (data: UseServersStoreProps["manageToolData"]) => {
    set({ manageToolData: data });
  },

  getConfig: () => {
    return JSON.parse(
      localStorage.getItem(MCP_SERVERS_NAME) ||
        JSON.stringify({ mcpServers: {} })
    );
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

  getWebSearchEnabled: () => {
    return client.getWebSearchEnabled();
  },
}));

export default useServersStore;
