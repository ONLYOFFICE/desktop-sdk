import { create } from "zustand";

import type { TMCPItem } from "@/lib/types";
import client from "@/servers";

const DISABLED_TOOLS_NAME = "disabledTools";

type UseServersStoreProps = {
  servers: Record<string, TMCPItem[]>;
  tools: TMCPItem[];
  disabledTools: string[];

  initServers: () => Promise<void>;
  changeToolStatus: (type: string, name: string, enabled: boolean) => void;
  callTools: (name: string, args: Record<string, unknown>) => unknown;
};

const useServersStore = create<UseServersStoreProps>((set, get) => ({
  servers: {},
  tools: [],
  disabledTools: [],

  initServers: async () => {
    client.init();

    const allTools = await client.getTools();
    const disabledToolsNamesStr = localStorage.getItem(DISABLED_TOOLS_NAME);

    const tools: TMCPItem[] = [];
    const servers: Record<string, TMCPItem[]> = {};

    if (disabledToolsNamesStr) {
      const disabledTools = disabledToolsNamesStr.split(",");

      Object.entries(allTools).map(([type, tools]) => {
        servers[type] = tools.map((tool) => {
          const enabled = !disabledTools.includes(tool.name);

          if (enabled) tools.push(tool);

          return {
            ...tool,
            enabled,
          };
        });
      });

      set({ disabledTools });
    } else {
      Object.entries(allTools).map(([type, tools]) => {
        servers[type] = tools.map((tool) => {
          tools.push(tool);

          return {
            ...tool,
            enabled: true,
          };
        });
      });
    }

    set({ tools, servers });
  },

  changeToolStatus: (type: string, name: string, enabled: boolean) => {
    const thisStore = get();

    const servers = thisStore.servers;

    const tool = servers[type].find((tool) => tool.name === name);

    if (!tool) return;

    if (enabled) {
      set({
        tools: [...thisStore.tools, tool],
        disabledTools: thisStore.disabledTools.filter((tool) => tool !== name),
      });
    } else {
      set({
        tools: thisStore.tools.filter((tool) => tool.name !== name),
        disabledTools: [...thisStore.disabledTools, name],
      });
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

  callTools: (name: string, args: Record<string, unknown>) => {
    const thisStore = get();

    const tool = thisStore.disabledTools.find((tool) => tool === name);

    if (!tool) return;

    return client.callTools(name, args);
  },
}));

export default useServersStore;
