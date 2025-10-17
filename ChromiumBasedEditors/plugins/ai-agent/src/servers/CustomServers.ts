import type { TProcess, TMCPItem } from "@/lib/types";

const getParams = (config: Record<string, unknown>) => {
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

  return { commandLine, env };
};

const ALLOW_ALWAYS_CUSTOM_SERVERS = "allowAlwaysCustomServers";

class CustomServers {
  customServers: Record<string, Record<string, unknown>>;
  startedCustomServers: Record<string, string>;
  initedCustomServers: Record<string, boolean>;
  stopedCustomServers: string[];
  customServersProcesses: Record<string, TProcess>;
  customServersLogs: Record<string, string[]>;
  tools: Record<string, TMCPItem[]>;

  constructor() {
    this.customServers = {};
    this.startedCustomServers = {};
    this.initedCustomServers = {};
    this.customServersProcesses = {};
    this.customServersLogs = {};
    this.tools = {};
    this.stopedCustomServers = [];
  }

  checkAllowAlways = (type: string) => {
    return localStorage.getItem(ALLOW_ALWAYS_CUSTOM_SERVERS + type) === "true";
  };

  setAllowAlways = (value: boolean, type: string) => {
    localStorage.setItem(ALLOW_ALWAYS_CUSTOM_SERVERS + type, value.toString());
  };

  onProcess = (type: string, t: number, message: string) => {
    try {
      const correctJson = JSON.parse(message);

      if (
        correctJson.jsonrpc === "2.0" &&
        correctJson.id &&
        correctJson.id.includes("init-" + type)
      ) {
        this.initedCustomServers[type] = true;
        this.stopedCustomServers = this.stopedCustomServers.filter(
          (s) => s !== type
        );
      }

      if (
        correctJson.jsonrpc === "2.0" &&
        correctJson.id &&
        correctJson.id.includes("tools-" + type)
      ) {
        this.tools[type] = correctJson.result.tools;
      }
    } catch {
      // ignore
    }

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
        this.stopedCustomServers.push(type);
        break;
      }
      default:
        break;
    }
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

      const { commandLine, env } = getParams(config);

      if (
        this.startedCustomServers[type] &&
        this.startedCustomServers[type] === commandLine
      ) {
        return;
      }

      if (this.customServersProcesses[type]) {
        this.customServersProcesses[type].end();
        console.log("stop server", type);
      }

      this.customServersLogs[type] = [
        `${new Date().toLocaleString()}: ${commandLine}\n`,
      ];

      const process = new window.ExternalProcess(commandLine, env);

      process.onprocess = this.onProcess.bind(this, type);

      this.customServersProcesses[type] = process;

      process.start();

      this.initCustomServer(type);
    });

    // remove deleted servers
    Object.keys(this.customServersProcesses).forEach((type) => {
      if (!servers.includes(type)) {
        this.deleteCustomServer(type);
      }
    });
  };

  restartCustomServer = (type: string) => {
    Object.entries(this.customServers).forEach(([serverType, config]) => {
      if (type !== serverType) return;

      const { commandLine, env } = getParams(config);

      this.customServersLogs[type] = [
        `${new Date().toLocaleString()}: ${commandLine}\n`,
      ];

      this.tools[type] = [];

      const process = new window.ExternalProcess(commandLine, env);

      process.onprocess = this.onProcess.bind(this, type);

      this.customServersProcesses[type] = process;

      process.start();

      this.initCustomServer(type);
    });
  };

  deleteCustomServer = (type: string) => {
    if (this.customServersProcesses[type]) {
      this.customServersProcesses[type].end();
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
    if (this.tools[type]) {
      delete this.tools[type];
    }
  };

  initCustomServer = (type: string) => {
    const process = this.customServersProcesses[type];

    if (!process) {
      return;
    }

    const interval = setInterval(() => {
      if (this.initedCustomServers[type]) {
        clearInterval(interval);
        this.getToolsFromMCP(type);
        return;
      }

      try {
        // First send initialize request
        const initRequest = {
          jsonrpc: "2.0",
          id: `init-${type}`,
          method: "initialize",
          params: {
            protocolVersion: "2024-11-05",
            capabilities: {
              tools: {},
            },
            clientInfo: {
              name: "ai-agent",
              version: "1.0.0",
            },
          },
        };

        const initBody = JSON.stringify(initRequest);
        const initMessage = `${initBody}\n`;

        // Send initialize request
        process.stdin(initMessage);
      } catch (error) {
        console.error(`Error initializing custom server ${type}:`, error);
      }
    }, 1000);
  };

  getToolsFromMCP = async (type: string) => {
    // Get all running custom server processes

    const process = this.customServersProcesses[type];

    try {
      // Create JSON-RPC request for tools/list
      const request = {
        jsonrpc: "2.0",
        id: `tools-${type}-${Date.now()}`,
        method: "tools/list",
        params: {},
      };

      const requestBody = JSON.stringify(request);
      const requestMessage = `${requestBody}\n`;

      process.stdin(requestMessage);
    } catch (error) {
      console.error(`Error getting tools from MCP server ${type}:`, error);
    }
  };

  callToolFromMCP = async (
    serverType: string,
    toolName: string,
    args: Record<string, unknown>
  ): Promise<unknown> => {
    const process = this.customServersProcesses[serverType];

    if (!process) {
      throw new Error(`MCP server ${serverType} is not running`);
    }

    // Check if tool exists and get its schema
    const serverTools = this.tools[serverType] || [];
    const tool = serverTools.find((t) => t.name === toolName);

    if (!tool) {
      throw new Error(`Tool ${toolName} not found on server ${serverType}`);
    }

    try {
      const request = {
        jsonrpc: "2.0",
        id: `call-${serverType}-${toolName}-${Date.now()}`,
        method: "tools/call",
        params: {
          name: toolName,
          arguments: args,
        },
      };

      const requestBody = JSON.stringify(request);
      const requestMessage = `${requestBody}\n`;

      // Send the tool call request
      process.stdin(requestMessage);

      // Return a promise that resolves when we get the response
      return new Promise((resolve, reject) => {
        const timeout = setTimeout(() => {
          reject(
            new Error(`Timeout waiting for tool response from ${serverType}`)
          );
        }, 30000); // 30 second timeout for tool execution

        // Store original onprocess handler
        const originalOnProcess = process.onprocess;

        // Temporarily override to capture tool response
        process.onprocess = (t: number, message: string) => {
          // Call original handler to maintain logging
          originalOnProcess(t, message);

          if (t === 0) {
            // stdout
            try {
              const response = JSON.parse(message);

              // Check if this is our tool call response
              if (
                response.id &&
                response.id.startsWith(`call-${serverType}-${toolName}`)
              ) {
                // Restore original handler
                process.onprocess = originalOnProcess;
                clearTimeout(timeout);

                if (response.error) {
                  console.error(`MCP tool error response:`, response.error);
                  reject(
                    new Error(
                      `MCP tool error (${response.error.code}): ${response.error.message}`
                    )
                  );
                } else {
                  console.log(`MCP tool success response:`, response.result);
                  resolve(JSON.stringify(response.result));
                }
              }
            } catch {
              // Ignore parse errors, continue waiting
            }
          }
        };
      });
    } catch (error) {
      throw new Error(
        `Error calling MCP tool ${toolName} on server ${serverType}: ${error}`
      );
    }
  };

  getTools = () => {
    return this.tools;
  };
}

export { CustomServers };
