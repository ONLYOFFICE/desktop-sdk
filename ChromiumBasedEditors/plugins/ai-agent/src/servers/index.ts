import type { TMCPItem } from "@/lib/types";

import { DesktopEditorTool } from "./DesktopEditor";
import { CustomServers } from "./CustomServers";

class Servers {
  desktopEditorTool: DesktopEditorTool;
  customServers: CustomServers;

  constructor() {
    this.desktopEditorTool = new DesktopEditorTool();
    this.customServers = new CustomServers();
  }

  checkAllowAlways = (type: string) => {
    if (type === "desktop-editor") {
      return this.desktopEditorTool.getAllowAlways();
    }

    return this.customServers.checkAllowAlways(type);
  };

  setAllowAlways = (value: boolean, type: string) => {
    if (type === "desktop-editor") {
      this.desktopEditorTool.setAllowAlways(value);
      return;
    }

    this.customServers.setAllowAlways(value, type);
  };

  getTools = async () => {
    const [desktopEditorTools, customServersTools] = await Promise.all([
      this.desktopEditorTool.getTools(),
      this.customServers.getTools(),
    ]);

    const items: Record<string, TMCPItem[]> = {
      "desktop-editor": desktopEditorTools,
      ...customServersTools,
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

    // Call MCP server tool
    return this.customServers.callToolFromMCP(type, name, args);
  };

  setCustomServers = (servers: {
    mcpServers: Record<string, Record<string, unknown>>;
  }) => {
    this.customServers.setCustomServers(servers);
  };

  startCustomServers = () => {
    this.customServers.startCustomServers();
  };

  restartCustomServer = (type: string) => {
    this.customServers.restartCustomServer(type);
  };

  deleteCustomServer = (type: string) => {
    this.customServers.deleteCustomServer(type);
  };

  getCustomServers = () => {
    return this.customServers.customServers;
  };

  getCustomServersLogs = () => {
    return this.customServers.customServersLogs;
  };
}

const servers = new Servers();

export default servers;
