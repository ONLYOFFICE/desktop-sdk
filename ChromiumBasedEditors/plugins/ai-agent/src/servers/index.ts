import type { TMCPItem } from "@/lib/types";

import { DesktopEditorTool } from "./DesktopEditor";
import { CustomServers } from "./CustomServers";
import { WebSearch, type WebSearchData } from "./WebSearch";

const ALLOW_ALWAYS_TOOLS = "allowAlwaysTools";

class Servers {
  desktopEditorTool: DesktopEditorTool;
  customServers: CustomServers;
  webSearch: WebSearch;

  allowAlways: string[];

  constructor() {
    this.desktopEditorTool = new DesktopEditorTool();
    this.customServers = new CustomServers();
    this.webSearch = new WebSearch();

    this.allowAlways =
      localStorage.getItem(ALLOW_ALWAYS_TOOLS)?.split(",") ?? [];
  }

  checkAllowAlways = (type: string, name: string) => {
    if (type === "web-search") {
      return true;
    }

    if (this.allowAlways.includes(`${type}_${name}`)) {
      return true;
    }

    return false;
  };

  setAllowAlways = (value: boolean, type: string, name: string) => {
    if (type === "web-search") {
      return;
    }

    if (value) {
      this.allowAlways.push(`${type}_${name}`);
    } else {
      this.allowAlways = this.allowAlways.filter(
        (tool) => tool !== `${type}_${name}`
      );
    }

    localStorage.setItem(ALLOW_ALWAYS_TOOLS, this.allowAlways.join(","));
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

  getCustomServersStoped = () => {
    return this.customServers.stopedCustomServers;
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

  getWebSearchEnabled = () => {
    return this.webSearch.getWebSearchEnabled();
  };
}

const servers = new Servers();

export default servers;
