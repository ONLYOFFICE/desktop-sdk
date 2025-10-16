import type { TMCPItem } from "@/lib/types";

import { DesktopEditorTool } from "./DesktopEditor";
import { CustomServers } from "./CustomServers";
import { WebSearch, type WebSearchData } from "./WebSearch";

class Servers {
  desktopEditorTool: DesktopEditorTool;
  customServers: CustomServers;
  webSearch: WebSearch;

  constructor() {
    this.desktopEditorTool = new DesktopEditorTool();
    this.customServers = new CustomServers();
    this.webSearch = new WebSearch();
  }

  checkAllowAlways = (type: string) => {
    if (type === "desktop-editor") {
      return this.desktopEditorTool.getAllowAlways();
    }

    if (type === "web-search") {
      return this.webSearch.getAllowAlways();
    }

    return this.customServers.checkAllowAlways(type);
  };

  setAllowAlways = (value: boolean, type: string) => {
    if (type === "desktop-editor") {
      this.desktopEditorTool.setAllowAlways(value);
      return;
    }

    if (type === "web-search") {
      this.webSearch.setAllowAlways();
      return;
    }

    this.customServers.setAllowAlways(value, type);
  };

  getTools = async () => {
    const [desktopEditorTools, webSearchTools, customServersTools] =
      await Promise.all([
        this.desktopEditorTool.getTools(),
        this.webSearch.getTools(),
        this.customServers.getTools(),
      ]);

    const items: Record<string, TMCPItem[]> = {
      "desktop-editor": desktopEditorTools,
      "web-search": webSearchTools,
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

    if (type === "web-search") {
      return await this.webSearch.callTools(name, args);
    }

    // Call MCP server tool
    return await this.customServers.callToolFromMCP(type, name, args);
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

  setWebSearchData = (data: WebSearchData) => {
    this.webSearch.setWebSearchData(data);
  };

  getWebSearchData = () => {
    return this.webSearch.getWebSearchData();
  };
}

const servers = new Servers();

export default servers;
