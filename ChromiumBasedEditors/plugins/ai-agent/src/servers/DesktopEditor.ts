import type { TMCPItem } from "@/lib/types";

const ALLOW_ALWAYS_DESKTOP_TOOLS = "allowAlwaysDesktopTools";

export class DesktopEditorTool {
  private tools: TMCPItem[];

  private allowAlways = false;

  constructor() {
    this.tools = [];
    this.initTools();

    this.allowAlways =
      localStorage.getItem(ALLOW_ALWAYS_DESKTOP_TOOLS) === "true";
  }

  setAllowAlways = (value: boolean) => {
    this.allowAlways = value;
    localStorage.setItem(ALLOW_ALWAYS_DESKTOP_TOOLS, value.toString());
  };

  getAllowAlways = () => {
    return this.allowAlways;
  };

  setTools = (tools: TMCPItem[]) => {
    this.tools = tools;
  };

  getTools = () => {
    return [...this.tools];
  };

  callTools = async (name: string, args: Record<string, unknown>) => {
    const newArgs = { ...args };

    const result = await window.AscDesktopEditor?.callToolFunction(
      name,
      JSON.stringify(newArgs)
    );

    return result;
  };

  initTools = () => {
    try {
      const stringFunctions: string =
        window.AscDesktopEditor?.getToolFunctions() ?? "";

      const parsedTools = (
        JSON.parse(stringFunctions) as (TMCPItem & {
          parameters: Record<string, unknown>;
        })[]
      ).map((tool) => ({
        name: tool.name,
        description: tool.description,
        inputSchema: tool.parameters,
      }));

      this.setTools(parsedTools);
    } catch (error) {
      console.error("Error parsing tools:", error);
    }
  };
}
