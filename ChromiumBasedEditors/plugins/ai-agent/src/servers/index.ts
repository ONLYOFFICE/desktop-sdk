import type { TMCPItem, TProcess } from "@/lib/types";

import { DesktopEditorTool } from "./DesktopEditor";

class Servers {
  desktopEditorTool: DesktopEditorTool;
  customServers: Record<string, Record<string, unknown>>;
  startedCustomServers: Record<string, string>;
  customServersProcesses: Record<string, TProcess>;
  customServersLogs: Record<string, string[]>;

  constructor() {
    this.desktopEditorTool = new DesktopEditorTool();
    this.customServers = {};
    this.startedCustomServers = {};
    this.customServersProcesses = {};
    this.customServersLogs = {};
  }

  init = () => {
    this.desktopEditorTool.initTools();
  };

  checkAllowAlways = () => {
    return this.desktopEditorTool.getAllowAlways();
  };

  setAllowAlways = (value: boolean) => {
    this.desktopEditorTool.setAllowAlways(value);
  };

  getTools = async () => {
    const [desktopEditorTools] = await Promise.all([
      this.desktopEditorTool.getTools(),
    ]);

    const items: Record<string, TMCPItem[]> = {
      "desktop-editor": desktopEditorTools,
    };

    return items;
  };

  callTools = async (
    type: string,
    name: string,
    args: Record<string, unknown>
  ) => {
    if (type === "desktop-editor") {
      return this.desktopEditorTool.callTools(name, args);
    }

    return this.desktopEditorTool.callTools(name, args);

    // return this.customServers[type][toolName](args);
  };

  setCustomServers = (servers: {
    mcpServers: Record<string, Record<string, unknown>>;
  }) => {
    this.customServers = servers.mcpServers;
  };

  startCustomServers = () => {
    const servers: string[] = [];
    Object.entries(this.customServers).forEach(([type, config]) => {
      servers.push(type);
      let command = "";
      const env: Record<string, string> = {};
      let args = "";

      Object.entries(config).map(([key, value]) => {
        if (key == "env") {
          Object.entries(value as Record<string, string>).forEach(([k, v]) => {
            env[k] = v;
          });
        }

        if (key === "command") {
          command = value as string;
        }

        if (key == "args") {
          args = (value as string[]).join(" ");
        }
      });

      const commandLine = command + " " + args;

      if (
        this.startedCustomServers[type] &&
        this.startedCustomServers[type] === commandLine
      ) {
        return;
      }

      if (this.customServersProcesses[type]) {
        console.log("stop server");
        // this.customServersProcesses[type].end();
      }

      this.customServersLogs[type] = [
        `${new Date().toLocaleString()}: ${commandLine}\n`,
      ];

      const process = new window.ExternalProcess(commandLine, env);

      process.onprocess = (t: number, message: string) => {
        switch (t) {
          case 0: {
            this.customServersLogs[type].push(
              `${new Date().toLocaleString()}: ${message}\n`
            );
            break;
          }
          case 1: {
            this.customServersLogs[type].push(
              `${new Date().toLocaleString()}: ${message}\n`
            );
            break;
          }
          case 2: {
            this.customServersLogs[type].push(
              `${new Date().toLocaleString()}: [stop] ${message}\n`
            );
            // delete this.customServersProcesses[type];
            break;
          }
          default:
            break;
        }
      };

      this.customServersProcesses[type] = process;

      process.start();
    });

    Object.keys(this.customServersProcesses).forEach((type) => {
      if (!servers.includes(type)) {
        if (this.customServersProcesses[type]) {
          console.log("stop");
          // this.customServersProcesses[type]?.end();
          delete this.customServersProcesses[type];
        }
        if (this.customServersLogs[type]) {
          delete this.customServersLogs[type];
        }
        if (this.startedCustomServers[type]) {
          delete this.startedCustomServers[type];
        }
        if (this.customServers[type]) {
          delete this.customServers[type];
        }
      }
    });
  };
}

const servers = new Servers();

export default servers;
