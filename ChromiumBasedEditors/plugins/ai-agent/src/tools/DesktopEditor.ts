import type { TMCPItem } from "@/lib/types";

const openFileTool: TMCPItem = {
  name: "open_file",
  description: "Open a file",
  inputSchema: {
    type: "object",
    properties: {
      document: {
        type: "string",
        description: "The path of the file to open",
      },
      title: {
        type: "string",
        description: "The title of the file to open",
      },
    },
    required: ["document", "title"],
  },
};

export class DesktopEditorTool {
  private tools: TMCPItem[];

  constructor() {
    this.tools = [];
  }

  setTools = (tools: TMCPItem[]) => {
    this.tools = tools;
  };

  getTools = () => {
    return [...this.tools, openFileTool];
  };

  callTools = (name: string, args: Record<string, unknown>) => {
    const newArgs = { ...args };

    console.log("call tools", name, args);

    if (name === openFileTool.name) {
      window.AscDesktopEditor?.openTemplate(
        args.document as string,
        args.title as string
      );

      return "opened";
    }

    if (newArgs.document)
      newArgs.document = `/Users/fognir/Documents/${args.document}`;

    const result = window.AscDesktopEditor?.callToolFunction(
      name,
      JSON.stringify(newArgs)
    );

    if (result.includes("[prompt]")) {
      const subStr = result.substring(result.indexOf("]") + 1);

      return subStr.substring(subStr.indexOf("[") + 1, subStr.indexOf("]"));
    }

    if (result.includes("[file]")) {
      const subStr = result.substring(result.indexOf("]") + 1);

      return subStr;
    }

    return result;
  };

  initTools = () => {
    const stringFunctions: string =
      window.AscDesktopEditor?.getToolFunctions() ?? "";

    const parsedTools = this.parseFunctionSpec(stringFunctions);

    this.setTools(parsedTools);
  };

  /**
   * Parse a function spec string into an array with objects: { name, description, inputScheme }
   */
  parseFunctionSpec = (spec: string): TMCPItem[] => {
    if (!spec) return [];

    // Normalize: remove surrounding quotes and convert escaped newlines to real newlines
    const normalized = spec
      .replace(/^["'`]|["'`]$/g, "")
      .replace(/\\r/g, "")
      .replace(/\\n/g, "\n");
    const lines = normalized.split(/\r?\n/).map((l) => l.trim());

    // Identify block starts: lines like "- tool_name"
    const startIdxs: number[] = [];
    const names: string[] = [];
    const nameLineRegex = /^-\s*([A-Za-z0-9_-]+)\s*$/;
    for (let i = 0; i < lines.length; i++) {
      const m = nameLineRegex.exec(lines[i]);
      if (m) {
        startIdxs.push(i);
        names.push(m[1]);
      }
    }

    // If no explicit blocks found, treat whole input as a single block
    if (startIdxs.length === 0) {
      startIdxs.push(0);
      names.push(lines.find((l) => l.length > 0) ?? "");
    }

    const items: TMCPItem[] = [];
    for (let bi = 0; bi < startIdxs.length; bi++) {
      const start = startIdxs[bi];
      const end = bi + 1 < startIdxs.length ? startIdxs[bi + 1] : lines.length;
      const block = lines.slice(start, end);
      const name = names[bi];

      // Find section indices within block
      const bDescIdx = block.findIndex((l) => /^description\s*:/i.test(l));
      const bParamsIdx = block.findIndex((l) => /^parameters\s*:/i.test(l));
      const bExamplesIdx = block.findIndex((l) => /^examples\s*:/i.test(l));

      // Description
      let description = "";
      if (bDescIdx !== -1) {
        const descStart = bDescIdx + 1;
        const descEndCandidates = [
          bParamsIdx !== -1 ? bParamsIdx : block.length,
          bExamplesIdx !== -1 ? bExamplesIdx : block.length,
        ];
        const descEnd = Math.min(...descEndCandidates.filter((v) => v !== -1));
        description = block
          .slice(descStart, descEnd)
          .join(" \n ")
          .replace(/\s+/g, " ")
          .trim();
      }

      // Parameters
      const parameters: Array<{
        name: string;
        type: string;
        description: string;
      }> = [];
      if (bParamsIdx !== -1) {
        const paramsStart = bParamsIdx + 1;
        const paramsEnd = bExamplesIdx !== -1 ? bExamplesIdx : block.length;
        for (let i = paramsStart; i < paramsEnd; i++) {
          const line = block[i];
          if (!line) continue;
          const m = /^-\s*([^()\s:]+)\s*\(([^)]+)\)\s*:\s*(.+)$/.exec(line);
          if (m) {
            parameters.push({
              name: m[1].trim(),
              type: m[2].trim(),
              description: m[3].trim(),
            });
          }
        }
      }

      items.push({
        name,
        description,
        inputSchema: { parameters },
      });
    }

    return items;
  };
}
